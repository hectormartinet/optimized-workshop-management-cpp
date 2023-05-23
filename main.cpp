#include <iostream>
#include <future>
#include <chrono>
#include <array>
using namespace std::chrono;
using namespace std;
#include "system.h"

// parralelized simulations for better performances
array<double,2> simulation(int n){
    if(n==0){
        return {0.0,0.0};
    }
    // launch asynchronous simulations
    auto futur_means = async(launch::async,simulation,n-1);
    // seed
    srand(n);
    // simulation
    System sys{};
    sys.init_simul();
    sys.simul();
    array<double,2> means{};
    means[0] = sys.part_duration/(NB_MAX_PRODUCTS-NB_TRANS);
    means[1] = means[0]*means[0];
    array<double,2> result = futur_means.get();
    means[0] += result[0];
    means[1] += result[1];
    return means;
}

int main(){
    int NB_REPL=20;
    auto start = high_resolution_clock::now();
    array<double,2> means = simulation(NB_REPL);
    double mean = means[0]/NB_REPL;
    double std = pow(means[1]/NB_REPL-mean*mean,0.5);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout<<"average sojourn time with 95 percent confidence interval :"<<endl;
    cout<<mean<<"+-"<<1.96*std/pow(NB_REPL,0.5)<<endl;
    cout<<duration.count()<<"ms"<<endl;
    return 0;
}