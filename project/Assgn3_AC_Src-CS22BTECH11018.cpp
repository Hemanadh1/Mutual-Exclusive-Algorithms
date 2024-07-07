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

// Global atomic counter for synchronization
atomic<int> counter;

// Function to perform matrix multiplication for a chunk of rows
void calculate_chunk_Atomic(int rowinc, int n, const vector<vector<int>> &matrix, vector<vector<int>> &output)
{
    int local_counter;
    while (true)
    {
        // Atomically fetch and increment counter
        local_counter = counter.fetch_add(rowinc);
        if (local_counter >= n)
            break;

        int a = local_counter;
        int b = min(local_counter + rowinc, n);

        for (int i = a; i < b; i++)
        {
            for (int l = 0; l < n; l++)
            {
                int s = 0;
                for (int j = 0; j < n; j++)
                {
                    s += (matrix[i][j] * matrix[j][l]);
                }
                output[i][l] = s;
            }
        }
    }
}

int main()
{
    int n, k, rowinc;

    ifstream inputFile("inp.txt");

    if (!inputFile.is_open())
    {
        cerr << "Error opening the file" << endl;
        return 1;
    }

    inputFile >> n >> k >> rowinc;

    vector<vector<int>> output(n, vector<int>(n));
    vector<vector<int>> matrix;

    int x;
    vector<int> row;

    for (int i = 0; i < n; i++)
    {
        row.clear();
        for (int j = 0; j < n; j++)
        {
            inputFile >> x;
            row.push_back(x);
        }
        matrix.push_back(row);
    }

    // Initialize counter to 0
    counter.store(0);

    struct timeval f, l;
    gettimeofday(&f, NULL);

    vector<thread> threads;
    for (int i = 0; i < k; ++i)
    {
        threads.push_back(thread(calculate_chunk_Atomic, rowinc, n, ref(matrix), ref(output)));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    gettimeofday(&l, NULL);
    double tt = (l.tv_sec - f.tv_sec) + ((l.tv_usec - f.tv_usec) / 1000000.0);

    ofstream outputFile("out4.txt");
    if (!outputFile.is_open())
    {
        cerr << "Error opening the output file" << endl;
        return 1;
    }

    outputFile << "time:" << tt << endl;

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
