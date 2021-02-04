#include "tpch-q1.hpp"
#include <stdio.h>
#define DEBUG_OUTPUT
#include <iostream>
using namespace std;


void Data_size(const char* table, const char* date, int32_t orderByCol1, int32_t orderByCol2, int32_t quantity, int32_t ep, int32_t discount, int32_t tax, int no_of_runs){
    
    long int compileTime, executionTime;
    long int mean_compileTime, mean_executionTime;

    long int totalCompileTime = 0;
    long int totalExecutionTime = 0;
    
    for(int i = 0; i < no_of_runs; i++){
        struct time time;
        time = tpchq1::run(table, date, orderByCol1, orderByCol2, quantity, ep, discount, tax);
        totalCompileTime += time.compileTime;
        totalExecutionTime += time.executionTime;
        cout<<time.compileTime<<"    "<<time.executionTime<<endl;
    }
    long int  meanCompileTime = totalCompileTime/no_of_runs;
    long int meanExecTime = totalExecutionTime/no_of_runs;
    cout<<"mean cpmpile time: "<<meanCompileTime<<"    meane exec time: "<<meanExecTime<<endl;
    
}

int main() {
    //define arguments
    const char* table = "../../../../tpch-dbgen/data/lineitem.tbl";
    const char* date = "1995-12-01";
    int orderByCol1 = 8;
    int orderByCol2 = 9;
    int quantity = 4;
    int ep = 5;
    int discount = 6;
    int tax = 7;
    //tpchq1::run(table, date, orderByCol1, orderByCol2, quantity, ep, discount, tax);


    //benchmark
    int no_of_runs = 1;
    Data_size(table, date, orderByCol1, orderByCol2, quantity, ep, discount, tax, no_of_runs);
    #if defined(DEBUG_OUTPUT)
        printf("Done\n");
    #endif
}