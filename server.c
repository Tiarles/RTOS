#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

int newsockfd[5];
int id = 0;

void *cliente(void *arg) {
    int cid = (int)arg;
    int i, n;
    char buffer[10];
    
    while (1) {
        bzero(buffer,sizeof(buffer));
        
        n = read(newsockfd[cid], buffer, 10);
        
        printf("Recebeu: %s\n", buffer, strlen(buffer));
         
        if (n < 0) {
             printf("Erro lendo do socket!\n");
             exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    int sockfd;
    int portno = atoi(argv[1]);

    pthread_t t;
     
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("Erro abrindo o socket!\n");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro fazendo bind!\n");
        exit(1);
    }

    listen(sockfd,5);

    while (1) {
    	newsockfd[id] = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);

       	if (newsockfd[id] < 0) {
           	printf("Erro no accept!\n");
         	exit(1);
    	}

        pthread_create(&t, NULL, cliente, (void *)id);

    	id++;
    }
 
    return 0; 
}
