#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024


void *thread_envio(void *arg) {
  int socketFD = *(int *)arg;
  char buffer[BUFFER_SIZE];


  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
      break;
    }

    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) > 0) {
      if (send(socketFD, buffer, strlen(buffer), 0) <= 0) {
        perror("Erro ao enviar mensagem");
        break;
      }

      if (strcasecmp(buffer, "Exit") == 0) {
        break;
      }
    }
  }
  pthread_exit(NULL);
}


void *thread_recepcao (void *arg) {
  int socketFD = *(int *)arg;
  char buffer[BUFFER_SIZE];
  int bytes_lidos;

  while (1) {
    memset(buffer, 0, BUFFER_SIZE);
    bytes_lidos = recv(socketFD, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_lidos > 0) {
      buffer[bytes_lidos] = '\0';
      printf("%s\n", buffer);

    } else if (bytes_lidos == 0) {
        printf("\n [Info] Ligacao encerrada pelo servidor.\n");
        break;
    } else {
        perror("Erro ao receber dados");
        break;
    }

  }
  exit(EXIT_SUCCESS);
}



int main(int argc, char *argv[]) {

  if(argc < 3) {
    printf("Chamada correta do arquivo: %s <IP_do_Servidor> <Porta>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *ip_servidor = argv[1];
  int porta = atoi(argv[2]);

  int socket_cliente;
  if ((socket_cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Erro ao criar socket");
    return EXIT_FAILURE;
  }


  struct sockaddr_in endereco_servidor;
  endereco_servidor.sin_family = AF_INET;
  endereco_servidor.sin_port = htons(porta);

  if (inet_pton(AF_INET, ip_servidor, &endereco_servidor.sin_addr) <= 0) {
    printf("Endereco IP invalido: %s\n", ip_servidor);
    return EXIT_FAILURE;
  }

  if (connect(socket_cliente, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor)) < 0) {
    perror("Erro ao conectar no servidor");
    return EXIT_FAILURE;
  }

  pthread_t t_envio, t_recepcao;

  pthread_create(&t_envio, NULL, thread_envio, &socket_cliente);
  pthread_create(&t_recepcao, NULL, thread_recepcao, &socket_cliente);

  pthread_join(t_envio, NULL);
  pthread_join(t_recepcao, NULL);

  close(socket_cliente);
  return EXIT_SUCCESS;

}