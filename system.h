#pragma once 
#include <queue>
#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>
#include <random>
using namespace std;

// default depth of the simul method
// value recomended : 1 (or 2 but don't try more if you want to see the program end !)
// depending on your computer, with a depth of 1, runtime will problably be between 2-3 seconds and 10-15 seconds
// with a depth of 2 it takes less than a minute on my computer.
// can be set to 0 to check that it leads to the same results as FIRST policy
const int DEFAULT_DEPTH = 1;

// Nb of products treated in the simulation
const int NB_MAX_PRODUCTS = 40000;
const int NB_TRANS = NB_MAX_PRODUCTS/2;

// Common parameters
const int NB_MACHINES = 8;
const int NB_PRODUCT_TYPES = 4;
const double LAMBDAS[NB_PRODUCT_TYPES] = {0.29, 0.32, 0.47, 0.38};
const int MAX_SIZE_ROUTE = 5;
const int ROUTES[(MAX_SIZE_ROUTE+1)*NB_PRODUCT_TYPES] = {
    0, 1, 2, 3, 7,-1,
    1, 3, 6,-1,-1,-1,
    2, 4, 0,-1,-1,-1, 
    4, 5, 6, 7,-1,-1
};

const double LOWER_PT[NB_PRODUCT_TYPES*NB_MACHINES] = {
    0.58, 0.23, 0.81, 0.12, 0.0 , 0.0 , 0.0 , 0.82, 
    0.0 , 0.59, 0.0 , 0.74, 0.0 , 0.0 , 0.30, 0.0 , 
    0.57, 0.0 , 0.37, 0.0 , 0.35, 0.0 , 0.0 , 0.0 , 
    0.0 , 0.0 , 0.0 , 0.0 , 0.36, 0.61, 0.78, 0.18
};
            
const double UPPER_PT[NB_PRODUCT_TYPES*NB_MACHINES] = {
    0.78, 0.56, 0.93, 0.39, 0.0 , 0.0 , 0.0 , 1.04, 
    0.0 , 0.68, 0.0 , 0.77, 0.0 , 0.0 , 0.55, 0.0 ,
    0.64, 0.0 , 0.54, 0.0 , 0.63, 0.0 , 0.0 , 0.0 , 
    0.0 , 0.0 , 0.0 , 0.0 , 0.51, 0.70, 0.85, 0.37
};


// specific variables : uncomment one and only one instance, build and run to see performance 

// INSTANCE 1
const int NB_WORKERS = 4;
const bool QUALIFICATIONS[NB_WORKERS*NB_MACHINES] = {
    1,1,0,0,0,0,0,0,
    0,0,1,1,0,0,0,0,
    0,0,0,0,1,1,0,0,
    0,0,0,0,0,0,1,1
};

// INSTANCE 2
// const int NB_WORKERS = 4;
// const bool QUALIFICATIONS[NB_WORKERS*NB_MACHINES] = {
//     1,0,1,0,0,1,0,0,
//     0,1,0,0,1,0,1,1,
//     0,1,0,1,1,0,0,1,
//     1,0,1,1,0,1,1,0
// };

// INSTANCE 3
// const int NB_WORKERS = 6;
// const bool QUALIFICATIONS[NB_WORKERS*NB_MACHINES] = {
// 1,1,0,0,0,0,0,0,
// 0,0,1,0,0,0,0,0,
// 0,0,0,1,0,1,0,0,
// 0,0,0,0,1,0,0,1,
// 0,0,1,0,0,1,0,0,
// 1,0,0,0,0,0,1,0
// };

// INSTANCE 4
// const int NB_WORKERS = 6;
// const bool QUALIFICATIONS[NB_WORKERS*NB_MACHINES] = {
// 1,1,1,0,0,0,0,0,
// 0,0,0,1,1,1,0,0,
// 1,0,1,0,0,1,1,1,
// 0,0,1,0,1,0,1,1,
// 0,1,0,0,0,1,1,0,
// 1,0,0,1,0,0,1,1
// };



double inter_arrival(double lambda){
    return -log(1-double(rand())/(INT16_MAX+1))/lambda;
}

////////// EVENTS //////////

class Event{

public:
    double time;
    int id;     // 0 for product arrival, 1 for end of a task
    int info;   // either type of the arriving product or machine index of the ending task (depending on the id) 

