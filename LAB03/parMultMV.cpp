#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <mpi.h>

using namespace std;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<int> sizes = {1024, 2048, 4096, 8192, 16384};

    if (rank == 0) {
        cout << "========================================================================" << endl;
        cout << "   Multiplicacion Matriz-Vector MPI (Procesos p = " << size << ")" << endl;
        cout << "========================================================================" << endl;
        cout << left << setw(12) << "N (Tamano)" 
             << " | " << setw(16) << "Tiempo Tp (s)" 
             << " | " << setw(14) << "Speedup (Sp)" 
             << " | " << setw(16) << "Eficiencia (Ep)" << endl;
        cout << "------------------------------------------------------------------------" << endl;
    }

    for (int N : sizes) {
        double T1 = 0.0;

        // El proceso 0 mide T1 (tiempo base con 1 hilo) para poder calcular Speedup y Eficiencia
        if (rank == 0) {
            vector<double> A_base(N * N, 2.0);
            vector<double> x_base(N, 3.0);
            vector<double> y_base(N, 0.0);

            auto start1 = chrono::high_resolution_clock::now();
            for (int i = 0; i < N; i++) {
                double sum = 0.0;
                for (int j = 0; j < N; j++) {
                    sum += A_base[i * N + j] * x_base[j];
                }
                y_base[i] = sum;
            }
            auto end1 = chrono::high_resolution_clock::now();
            chrono::duration<double> dur1 = end1 - start1;
            T1 = dur1.count();
        }

        // Inicializar datos para la ejecución paralela
        vector<double> A, x(N, 3.0);
        if (rank == 0) {
            A.resize(N * N, 2.0);
        }

        // Distribución dinámica de filas entre los 'p' procesos
        int base_rows = N / size;
        int rem = N % size;
        int local_rows = base_rows + (rank < rem ? 1 : 0);

        vector<int> sendcounts(size), displs(size);
        vector<int> recvcounts_y(size), displs_y(size);

        int offset = 0;
        for (int r = 0; r < size; r++) {
            int r_rows = base_rows + (r < rem ? 1 : 0);
            sendcounts[r] = r_rows * N;
            displs[r] = offset * N;
            recvcounts_y[r] = r_rows;
            displs_y[r] = offset;
            offset += r_rows;
        }

        vector<double> local_A(local_rows * N);
        vector<double> local_y(local_rows, 0.0);
        vector<double> y_mpi;
        if (rank == 0) y_mpi.resize(N);

        // --- Inicio de Ejecución Paralela MPI ---
        MPI_Barrier(MPI_COMM_WORLD);
        double t_start = MPI_Wtime();

        // 1. Difundir vector x a todos los procesos
        MPI_Bcast(x.data(), N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // 2. Distribuir bloques de filas de la matriz A
        MPI_Scatterv(A.data(), sendcounts.data(), displs.data(), MPI_DOUBLE,
                     local_A.data(), local_rows * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // 3. Multiplicación local
        for (int i = 0; i < local_rows; i++) {
            local_y[i] = 0.0;
            for (int j = 0; j < N; j++) {
                local_y[i] += local_A[i * N + j] * x[j];
            }
        }

        // 4. Reunir resultados parciales
        MPI_Gatherv(local_y.data(), local_rows, MPI_DOUBLE,
                    y_mpi.data(), recvcounts_y.data(), displs_y.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

        double t_end = MPI_Wtime();
        double local_time = t_end - t_start;

        // Obtener tiempo máximo (Tp) entre todos los procesos
        double Tp = 0.0;
        MPI_Reduce(&local_time, &Tp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        // Cálculo de métricas e impresión en consola
        if (rank == 0) {
            double speedup = T1 / Tp;
            double efficiency = speedup / size;

            cout << left << setw(12) << N 
                 << " | " << setw(13) << fixed << setprecision(6) << Tp << " s"
                 << " | " << setw(14) << setprecision(4) << speedup 
                 << " | " << setw(16) << setprecision(4) << efficiency << endl;
        }
    }

    if (rank == 0) {
        cout << "========================================================================" << endl << endl;
    }

    MPI_Finalize();
    return 0;
}