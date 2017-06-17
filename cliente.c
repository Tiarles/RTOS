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

#include "periodic.h"

/**
    \def SIZE Tamanho do buffer que se comunica com o socket.
    \def NEXTPOS_PROD Macro que calcula a próxima posição
                      em forma de buffer circular do produtor.
    \def NEXTPOS_CONS Macro que calcula a próxima posição
                      em forma de buffer circular do consumidor. 
**/

#define SIZE 20 
#define NEXTPOS_PROD (posProdutor   + 1) % SIZE
#define NEXTPOS_CONS (posConsumidor + 1) % SIZE

#define NEXT_TEMPCOUNT (_TemperatureSensor_count + 1) % 10
#define NEXT_GEOLCOUNT (_GeolocalizationSensor_count + 1) % 10
#define NEXT_PRESCOUNT (_PressionSensor_count + 1) % 10

/**
    \struct DataBuffer É o tipo de dado que vai ser lido e 
                       escrito do e no buffer de comunicação.
**/

typedef struct{
    float dado;
} DataBuffer;

/**
    \brief Variáveis Globais
    \var posProdutor 
    \var posConsumidor 
    \var tamanhoAtual  
**/

int posProdutor   = 0;
int posConsumidor = 0;
int tamanhoAtual  = 0;

// END DOXYGEN - END DOXYGEN - END DOXYGEN - END DOXYGEN - END DOXYGEN

// Dados que o sensor estaria recebendo
DataBuffer dataTemperature[]     = {10.0, 10.1, 10.2, 10.3, 10.4,\
                                    10.5, 10.6, 10.7, 10.8, 10.9};
DataBuffer dataGeolocalization[] = {20.0, 20.1, 20.2, 20.3, 20.4,\
                                    20.5, 20.6, 20.7, 20.8, 20.9};
DataBuffer dataPression[]        = {30.0, 30.1, 30.2, 30.3, 30.4,\
                                    30.5, 30.6, 30.7, 30.8, 30.9};

DataBuffer buffer[SIZE];

static int _TemperatureSensor_count     = 0;
static int _GeolocalizationSensor_count = 0;
static int _PressionSensor_count        = 0;

pthread_mutex_t m_buffer = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  c_buffer = PTHREAD_COND_INITIALIZER;

void printBuffer()  //TEMPORÁRIA
{
    int i;    
    printf("\nBUFFER = [");

    for(i = 0; i < SIZE - 1; i++)
        printf("%.1f, ", buffer[i].dado);        
    
    printf("%.1f]\n", buffer[SIZE - 1].dado);
    return;
}

static void *readTemperatureSensor (void *arg)
{
	struct periodic_info info;
	
	make_periodic (1000000, &info);
	while (1)
	{
	    pthread_mutex_lock(&m_buffer);
	    
	    printf ("Temperature Sensor period 1s\n");
	    
	    while(tamanhoAtual == SIZE)
            pthread_cond_wait(&c_buffer, &m_buffer);
	    
	    buffer[posProdutor] = dataTemperature[_TemperatureSensor_count];
		
		_TemperatureSensor_count = NEXT_TEMPCOUNT;
		posProdutor = NEXTPOS_PROD;
		tamanhoAtual++;
		
		printBuffer();
		
		pthread_cond_signal(&c_buffer);
        pthread_mutex_unlock(&m_buffer);
        
		wait_period (&info);
	}
	return NULL;
}

static void *readGeolocalizationSensor (void *arg)
{
	struct periodic_info info;
	
	make_periodic (2000000, &info);
	while (1)
	{
	    pthread_mutex_lock(&m_buffer);
	    
	    printf ("_Geolocalization Sensor period 2s\n");
	    
	    while(tamanhoAtual == SIZE)
            pthread_cond_wait(&c_buffer, &m_buffer);
	    
	    buffer[posProdutor] = dataGeolocalization[_GeolocalizationSensor_count];
		
		_GeolocalizationSensor_count = NEXT_GEOLCOUNT;
		posProdutor = NEXTPOS_PROD;
		tamanhoAtual++;
		
        printBuffer();
		
		pthread_cond_signal(&c_buffer);
        pthread_mutex_unlock(&m_buffer);
		
		wait_period (&info);
	}
	return NULL;
}

static void *readPressionSensor (void *arg)       // By Tiarles
{
	struct periodic_info info;

	make_periodic (5000000, &info);
	while (1)
	{
        pthread_mutex_lock(&m_buffer);
	    
        printf ("Pression Sensor period 5s\n");   
	       
	    while(tamanhoAtual == SIZE)
            pthread_cond_wait(&c_buffer, &m_buffer);
	    
	    buffer[posProdutor] = dataPression[_PressionSensor_count];
		
		_PressionSensor_count = NEXT_PRESCOUNT;
		posProdutor = NEXTPOS_PROD;
		tamanhoAtual++;
		
	    printBuffer();
		
		pthread_cond_signal(&c_buffer);
        pthread_mutex_unlock(&m_buffer);
		
		wait_period (&info);
	}
	return NULL;
}

int main(int argc, char *argv[]) {
    
    pthread_t t_1;  // Sensores
	pthread_t t_2;
	pthread_t t_3;
    
    int i;
   
    sigset_t alarm_sig;
    sigemptyset (&alarm_sig);

	for (i = SIGRTMIN; i <= SIGRTMAX; i++)
		sigaddset (&alarm_sig, i);
	
    sigprocmask (SIG_BLOCK, &alarm_sig, NULL);
	
	pthread_create (&t_1, NULL, readTemperatureSensor, NULL);
	pthread_create (&t_2, NULL, readGeolocalizationSensor, NULL);
	pthread_create (&t_3, NULL, readPressionSensor, NULL);

    pthread_join(&t_1, NULL);
    pthread_join(&t_2, NULL);
    pthread_join(&t_3, NULL);
    
    return 0;
}
