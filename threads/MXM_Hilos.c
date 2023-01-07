#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

void mult(void *ptr);

int **A, **B, **C;
int n,m,q; 
int nHilos; 

//compilar: gcc MXM_Hilos.c -lpthread -o MXM_Hilos
//ejecutar: ./MXM_Hilos n m q nHilos

int main(int argc, char const *argv[]){
	//recoge variables n, m, q y nHilos desde consola
	sscanf(argv[1], "%i", &n);
	sscanf(argv[2], "%i", &m);
	sscanf(argv[3], "%i", &q);
	sscanf(argv[4], "%i", &nHilos);

	//aloja espacio en memoria para A, B y C
	A = (int **) malloc (sizeof (int *)*n);
	B = (int **) malloc (sizeof (int *)*m);
	C = (int **) malloc (sizeof (int *)*n);
	
	for(int i=0; i<n; i++)
		A[i]= (int *) malloc(sizeof(int)*m);
		
	for(int i=0; i<m; i++)
		B[i]= (int *) malloc(sizeof(int)*q);
		
	for(int i=0; i<n; i++)
		C[i]= (int *) malloc(sizeof(int)*q);

	//asignacion de numeros aleatorios a A y B
	srand(time(NULL));

	for(int i=0;i<n;i++){
		for(int j=0;j<m;j++){
			A[i][j]=(rand() % 15)+1;
			//printf("%d ",A[i][j]);
		}
        //printf("\n");
	}
	//printf("\nx\n\n");
	for(int i=0;i<m;i++){
		for(int j=0;j<q;j++){
			B[i][j]=(rand() % 15)+1;
			//printf("%d ",B[i][j]);
		}
        //printf("\n");
	}

	//inicializa C con ceros
	for(int i=0;i<n;i++){
		for(int j=0;j<q;j++)
			C[i][j]=0;
	}
	
	//crear hilos
	pthread_t Hilos[nHilos];	//arreglo de hilos
	int  id_Hilo[nHilos];			//IDs de los hilos

	for(int i=0;i<nHilos; i++){
        id_Hilo[i]=i;
		pthread_create(&Hilos[i], NULL, (void *) mult, &id_Hilo[i]);
	}

	for(int i=0; i<nHilos;i++){
		pthread_join(Hilos[i],NULL);
	}

	//imprime matriz de resultados
	/*printf("\n=\n\n");
	for(int i=0;i<n;i++){
		for(int j=0;j<q;j++)
			printf("%d ",C[i][j]);
        printf("\n");
	}*/

	return 0;
}

void mult(void * id_Hilo){
	
	
	int id = *((int*)id_Hilo);				
	int nfilas = n/nHilos;							//cantidad de filas a multiplicar
	int i_fila = id*nfilas;							//obtiene indice de inicio del hilo actual

	if(n-(i_fila+nfilas) < nfilas)					//si las filas restantes por evaluar son menores al total de filas
		nfilas+=n-(i_fila+nfilas);					//agregar el residuo a las filas del hilo actual

	for(int i=i_fila; i<i_fila+nfilas; i++){		//multiplicacion de renglones correspondientes al hilo actual
		for(int j=0; j<q; j++){
			for(int h=0; h<m; h++)
				C[i][j]+=A[i][h]*B[h][j];	
		}
	} 
}