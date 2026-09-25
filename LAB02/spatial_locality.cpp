#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

double row_traversal(vector<vector<double>>& matrix, const int& n){
    auto start_time = high_resolution_clock::now();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] *= 2.0;
        }
    }
    auto end_time = high_resolution_clock::now();
    
    duration<double, std::milli> duration_ms = end_time - start_time;
    return duration_ms.count();
}

double column_traversal(vector<vector<double>>& matrix, const int& n){
    auto start_time = high_resolution_clock::now();
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            matrix[i][j] *= 2.0;
        }
    }
    auto end_time = high_resolution_clock::now();
    
    duration<double, std::milli> duration_ms = end_time - start_time;
    return duration_ms.count();
}

void export_to_csv(const std::string& filename, const std::vector<int>& sizes, std::vector<vector<double>>& matrix) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening the file " << filename << "\n";
        return;
    }

    file << "Size,Row_Time_ms,Column_Time_ms,Ratio\n";

    for (int size : sizes) {
        double row_time = row_traversal(matrix, size);
        double column_time = column_traversal(matrix, size);
        double ratio = column_time / row_time;

        file << size << "," << row_time << "," << column_time << "," << ratio << "\n";
    }

    file.close();
    std::cout << "Succesfully exported as: " << filename << "\n";
}

int main(){
    int n = 4000;
    vector<vector<double>> matrix(n,vector<double>(n,1.0));

    //vector<int> sizes = {500,1000,2000,4000};
    //export_to_csv("first.csv",sizes,matrix);

    vector<int> sizes = {100,250,500,1000,2000,4000};
    export_to_csv("data.csv",sizes,matrix);

    /*

    cout << "------------------------------------------------------\n" 
        << "First comparison\n"
        << "------------------------------------------------------\n";
    
    vector<int> sizes = {500,1000,2000,4000};

    cout << "Size\tRows\t\tColumns\t\tRatio\n";
    for(auto& size : sizes){
        double row_time, column_time,ratio;
        row_time = row_traversal(matrix,size);
        column_time = column_traversal(matrix,size);
        ratio = column_time/row_time;

        cout << size << "\t" << row_time << "\t\t" << column_time << "\t\t" << ratio << "\n";
    }




    cout << "------------------------------------------------------\n" 
        << "Second comparison\n"
        << "------------------------------------------------------\n";
    
    sizes = {100,250,500,1000,2000,4000};

    cout << "Size\tRows\t\tColumns\t\tRatio\n";
    for(auto& size : sizes){
        double row_time, column_time,ratio;
        row_time = row_traversal(matrix,size);
        column_time = column_traversal(matrix,size);
        ratio = column_time/row_time;

        cout << size << "\t" << row_time << "\t\t" << column_time << "\t\t" << ratio << "\n";
    }
    */
    
    
    

    return 0;
}