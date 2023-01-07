/*
* Distribución de un arreglo entre procesos para la suma de sus elementos.
* MAESTRO-TRABAJADOR
*/
#include<stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h> /* Para la funciones de MPI, etc. */

#define MAESTRO 0
#define MAX_BITS 64
// Tamaño del arreglo que demora aprox. 30 seg. de ejecucion en servidor fsoft
const long long ARRAY_TAM = 18000000;

//funciones para suma binaria
long long suma_binaria(long long a, long long b);
long long bin2dec(int* bin);
void dec2bin(long long a, int* bin);
void printBin(int* bin);
void initBin(int* bin);


int main(int argc, char *argv[])
{
   int numProcs;  //Número de procesos.
   int myRank;    //identificador de proceso.

   long* arreglo;       // El proceso 0 deberá crearlo dinámicamente.
   long inicio = 0;     // indice donde inicia el subarreglo que enviará el MAESTRO.
   long tamSubarreglo;  // Número de elementos de cada subarreglo.
   long res;            //Residuo

   MPI_Init(&argc, &argv);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
   MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

   tamSubarreglo = ARRAY_TAM/numProcs;    //Cantidad de elementos a sumar por proceso sin residuo
   res = ARRAY_TAM%numProcs;              //Residuo a repartir
   
   if (myRank == MAESTRO) {
      // SOLAMENTE el proceso MAESTRO

      // Crear el arreglo de tamaño ARRAY_TAM
      arreglo = (long*) malloc(ARRAY_TAM * sizeof(long));

      // Llenarlo con los número 0, 1, 2,...,ARRAY_TAM-1
      long i;
      for (i = 0; i < ARRAY_TAM; i++){
         arreglo[i] = i+1;
         //printf("%ld | ", arreglo[i]);
      }

      if(res>=1){                          //si hay residuo
         inicio = tamSubarreglo + 1;       //indice de inicio de siguiente proceso se recorre un elemento 
      }else{
         inicio = inicio + tamSubarreglo;  //residuo no hay residuo
      }

      //INICIA Distribución de elementos del arreglo entre los procesos

      int j=1;                             //indice primer proceso trabajador
      for(j=1; j<res; j++){                //para todo trabajador menor que el residuo (si hay) se aumenta un elemento
         MPI_Send(&arreglo[inicio], tamSubarreglo+1, MPI_LONG, j, 0, MPI_COMM_WORLD);
         inicio = inicio + tamSubarreglo+1;
      }

      int k;               
      for(k=j; k<numProcs; k++){             //para todo trabajador mayor al residuo recibe ARRAY_TAM/numProcs elementos
         MPI_Send(&arreglo[inicio], tamSubarreglo, MPI_LONG, k, 0, MPI_COMM_WORLD);
         inicio = inicio + tamSubarreglo;
      }

      if(res>0){                             //Si hay residuo entonces proceso MAESTRO hace un elemento mas                   
         tamSubarreglo = tamSubarreglo+1;
      }
   
      long long suma = 0;                    //Acumulador parcial
         
      
      for (i = 0; i < tamSubarreglo; i++)    //Suma binaria parcial del proceso MAESTRO
         suma = suma_binaria(suma, arreglo[i]);
     
      long long sumaTotal = suma;            //Acumulador de suma total

      for(i = 1; i< numProcs; i++){          //Recepción de mensajes de procesos trabajadores y calculo de suma total
            MPI_Recv(&suma, 1, MPI_LONG_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sumaTotal = sumaTotal + suma;
      }

      printf("\n %d: la suma total es: %lld\n", myRank, sumaTotal);

      free(arreglo);       //libera memoria
   }
   else { // Esto lo harán los trabajadores.
     
      long long suma = 0;

      if(myRank<res)           //si es un trabajador menor que el residuo se aumenta un elemento
         tamSubarreglo = tamSubarreglo+1;
      
      arreglo = (long*) malloc(tamSubarreglo * sizeof(long));  //reserva memoria para subarreglo correspondiente       
      MPI_Recv(arreglo, tamSubarreglo, MPI_LONG, MAESTRO, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);   //recibe mensaje de proceso MAESTRO
      
      long i;
      for (i = 0; i < tamSubarreglo; i++)          //suma binaria parcial de elementos correspondientes al trabajador
         suma = suma_binaria(suma, arreglo[i]);
      
      MPI_Send(&suma, 1, MPI_LONG_LONG, MAESTRO, 0, MPI_COMM_WORLD); //envio de suma parcial al proceso MAESTRO
      free(arreglo);       //libera memoria
   
   }

   MPI_Finalize();

   return 0;
   
} // fin del main

long long suma_binaria(long long a, long long b) {
    // Convertir a y b a binario
    int binA[MAX_BITS];
    int binB[MAX_BITS];
    int binC[MAX_BITS];

    initBin(binA);
    initBin(binB);
    initBin(binC);

    dec2bin(a, binA);
    dec2bin(b, binB);
    
    //printBin(binA);
    //printBin(binB);

    int res = 0;
    int i = 0;
     do {
        int sumaBits = binA[i] + binB[i] + res;
        binC[i] = sumaBits % 2;
        res = sumaBits / 2;
        i++;
    } while (i < MAX_BITS);

    //printBin(binC);

    long long resultado = bin2dec(binC);

    return resultado;
}

void dec2bin(long long a, int* bin) {
    int i = 0;
    while (a != 0 && i < MAX_BITS) {
        long long residuo = a % 2;
        bin[i] = residuo;
        a = a/2;
        i++;
    }
}

long long bin2dec(int* bin) {
    long long potencia = 1;
    long long decimal = 0;

    for (size_t i = 0; i < MAX_BITS; i++)
    {
        decimal = decimal + bin[i]*potencia;
        potencia *= 2;
    }

    return decimal;
}

void printBin(int* bin) {
    printf("\n");
    for (size_t i = 0; i < MAX_BITS; i++)
        printf("%d", bin[i]);
}

void initBin(int* bin) {
    for (size_t i = 0; i < MAX_BITS; i++)
        bin[i] = 0;
}

