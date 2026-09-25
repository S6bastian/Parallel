#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;

// Algoritmo secuencial de multiplicación Matriz-Vector (A * x = y)
void matrix_vector_mult_seq(const vector<double>& A, const vector<double>& x, vector<double>& y, int N) {
    for (int i = 0; i < N; i++) {
        y[i] = 0.0;
        for (int j = 0; j < N; j++) {
            y[i] += A[i * N + j] * x[j];
        }
    }
}

int main() {
    vector<int> sizes = {1024, 2048, 4096, 8192, 16384};

    cout << "==========================================" << endl;
    cout << " Multiplicacion Matriz-Vector Secuencial  " << endl;
    cout << "==========================================" << endl;
    cout << left << setw(12) << "N (Tamano)" << " | " << "Tiempo T1 (s)" << endl;
    cout << "------------------------------------------" << endl;

    for (int N : sizes) {
        vector<double> A(N * N, 2.0);
        vector<double> x(N, 3.0);
        vector<double> y(N, 0.0);

        auto start = chrono::high_resolution_clock::now();
        matrix_vector_mult_seq(A, x, y, N);
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> duration = end - start;
        double T1 = duration.count();

        cout << left << setw(12) << N << " | " << fixed << setprecision(6) << T1 << " s" << endl;
    }

    cout << "==========================================" << endl << endl;
    return 0;
}