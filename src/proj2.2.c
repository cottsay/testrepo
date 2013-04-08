/*!
 * \file proj2.2.c
 *
 * \brief OpenMPI Latency Calculator
 *
 * \details
 * This OpenMPI program sends data between two nodes in an effort to calculate
 * the latency and bandwidth for such operations. Please note that this program
 * will ONLY run on two nodes, so -np MUST be 2.
 */

#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/// \brief Number of latency trials to perform
#define LAT_TRIALS 100
/// \brief Total number of bandwidth trials to perform
#define BW_TRIALS 6

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
 * This is the main function of the program. It runs all of the necessary
 * routines for calculating the latency and bandwidth of the system.
 */
int main( int argc, char *argv[] )
{
	int my_rank;
	int comm_sz;
	MPI_Comm comm;
	MPI_Status status;
	unsigned int trial;
	double local_start;
	double local_finish;
	double lat_trials = 0;
	double bw_trials = 0;
	const unsigned int bw_trial_sizes[BW_TRIALS] = { (1 << 15), (1 << 17), (1 << 19), (1 << 21), (1 << 24), (1 << 27) };
	char smalldata;
	char *largedata = NULL;

	// Initialize MPI
	MPI_Init( &argc, &argv );
	comm = MPI_COMM_WORLD;
	MPI_Comm_size( comm, &comm_sz );
	MPI_Comm_rank( comm, &my_rank );

	if( my_rank == 0 )
	{
		// Make sure we have enough processors to do the benchmark
		if( comm_sz != 2 )
		{
			fprintf( stderr, "Error: Wrong number of cores for benchmark (requires == 2)\n" );
			exit( EXIT_FAILURE );
		}

		// Seed Rand
		srand( time( NULL ) + my_rank * 100 );
	}

	// Latency Trials
	if( my_rank == 0 )
		printf( "Running %d latency tests...\n", LAT_TRIALS );
	for( trial = 0; trial < LAT_TRIALS; trial++ )
	{
		MPI_Barrier( comm );
		if( my_rank == 0 )
		{
			local_start = MPI_Wtime();
			MPI_Send( &smalldata, 0, MPI_CHAR, 1, 8, comm );
			MPI_Recv( &smalldata, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status );
			local_finish = MPI_Wtime();
			lat_trials += ( local_finish - local_start ) / 2.0;
		}
		else
		{
			MPI_Recv( &smalldata, 0, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status );
			MPI_Send( &smalldata, 0, MPI_CHAR, 0, 8, comm );
		}
	}
	if( my_rank == 0 )
	{
		lat_trials /= LAT_TRIALS;
		printf( "Average latency: %fs\n", lat_trials );
	}

	// Bandwidth Trials
	for( trial = 0; trial < BW_TRIALS; trial++ )
	{
		if( my_rank == 0 )
			printf( "Running bandwidth test (size %d)...", bw_trial_sizes[trial] );
		largedata = realloc( largedata, bw_trial_sizes[trial] );
		memset( largedata, rand( ) % 255, bw_trial_sizes[trial] );
		MPI_Barrier( comm );
		if( my_rank == 0 )
		{
			local_start = MPI_Wtime();
			MPI_Send( largedata, bw_trial_sizes[trial], MPI_CHAR, 1, 8, comm );
			MPI_Recv( largedata, bw_trial_sizes[trial], MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status );
			local_finish = MPI_Wtime();
			printf( "%fs, ", ( local_finish - local_start ) / 2.0 );
			bw_trials += bw_trial_sizes[trial] / ( ( local_finish - local_start ) / 2.0 - lat_trials );
			printf( "%f B/s.\n", bw_trial_sizes[trial] / ( ( local_finish - local_start ) / 2.0 - lat_trials ) );
		}
		else
		{
			MPI_Recv( largedata, bw_trial_sizes[trial], MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status );
			MPI_Send( largedata, bw_trial_sizes[trial], MPI_CHAR, 0, 8, comm );
		}
	}
	if( my_rank == 0 )
	{
		bw_trials /= BW_TRIALS;
		printf( "Average bandwidth: %f KB/s\n", bw_trials / 1024 );
	}

	// Done
	MPI_Finalize( );
	free( largedata );
	exit( EXIT_SUCCESS );
}
