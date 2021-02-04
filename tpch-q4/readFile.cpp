#include "tpch-q4_JIT.hpp"
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
