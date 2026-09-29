#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_MONITORES 10

#define MON_CPU 1
#define MON_MEM 2

int clienteFD;
pthread_mutex_t mutex_envio = PTHREAD_MUTEX_INITIALIZER;
volatile int parar_monitores = 0;
pthread_t monitores[MAX_MONITORES];
int n_monitores = 0;

typedef struct {
    int tipo;
    int intervalo;
} ParamMonitor;


void enviar(const char *msg) {
    pthread_mutex_lock(&mutex_envio);
    send(clienteFD, msg, strlen(msg), MSG_NOSIGNAL);
    pthread_mutex_unlock(&mutex_envio);
}

int ler_cpu(unsigned long long *total, unsigned long long *idle) {
    FILE *f = fopen("/proc/stat", "r");
    if (f == NULL) return -1;

    unsigned long long u, n, s, i, io, irq, sirq, st;
    if (fscanf(f, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &u, &n, &s, &i, &io, &irq, &sirq, &st) != 8) {
        fclose(f);
        return -1;
    }
    fclose(f);

    *idle = i + io;
    *total = u + n + s + i + io + irq + sirq + st;
    return 0;
}

int ler_memoria(long *total_kb, long *disp_kb) {
    FILE *f = fopen("/proc/meminfo", "r");
    if (f == NULL) return -1;

    char linha[256];
    *total_kb = -1;
    *disp_kb = -1;
    while (fgets(linha, sizeof(linha), f) != NULL) {
        sscanf(linha, "MemTotal: %ld kB", total_kb);
        sscanf(linha, "MemAvailable: %ld kB", disp_kb);
    }
    fclose(f);

    if (*total_kb <= 0 || *disp_kb < 0) return -1;
    return 0;
}


void *thread_monitor(void *arg) {
    ParamMonitor p = *(ParamMonitor *)arg;
    free(arg);

    char msg[BUFFER_SIZE];
    unsigned long long total_ant = 0, idle_ant = 0;

    if (p.tipo == MON_CPU) ler_cpu(&total_ant, &idle_ant);

    while (!parar_monitores) {
        for (int i = 0; i < p.intervalo && !parar_monitores; i++) sleep(1);
        if (parar_monitores) break;

        if (p.tipo == MON_CPU) {
            unsigned long long total, idle;
            if (ler_cpu(&total, &idle) == 0 && total > total_ant) {
                double uso = 100.0 * (1.0 - (double)(idle - idle_ant) / (double)(total - total_ant));
                snprintf(msg, sizeof(msg), "[CPU] Uso: %.1f%%", uso);
                enviar(msg);
                total_ant = total;
                idle_ant = idle;
            }
        } else {
            long total_kb, disp_kb;
            if (ler_memoria(&total_kb, &disp_kb) == 0) {
                double uso = 100.0 * (double)(total_kb - disp_kb) / (double)total_kb;
                snprintf(msg, sizeof(msg), "[MEMORIA] Uso: %.1f%% (%ld MB de %ld MB)",
                         uso, (total_kb - disp_kb) / 1024, total_kb / 1024);
                enviar(msg);
            }
        }
    }
    return NULL;
}

void iniciar_monitor(int tipo, int intervalo) {
    if (intervalo <= 0) {
        enviar("Intervalo invalido. Exemplo: CPU-5");
        return;
    }
    if (n_monitores >= MAX_MONITORES) {
        enviar("Limite de monitores atingido. Use Quit para parar.");
        return;
    }

    ParamMonitor *p = malloc(sizeof(ParamMonitor));
    p->tipo = tipo;
    p->intervalo = intervalo;

    pthread_create(&monitores[n_monitores], NULL, thread_monitor, p);
    n_monitores++;
    enviar("Monitor iniciado.");
}

void parar_todos_monitores() {
    parar_monitores = 1;
    for (int i = 0; i < n_monitores; i++) {
        pthread_join(monitores[i], NULL);
    }
    n_monitores = 0;
    parar_monitores = 0;
}


int main() {
    int socketFD;

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro no socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(socketFD, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erro no bind");
        exit(EXIT_FAILURE);
    }

    if (listen(socketFD, 3) < 0) {
        perror("Erro no listen");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando conexoes na porta %d...\n", PORT);

    int addrlen = sizeof(address);
    if ((clienteFD = accept(socketFD, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Erro no accept");
        exit(EXIT_FAILURE);
    }
    printf("Cliente conectado.\n");

    char hora[64];
    time_t agora = time(NULL);
    strftime(hora, sizeof(hora), "%H:%M:%S", localtime(&agora));

    char msg1[BUFFER_SIZE];
    snprintf(msg1, sizeof(msg1),
             "%s: CONECTADO!!\n"
             "=== MENU ===\n"
             "CPU-<segundos>      monitora uso de CPU (ex: CPU-5)\n"
             "memoria-<segundos>  monitora uso de memoria (ex: memoria-5)\n"
             "Quit                para os monitores\n"
             "Exit                encerra tudo e sai",
             hora);
    enviar(msg1);

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = recv(clienteFD, buffer, BUFFER_SIZE - 1, 0);
        if (n <= 0) break;

        buffer[strcspn(buffer, "\r\n")] = 0;

        if (strncasecmp(buffer, "CPU-", 4) == 0) {
            iniciar_monitor(MON_CPU, atoi(buffer + 4));
        } else if (strncasecmp(buffer, "memoria-", 8) == 0) {
            iniciar_monitor(MON_MEM, atoi(buffer + 8));
        } else if (strcasecmp(buffer, "Quit") == 0) {
            parar_todos_monitores();
            enviar("Monitores encerrados.");
        } else if (strcasecmp(buffer, "Exit") == 0) {
            enviar("Encerrando...");
            break;
        } else {
            enviar("Comando invalido.");
        }
    }

    parar_todos_monitores();
    close(clienteFD);
    close(socketFD);
    printf("Servidor encerrado.\n");
    return EXIT_SUCCESS;
}