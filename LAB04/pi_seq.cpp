#include <bits/stdc++.h>

using namespace std;

double leibniz_serie(unsigned long long n){
    double sum = 0.0;
    for(unsigned long long i = 0; i <= n; i++){
        sum += (i % 2 == 0 ? 1.0 : -1.0)/(2.0*i+1.0);
    }
    sum *= 4.0;

    return sum;
}


int main(){
    unsigned long long n = 1e6;

    double sum = leibniz_serie(n);

    cout << fixed << setprecision(10) << sum << "\n";

    return 0;
}