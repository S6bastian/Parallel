#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <random>
#include <mpi.h>

using namespace std;

// Función Compare-Split: mezcla los datos y conserva la mitad correspondiente
void compare_split(vector<int>& local_keys, const vector<int>& recv_keys, bool keep_smaller) {
    int local_n = local_keys.size();
    int partner_n = recv_keys.size();
    vector<int> temp(local_n);

    if (keep_smaller) {
        int i = 0, j = 0;
        for (int k = 0; k < local_n; k++) {
            if (i < local_n && (j >= partner_n || local_keys[i] <= recv_keys[j])) {
                temp[k] = local_keys[i++];
            } else {
                temp[k] = recv_keys[j++];
            }
        }
    } else {
        int i = local_n - 1, j = partner_n - 1;
        for (int k = local_n - 1; k >= 0; k--) {
            if (i >= 0 && (j < 0 || local_keys[i] >= recv_keys[j])) {
                temp[k] = local_keys[i--];
            } else {
                temp[k] = recv_keys[j--];
            }
        }
    }
    local_keys = temp;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    vector<int> sizes = {100000, 200000, 400000, 800000, 1600000};

    if (rank == 0) {
        cout << "========================================================================" << endl;
        cout << "      Ordenamiento Odd-Even Transposition MPI (Procesos p = " << size << ")" << endl;
        cout << "========================================================================" << endl;
        cout << left << setw(12) << "N (Tamano)" 
             << " | " << setw(16) << "Tiempo Tp (s)" 
             << " | " << setw(14) << "Speedup (Sp)" 
             << " | " << setw(16) << "Eficiencia (Ep)" << endl;
        cout << "------------------------------------------------------------------------" << endl;
    }

    for (int N : sizes) {
        double T1 = 0.0;

        // Medición de T1 (tiempo base de ordenamiento) en rank 0
        if (rank == 0) {
            vector<int> data_base(N);
            mt19937 rng(42);
            uniform_int_distribution<int> dist(1, 1000000);
            for (int i = 0; i < N; i++) data_base[i] = dist(rng);

            auto start1 = chrono::high_resolution_clock::now();
            sort(data_base.begin(), data_base.end());
            auto end1 = chrono::high_resolution_clock::now();
            chrono::duration<double> dur1 = end1 - start1;
            T1 = dur1.count();
        }

        // Generar arreglo desordenado original para la ejecución paralela
        vector<int> global_data;
        if (rank == 0) {
            global_data.resize(N);
            mt19937 rng(42);
            uniform_int_distribution<int> dist(1, 1000000);
            for (int i = 0; i < N; i++) global_data[i] = dist(rng);
        }

        // Distribución de subarreglos entre los procesos p
        int base_n = N / size;
        int rem = N % size;
        int local_n = base_n + (rank < rem ? 1 : 0);

        vector<int> sendcounts(size), displs(size);
        int offset = 0;
        for (int r = 0; r < size; r++) {
            int r_n = base_n + (r < rem ? 1 : 0);
            sendcounts[r] = r_n;
            displs[r] = offset;
            offset += r_n;
        }

        vector<int> local_keys(local_n);

        // --- Inicio de Ejecución Paralela MPI ---
        MPI_Barrier(MPI_COMM_WORLD);
        double t_start = MPI_Wtime();

        // 1. Distribuir datos desordenados
        MPI_Scatterv(global_data.data(), sendcounts.data(), displs.data(), MPI_INT,
                     local_keys.data(), local_n, MPI_INT, 0, MPI_COMM_WORLD);

        // 2. Ordenamiento local inicial
        sort(local_keys.begin(), local_keys.end());

        // 3. Fases de comunicación Odd-Even (p fases)
        for (int phase = 0; phase < size; phase++) {
            int partner = -1;

            if (phase % 2 == 0) { // Fase Par
                if (rank % 2 == 0) {
                    if (rank + 1 < size) partner = rank + 1;
                } else {
                    partner = rank - 1;
                }
            } else { // Fase Impar
                if (rank % 2 != 0) {
                    if (rank + 1 < size) partner = rank + 1;
                } else {
                    if (rank > 0) partner = rank - 1;
                }
            }

            if (partner != -1) {
                int partner_n = base_n + (partner < rem ? 1 : 0);
                vector<int> recv_keys(partner_n);

                // Intercambio punto a punto de subbloques entre vecinos
                MPI_Sendrecv(local_keys.data(), local_n, MPI_INT, partner, 0,
                             recv_keys.data(), partner_n, MPI_INT, partner, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                bool keep_smaller = (rank < partner);
                compare_split(local_keys, recv_keys, keep_smaller);
            }
        }

        // 4. Reunir el arreglo ordenado en rank 0
        vector<int> sorted_data;
        if (rank == 0) sorted_data.resize(N);

        MPI_Gatherv(local_keys.data(), local_n, MPI_INT,
                    sorted_data.data(), sendcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

        double t_end = MPI_Wtime();
        double local_time = t_end - t_start;

        // Tiempo máximo (Tp) entre todos los procesos
        double Tp = 0.0;
        MPI_Reduce(&local_time, &Tp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

        // Impresión de resultados en la terminal
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