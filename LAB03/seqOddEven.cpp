#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>

using namespace std;

// Algoritmo secuencial Odd-Even Transposition Sort O(N^2)
void odd_even_sort_seq(vector<int>& arr) {
    int n = arr.size();
    bool isSorted = false;

    while (!isSorted) {
        isSorted = true;

        // Fase Impar
        for (int i = 1; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }

        // Fase Par
        for (int i = 0; i <= n - 2; i += 2) {
            if (arr[i] > arr[i + 1]) {
                swap(arr[i], arr[i + 1]);
                isSorted = false;
            }
        }
    }
}

int main() {
    // Tamaños adecuados para medir el comportamiento O(N^2) secuencial
    vector<int> sizes = {5000, 10000, 20000, 30000, 40000};

    cout << "==========================================" << endl;
    cout << "  Ordenamiento Odd-Even Secuencial (O(N^2))" << endl;
    cout << "==========================================" << endl;
    cout << left << setw(12) << "N (Tamano)" << " | " << "Tiempo T1 (s)" << endl;
    cout << "------------------------------------------" << endl;

    for (int N : sizes) {
        vector<int> arr(N);
        mt19937 rng(42);
        uniform_int_distribution<int> dist(1, 1000000);
        for (int i = 0; i < N; i++) arr[i] = dist(rng);

        auto start = chrono::high_resolution_clock::now();
        odd_even_sort_seq(arr);
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> duration = end - start;
        double T1 = duration.count();

        cout << left << setw(12) << N << " | " << fixed << setprecision(6) << T1 << " s" << endl;
    }

    cout << "==========================================" << endl << endl;
    return 0;
}