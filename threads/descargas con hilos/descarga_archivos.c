//programa que realiza una serie de descargas de YT mediante 
//youtube-dl, cada descarga es atendida por un hilo de ejecucion
//PRODUCTOR-CONSUMIDOR

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

/**DEFINICIONES PARA LECTURA DEL ARCHIVO DE ENTRADA**/
#define MAXIMA_LONGITUD_CADENA 12 // Debe ser de la máxima + 1.
#define CANTIDAD_LINEAS 10
#define NOMBRE_ARCHIVO "urls.txt"
char URLS[CANTIDAD_LINEAS][MAXIMA_LONGITUD_CADENA];

typedef struct {
   char** elementos;  // almacenamiento interno del buffer.
   int capacidad;   // capacidad máxima del buffer.
   int frente;      // índice para apuntar al elemento que está al frente.
   int final;       // índice para apuntar al elemento que está al final.
} Buffer;


void creaBuffer(Buffer* buf, int tam);

static inline void meterProducto(Buffer *b, char *p);

static inline void sacarProducto(Buffer *b, char *p);

void destruyeBuffer(Buffer* buf);

static inline bool bufLleno(Buffer* buf);  
static inline bool bufVacio(Buffer* buf);


void consumidor(void* ptr);

void leerArchivo();

#define TAREA_ESPECIAL "det_sistema"
#define NUM_CONSUMIDORES 5

// Número máximo de peticiones y tamaño del buffer.
#define  TAM_BUFF 3
Buffer bufTareas;    // Buffer de tareas para los consumidores


/* Declaración de semáforos y mutex */
sem_t lugares; /*Núm. de lugares disp. en el bufer*/
sem_t productos; /*Núm. de productos en el bufer*/
pthread_mutex_t mutexBuffer; /*Acceso exclusivo al bufer*/



int main(int argc, char const *argv[]) 
{
    //LEER ARCHIVO DE ENTRADA
    leerArchivo();
    srand(time(NULL));

    // Inicializar el buffer con una capacidad máxima FIJA.
    creaBuffer(&bufTareas, TAM_BUFF);

    // Incialización de semáforos y mutex (Variables de sincronización)
    
    sem_init(&lugares, 0, TAM_BUFF); 
    sem_init(&productos,0,0);
    pthread_mutex_init(&mutexBuffer, NULL);

    pthread_t hilosCons[NUM_CONSUMIDORES];

    for(size_t i=0; i<NUM_CONSUMIDORES;i++){
        pthread_create(&hilosCons[i], NULL,(void *)consumidor,NULL);
    }

    /** El hilo principal será el PRODUCTOR **/
    int n = 0;

    while (CANTIDAD_LINEAS>n) { 

        sem_wait(&lugares);
        pthread_mutex_lock(&mutexBuffer);
            meterProducto(&bufTareas, URLS[n]);
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&productos);
        
        n++;
        sleep((rand() % 10)+1);
    }

    printf("\nPROD: Ya terminé, esperaré a los consumidores...");
   
    // Esperar a TODOS los hilos consumidores
    for(size_t i=0; i<NUM_CONSUMIDORES;i++){
        pthread_join(hilosCons[i],NULL);
    }

    destruyeBuffer(&bufTareas);

    printf("\n\nTODOS LOS ARCHIVOS FUERON DESCARGADOS.\n\n");

    return 0;
}

/* Definición de la tarea que hará un consumidor */
void consumidor(void* ptr) {
    
    char url[20];
    bool continuar=true;
    char* dl = "youtube-dl -o '~/Desktop/descargas/";
    char* dl2= "' https://www.youtube.com/watch?v=";
    char command[150];

    while(continuar){

        printf("\n\nCONS: Quiero tomar un producto...\n");
        sem_wait(&productos);
        pthread_mutex_lock(&mutexBuffer);
            sacarProducto(&bufTareas, url);
        pthread_mutex_unlock(&mutexBuffer);
        sem_post(&lugares);
        printf("tome el producto: %s\n", url);

        if(strcmp(url, TAREA_ESPECIAL) != 0){
            //armando el comando youtube-dl
            strcpy(command,dl);
            strcat(command,url);
            strcat(command,dl2);
            strcat(command,url);
            printf("%s\n", command);
            system(command);

        }else{
            continuar=false;
        }
    }

    sem_wait(&lugares);
    pthread_mutex_lock(&mutexBuffer);
        meterProducto(&bufTareas, TAREA_ESPECIAL);
    pthread_mutex_unlock(&mutexBuffer);
    sem_post(&productos);
}

void leerArchivo (){
    char buferArchivo[MAXIMA_LONGITUD_CADENA];
    
    FILE *archivo = fopen(NOMBRE_ARCHIVO, "r");
    if (archivo == NULL)
    {
        printf("ERROR: No se puede abrir el archivo");
        return;
    }

    int indice = 0;

    // Mientras podamos leer una línea del archivo
    while (fgets(buferArchivo, MAXIMA_LONGITUD_CADENA, archivo))
    {
        if(strcmp(buferArchivo, "\n") != 0){
            strcpy(URLS[indice], buferArchivo);
            indice++;
        }
    }
    fclose(archivo);
    
    printf("Carga de entrada completada \n");
    return;
}

void creaBuffer(Buffer *buf, int tam)
{
   buf->capacidad = tam;
   buf->elementos = (char **)malloc(tam * sizeof(char*));
   buf->frente = -1; // Para indicar que está vacía la cola
   buf->final = -1;  // Para indicar que está vacía la cola
}

void destruyeBuffer(Buffer *buf)
{
   free(buf->elementos);
}

static inline bool bufLleno(Buffer *b)
{
   int next = (b->final + 1) % b->capacidad;

   if (b->frente == next)
      return true;
   else
      return false;
}

static inline bool bufVacio(Buffer *b)
{
   if (b->frente == -1)
      return true;
   else
      return false;
}

static inline void meterProducto(Buffer *b, char *p)
{
    if (!bufLleno(b))
    {
        b->final = (b->final + 1) % b->capacidad;
        b->elementos[b->final] = (char*)malloc(sizeof(char)*20);
        strcpy(b->elementos[b->final], p);
        if (b->frente == -1) // Cuando estaba vacia, el nuevo elemento esta en pos 0.
            b->frente = 0;
    }
}

static inline void sacarProducto(Buffer *b, char *p)
{
   if (!bufVacio(b))
   {
      strcpy(p, b->elementos[b->frente]);

      if (b->frente == b->final)
      {  // Era el unico elemento.
        b->frente = -1;
        b->final = -1;
      }
      else
        b->frente = (b->frente + 1) % b->capacidad;
   }
}


