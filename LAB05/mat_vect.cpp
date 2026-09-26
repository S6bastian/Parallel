#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <pthread.h>

using namespace std;

struct Data {
    size_t startRow, numRows;
    size_t n;
    const vector<vector<int>>* mat;
    const vector<int>* vect;
    vector<int>* result;
};

void* mat_vectPar_worker(void* arg) {
    Data* data = (Data*) arg;
    size_t endRow = data->startRow + data->numRows;
    size_t n = data->n;

    for (size_t i = data->startRow; i < endRow; i++) {
        int temp = 0;
        for (size_t j = 0; j < n; j++) {
            temp += (*data->mat)[i][j] * (*data->vect)[j];
        }
        (*data->result)[i] = temp;
    }

    return NULL;
}

void mat_vectSeq(size_t n, const vector<vector<int>>& mat, const vector<int>& vect, vector<int>& result, double& duration_out) {
    auto start = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < n; i++) {
        int temp = 0;
        for (size_t j = 0; j < n; j++) {
            temp += mat[i][j] * vect[j];
        }
        result[i] = temp;
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    duration_out = duration.count();
}

void mat_vectPar(size_t n, int nThreads, const vector<vector<int>>& mat, const vector<int>& vect, vector<int>& result, double& duration_out) {
    vector<pthread_t> threadId(nThreads);
    vector<Data> data(nThreads);

    size_t segmentSize = n / nThreads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < nThreads; i++) {
        data[i].startRow = i * segmentSize;
        data[i].numRows = (i == nThreads - 1) ? (segmentSize + (n % nThreads)) : segmentSize;
        data[i].n = n;
        data[i].mat = &mat;
        data[i].vect = &vect;
        data[i].result = &result;

        pthread_create(&threadId[i], NULL, mat_vectPar_worker, &data[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threadId[i], NULL);
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    duration_out = duration.count();
}

int main() {
    cout << fixed << setprecision(7);

    vector<size_t> nValues = {1000, 2000, 4000, 8000, 16000, 32000};
    vector<double> nSeqTime(nValues.size());

    for (size_t k = 0; k < nValues.size(); k++) {
        size_t n = nValues[k];
        vector<vector<int>> mat(n, vector<int>(n, 3));
        vector<int> vect(n, 5);
        vector<int> result(n, 0);

        mat_vectSeq(n, mat, vect, result, nSeqTime[k]);
    }

    int maxThreads = thread::hardware_concurrency();
    
    for (int numThreads = 1; numThreads <= maxThreads; numThreads *= 2) {
        cout << "\nThreads: " << numThreads << "\n";

        for (size_t k = 0; k < nValues.size(); k++) {
            size_t n = nValues[k];
            double tSeq = nSeqTime[k];
            double tPar = 0.0;

            vector<vector<int>> mat(n, vector<int>(n, 3));
            vector<int> vect(n, 5);
            vector<int> result(n, 0);

            mat_vectPar(n, numThreads, mat, vect, result, tPar);

            double speedup = tSeq / tPar;
            double efficiency = speedup / numThreads;

            cout << "N: " << n 
                 << " -> tSeq: " << tSeq << "s"
                 << " -> tPar: " << tPar << "s"
                 << " -> Speedup: " << speedup
                 << " -> Eficiencia: " << efficiency << "\n";
        }
    }

    return 0;
}