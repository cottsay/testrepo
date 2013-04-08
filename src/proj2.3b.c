/*!
 * \file proj2.3b.c
 *
 * \brief OpenMPI Butterfly Benchmark (any np)
 *
 * \details
 * This OpenMPI program generates a random number on all nodes and sums them
 * in parallel, distributing the answer to all of the nodes. This
 * implementation does NOT require that the number of nodes is an even
 * two-power.
 */

#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/// \brief Number of trials to average for benchmark results
#define NUM_TRIALS 50

/*!
 * \brief Determines nearest two-power less than n
 *
 * \author Scott K Logan
 *
 * \param n Number in question
 *
 * \returns Nearest two-power less than n
 *
 * \details
 * This function simply loops through testing all bits in the data type. It
 * then returns a number with the single highest bit from the original
 * number set and no others.
 */
int get_even_2pow( int n );

/*!
 * \brief Main Function
 *
 * \author Scott K Logan
 *
 * \param argc Command Line Argument Count
 * \param argv Command Line Arguments
 *
 * \returns EXIT_SUCCESS
 *
 * \details
 * This is the main function of the program. It sums random numbers from each
 * node.
 */
int main( int argc, char *argv[] )
{
	int my_rank;
	int comm_sz;
	struct drand48_data seed;
	double local_start;
	double local_finish;
	double local_elapsed;
	double elapsed;
	double allreduce_avg;
	double butterfly_avg;
	int mynum;
	int theirnum;
	int their_rank;
	int globalsum;
	long int tmp;
	int levelshift;
	MPI_Comm comm;
	MPI_Status status;
	int spill;
	unsigned int trial;

	// Initialize MPI
	MPI_Init( &argc, &argv );
	comm = MPI_COMM_WORLD;
	MPI_Comm_size( comm, &comm_sz );
	MPI_Comm_rank( comm, &my_rank );

	// Seed Rand
	srand48_r( time( NULL ) + my_rank * 100, &seed );

	if( my_rank == 0 )
		printf( " NP  | MPI_Allreduce | Butterfly     | Ratio\n" );

	// Configure Run
	spill = comm_sz - get_even_2pow( comm_sz );
	comm_sz -= spill;
	allreduce_avg = 0;
	butterfly_avg = 0;

	for( trial = 0; trial < NUM_TRIALS; trial++ )
	{
		// Generate our special number
		lrand48_r( &seed, &tmp );
		mynum = tmp;

		// MPI_Allreduce Test
		MPI_Barrier( comm );
		local_start = MPI_Wtime();
		MPI_Allreduce( &mynum, &globalsum, 1, MPI_INT, MPI_SUM, comm );
		local_finish = MPI_Wtime();
		local_elapsed = local_finish - local_start;
		MPI_Reduce( &local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_SUM, 0, comm );
		if( my_rank == 0 )
			allreduce_avg += elapsed / ( comm_sz + spill );

		// Butterfly Test
		MPI_Barrier( comm );
		local_start = MPI_Wtime();
		if( my_rank < comm_sz )
		{
			if( my_rank < spill )
			{
				MPI_Recv( &theirnum, 1, MPI_INT, my_rank + comm_sz, 0, comm, &status );
				mynum += theirnum;
			}
			for( levelshift = 1; levelshift < comm_sz; levelshift <<= 1 )
			{
				their_rank = my_rank ^ levelshift;
				if( my_rank & levelshift )
				{
					MPI_Send( &mynum, 1, MPI_INT, their_rank, levelshift, comm );
					MPI_Recv( &theirnum, 1, MPI_INT, their_rank, levelshift, comm, &status );
				}
				else
				{
					MPI_Recv( &theirnum, 1, MPI_INT, their_rank, levelshift, comm, &status );
					MPI_Send( &mynum, 1, MPI_INT, their_rank, levelshift, comm );
				}
				mynum += theirnum;
			}
			if( my_rank < spill )
				MPI_Send( &mynum, 1, MPI_INT, my_rank + comm_sz, comm_sz, comm );
		}
		else
		{
			MPI_Send( &mynum, 1, MPI_INT, my_rank - comm_sz, 0, comm );
			MPI_Recv( &mynum, 1, MPI_INT, my_rank - comm_sz, comm_sz, comm, &status );
		}
		local_finish = MPI_Wtime();
		local_elapsed = local_finish - local_start;
		MPI_Reduce( &local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_SUM, 0, comm );
		if( my_rank == 0 )
			butterfly_avg += elapsed / ( comm_sz + spill );
	}

	if( my_rank == 0 )
		printf( " %03d |%14.11f |%14.11f |%14.11f\n", comm_sz + spill, allreduce_avg / NUM_TRIALS, butterfly_avg / NUM_TRIALS, allreduce_avg / butterfly_avg );

	// Done
	MPI_Finalize( );
	exit( EXIT_SUCCESS );
}

int get_even_2pow( int n )
{
	unsigned short int i;
	unsigned short int j = 0;

	for( i = 0; i < 8 * sizeof( int ); i++ )
	{
		if( n & (1 << i) )
			j = i;
	}

	return (1 << j);
}
