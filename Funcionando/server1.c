#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include"sds.h"
#include"sdsalloc.h"

void *nova_conexao(void *);
char *tratahttp(char *menssagem_cliente, int client_sock);
int findFileSize(FILE *arq);
int file_exist(char*);

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
    server.sin_port = htons( atoi(argv[1]) );

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
        char *resposta;
        //aceita uma nova conexão e atribui ao client_sock[i] o endereço do socket onde ocorrerá a comunicação
        if( client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c) ){
            puts("Conexao aceita");
        }

        if (client_sock < 0){
            perror("Falhou");
            return 1;
        }

        int read_size;
        char *messagem , *menssagem_cliente; //variaveis que serao utilizadas na troca de mensagens
    
        messagem = calloc(sizeof(char), 2000);
        menssagem_cliente = calloc(sizeof(char), 2000);

        if( (read_size = recv(client_sock , menssagem_cliente , 2000 , 0)) > 0 ){ //escuta no sock para receber msg do cliente
            menssagem_cliente[read_size]='\0';
            printf("%s\n", menssagem_cliente);

            printf("------------------Resposta-----------------------\n");

            resposta = tratahttp(menssagem_cliente, client_sock);
            printf("--------------------------------------------------\n");

        }

        if(read_size == 0){  //identifica se o cliente ainda está conectado
            puts("Cliente desconectado");
            fflush(stdout);
        }
        else if(read_size == -1){ //identifica se houve erro ao receber a mensagem do cliente
            perror("Recebimento falhou");
        }
        
        puts("conexao encerrada");
       

    }

    

    return 0;
}

char* tratahttp(char *menssagem_cliente, int client_sock){

    int tokens_size;
    int aux, fileSize, type_size;

    sds xablau = sdsnew(menssagem_cliente);
    sds *tokens = sdssplitlen(xablau, sdslen(xablau), " ", 1, &tokens_size);
    
    if(strcmp("GET", tokens[0]) != 0){
        return sdsnew("HTTP/1.1 501 Not Implemented\r\n");
    }
    
    sds *tipo = sdssplitlen(tokens[2], sdslen(tokens[2]), "\r\n", 1, &aux);

    if(strcmp("HTTP/1.1", tipo[0]) != 0){
        return sdsnew("HTTP/1.1 505 HTTP Version Not Supported\r\n");
    }
    sdstrim(tokens[1], "/");

    
     
    if (!file_exist(tokens[1])){
        return sdsnew("HTTP/1.1 404 Not Found\r\n");
    }
    FILE *arq = fopen(tokens[1], "rb");
    fileSize = findFileSize(arq);
  
    sds resposta = sdsnew("HTTP/1.1 200 OK\r\n");
    
    resposta = sdscatprintf(resposta, "Content-Length: %d\r\n", fileSize);

    sds *type = sdssplitlen(tokens[1], sdslen(tokens[1]), ".", 1, &aux);

    if (strcmp(type[1], "html") == 0 || strcmp(type[1], "htm") == 0){
        resposta = sdscat(resposta, "Content-Type: text/html\r\n"); 
    }
    if (strcmp(type[1], "jpg") == 0 || strcmp(type[1], "jpeg") == 0){
        resposta = sdscat(resposta, "Content-Type: image/jpeg\r\n"); 
    }
    if (strcmp(type[1], "gif") == 0){
        resposta = sdscat(resposta, "Content-Type: image/gif\r\n");
    }
    if (strcmp(type[1], "png") == 0){
        resposta = sdscat(resposta, "Content-Type: image/png\r\n");
    }
    if (strcmp(type[1], "css") == 0){
        resposta = sdscat(resposta, "Content-Type: text/css\r\n");
    }
    if (strcmp(type[1], "au") == 0){
        resposta = sdscat(resposta, "Content-Type: audio/basic\r\n");
    }
    if (strcmp(type[1], "wav") == 0){
        resposta = sdscat(resposta, "Content-Type: audio/wav\r\n");
    }
    if (strcmp(type[1], "avi") == 0){
        resposta = sdscat(resposta, "Content-Type: video/x-msvideo\r\n");
    }
    if (strcmp(type[1], "mpeg") == 0 || strcmp(type[1], "mpg") == 0){
        resposta = sdscat(resposta, "Content-Type: video/mpeg\r\n");
    }
    if (strcmp(type[1], "mp3") == 0){
        resposta = sdscat(resposta, "Content-Type: audio/mpeg\r\n");
    }
    if (strcmp(type[1], "js") == 0){
        resposta = sdscat(resposta, "Content-Type: text/javascript\r\n");
    }
    if (strcmp(type[1], "ico") == 0){
        resposta = sdscat(resposta, "Content-Type: image/x-icon\r\n");
    }

    resposta = sdscat(resposta, "Connection: close\r\n\r\n");

    printf("%s\n", resposta);

    char *linha = malloc(fileSize*sizeof(char));

    while(!feof(arq)){
        fread(linha, 1, fileSize, arq);
    }
    
    int tamResposta = strlen(resposta);

    memcpy(resposta + tamResposta, linha, fileSize);

    //resposta = sdscat(resposta, linha);
    if(send(client_sock, resposta, tamResposta + fileSize, 0) < 0){
        perror("Erro");
    }
    close(client_sock);
    fclose(arq);

    return resposta;
}

int findFileSize(FILE *arq){

    struct stat size;
    fstat(fileno(arq), &size);
    return size.st_size;
}

int file_exist (char *filename){
  struct stat buffer;   
  return (stat (filename, &buffer) == 0);
}