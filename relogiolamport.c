
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int rank, i, size, root = 0, rc;
	int clock = 0, clocks[100], signal[3], num_processes, send_target, ack, line_length, ln;
	FILE *inputFile;
	char line[500], message[500], number[4];
	
	MPI_Status status;
	
	rc = MPI_Init(&argc, &argv);
	if (rc!=0) {
		printf("Erro ao iniciar MPI\n"); 
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	num_processes = size-1;
	
	if(rank == root) //processo gerente
	{
		inputFile = fopen(argv[1], "r");
		if(!inputFile)
		{
			printf("Erro ao abrir arquivo de comandos\n");
			MPI_Finalize();
			return 0;
		}

		while( fgets(line, 500, inputFile) != NULL ) // ler cada linha
		{
			line_length = strlen(line)-2;
			if( strncmp(line, "end", 3) == 0 ) // comando para finalizar
			{
				signal[0] = 3; // end signal
				for(i=1; i<=num_processes; i++)
					MPI_Send(signal, 3, MPI_INT, i, 0, MPI_COMM_WORLD); // pede aos outros processos seu valor de clock
				
				for(i=1; i<=num_processes; i++)
				{
					MPI_Recv(&clock, 2, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status); // recebe os valores requisitado
					clocks[status.MPI_SOURCE] = clock;
					printf("Valor do relógio logico do processo %d é %d\n", i, clocks[i]);
				}

				fclose(inputFile);
				MPI_Finalize(); // fecha arquivo e encerra MPI
				return 0;
			}
			else if( strncmp(line, "exec", 4) == 0 ) // comando para execução (print)
			{
				signal[0] = 1; // exec signal

				for(i=5; i<=line_length; i++){
					number[i-5] = line[i];
				}
				number[i-5] = '\0';
				send_target = atoi(number); // tratamento de string para identificar processo

				MPI_Send(signal, 3, MPI_INT, send_target, 0, MPI_COMM_WORLD); // enviar mensagem para processo executar
			}
			else if( strncmp(line, "send", 4) == 0 ) // comando send
			{
				signal[0] = 2; // send signal
				for(i=0; line[i+5] != ' '; i++)
					number[i] = line[i+5];
				number[i] = '\0';
				ln = i+6;
				send_target = atoi(number); // processo a ser executado

				for(i=0; line[i+ln] != ' '; i++)
					number[i] = line[i+ln];
				number[i] = '\0';
				ln = i+ln+2;
				signal[1] = atoi(number); // processo destino

				for(i=0; line[i+ln] != '"'; i++)
					message[i] = line[i+ln]; 
				message[i] = '\0'; // ler mensagem a ser enviada
				signal[2] = i; // length of the message

				MPI_Send(signal, 3, MPI_INT, send_target, 0, MPI_COMM_WORLD); // enviar sinal de mensagem junto ao tamanho da mensagem
				MPI_Send(message, i, MPI_CHAR, send_target, 2, MPI_COMM_WORLD); // enviar mensagem
				MPI_Recv(&ack, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status); // recebe confirmação que a mensagem foi recebida
			}
		}
	}
	else // processos normais
	{
		while(1)
		{
			MPI_Recv(signal, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status); // recebe instrução do processos gerente
			
			if(signal[0] == 3) // signal == 3 -> comando end, envia valor de clock para gerente
				MPI_Send(&clock, 1, MPI_INT, root, 1, MPI_COMM_WORLD); // envia valor de clock
			
			if(signal[0] == 0 || signal[0] == 3) // se é singal end, finaliza MPI
			{
				MPI_Finalize();
				return 0;
			}
			clock++; // qualquer comando aumenta valor de clock
			if(signal[0] == 1) // execute signal
				printf("Execução do processo %d\n", rank);
			
			else if(signal[0] == 2 || signal[0] == 4) // sinal de envio ou recepção
			{
				MPI_Recv(message, signal[2], MPI_CHAR, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status); // recebe mensagem do processo gerente
				message[signal[2]] = '\0';
				if(signal[0] == 4) // se sinal é receber
				{
					if(clock <= signal[1]) // atualiza valor de clock caso o valor de clock enviado seja maior do que o valor atual do processo
						clock = signal[1] + 1;
					printf("Mensagem recebida pelo processo %d do processo %d: %s\n", status.MPI_SOURCE, rank, message);
					MPI_Send(&ack, 1, MPI_INT, root, 3, MPI_COMM_WORLD);
				}
				else // sinal de envio
				{
					signal[0] = 4; // sinal de recepção
					send_target = signal[1]; // valor do processo destino
					signal[1] = clock; // valor de clock atual para ser enviado
					printf("Mensagem enviada pelo processo %d para o processo %d: %s\n", rank, send_target, message);
					MPI_Send(signal, 3, MPI_INT, send_target, 0, MPI_COMM_WORLD); // envia mensagem de envio para receptor
					MPI_Send(message, signal[2], MPI_CHAR, send_target, 2, MPI_COMM_WORLD); // envia mensagem
				}
			}
			printf("Tempo lógico no processo %d é %d\n", rank, clock); // depois da operação, print do clock atual do processo
		}
	}
	
	MPI_Finalize(); //finaliza MPI
	return 0;
}