    Event(){}
    Event(double time, int info,int id):time{time},id{id},info{info}{}
    virtual ~Event(){}
    bool operator<(const Event& e) const{return time>e.time;}
};

////////// PRODUCTS //////////

class Product{
public:
    int type;
    int cur_step{};
    double arrival_time;
    double processing_times[MAX_SIZE_ROUTE];

    Product(int type, double time): type{type}, arrival_time{time}{
        for(int i=0; i<MAX_SIZE_ROUTE; i++){
            int id_machine = ROUTES[type*(MAX_SIZE_ROUTE+1)+i];
            if(id_machine==-1)
                break;
            processing_times[i]=LOWER_PT[type*NB_MACHINES+id_machine] + double(rand())/INT16_MAX*(UPPER_PT[type*NB_MACHINES+id_machine]-LOWER_PT[type*NB_MACHINES+id_machine]);
        }
    }
    ~Product(){};
    int next_machine()const{return ROUTES[type*(MAX_SIZE_ROUTE+1)+cur_step];}


};


////////// WORKERS //////////

class Worker{
public:
    int id;
    bool qual[NB_MACHINES];
    bool free=true;

    Worker(){}
    Worker(int id):id{id} {
        for(int i=0; i<NB_MACHINES; i++){
            qual[i] = QUALIFICATIONS[id*NB_MACHINES+i];
        }
    }
    ~Worker(){}
};


////////// MACHINES //////////

class Machine{
public:
    int id;
    bool waiting=true;
    int worker_id = -1; // worker curently working on the machine (if waiting=false)
    queue<Product> line;

    Machine(){}
    Machine(int id):id{id}{}
};



////////// SYSTEM //////////


class System{
public:
    int nb_arrived=0;
    int nb_left=0;
    int depth=DEFAULT_DEPTH;
    priority_queue<Event> schedule;
    Machine machines[NB_MACHINES];
    Worker workers[NB_WORKERS];
    double total_duration;  // keep track of the total durations of sojourn times
    double part_duration;   // keep track of the total durations of sojourn times after passing the transition part

    System(){
        for(int i=0; i<NB_MACHINES; i++){
            machines[i] = Machine(i);
        }
        for(int i=0; i<NB_WORKERS; i++){
            workers[i] = Worker(i);
        }
    }

    int worker_available(int machine_id, double time){
        if(depth==0){
            for(int i=0; i<NB_WORKERS; i++){
                if(workers[i].free && workers[i].qual[machine_id]){
                    return i;
                }
            }
            return -1;
        }
        vector<int> w_dispo;
        for(int i=0; i<NB_WORKERS; i++){
            if(workers[i].free && workers[i].qual[machine_id]){
                w_dispo.push_back(i);
            }
        }
        if(w_dispo.empty()){
            return -1;
        }
        else if(w_dispo.size()==1){
            return w_dispo[0];
        }
        else{
            double best_time=1.0e10;
            int best_worker=-1;
            for(int i=0; i<w_dispo.size();i++){
                int worker_id = w_dispo[i];

                // copy the system
                System sys_copy(*this);

                // make the choice
                sys_copy.depth--;
                sys_copy.workers[worker_id].free = false;
                sys_copy.machines[machine_id].worker_id = worker_id;
                sys_copy.machines[machine_id].waiting = false;
                Product product = sys_copy.machines[machine_id].line.front();
                double pt = product.processing_times[product.cur_step];
                sys_copy.schedule.push(Event(pt+time,machine_id,1));

                // Is it a good choice ?
                double value = sys_copy.part_simul();
                if(value<best_time){
                    best_time = value;
                    best_worker=worker_id;
                }
            }
            return best_worker;
        }
    }

