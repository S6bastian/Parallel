#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

struct Data {
    size_t startIdx, size;
    double* sum;
    int nThreads;
};

/* =========================================================
   1. BARRERA CON BUSY-WAITING + MUTEX
   ========================================================= */

pthread_mutex_t busyMutex;
int busyCounter = 0;

void busyBarrier(int nThreads) {
    pthread_mutex_lock(&busyMutex);
    busyCounter++;
    pthread_mutex_unlock(&busyMutex);

    while (true) {
        pthread_mutex_lock(&busyMutex);
        bool ready = (busyCounter == nThreads);
        pthread_mutex_unlock(&busyMutex);

        if (ready)
            break;
    }
}

/* =========================================================
   2. BARRERA CON SEMAFOROS
   ========================================================= */

sem_t countSem;
sem_t barrierSem;
int semCounter = 0;

void semaphoreBarrier(int nThreads) {
    sem_wait(&countSem);

    if (semCounter == nThreads - 1) {
        semCounter = 0;
        sem_post(&countSem);

        for (int i = 0; i < nThreads - 1; i++)
            sem_post(&barrierSem);
    } else {
        semCounter++;
        sem_post(&countSem);
        sem_wait(&barrierSem);
    }
}

/* =========================================================
   3. BARRERA CON VARIABLE DE CONDICION
   ========================================================= */

pthread_mutex_t condMutex;
pthread_cond_t condVar;
int condCounter = 0;

void conditionBarrier(int nThreads) {
    pthread_mutex_lock(&condMutex);

    condCounter++;

    if (condCounter == nThreads) {
        condCounter = 0;
        pthread_cond_broadcast(&condVar);
    } else {
        while (condCounter != 0)
            pthread_cond_wait(&condVar, &condMutex);
    }

    pthread_mutex_unlock(&condMutex);
}

/* =========================================================
   TRABAJO CON LEIBNIZ
   ========================================================= */

void* leibniz_busy(void* arg) {
    Data* data = (Data*)arg;

    size_t half = data->size / 2;
    double localSum = 0.0;

    // Primera fase
    for (size_t i = data->startIdx; i < data->startIdx + half; i++) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    busyBarrier(data->nThreads);

    // Segunda fase
    for (size_t i = data->startIdx + half;
         i < data->startIdx + data->size; i++) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    pthread_mutex_lock(&busyMutex);
    *(data->sum) += localSum;
    pthread_mutex_unlock(&busyMutex);

    return NULL;
}

void* leibniz_semaphore(void* arg) {
    Data* data = (Data*)arg;

    size_t half = data->size / 2;
    double localSum = 0.0;

    // Primera fase
    for (size_t i = data->startIdx; i < data->startIdx + half; i++) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    semaphoreBarrier(data->nThreads);

    // Segunda fase
    for (size_t i = data->startIdx + half;
         i < data->startIdx + data->size; i++) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    pthread_mutex_lock(&busyMutex);
    *(data->sum) += localSum;
    pthread_mutex_unlock(&busyMutex);

    return NULL;
}

