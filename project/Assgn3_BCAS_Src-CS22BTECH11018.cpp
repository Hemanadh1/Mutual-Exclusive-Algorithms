#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <atomic>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <sys/time.h>
using namespace std;
int counter;

// Global atomic flag for synchronization
int lock = 0;

// Function to perform matrix multiplication for a chunk of rows
void calculate_chunk_Bounded_CAS(int rowinc, int n, int k, const vector<vector<int>> &matrix, vector<vector<int>> &output, vector<bool> &waiting, int index)
{
    while (true)
    {
        waiting[index] = true;
        int key = 1;
        while (waiting[index] && key)
        {
            key = __sync_val_compare_and_swap(&lock, 0, 1);
        }
        waiting[index] = false;
        if (counter >= n)
        {
            lock = 0;
            break;
        }
        int a = counter;
        counter = counter + rowinc;
        lock = 0;

        int j = (index + 1) % k;
        while ((j != index) && !waiting[j])
        {
            j = (j + 1) % k;
        }
        if (j == index)
            lock = 0;
        else
            waiting[j] = false;

        int b = min(counter, n);

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
    vector<bool> waiting(true);

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
        threads.push_back(thread(calculate_chunk_Bounded_CAS, rowinc, n, k, ref(matrix), ref(output), ref(waiting), i));
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
    ofstream outputFile("out3.txt");
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
