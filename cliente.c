/** 
    \file cliente.c
    \brief O cliente (no caso a cabine de um avião) da comunicação
           socket.
    O cliente possui três semsores (no caso funções que leem de 
    vetores previamente inicializados).
**/

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

#include <signal.h>
#include <errno.h>
#include <time.h>

#define SIZE_A 9            // Neste periodo, pode escrever em até 10 posições (9 para fazer travá-lo)
#define SIZE_B 4            // Neste periodo, pode escrever em até 5 posições  (4 para fazer travá-lo)

#define NEXTPOS_PROD (posProdutor   + 1) % SIZE
#define NEXTPOS_CONS (posConsumidor + 1) % SIZE

struct periodic_info
{
	int sig;
	sigset_t alarm_sig;
};

int posProdutor = 0;
int tamanhoA    = 0;
int tamanhoB    = 0;

int dataRandom[] = { 0,  1,  2,  3,  4,\
                     5,  6,  7,  8,  9,\
                    10, 11, 12, 13, 14,\
                    15, 16, 17, 18, 19,\
                    20, 21, 22, 23, 24,\
                    25, 26, 27, 28, 29,\
                    30, 31, 32, 33, 34,\
                    35, 36, 37, 38, 39,\
                    40, 41, 42, 43, 44,\
                    45, 46, 47, 48, 49};

int filaA[SIZE_A];
int filaB[SIZE_B];

int sockfd;

pthread_mutex_t m_fila = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  c_fila = PTHREAD_COND_INITIALIZER;

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

void printFila(int* fila, int size)  //TEMPORÁRIA
{
    int i;    
    printf("\nFila = [");

    for(i = 0; i < size - 1; i++)
        printf("%d, ", fila[i]);        
    
    printf("%d]\n", fila[size - 1]);
    return;
}

static void *generateDataA(void *arg)
{
	struct periodic_info info;
	
	printf("Inside generateDataA\n");
	
	make_periodic (1000000, &info);
	while (1)
	{
	    wait_period (&info);
	
	    pthread_mutex_lock(&m_fila);
	    
        printf ("Generate Data A period 1s\n");
	    
	    while(tamanhoA == SIZE_A)
            pthread_cond_wait(&c_fila, &m_fila);
	    
	    filaA[tamanhoA] = dataRandom[posProdutor % 50];
		
		posProdutor++;
		tamanhoA++;
		
        printFila(filaA, SIZE_A);
		
		pthread_cond_signal(&c_fila);
        pthread_mutex_unlock(&m_fila);
	}
	return NULL;
}

static void *generateDataB(void *arg)       // By Tiarles
{
	struct periodic_info info;
	
	printf("Inside generateDataB\n");
	
	make_periodic (2000000, &info);
	while (1)
	{
	    wait_period (&info);
	    
        pthread_mutex_lock(&m_fila);
	    
        printf ("Generate Data B period 2s\n");   
	       
	    while(tamanhoB == SIZE_B)
            pthread_cond_wait(&c_fila, &m_fila);
	    
	    filaB[tamanhoB] = dataRandom[posProdutor % 50];
		
		posProdutor++;
		tamanhoB++;
		
	    printFila(filaB, SIZE_B);
		
		pthread_cond_signal(&c_fila);
        pthread_mutex_unlock(&m_fila);
	}
	return NULL;
}

static void *passMedianBySocket(void *arg)
{

    struct periodic_info info;
    char str[10];
    
   	printf("Inside passMedianBySocket\n");
    
	int median_filaA = 0;
	int median_filaB = 0;
	int i, n;
	make_periodic(10000000, &info);
	while (1)
	{
	
	    wait_period (&info);
	    
        pthread_mutex_lock(&m_fila);
	    
        printf ("Send to Socket Data! 10s\n");
            	    
	    for(i = 0; i < SIZE_A; i++)
	        median_filaA += filaA[i];
	    median_filaA /= SIZE_A;

	    for(i = 0; i < SIZE_B; i++)
	        median_filaB += filaB[i];
	    median_filaB /= SIZE_B;
	    	    	
	    // Format and send by socket

        sprintf(str, "%d - %d", median_filaA, median_filaB);      	    

	    n = send(sockfd, str, 10, 0);
        
        if (n == -1) {
            printf("Erro escrevendo no socket!\n");
            return -1;
        }
		
		tamanhoA = 0;
		tamanhoB = 0;
		// Zera as duas filas
		
	    for(i = 0; i < SIZE_A; i++)
	        filaA[i] = 0;

	    for(i = 0; i < SIZE_B; i++)
            filaB[i] = 0;		
		
	    printFila(filaA, SIZE_A);
   	    printFila(filaB, SIZE_B);
		
		pthread_cond_signal(&c_fila);
        pthread_mutex_unlock(&m_fila);
	}
	return NULL;
	

   
    close(sockfd);
}

int main(int argc, char *argv[]) {
    
    printf("Inside main!\n");
     
    pthread_t t_1;
	pthread_t t_2;
	pthread_t t_3;
    
    int portno = atoi(argv[2]);           // portno já configurado. 3000
    int i;
    struct sockaddr_in serv_addr;
   
    sigset_t alarm_sig;
    sigemptyset (&alarm_sig);
   
    printf("Inside main 2\n");
   
	for (i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaddset (&alarm_sig, i);
	
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
	
	// Socket AREA

    printf("Inside main 3\n");
    
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Erro criando socket!\n");
        return -1;
    }
	
    printf("Inside main 4\n");
	
	bzero((char *) &serv_addr, sizeof(serv_addr));

    printf("Inside main 5\n");

    serv_addr.sin_family        = AF_INET;
    inet_aton(argv[1], &serv_addr.sin_addr);                // AviaoNome
    serv_addr.sin_port          = htons(portno);

    printf("Inside main 6\n");
    
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Erro conectando!\n");
        return -1;
    }
    
	pthread_create (&t_1, NULL, generateDataA,      NULL);
	pthread_create (&t_2, NULL, generateDataB,      NULL);
    pthread_create (&t_3, NULL, passMedianBySocket, NULL);

    pthread_join(&t_1, NULL);
    pthread_join(&t_2, NULL);
    pthread_join(&t_3, NULL);
    
    return 0;
}
