#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

double pattern1(double**matrix, const int& n){
    auto start_time = high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        matrix[i][0] += 1;
        matrix[i][0] += 1;
        matrix[i][0] += 1;
    }
    auto end_time = high_resolution_clock::now();
    
    duration<double, std::milli> duration_ms = end_time - start_time;
    return duration_ms.count();
}

double pattern2(double** matrix, const int& n){
    auto start_time = high_resolution_clock::now();
    for (int k = 0; k < 3; k++) {
        for (int i = 0; i < n; i++) {
            matrix[i][0] += 1;
        }
    }
    auto end_time = high_resolution_clock::now();
    
    duration<double, std::milli> duration_ms = end_time - start_time;
    return duration_ms.count();
}

void export_to_csv(const string& filename, const vector<int>& sizes, double** matrix) {
    ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening the file " << filename << "\n";
        return;
    }

    file << "Size,pattern1_time_ms,pattern2_time_ms,Ratio\n";

    for (int size : sizes) {
        double pattern1_time = pattern1(matrix, size);
        double pattern2_time = pattern2(matrix, size);
        double ratio = pattern2_time / pattern1_time;

        file << size << "," << pattern1_time << "," << pattern2_time << "," << ratio << "\n";
    }

    file.close();
    std::cout << "Succesfully exported as: " << filename << "\n";
}

int main(){
    int n = 10000;
    double** matrix = new double*[n];
    for(int i = 0; i < n; i++){
        matrix[i] = new double[n];
        for(int j = 0; j < n; j++) matrix[i][j] = 1.0;
    }

    //vector<int> sizes = {500,1000,2000,4000};
    //export_to_csv("first.csv",sizes,matrix);

    vector<int> sizes = {100,250,500,1000,2000,4000,7000,10000};
    export_to_csv("data.csv",sizes,matrix);

    for (int i = 0; i < n; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;

    return 0;
}