#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char* argv[])
{
	int size, rank, tag, rc, min, lcounter;	
	double t1, t2; 
	int inmsg[2];	//mensagem de chegada [rank, contador]
	int msg[2]; 	//mensagem de saida [rank, contador]

	MPI_Status Stat;
	rc=MPI_Init(&argc,&argv);

	bool terminated=false;
	if (rc!=0) {
		printf("Erro ao iniciar MPI\n"); 
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	

	msg[0]=rank;
	msg[1]=0;
	min=rank; 	//cada processo inicialmente tem como minimo valor seu proprio rank
	lcounter=0;	//contador de mensagens recebidas

	MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD); //envia mensagem para vizinho da direita (rank + 1), circulando as pontas do anel

	while (!terminated){
		MPI_Recv(&inmsg, 2, MPI_INT, (rank-1)%size, 1, MPI_COMM_WORLD, &Stat); //recebe mensagem do vizinho da esquerda
		lcounter++;	//incrementa contador
		if (min>inmsg[0]){
			min=inmsg[0];
		}	// se valor recebido é menor que mínimo local, altera candidato a líder
		inmsg[1]++;	//aumenta contador da mensagem chegada

		if (inmsg[0]==rank && lcounter==(inmsg[1])) { //caso valores recebidos sejam iguais aos valores locais, valor local de min é eleito liíder
			printf("Processo %d elege processo %d como líder\n", rank, min);	
			terminated=true; //encerra ciclo
		}
	
		// passa mensagem recebida para valor da direita
		MPI_Send(&inmsg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
			
	}
	MPI_Finalize();
}