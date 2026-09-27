#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <pthread.h>

using namespace std;

pthread_mutex_t pMutex;

struct Data {
    size_t startIdx, size;
    double* sum;
};

void* leibniz_serie_par(void* arg) {
    Data* data = (Data*) arg;
    double sum = 0.0;

    for (size_t i = data->startIdx; i < data->startIdx + data->size; i++) {
        sum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    pthread_mutex_lock(&pMutex);
    *(data->sum) += sum;
    pthread_mutex_unlock(&pMutex);

    return NULL;
}

double leibniz_seq(size_t n, double& duration_out) {
    auto start = chrono::high_resolution_clock::now();

    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        sum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }
    sum *= 4.0;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    duration_out = duration.count();

    return sum;
}

double leibniz_par(size_t size, int nThreads, double& duration_out) {
    vector<pthread_t> threadId(nThreads);
    vector<Data> data(nThreads);

    pthread_mutex_init(&pMutex, NULL);

    double sum = 0.0;
    size_t segmentSize = size / nThreads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < nThreads; i++) {
        data[i].startIdx = i * segmentSize;
        data[i].sum = &sum;

        if (i == nThreads - 1) {
            data[i].size = segmentSize + (size % nThreads);
        } else {
            data[i].size = segmentSize;
        }

        pthread_create(&threadId[i], NULL, leibniz_serie_par, &data[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threadId[i], NULL);
    }

    sum *= 4.0;

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    duration_out = duration.count();

    pthread_mutex_destroy(&pMutex);

    return sum;
}

int main() {
    cout << fixed << setprecision(7);

    vector<size_t> nValues;
    size_t nTemp = 1;
    for (int i = 0; i < 10; i++) {
        nTemp *= 10;
        nValues.push_back(nTemp);
    }

    vector<double> nSeqTime(nValues.size());
    for (size_t i = 0; i < nValues.size(); i++) {
        leibniz_seq(nValues[i], nSeqTime[i]);
    }

    int maxThreads = thread::hardware_concurrency();
    
    for (int numThreads = 1; numThreads <= maxThreads; numThreads *= 2) {
        
        cout << "\nThreads: " << numThreads << "\n";

        for (size_t i = 0; i < nValues.size(); i++) {
            size_t n = nValues[i];
            double tSeq = nSeqTime[i];
            double tPar = 0.0;

            leibniz_par(n, numThreads, tPar);

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