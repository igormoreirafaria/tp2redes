#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>

void *nova_conexao(void *);

int main(int argc , char *argv[]){
    //declaracao de variaveis
    int socket_desc , c , *new_sock;
    struct sockaddr_in server , client;
    
    //cria um socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1){
        printf("Nao foi possivel criar socket");
    }
    puts("Socket criado");

    //criacao do servidor
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8080 );

    //ligando o socket ao servidor
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
        perror("Erro");
        return 1;
    }
    puts("ligacao feita");

    //seta o socket para aceitar conexoes
    listen(socket_desc , 3);

    puts("Esperando por conexoes...");
    c = sizeof(struct sockaddr_in);
  
    while(1){
        int client_sock;
        //aceita uma nova conexão e atribui ao client_sock[i] o endereço do socket onde ocorrerá a comunicação
        if( client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c) ){
            puts("Conexao aceita");
        }

        if (client_sock < 0){
            perror("Falhou");
            return 1;
        }

        pthread_t xablau; //declaração da thread
        new_sock = malloc(1);
        *new_sock = client_sock; //atribuição para passar o socket como um
                                    //argumento na função a ser disparada pela thread
        if( pthread_create( &xablau , NULL ,  nova_conexao , (void*) new_sock) < 0){ //dispara a thread
            perror("Nao foi possivel criar thread");
            return 1;
        }
        puts("thread rodando");
    }

    

    return 0;
}


void *nova_conexao(void *socket_desc){
    int sock = *(int*)socket_desc; //atrinui o socket para comunicar com o cliente à variavel sock
    int read_size;
    
    char *messagem , *menssagem_cliente; //variaveis que serao utilizadas na troca de mensagens
    
    messagem = calloc(sizeof(char), 2000);
    menssagem_cliente = calloc(sizeof(char), 2000);

    while( (read_size = recv(sock , menssagem_cliente , 2000 , 0)) > 0 ){ //escuta no sock para receber msg do cliente
        
        write(sock , menssagem_cliente , strlen(menssagem_cliente));
    }

    if(read_size == 0){  //identifica se o cliente ainda está conectado
        puts("Cliente desconectado");
        fflush(stdout);
    }
    else if(read_size == -1){ //identifica se houve erro ao receber a mensagem do cliente
        perror("Recebimento falhou");
    }

    free(socket_desc);

    return 0;
}
