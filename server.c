#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#define PORT 8080


int main() {
                
     int socketFD, clienteFD;         // Socket() -> retorna um inteiro fd que sera para manipula-lo na memoria futuramente.
                          // a1 -> Domain, familia de protocolo que sera usada (ipv4, ipv6...)
                          // a2 -> Type, SOCK_STREM ou SOCK_DTGRAM, usado baseado se for udp ou tcp a conexcao.
                          // a3 -> Protocol, 0 para padrao, escolhe tcp, udp...
                          // Basicamente estamos alocando na memoria do kernel a estrutura responsavel pela futura conexao que estabeleceremos.
                          // FALHA -> -1 
  
    if((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro no socket");
        exit(EXIT_FAILURE);
}


// Agora que temos um socket, temos que configurar qual porta e endereco esta utilizando.
struct sockaddr_in address;
address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);

    if (bind(socketFD, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Erro no bind");
        exit(EXIT_FAILURE);
}

// Com a porta setada, podemos entao falara ao kernel que qualquer tentativa de conexcao naquela porta deve er mandada para uma fila que iremos ter acesso pela funcao accept();

     if (listen(socketFD, 3) < 0) {
       perror("Erro no listen");
       exit(EXIT_FAILURE);
    }

printf("Aguardando conexões na porta %d...\n", PORT);


int addrlen = sizeof(address);

    if (( clienteFD = accept(socketFD, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {}

}
