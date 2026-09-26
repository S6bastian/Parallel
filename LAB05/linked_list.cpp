#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <pthread.h>

using namespace std;

pthread_mutex_t pMutex;

struct Node {
    int val;
    Node* next;
    Node(int v) : val(v), next(nullptr) {}
};

class LinkedList {
public:
    Node* head;

    LinkedList() : head(nullptr) {}

    ~LinkedList() {
        Node* curr = head;
        while (curr) {
            Node* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

    void insert(int val, bool useSync) {
        if (useSync) pthread_mutex_lock(&pMutex);

        Node* newNode = new Node(val);
        newNode->next = head;
        head = newNode;

        if (useSync) pthread_mutex_unlock(&pMutex);
    }

    bool contains(int val, bool useSync) {
        if (useSync) pthread_mutex_lock(&pMutex);

        Node* curr = head;
        while (curr) {
            if (curr->val == val) {
                if (useSync) pthread_mutex_unlock(&pMutex);
                return true;
            }
            curr = curr->next;
        }

        if (useSync) pthread_mutex_unlock(&pMutex);
        return false;
    }

    bool remove(int val, bool useSync) {
        if (useSync) pthread_mutex_lock(&pMutex);

        Node* curr = head;
        Node* prev = nullptr;

        while (curr) {
            if (curr->val == val) {
                if (prev) prev->next = curr->next;
                else head = curr->next;
                delete curr;

                if (useSync) pthread_mutex_unlock(&pMutex);
                return true;
            }
            prev = curr;
            curr = curr->next;
        }

        if (useSync) pthread_mutex_unlock(&pMutex);
        return false;
    }

    size_t count(bool useSync) {
        if (useSync) pthread_mutex_lock(&pMutex);

        size_t c = 0;
        Node* curr = head;
        while (curr) {
            c++;
            curr = curr->next;
        }

        if (useSync) pthread_mutex_unlock(&pMutex);
        return c;
    }
};

struct Data {
    size_t opsPerThread;
    int threadId;
    LinkedList* list;
    bool useSync;
};

void* linked_listPar_worker(void* arg) {
    Data* data = (Data*) arg;
    LinkedList* list = data->list;
    int baseVal = data->threadId * data->opsPerThread;
    bool useSync = data->useSync;

    for (size_t i = 0; i < data->opsPerThread; i++) {
        list->insert(baseVal + i, useSync);
    }

    for (size_t i = 0; i < data->opsPerThread / 2; i++) {
        list->contains(baseVal + i, useSync);
    }

    for (size_t i = 0; i < data->opsPerThread / 2; i++) {
        list->remove(baseVal + i, useSync);
    }

    return NULL;
}

void linked_listSeq(size_t n, double& duration_out, size_t& finalCount) {
    LinkedList list;
    auto start = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < n; i++) {
        list.insert(i, false);
    }
    for (size_t i = 0; i < n / 2; i++) {
        list.contains(i, false);
    }
    for (size_t i = 0; i < n / 2; i++) {
        list.remove(i, false);
    }

    finalCount = list.count(false);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    duration_out = duration.count();
}

void linked_listPar(size_t n, int numThreads, bool useSync, double& duration_out, size_t& finalCount) {
    vector<pthread_t> threadId(numThreads);
    vector<Data> data(numThreads);
    LinkedList list;

    if (useSync) pthread_mutex_init(&pMutex, NULL);

    size_t opsPerThread = n / numThreads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++) {
        data[i].opsPerThread = (i == numThreads - 1) ? (opsPerThread + (n % numThreads)) : opsPerThread;
        data[i].threadId = i;
        data[i].list = &list;
        data[i].useSync = useSync;

        pthread_create(&threadId[i], NULL, linked_listPar_worker, &data[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threadId[i], NULL);
    }

    finalCount = list.count(useSync);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    duration_out = duration.count();

    if (useSync) pthread_mutex_destroy(&pMutex);
}

int main() {
    cout << fixed << setprecision(7);

    vector<size_t> nValues = {10000, 20000, 40000, 80000};
    vector<double> nSeqTime(nValues.size());

    for (size_t k = 0; k < nValues.size(); k++) {
        size_t n = nValues[k];
        size_t finalCount = 0;
        linked_listSeq(n, nSeqTime[k], finalCount);
    }

    int maxThreads = thread::hardware_concurrency();
    if (maxThreads == 0) maxThreads = 4;

    cout << "CON SINCRONIZACION (MUTEX)\n";
    for (int numThreads = 1; numThreads <= maxThreads; numThreads *= 2) {
        cout << "\nThreads: " << numThreads << "\n";

        for (size_t k = 0; k < nValues.size(); k++) {
            size_t n = nValues[k];
            double tSeq = nSeqTime[k];
            double tPar = 0.0;
            size_t finalCount = 0;

            linked_listPar(n, numThreads, true, tPar, finalCount);

            double speedup = tSeq / tPar;
            double efficiency = speedup / numThreads;

            cout << "N: " << n 
                 << " -> tSeq: " << tSeq << "s"
                 << " -> tPar: " << tPar << "s"
                 << " -> Speedup: " << speedup
                 << " -> Eficiencia: " << efficiency
                 << " -> Nodos: " << finalCount << "\n";
        }
    }

    cout << "\nSIN SINCRONIZACION (CONDICION DE CARRERA) ===\n";
    for (int numThreads = 2; numThreads <= maxThreads; numThreads *= 2) {
        cout << "\nThreads: " << numThreads << "\n";

        for (size_t k = 0; k < nValues.size(); k++) {
            size_t n = nValues[k];
            double tSeq = nSeqTime[k];
            double tPar = 0.0;
            size_t finalCount = 0;

            linked_listPar(n, numThreads, false, tPar, finalCount);

            double speedup = tSeq / tPar;
            double efficiency = speedup / numThreads;

            cout << "N: " << n 
                 << " -> tSeq: " << tSeq << "s"
                 << " -> tPar: " << tPar << "s"
                 << " -> Speedup: " << speedup
                 << " -> Eficiencia: " << efficiency
                 << " -> Nodos: " << finalCount << " (Inconsistente)\n";
        }
    }

    return 0;
}