#include "tpch-q2_JIT.hpp"
#include <stdio.h>
#define DEBUG_OUTPUT
#include <iostream>
#include <cmath>
using namespace std;


void calculateSD(long int data[], int no_of_runs, const char* type)
{
    cout<<type<<endl;
    int i;
    for(i = 0; i < no_of_runs; i++)
    {
        cout<<data[i]<<endl;
    }
    float sum = 0.0, mean, standardDeviation = 0.0;

    long int min = data[0];
    long int max = data[0];

    for(i = 1; i < no_of_runs; i++)
    {
        if(data[i] < min)
        {
            min = data[i];
        }
        if(data[i] > max){
            max = data[i];
        }
    }
    for(i = 0; i < no_of_runs; i++)
    {
        /* if(data[i] != min && data[i] != max)
        {
            sum += data[i];
        } */
        sum += data[i];
    }

    mean = sum/(no_of_runs);

    /* for(i = 0; i < no_of_runs; i++)
    {
        if(data[i] != min && data[i] != max)
        {
            standardDeviation += pow(data[i] - mean, 2);
        }
    } */
    float var = 0.0;
    for( i = 0; i < no_of_runs; i++ )
    {
       var += (data[i] - mean) * (data[i] - mean);
    }
    var /= no_of_runs;
    float sd = sqrt(var);
    cout<<"min: "<<min<<"   max:   "<<max<<"  mean:  "<<mean<<endl;
    cout<<"S.D:  "<<sd<<endl;
}


void Data_size(const char* partTable, const char* supplierTable, const char* partSuppTable, const char* nationTable, const char* regionTable, 
             int64_t p_size, const char* region, const char* like_substring, int no_of_runs){
    long int compile_data[no_of_runs];
    long int exec_data[no_of_runs];
    long int compileTime, executionTime;
    //tie(compileTime , executionTime) = NestedLoop(filename1, filename2);
    long int totalCompileTime = 0;
    long int totalExecutionTime = 0;

    long int totalCompileTime_mergeSort = 0;
    long int totalExecutionTime_mergeSort = 0;
    
    for(int i = 0; i < no_of_runs; i++){
        struct time time;
        time = tpchq2::run(partTable, supplierTable, partSuppTable, nationTable, regionTable, p_size, region, like_substring);

        totalCompileTime += time.compileTime;
        totalExecutionTime += time.executionTime;
        
        totalCompileTime_mergeSort += time.compileTime_mergeSort;
        totalExecutionTime_mergeSort += time.executionTime_mergeSort;

        compile_data[i] = time.compileTime + time.compileTime_mergeSort;
        exec_data[i] = time.executionTime + time.executionTime_mergeSort;

        cout<<i+1<<" : iteration"<<endl;
        cout<<"compile time in microseconds: "<<time.compileTime<<"  exec time microseconds: "<<time.executionTime
            <<"   mergeSort compile time in microseconds: "<<time.compileTime_mergeSort<<"  mergeSort exec time microseconds: "
            <<time.executionTime_mergeSort<<endl;
    }
    long int meanCompileTime = totalCompileTime/no_of_runs;
    long int meanExecTime = totalExecutionTime/no_of_runs;

    long int meanCompileTime_mergeSort = totalCompileTime_mergeSort/no_of_runs;
    long int meanExecTime_mergeSort  = totalExecutionTime_mergeSort/no_of_runs;
    cout<<"mean compile time in microseconds: "<<meanCompileTime<<"    mean exec time microseconds: "<<meanExecTime
        <<"    mergeSort mean compile time in microseconds: "<<meanCompileTime_mergeSort<<"    mergeSort mean exec time microseconds: "
        <<meanExecTime_mergeSort<<endl;

    cout<<"Total Compilation time: "<<meanCompileTime+meanCompileTime_mergeSort<<"  Total exec time:  "<<
        meanExecTime+meanExecTime_mergeSort<<endl;
    
    calculateSD(compile_data, no_of_runs, "compile data");
    calculateSD(exec_data, no_of_runs, "exectution data");
}

#define DEBUG_OUTPUT

int main() {
    //define arguments
    
    const char* partTable = "../../../../tpch-dbgen/data/part.tbl";
    const char* supplierTable = "../../../../tpch-dbgen/data/supplier.tbl";
    const char* partSuppTable = "../../../../tpch-dbgen/data/partsupp.tbl";
    const char* nationTable = "../../../../tpch-dbgen/data/nation.tbl";
    const char* regionTable = "../../../../tpch-dbgen/data/region.tbl";

    /* const char* partTable = "../../../../tpch-dbgen/data_scale4/part.tbl";
    const char* supplierTable = "../../../../tpch-dbgen/data_scale4/supplier.tbl";
    const char* partSuppTable = "../../../../tpch-dbgen/data_scale4/partsupp.tbl";
    const char* nationTable = "../../../../tpch-dbgen/data_scale4/nation.tbl";
    const char* regionTable = "../../../../tpch-dbgen/data_scale4/region.tbl"; */

    int64_t p_size = 37;
    const char* region = "AFRICA";
    const char* like_substring = "TIN";


    int no_of_runs = 10;

    Data_size(partTable, supplierTable, partSuppTable, nationTable, regionTable, p_size, region, like_substring, no_of_runs);
    #if defined(DEBUG_OUTPUT)
    //printf("Result=%d\n", result);
    #endif
}