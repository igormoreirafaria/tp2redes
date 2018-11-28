/*******select.c*********/
/*******Using select() for I/O multiplexing */
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
/* port we're listening on */
#define PORT 8080

int findFileSize(FILE *arq);
int file_exist(char*);
void lidaComHTTP(int sock);

int main(int argc, char *argv[])
{
    /* master file descriptor list */
    fd_set master;
    /* temp file descriptor list for select() */
    fd_set read_fds;
    /* server address */
    struct sockaddr_in serveraddr;
    /* client address */
    struct sockaddr_in clientaddr;
    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener;
    /* newly accept()ed socket descriptor */
    int newfd;
    /* buffer for client data */
    char buf[1024];
    int nbytes;
    /* for setsockopt() SO_REUSEADDR, below */
    int yes = 1;
    int addrlen;
    int i, j;
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    /* get the listener */
    if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Server-socket() error lol!");
        /*just exit lol!*/
        exit(1);
    }
    printf("Server-socket() is OK...\n");
    /*"address already in use" error message */
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }
    printf("Server-setsockopt() is OK...\n");

    /* bind */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);

    if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("Server-bind() error lol!");
        exit(1);
    }
    printf("Server-bind() is OK...\n");

    /* listen */
    if(listen(listener, 10) == -1)
    {
        perror("Server-listen() error lol!");
        exit(1);
    }
    printf("Server-listen() is OK...\n");

    /* add the listener to the master set */
    FD_SET(listener, &master);
    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one*/

    /* loop */
    for(;;)
    {
        /* copy it */
        read_fds = master;

        if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Server-select() error lol!");
            exit(1);
        }
        printf("Server-select() is OK...\n");

        /*run through the existing connections looking for data to be read*/
        for(i = 0; i <= fdmax; i++)
        {
            if(FD_ISSET(i, &read_fds))
            { /* we got one... */
                if(i == listener)
                {
                    /* handle new connections */
                    addrlen = sizeof(clientaddr);
                    if((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
                    {
                        perror("Server-accept() error lol!");
                    }
                    else
                    {
                        printf("Server-accept() is OK...\n");

                        FD_SET(newfd, &master); /* add to master set */
                        if(newfd > fdmax)
                        { /* keep track of the maximum */
                            fdmax = newfd;
                        }
                        printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
                        
                    }
                }
                else
                {
                    lidaComHTTP(i);
                    /* close it... */
                    close(i);
                    /* remove from master set */
                    FD_CLR(i, &master);
                }
            }
        }
    }
    return 0;
}

void lidaComHTTP(int sock){

    int read_size;
    char menssagem_cliente[2000];
    sds resposta;
    if( (read_size = recv(sock , menssagem_cliente , 2000 , 0)) <= 0 ){ //escuta no sock para receber msg do cliente
       perror("falha ao receber dados do cliente");
    }
    menssagem_cliente[read_size]='\0';
    printf("%s\n", menssagem_cliente);

    printf("------------------Resposta-----------------------\n");
    
    int tokens_size;
    int aux, fileSize, type_size;

    sds xablau = sdsnew(menssagem_cliente);
    sds *tokens = sdssplitlen(xablau, sdslen(xablau), " ", 1, &tokens_size);

    if(strcmp("GET", tokens[0]) != 0){
        resposta = sdsnew("HTTP/1.1 501 Not Implemented\r\n");
        send(sock, resposta, strlen(resposta), 0);
        return;
    }

    sds *tipo = sdssplitlen(tokens[2], sdslen(tokens[2]), "\r\n", 1, &aux);

    if(strcmp("HTTP/1.1", tipo[0]) != 0){
        resposta = sdsnew("HTTP/1.1 505 HTTP Version Not Supported\r\n");
        send(sock, resposta, strlen(resposta), 0);
        return;
    }
    sdstrim(tokens[1], "/");

    if (!file_exist(tokens[1])){
       resposta = sdsnew("HTTP/1.1 404 Not Found\r\n");
       printf("%s\n", resposta);
       send(sock, resposta, strlen(resposta), 0);
       return;
    }

    FILE *arq = fopen(tokens[1], "rb");
    fileSize = findFileSize(arq);

    resposta = sdsnew("HTTP/1.1 200 OK\r\n");
    
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
    send(sock, resposta, strlen(resposta), 0);


    char *buffer = malloc(fileSize*sizeof(char));
    fread(buffer, 1, fileSize, arq);
    send(sock, buffer, fileSize, 0);
    fclose(arq);
    free(buffer);
    
    printf("------------------------FIM--------------------------\n");

    return;
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