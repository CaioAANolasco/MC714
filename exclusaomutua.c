#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

int main( int argc, char *argv[] ) {
    int rank, size, len;
    int rc = MPI_Init( &argc, &argv);
    if (rc!=0) {
		printf("Erro ao iniciar MPI\n"); 
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    char name[MPI_MAX_PROCESSOR_NAME];

    // no processo de rank 0, inicia-se a janela da região compartilhada entre processos
    int *addr = 0, winSz = 0;
    if (rank == 0) {
        winSz = sizeof(int);
        MPI_Alloc_mem(winSz, MPI_INFO_NULL, &addr); //aloca memoria compartilhada
        *addr = 1; 
    }

    MPI_Win win;
    MPI_Win_create( addr, winSz, sizeof( int ), MPI_INFO_NULL, MPI_COMM_WORLD, &win ); //inicia objeto window de MPI para compartilhar entre processos

    int counter, one = 1; //counter é a variável compratilhada que é incrementada em um para cada processo
    MPI_Win_lock( MPI_LOCK_EXCLUSIVE, 0, 0, win );
    printf("Processo %d pediu acesso a região crítica\n", rank);
    MPI_Fetch_and_op( &one, &counter, MPI_INT, 0, 0, MPI_SUM, win ); //aumenta o valor de clock 1
    MPI_Win_unlock( 0, win );
    printf("Process %d liberou lock\n", rank);

    printf("Process %d aumentou contador para %d\n", rank,counter);

    // encerrar e liberar memório alocada
    MPI_Win_free(&win);
    if ( rank == 0 ) {
        MPI_Free_mem( addr );
    }
    MPI_Finalize();
    return 0;
}