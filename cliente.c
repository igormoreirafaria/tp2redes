#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>

int main(int argc , char *argv[]){
    //declaração e alocação de variaveis
    int sock;                                       
    struct sockaddr_in server;
    char *message , *server_reply;
    server_reply = calloc(sizeof(char), 2000);
    message = malloc(1000 * sizeof(char));
    char *id = malloc(3 * sizeof(char));

    sock = socket(AF_INET , SOCK_STREAM , 0); // cria o socket
    if (sock == -1){
        printf("Nao foi possivel criar o socket");
    }
    puts("Socket criado");

    server.sin_addr.s_addr = inet_addr("127.0.0.1"); //configura qual servidor
    server.sin_family = AF_INET;                     //havera a conexao
    server.sin_port = htons( 8080 );


    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0){ //tenta conectar 
        perror("Erro, falha na conexao");                                 //ao servidor
        return 1;
    }

    puts("Conectado\n");

   
   
    while(1){

        printf("Digite uma mensagem: ");
        fgets(message, 1000, stdin);

        if(send(sock , message , strlen(message) , 0) < 0){//envia a carta escolhida pelo usuario
            puts("Falha ao enviar");
            return 1;
        }

        if( recv(sock , server_reply , 2000 , 0) < 0){ //recebe o broadcast
            puts("Falha ao receber mensagem");
            break;
        }
        
        puts(server_reply);//imprime a carta recebida do broadcast
        server_reply = calloc(sizeof(char), 2000);//zera a variavel para ser usada depois
    }

    close(sock);
    return 0;
}
