#include <bits/stdc++.h>
#include <omp.h>
using namespace std;
using namespace std::chrono;

size_t n = 1e5;
vector<double> a(n, 1.1);

double sequential() {
    double total = 0.0;
    auto start_time = high_resolution_clock::now();

    for (size_t i = 0; i < n; i++) {
        total += a[i];
    }

    auto end_time = high_resolution_clock::now();
    duration<double> duration_sec = end_time - start_time;
    
    return duration_sec.count(); 
}

double parallel(const int num_threads) {
    omp_set_num_threads(num_threads); 
    
    double total = 0.0;
    auto start_time = high_resolution_clock::now();

    #pragma omp parallel for reduction(+:total)
    for (size_t i = 0; i < n; i++) {
        total += a[i];
    }

    auto end_time = high_resolution_clock::now();
    duration<double> duration_sec = end_time - start_time;
    
    return duration_sec.count();
}

int main() {
    cout << "Detected OpenMP Max Threads: " << omp_get_max_threads() << "\n\n";

    double t_sec = sequential();
    cout << fixed << setprecision(6);
    cout << "[Sequential] Execution Time: " << t_sec << " s\n\n";

    vector<int> thread_counts = {1, 2, 4, 8, 16, 32};

    cout << "Threads\t\tTime (s)\tSpeedup\t\tEfficiency\n";
    cout << "----------------------------------------------------------\n";

    for (int p : thread_counts) {
        double t_par = parallel(p);
        double speedup = t_sec / t_par;
        double efficiency = speedup / p;

        cout << p << "\t\t" << t_par << "\t" << speedup << "\t" << efficiency << "\n";
    }

    return 0;
}