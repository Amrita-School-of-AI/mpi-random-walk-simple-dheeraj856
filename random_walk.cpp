#include <iostream>
#include <cstdlib> // For atoi, rand, srand
#include <ctime>   // For time
#include <mpi.h>

void walker_process();
void controller_process();

int domain_size;
int max_steps;
int world_rank;
int world_size;

int main(int argc, char **argv)
{
    // Initialize the MPI environment
    MPI_Init(&argc, &argv);

    // Get the number of processes and the rank of this process
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);



    if (world_rank == 0) {
        if (argc == 3) {
            domain_size = atoi(argv[1]);
            max_steps = atoi(argv[2]);
        }
        // Always try to read from stdin, even if arguments were provided
        int d, m;
        if (std::cin >> d >> m) {
            domain_size = d;
            max_steps = m;
        } else if (argc != 3) {
            // No stdin and no arguments
            std::cerr << "Usage: mpirun -np <p> " << argv[0] << " <domain_size> <max_steps>" << std::endl;
            MPI_Finalize();
            return 1;
        }
    }
    // Broadcast values to all processes
    MPI_Bcast(&domain_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_steps, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        // Rank 0 is the controller
        controller_process();
    }
    else
    {
        // All other ranks are walkers
        walker_process();
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}

void walker_process()
{
    // Seed the random number generator.
    // Using rank ensures each walker gets a different sequence of random numbers.
    srand(time(NULL) + world_rank);

    // Initialize the walker's position to 0
    int position = 0;
    int steps = 0;
    
    // Loop for a maximum of max_steps
    for (steps = 0; steps < max_steps; steps++)
    {
        // Randomly move left (-1) or right (+1)
        int direction = (rand() % 2 == 0) ? -1 : 1;
        position += direction;
        
        // Check if the walker has moved outside the domain [-domain_size, +domain_size]
        if (position <= -domain_size || position >= domain_size)
        {
            break; // Walk is finished - out of bounds
        }
    }
    
    
    // Send an integer message to the controller (rank 0) to signal completion
    int completion_signal = steps + 1; // Send the number of steps taken
    MPI_Send(&completion_signal, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

void controller_process()
{
    int num_walkers = world_size - 1;
    for (int i = 0; i < num_walkers; i++)
    {
        int received_steps;
        MPI_Status status;
        MPI_Recv(&received_steps, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int walker_rank = status.MPI_SOURCE;
        std::cout << "Walker " << walker_rank << " finished in " << received_steps << " steps." << std::endl;
    }
    std::cout << "All walkers have completed their walks." << std::endl;
}