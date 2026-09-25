#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

pthread_mutex_t pMutex;

struct Data{
    size_t startIdx, size;
    double* sum;
};


void* leibniz_serie_par(void* arg){
    Data* data = (Data*) arg;

    double sum = 0.0;

    for(size_t i = data->startIdx; i < data->startIdx + data->size; i++){
        sum += (i % 2 == 0 ? 1.0 : -1.0)/(2.0*i+1.0);
    }

    pthread_mutex_lock(&pMutex);
    *(data->sum) += sum;
    pthread_mutex_unlock(&pMutex);

    return NULL;
}

double leibniz_serie(size_t size, int nThreads = thread::hardware_concurrency()){
    pthread_t threadId[nThreads];
    pthread_mutex_init(&pMutex, NULL);

    double sum = 0.0;
    Data data[nThreads];
    size_t segmentSize = size/nThreads;

    for(int i = 0; i < nThreads; i++){

        data[i].startIdx = i * segmentSize;
        data[i].size = segmentSize;
        data[i].sum = &sum;

        if(i == nThreads-1){
            data[i].size += (size % nThreads);
        }

        pthread_create(&threadId[i], NULL, leibniz_serie_par, &data[i]);
    }
    
    for (int i = 0; i < nThreads; i++) {
        pthread_join(threadId[i], NULL);
    }

    pthread_mutex_destroy(&pMutex);

    sum *= 4.0;

    return sum;
}


int main(){
    size_t n = 1e6;

    double sum = leibniz_serie(n);

    cout << fixed << setprecision(10) << sum << "\n";

    return 0;
}