void* leibniz_condition(void* arg) {
    Data* data = (Data*)arg;

    size_t half = data->size / 2;
    double localSum = 0.0;

    // Primera fase
    for (size_t i = data->startIdx; i < data->startIdx + half; i++) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    conditionBarrier(data->nThreads);

    // Segunda fase
    for (size_t i = data->startIdx + half;
         i < data->startIdx + data->size; i++) {
        localSum += (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
    }

    pthread_mutex_lock(&busyMutex);
    *(data->sum) += localSum;
    pthread_mutex_unlock(&busyMutex);

    return NULL;
}

/* =========================================================
   FUNCION GENERAL PARA EJECUTAR CADA BARRERA
   ========================================================= */

double ejecutarPi(size_t n, int nThreads, int tipo, double& duration_out) {

    vector<pthread_t> threadId(nThreads);
    vector<Data> data(nThreads);

    double sum = 0.0;
    size_t segmentSize = n / nThreads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < nThreads; i++) {

        data[i].startIdx = i * segmentSize;
        data[i].nThreads = nThreads;

        if (i == nThreads - 1)
            data[i].size = segmentSize + (n % nThreads);
        else
            data[i].size = segmentSize;

        data[i].sum = &sum;

        if (tipo == 1)
            pthread_create(&threadId[i], NULL, leibniz_busy, &data[i]);

        else if (tipo == 2)
            pthread_create(&threadId[i], NULL, leibniz_semaphore, &data[i]);

        else
            pthread_create(&threadId[i], NULL, leibniz_condition, &data[i]);
    }

    for (int i = 0; i < nThreads; i++)
        pthread_join(threadId[i], NULL);

    sum *= 4.0;

    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration = end - start;
    duration_out = duration.count();

    return sum;
}

/* =========================================================
   MAIN
   ========================================================= */

int main() {

    cout << fixed << setprecision(7);

    vector<size_t> nValues;

    size_t nTemp = 1;

    for (int i = 0; i < 8; i++) {
        nTemp *= 10;
        nValues.push_back(nTemp);
    }

    int maxThreads = thread::hardware_concurrency();

    if (maxThreads == 0)
        maxThreads = 4;

    cout << "Numero maximo de threads: " << maxThreads << "\n";

    /*
       BARRERA 1: BUSY-WAITING + MUTEX
    */
    for (int numThreads = 1; numThreads <= maxThreads; numThreads *= 2) {

        cout << "\n==============================\n";
        cout << "Threads: " << numThreads << "\n";
        cout << "==============================\n";

        for (size_t n : nValues) {

            busyCounter = 0;
            pthread_mutex_init(&busyMutex, NULL);

            double t = 0.0;
            double pi = ejecutarPi(n, numThreads, 1, t);

            pthread_mutex_destroy(&busyMutex);

            cout << "N: " << n
                 << " -> Tiempo Busy-Wait: " << t << " s"
                 << " -> Pi: " << pi << "\n";
        }
    }

    /*
       BARRERA 2: SEMAFOROS
    */
    for (int numThreads = 1; numThreads <= maxThreads; numThreads *= 2) {

        cout << "\n==============================\n";
        cout << "Threads: " << numThreads << "\n";
        cout << "==============================\n";

        for (size_t n : nValues) {

            semCounter = 0;

            sem_init(&countSem, 0, 1);
            sem_init(&barrierSem, 0, 0);
            pthread_mutex_init(&busyMutex, NULL);

            double t = 0.0;
            double pi = ejecutarPi(n, numThreads, 2, t);

            pthread_mutex_destroy(&busyMutex);
            sem_destroy(&countSem);
            sem_destroy(&barrierSem);

            cout << "N: " << n
                 << " -> Tiempo Semaforo: " << t << " s"
                 << " -> Pi: " << pi << "\n";
        }
    }

    /*
       BARRERA 3: VARIABLE DE CONDICION
    */
    for (int numThreads = 1; numThreads <= maxThreads; numThreads *= 2) {

        cout << "\n==============================\n";
        cout << "Threads: " << numThreads << "\n";
        cout << "==============================\n";

        for (size_t n : nValues) {

            condCounter = 0;

            pthread_mutex_init(&condMutex, NULL);
            pthread_cond_init(&condVar, NULL);
            pthread_mutex_init(&busyMutex, NULL);

            double t = 0.0;
            double pi = ejecutarPi(n, numThreads, 3, t);

            pthread_mutex_destroy(&busyMutex);
            pthread_mutex_destroy(&condMutex);
            pthread_cond_destroy(&condVar);

            cout << "N: " << n
                 << " -> Tiempo Variable Condicion: " << t << " s"
                 << " -> Pi: " << pi << "\n";
        }
    }

    return 0;
}
