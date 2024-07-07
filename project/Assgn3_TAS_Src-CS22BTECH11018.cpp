#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>
#include <algorithm> // Added for min function
#include <unistd.h>  // Added for gettimeofday

using namespace std;

int counter;

// Global atomic flag for synchronization
atomic_flag lock = ATOMIC_FLAG_INIT;

// Function to perform matrix multiplication for a chunk of rows
void calculate_chunk_TAS(int rowinc, int n, const vector<vector<int>> &matrix, vector<vector<int>> &output)
{
    while (true)
    {
        // Loop until successfully acquiring the lock
        while (lock.test_and_set())
        {
        } // Test and Set

        // Critical section
        if (counter >= n)
        {
            lock.clear();
            break;
        }
        int a = counter;
        counter = counter + rowinc;
        int b = min(counter, n);

        // Release the lock after the critical section
        lock.clear();
        // Iterate over the assigned chunk of rows

        for (int i = a; i < b; i++)
        {
            for (int l = 0; l < n; l++)
            {
                int s = 0;
                // Perform the dot product for the current row and column
                for (int j = 0; j < n; j++)
                {
                    s += (matrix[i][j] * matrix[j][l]);
                }
                // Store the result in the output matrix
                output[i][l] = s;
            }
        }
    }
}

int main()
{
    int n, k, rowinc;

    // Open the input file
    ifstream inputFile("inp.txt");

    // Check if the file is successfully opened
    if (!inputFile.is_open())
    {
        cerr << "Error opening the file" << endl;
        return 1; // return an error code
    }

    // Read matrix dimensions and number of threads
    inputFile >> n >> k >> rowinc;

    // Initialize output matrix
    vector<vector<int>> output(n, vector<int>(n));
    vector<vector<int>> matrix;

    int x;
    vector<int> row;

    // Read values from the input file and populate the matrix
    for (int i = 0; i < n; i++)
    {
        row.clear(); // Clear the row vector
        for (int j = 0; j < n; j++)
        {
            inputFile >> x;
            row.push_back(x);
        }
        matrix.push_back(row); // Add the row to the matrix vector
    }
    counter = 0;
    // Measure the start time
    struct timeval f, l;
    gettimeofday(&f, NULL);

    // Create vector to store threads
    vector<thread> threads;
    // Assign rows to threads and start the calculation
    for (int i = 0; i < k; ++i)
    {
        threads.push_back(thread(calculate_chunk_TAS, rowinc, n, ref(matrix), ref(output)));
    }

    // Wait for all threads to finish
    for (auto &thread : threads)
    {
        thread.join();
    }

    // Measure the end time
    gettimeofday(&l, NULL);
    // Calculate the total time taken
    double tt = (l.tv_sec - f.tv_sec) + ((l.tv_usec - f.tv_usec) / 1000000.0);

    // Open the output file
    ofstream outputFile("out1.txt");
    if (!outputFile.is_open())
    {
        cerr << "Error opening the output file" << endl;
        return 1;
    }

    // Write the total time taken to the output file
    outputFile << "time:" << tt << endl;

    // Output the resulting matrix to out1.txt
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            outputFile << output[i][j] << " ";
        }
        outputFile << endl;
    }

    return 0;
}
