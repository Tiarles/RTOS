#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>		// inet_aton
#include <pthread.h>

typedef struct{
    float x, y;
} Position; 

int _TemperatureSensorValues[] = {};
Postion _GeolocalizationSensorValues[] = {};
int _PressionValues[] = {};

int sockfd;

void *leitura(void *arg) {
    char buffer[256];
    int n;
    while (1) {
        bzero(buffer,sizeof(buffer));
        n = recv(sockfd,buffer,50,0);
        if (n <= 0) {
            printf("Erro lendo do socket!\n");
            exit(1);
        }
        printf("MSG: %s\n",buffer);
    }
}

char buffer[256];


static int _TemperatureSensor_count;
static int _GeolocalizationSensor_count;
static int _PressionSensor_count;

static void *readTemperatureSensor (void *arg)
{
	struct periodic_info info;

	printf ("Temperature Sensor period 1s\n");
	make_periodic (1000000, &info);
	while (1)
	{
		_TemperatureSensor_count++;
		wait_period (&info);
	}
	return NULL;
}

static void *readGeolocalizationSensor (void *arg)
{
	struct periodic_info info;

	printf ("_Geolocalization Sensor period 2s\n");
	make_periodic (2000000, &info);
	while (1)
	{
		_GeolocalizationSensor_count++;
		wait_period (&info);
	}
	return NULL;
}

static void *readPressionSensor (void *arg)       // By Tiarles
{
	struct periodic_info info;

	printf ("Pression Sensor period 5s\n");
	make_periodic (5000000, &info);
	while (1)
	{
		_PressionSensor_count++;
		wait_period (&info);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
    int portno, n;
    struct sockaddr_in serv_addr;
    
    pthread_t t_1;  // Sensores
	pthread_t t_2;
	pthread_t t_3;
    
    pthread_t t;    // Cliente

    sigset_t alarm_sig;
    
    sigemptyset (&alarm_sig);

	for (i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaddset (&alarm_sig, i);
	
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
	
	pthread_create (&t_1, NULL, readTemperatureSensor, NULL);
	pthread_create (&t_2, NULL, readGeolocalizationSensor, NULL);
	pthread_create (&t_3, NULL, readPressionSensor, NULL);

    char buffer[256];
    
    if (argc < 3) {
       fprintf(stderr,"Uso: %s nomehost porta\n", argv[0]);
       exit(0);
    }
    
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        printf("Erro criando socket!\n");
        return -1;
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;

    //serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    inet_aton(argv[1], &serv_addr.sin_addr);
    serv_addr.sin_port = htons(portno);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro conectando!\n");
        return -1;
    }
    
    pthread_create(&t, NULL, leitura, NULL);
    
    do {
        bzero(buffer,sizeof(buffer));
        
        printf("Digite a mensagem (ou sair):");
        
        fgets(buffer,50,stdin);
        
        n = send(sockfd,buffer,50,0);
        
        if (n == -1) {
            printf("Erro escrevendo no socket!\n");
            return -1;
        }
        
        if (strcmp(buffer,"sair\n") == 0) {
            break;
        }
    } while (1);
    
    close(sockfd);
    
    return 0;
}
