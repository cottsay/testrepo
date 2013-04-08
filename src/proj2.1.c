/*!
 * \file proj2.1.c
 *
 * \brief OpenMPI Pi Calculator
 *
 * \details
 * This library uses OpenMPI to calculate &pi; using a Monte Carlo approach.
 * It will calculate to various levels of precision based on the problem_size
 * parameter passed as a command line argument, and will also run on the given
 * number of threads specified to mpirun.
 */

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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
 * This is the main function of the program. It runs all of the MPI routines
 * needed to calculate &pi;.
 */
int main( int argc, char *argv[] )
{
	int my_rank;
	int comm_sz;
	int used_sz;
	long long int num_tosses;
	long long int num_hits;
	long long int toss;
	double x;
	double y;
	struct drand48_data seed;
	double local_start;
	double local_finish;
	double local_elapsed;
	double elapsed;
	MPI_Comm comm;
	unsigned short int trial;
	const int trials[5] = { 1, 8, 16, 32, 64 };

	// Initialize MPI
	MPI_Init( &argc, &argv );
	comm = MPI_COMM_WORLD;
	MPI_Comm_size( comm, &comm_sz );
	MPI_Comm_rank( comm, &my_rank );

	// Figure out the num_tosses
	if( my_rank == 0 )
	{
		if( argc != 2 )
		{
			fprintf( stderr, "Error: Invalid number of arguments\n" );
			fprintf( stderr, "Usage: %s <number of tosses>\n", argv[0] );
			exit( EXIT_FAILURE );
		}
		// Make sure we have enough processors to do the benchmark
		if( comm_sz < 64 )
		{
			fprintf( stderr, "Error: Not enough cores for benchmark (requires >= 64)\n" );
			exit( EXIT_FAILURE );
		}
	}

	// Trial Loop
	for( trial = 0; trial < 5; trial++ )
	{
		num_hits = 0;
		used_sz = trials[trial];

		MPI_Bcast( &used_sz, 1, MPI_LONG_LONG_INT, 0, comm );

		if( my_rank == 0 )
			num_tosses = atoi( argv[1] ) / used_sz;

		MPI_Bcast( &num_tosses, 1, MPI_LONG_LONG_INT, 0, comm );

		// Let process 0 pick up the slack
		if( my_rank == 0 )
			num_tosses += atoi( argv[1] ) - num_tosses * used_sz;

		// Seed the random number generator
		srand48_r( time( NULL ) + my_rank * 100, &seed );

		// Go
		MPI_Barrier( comm );
		local_start = MPI_Wtime();
		if( my_rank < used_sz )
			for( toss = 0; toss < num_tosses; toss++ )
			{
				drand48_r( &seed, &x );
				drand48_r( &seed, &y );
				if( ( x * x ) + ( y * y ) <= 1.0 )
					num_hits++;
			}
		local_finish = MPI_Wtime();

		// Reduce answer
		local_elapsed = local_finish - local_start;
		MPI_Reduce( &local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm );
		MPI_Reduce( &num_hits, &toss, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, comm );

		// Output result
		if( my_rank == 0 )
		{
			static double single_runtime = 1.0;
			const double my_pi_error = M_PI - 4 * toss / (double)atoi( argv[1] );
			if( used_sz == 1 )
			{
				single_runtime = elapsed;
				printf( "  np |    Error |     Time |  Speedup | Efficiency\n" );
				printf( "   1 |%9.6f |%9.6f |      N/A |      N/A\n", my_pi_error, elapsed );
			}
			else
			{
				const double speedup = single_runtime / elapsed;
				printf( "%4d |%9.6f |%9.6f |%9.6f |%9.6f\n", used_sz, my_pi_error,
					elapsed, speedup, speedup / used_sz );
			}
		}
	}

	// Done
	MPI_Finalize( );
	exit( EXIT_SUCCESS );
}
