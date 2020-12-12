#include <iostream>
#include <chrono>
#include <random>

#include "flatalg_tests.hpp"

// #include <C:/C++/HConLib/include/FlatAlg.hpp>
#define FLATALG_ALLOW_NO_AVX
#include <FlatAlg.hpp>
using namespace std;

int main() {

  run_flatalg_tests();

  /* constexpr int n = 60;
    constexpr int m = 1000000;
    falg::Matrix<n, 1>* vec1;
    falg::Matrix<n, 1>* vec2;

    falg::flatalg_t* fs;

    vec1 = new falg::Matrix<n, 1>[m];
    vec2 = new falg::Matrix<n, 1>[m];
    // fs = new falg::flatalg_t[m];
    for (int j = 0; j < m; j++) {
        for (int i = 0; i < n; i++) {
            vec1[j][i] = rand() % 25324;
            vec2[j][i] = rand() % 12312;
        }
        // fs[j] = rand() % 1231;
    }

    falg::Matrix<n, 1>* res = new falg::Matrix<n, 1>[m];
    falg::Matrix<n, 1>* refres = new falg::Matrix<n, 1>[m];



    auto rstart = chrono::steady_clock::now();
    for (int j = 0; j < m; j++) {
        for (int i = 0; i < n; i++) {
	  // refres[j][i] *= fs[j]; // vec2[j][i];
	  refres[j][i] = vec1[j][i] - vec2[j][i];
        }
    }
    auto rstop = chrono::steady_clock::now();
    auto rdiff = rstop - rstart;

    double rmsecs = chrono::duration<double, milli>(rdiff).count();

    auto start = chrono::steady_clock::now();
    for (int j = 0; j < m; j++) {
        // vec1[j] -= vec2[j];
        // res[j] = vec1[j] * fs[j];
      res[j] = vec1[j] - vec2[j];
    }
    auto stop = chrono::steady_clock::now();
    auto diff = stop - start;

    double msecs = chrono::duration<double, milli>(diff).count();

    bool ok = true;
    int a = 0;
    int b = 0;
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (abs(res[i][j] - refres[i][j]) >= 1e-3) {
                ok = false;
                a = i;
                b = j;
                break;
            }
        }
        
        if (!ok) {
            break;
        }
    }
    
    if (!ok) {
        cout << "Diff at index " << a << ", " << b << " , wanted " << refres[a][b] << ", got " << vec1[a][b] << endl;
        cout << "Vector was " << vec1[a].str() << endl;
    }
    else {
        cout << "Results were the same!" << endl;
    }

    // cout << "Computed value: " << dsum << ", actual value: " << sum << endl;

    cout << "Computing time: " << msecs << ", reference time: " << rmsecs << endl; */
}
