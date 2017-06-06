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

tyṕedef struct{
    float x, y;
} Position; 

int _TemperatureSensorValues[] = {};
Postion _GeolocalizationSensorValues[] = {};
int _PressionValues[] = {};

int sockfd;

static int make_periodic (int unsigned period, struct periodic_info *info)
{
	static int next_sig;
	int ret;
	unsigned int ns;
	unsigned int sec;
	struct sigevent sigev;
	timer_t timer_id;
	struct itimerspec itval;

	/* Initialise next_sig first time through. We can't use static
	   initialisation because SIGRTMIN is a function call, not a constant */
	if (next_sig == 0)
		next_sig = SIGRTMIN;
	/* Check that we have not run out of signals */
	if (next_sig > SIGRTMAX)
		return -1;
    
    //printf("TEMP(make_periodic) %d\n", next_sig); // By Tiarles
	
	info->sig = next_sig;
	next_sig++;
	/* Create the signal mask that will be used in wait_period */
	sigemptyset (&(info->alarm_sig));
	sigaddset (&(info->alarm_sig), info->sig);

	/* Create a timer that will generate the signal we have chosen */
	sigev.sigev_notify = SIGEV_SIGNAL;
	
	//printf("TEMP(make_periodic) %d\n", SIGEV_SIGNAL); // By Tiarles
	
	sigev.sigev_signo = info->sig;
	sigev.sigev_value.sival_ptr = (void *) &timer_id;
	ret = timer_create (CLOCK_MONOTONIC, &sigev, &timer_id);
	if (ret == -1)
		return ret;

	/* Make the timer periodic */
	sec = period/1000000;
	ns = (period - (sec * 1000000)) * 1000;
	
	//printf("TEMP(make_periodic) %d\n", period); // By Tiarles
	//printf("TEMP(make_periodic) %d\n", sec); // By Tiarles
	//printf("TEMP(make_periodic) %d\n", ns); // By Tiarles
	
	itval.it_interval.tv_sec = sec;
	itval.it_interval.tv_nsec = ns;
	itval.it_value.tv_sec = sec;
	itval.it_value.tv_nsec = ns;
	ret = timer_settime (timer_id, 0, &itval, NULL);
	return ret;
}


static void wait_period (struct periodic_info *info)
{
	int sig;
	sigwait (&(info->alarm_sig), &sig);
}

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

static int _TemperatureSensor_count;
static int _GeolocalizationSensor_count;
static int _PressionSensor_count;

static void *readTemperatureSensor (void *arg)
{
	struct periodic_info info;

	printf ("Thread 1 period 10ms\n");
	make_periodic (10000, &info);
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

	printf ("Thread 2 period 20ms\n");
	make_periodic (20000, &info);
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

	printf ("Thread 3 period 30ms\n");
	make_periodic (30000, &info);
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
    
    pthread_t t_1;
	pthread_t t_2;
	pthread_t t_3;
    sigset_t alarm_sig;
    
    pthread_t t;
    
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
//    serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
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