#include "tpch-q3_JIT.hpp"
#include <stdio.h>
#define DEBUG_OUTPUT
#include <iostream>
using namespace std;


void Data_size(const char* customer, const char* orders, const char* lineitem, const char* c_mktsegment, const char* o_orderdate, 
               const char* l_shipdate, int no_of_runs){
    long int compileTime, executionTime;
    //tie(compileTime , executionTime) = NestedLoop(filename1, filename2);
    long int totalCompileTime = 0;
    long int totalExecutionTime = 0;

    long int totalCompileTime_mergeSort = 0;
    long int totalExecutionTime_mergeSort = 0;

    long int totalCompileTime_group = 0;
    long int totalExecutionTime_group = 0;
    
    for(int i = 0; i < no_of_runs; i++){
        struct time time;
        time = tpchq3::run(customer, orders, lineitem, c_mktsegment, o_orderdate, l_shipdate);
        totalCompileTime += time.joincompileTime;
        totalExecutionTime += time.joinexecutionTime;
        
        totalCompileTime_group += time.groupcompileTime;
        totalExecutionTime_group += time.groupexecutionTime;

        totalCompileTime_mergeSort += time.orderbycompileTime;
        totalExecutionTime_mergeSort += time.orderbyexecutionTime;

        cout<<"iteration number: "<<i+1<<endl;
        cout<<"join compile time in microseconds: "<<time.joincompileTime<<"  join exec time microseconds: "<<time.joinexecutionTime
            <<"   groupby compile time in microseconds: "<<time.groupcompileTime<<"  groupby exec time microseconds: "
            <<time.groupexecutionTime
            <<"   orderby compile time in microseconds: "<<time.orderbycompileTime<<"  orderby exec time microseconds: "
            <<time.orderbyexecutionTime<<endl;
    }
    long int meanCompileTime = totalCompileTime/no_of_runs;
    long int meanExecTime = totalExecutionTime/no_of_runs;

    long int meanCompileTimeGroupby = totalCompileTime_group/no_of_runs;
    long int meanExecTimeGroupby = totalExecutionTime_group/no_of_runs;

    long int meanCompileTime_mergeSort = totalCompileTime_mergeSort/no_of_runs;
    long int meanExecTime_mergeSort  = totalExecutionTime_mergeSort/no_of_runs;

    cout<<"join mean compile time in microseconds: "<<meanCompileTime<<"    join mean exec time microseconds: "<<meanExecTime
        <<"    groupby mean compile time in microseconds: "<<meanCompileTimeGroupby<<"    groupby mean exec time microseconds: "
        <<meanExecTimeGroupby
        <<"    orderby mean compile time in microseconds: "<<meanCompileTime_mergeSort<<"    orderby mean exec time microseconds: "
        <<meanExecTime_mergeSort<<endl;

    cout<<"Total Compilation time: "<<meanCompileTime+meanCompileTimeGroupby+meanCompileTime_mergeSort<<"  Total exec time:  "<<
        meanExecTime+meanExecTimeGroupby+meanExecTime_mergeSort<<endl;
}

#define DEBUG_OUTPUT

int main() {
    //define arguments

    const char* customer = "../../../../tpch-dbgen/data/customer.tbl";
    const char* orders = "../../../../tpch-dbgen/data/orders.tbl";
    const char* lineitem = "../../../../tpch-dbgen/data/lineitem.tbl";

    
    const char* c_mktsegment = "AUTOMOBILE";
    const char* o_orderdate = "1995-03-19"; /* < conition */
    const char* l_shipdate = "1995-03-19"; /* > conition */
    
    int no_of_runs = 1;

    Data_size(customer, orders, lineitem, c_mktsegment, o_orderdate, l_shipdate, no_of_runs);
    #if defined(DEBUG_OUTPUT)
    //printf("Result=%d\n", result);
    #endif
}