    void route(const Product &product, double time){
        int machine_id = product.next_machine();
        if(machine_id==-1){
            //cout<<"product of type "<<product.type<<" left at t="<<time<<endl;
            total_duration += (time-product.arrival_time);
            part_duration += (time-product.arrival_time)*(nb_left>=NB_TRANS);
            nb_left++;
        }
        else{
            bool test=machines[machine_id].line.empty();
            machines[machine_id].line.push(product);
            if(test){
                int worker_id = worker_available(machine_id,time);
                if(worker_id!=-1){
                    workers[worker_id].free = false;
                    machines[machine_id].worker_id = worker_id;
                    machines[machine_id].waiting = false;
                    schedule.push(Event(product.processing_times[product.cur_step]+time,machine_id,1));
                    //cout<<"worker "<<worker_id<<" started working on machine "<<machine_id<<endl;
                }
            }
        }

    }
    void algo(int worker_id, double time){
        int best_machine=-1;
        if(depth==0){
            // choice of the machine
            for(int i=0; i<NB_MACHINES; i++){
                if(workers[worker_id].qual[i] && machines[i].waiting && !machines[i].line.empty()){
                    best_machine = i;
                    break;
                }
            }
        }
        else{
            vector<int> choices;
            for(int i=0; i<NB_MACHINES; i++){
                if(workers[worker_id].qual[i] && machines[i].waiting && !machines[i].line.empty()){
                    choices.push_back(i);
                }
            }
            if(choices.empty()){
                return;
            }
            else if (choices.size()==1){
                best_machine = choices[0];
            }
            else{
                double best_time=1.0e10;
                for(int i=0; i<choices.size();i++){
                    int machine = choices[i];

                    // copy the system
                    System sys_copy(*this);

                    // make the choice
                    sys_copy.depth--;
                    sys_copy.workers[worker_id].free = false;
                    sys_copy.machines[machine].worker_id = worker_id;
                    sys_copy.machines[machine].waiting = false;
                    Product product = sys_copy.machines[machine].line.front();
                    double pt = product.processing_times[product.cur_step];
                    sys_copy.schedule.push(Event(pt+time,machine,1));

                    // Is it a good choice ?
                    double value = sys_copy.part_simul();
                    if(value<best_time){
                        best_time = value;
                        best_machine=machine;
                    }
                }
            }
        }
        // update system
        if(best_machine!=-1){
            workers[worker_id].free = false;
            machines[best_machine].worker_id = worker_id;
            machines[best_machine].waiting = false;
            Product product = machines[best_machine].line.front();
            double pt = product.processing_times[product.cur_step];
            schedule.push(Event(pt+time,best_machine,1));
            //cout<<"worker "<<worker_id<<" started working on machine "<<best_machine<<" (algo)"<<endl;
        }
    }
    void action(const Event& e){
        // a product has arrived
        if(e.id==0){
            if(nb_arrived<NB_MAX_PRODUCTS){
                //cout<<"product of type "<<e.info<<" arrived at t="<<e.time<<endl;
                nb_arrived++;
                Event e_next(e.time+inter_arrival(LAMBDAS[e.info]),e.info,0);
                schedule.push(e_next);
                Product product(e.info,e.time);
                route(product,e.time);
            }
        }
        // end of a task
        if(e.id==1){
            int machine_id = e.info;
            int worker_id = machines[machine_id].worker_id;
            Product product = machines[machine_id].line.front();
            machines[machine_id].line.pop();
            machines[machine_id].waiting = true;
            product.cur_step++;
            //cout<<"worker "<<worker_id<<" finished working on machine "<<machine_id<<endl;
            route(product,e.time);
            workers[worker_id].free = true;
            algo(worker_id,e.time);

            if(machines[machine_id].waiting && !machines[machine_id].line.empty()){
                worker_id = worker_available(machine_id,e.time);
                if(worker_id!=-1){
                    workers[worker_id].free = false;
                    machines[machine_id].worker_id = worker_id;
                    machines[machine_id].waiting = false;
                    schedule.push(Event(product.processing_times[product.cur_step]+e.time,machine_id,1));
                    //cout<<"worker "<<worker_id<<" started working on machine "<<machine_id<<endl;
                }
            }
        }
    }

    void init_simul(){
        //cout<<"simulation started"<<endl;
        for(int i=0; i<NB_PRODUCT_TYPES; i++){
            Event e(inter_arrival(LAMBDAS[i]),i,0);
            schedule.push(e);
        }
    }

    void simul(){
        while(!schedule.empty()){
            Event e = schedule.top();
            schedule.pop();
            action(e);
        }
    }

    double part_simul(){
        total_duration=0;
        while(!schedule.empty()){
            Event e = schedule.top();
            schedule.pop();
            if(e.id==1){
                action(e);
            }
        }
        return total_duration;
    }
};