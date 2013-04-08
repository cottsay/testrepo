/*!
 * \file proj2.3a.c
 *
 * \brief OpenMPI Butterfly Benchmark
 *
 * \details
 * This OpenMPI program generates a random number on all nodes and sums them
 * in parallel, distributing the answer to all of the nodes. This
 * implementation requires that the number of nodes is an even two-power.
 */

#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/*!
 * \brief Determines if number is even two-power
 *
 * \author Scott K Logan
 *
 * \param n Number in question
 *
 * \retval 1 Number is even two-power
 * \retval 0 Number is NOT an even two-power
 *
 * \details
 * This function simply loops through testing all bits in the data type. If
 * only one of the bits is set, the number is considered an even two-power.
 */
short unsigned int is_even_2pow( int n );

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
 * node. Note that this implementation must have an even two-power, ie 1, 2, 4,
 * 8, 16, 32, etc.
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
	int mynum;
	int theirnum;
	int their_rank;
	int globalsum;
	long int tmp;
	int levelshift;
	MPI_Comm comm;
	MPI_Status status;

	// Initialize MPI
	MPI_Init( &argc, &argv );
	comm = MPI_COMM_WORLD;
	MPI_Comm_size( comm, &comm_sz );
	MPI_Comm_rank( comm, &my_rank );

	if( my_rank == 0 )
	{
		if( !is_even_2pow( comm_sz ) )
		{
			fprintf( stderr, "Error: Wrong number of cores for benchmark (2^k == n)\n" );
			exit( EXIT_FAILURE );
		}
	}

	// Seed Rand
	srand48_r( time( NULL ) + my_rank * 100, &seed );

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
		printf( "MPI_Allreduce answered %d in an average of %fs\n", globalsum, elapsed / comm_sz );

	// Butterfly Test
	MPI_Barrier( comm );
	local_start = MPI_Wtime();
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
	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce( &local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_SUM, 0, comm );
	if( my_rank == 0 )
		printf( "Butterfly     answered %d in an average of %fs\n", mynum, elapsed / comm_sz );

	// Done
	MPI_Finalize( );
	exit( EXIT_SUCCESS );
}

short unsigned int is_even_2pow( int n )
{
	unsigned short int i;
	unsigned short int j = 0;

	for( i = 0; i < 8 * sizeof( int ); i++ )
	{
		if( n & (1 << i) )
			j++;
	}

	return (j == 1);
}
