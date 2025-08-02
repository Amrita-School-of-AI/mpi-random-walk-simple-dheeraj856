#!/bin/bash

# Exit immediately if any command fails
set -e

# --- Test Configuration ---
# Number of MPI processes to use for the test
NP=4
# Name of the executable file
EXEC="./random_walk"
# Command-line arguments for the executable
# Format: <domain_size> <max_walk_iterations>
ARGS="20 1000"

# --- Test Execution ---
# Run the MPI program and capture its standard output
echo "Running command: mpirun -np $NP $EXEC $ARGS"
OUTPUT=$(mpirun --oversubscribe -np $NP $EXEC $ARGS)

# --- Test Validation ---
# We expect each of the $NP walkers to print a "finished" message.
# Count how many times the word "finished" appears in the output.
COUNT=$(echo "$OUTPUT" | grep -c "finished")

# The expected number of finished messages is equal to the number of processes.
EXPECTED_COUNT=$NP

if [ "$COUNT" -eq "$EXPECTED_COUNT" ]; then
    echo "✅ Test Passed: Found $COUNT 'finished' messages, as expected."
    echo "--- Program Output ---"
    echo "$OUTPUT"
    exit 0 # Success
else
    echo "❌ Test Failed: Expected $EXPECTED_COUNT 'finished' messages, but found $COUNT."
    echo "--- Program Output ---"
    echo "$OUTPUT"
    echo "----------------------"
    exit 1 # Failure
fi