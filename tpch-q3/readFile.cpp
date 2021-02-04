#include "tpch-q3_JIT.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
using namespace std;

matrix readFile(string filename)
{
    char separator = '|';
    matrix result;
    string row, item;

    ifstream in(filename);
    int i = 0;
    while (getline(in, row))
    {
        vec R;
        stringstream ss(row);
        while (getline(ss, item, separator)) R.push_back(item);
        result.push_back(R);
    }
    in.close();
    return result;
}

void printMatrix(const matrix& M)
{
    for (vec row : M)
    {
        for (string s : row) cout << setw(18) << left << s << " ";  
        cout << '\n';
    }
}

void printFinalResult(struct GROUPRESULT *result, int32_t size){
    int32_t i;
    for(i = 0; i < size; i++){

        cout << setw(18) << left << result[i].l_orderkey<< " ";

        char str1[20];
        sprintf(str1, "%f", result[i].revenue);
        cout << setw(18) << left << str1<< " ";

        cout << setw(18) << left << result[i].o_orderdate<< " ";
        cout << setw(18) << left << result[i].o_shippriority<< " ";

        cout<<endl;
    }
    cout<<"Total Rows: "<<size<<endl;
}
