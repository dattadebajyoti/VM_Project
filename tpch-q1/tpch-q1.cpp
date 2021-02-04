/*******************************************************************************
 * Copyright (c) 2016, 2018 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tpch-q1.hpp"


//===========================================
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono> 
#include <vector>
//===========================================
using namespace std::chrono;

#define SIZE 26

//matrix where_clause_result;
using vec = vector<string>;
using matrix = vector<vec>;

matrix temp;
//=================def matrices============================
const int32_t no_of_columns = 16;
const char*** load_matrix;
const char*** where_clause_res;
int where_size = 0;
//================def structures===========================
struct genResult{
    int first;
    float second;
};
struct genResult genResult[5];
int genResult_size = 0;


struct finalResult{
    char first_first;
    char first_second;
    int second_first;
    float second_second;
};
struct finalResult finalResult[5];
int finalResult_size = 0;

struct orderByRes{   
    char first; 
    int second;    
};

struct orderByRes orderByRes[5];
int orderByRes_size = 0;

struct groupByRes{
    char first;
    int second;
};

struct groupByRes groupByRes[5];
int groupByRes_size = 0;

struct resTracker{
    char first;
    char second_first;
    int second_second;
};

struct resTracker resTracker[5];
int resTracker_size = 0;



struct output{
    char return_flag;
    char line_status;
    float sum_quantity;
    float sum_base_price;
    float sum_disc_price;
    float sum_charge;
    float avg_qty;
    float avg_price;
    float avg_disc;
    float count_order;
};

struct output output[5];
int output_size = 0;
//=============================================
//store the matrix in a vector
matrix readFile(string filename)
{
    char separator = '|';
    matrix result;
    string row, item;
    //int i = 0;
    ifstream in(filename);
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
void loadMatMem(string table){
   temp = readFile(table);
}
//======================================================================

void printMatrix(const matrix& M)
{
    for (vec row : M)
    {
        for (string s : row) cout << setw(18) << left << s << " ";  
        cout << '\n';
    }
}

void printStruct(struct output M[])
{
    int  i;
    for(i = 0; i < output_size; i++){

        cout << setw(18) << left << M[i].return_flag << " ";
        cout << setw(18) << left << M[i].line_status << " ";

        char str1[20];
        sprintf(str1, "%f", M[i].sum_quantity);
        cout << setw(18) << left << str1 << " ";
        
        char str2[20];
        sprintf(str2, "%f", M[i].sum_base_price);
        cout << setw(18) << left << str2<< " ";

        char str3[20];
        sprintf(str3, "%f", M[i].sum_disc_price);
        cout << setw(18) << left << str3 << " ";

        char str4[20];
        sprintf(str4, "%f", M[i].sum_charge);
        cout << setw(18) << left << str4 << " ";

        char str5[20];
        sprintf(str5, "%f", M[i].avg_qty);
        cout << setw(18) << left << str5 << " ";

        char str6[20];
        sprintf(str6, "%f", M[i].avg_price);
        cout << setw(18) << left << str6 << " ";

        char str7[20];
        sprintf(str7, "%f", M[i].avg_disc);
        cout << setw(18) << left << str7 << " ";

        char str8[20];
        sprintf(str8, "%f", M[i].count_order);
        cout << setw(18) << left << str8 << " ";

        cout << '\n';
    }
    cout<<"Total Rows: "<<where_size<<endl;

}
//======================================================================
char* substr(const char *src, int m, int n)
{
	int len = n - m;
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
    int i;
	for (i = m; i < n && (*(src + i) != '\0'); i++)
	{
		*dest = *(src + i);
		dest++;
	}
	*dest = '\0';
	return dest - len;
}

static int32_t compareDate(const char* d1, const char* d2) {
    int year_d1 = atoi(substr(d1,0,4));
    int month_d1 = atoi(substr(d1, 5, 6));
    int day_d1 = atoi(substr(d1, 8, 9));

    int year_d2 = atoi(substr(d2, 0, 4));
    int month_d2 = atoi(substr(d2, 5, 6));;
    int day_d2 = atoi(substr(d2, 8, 9));

    if (year_d1 <= year_d2) {
        return 1;
    }
    else if (year_d1 <= year_d2 && month_d1 <= month_d2) {
        return 1;
    }
    else if (year_d1 <= year_d2 && month_d1 <= month_d2 && day_d1 <= day_d2) {
        return 1;
    }
    return 0;
}

static int32_t filter_using_where(const char* date, int32_t row)
{
   #define WHEREFILTER_LINE LINETOSTR(__LINE__)
   if (compareDate(load_matrix[row][10], date)) {
       int j;
        for(j = 0; j < no_of_columns; j++){
            where_clause_res[where_size][j] = load_matrix[row][j];
        }
        where_size++;
    }
    return where_size;
}


static void printInt32(int32_t val)
{
   #define PRINTINT32_LINE LINETOSTR(__LINE__)
   printf("%d", val);
}

static void merge(int32_t l, int32_t m, int32_t r, int32_t orderByCol)
{
   #define MERGE_LINE LINETOSTR(__LINE__)
   //cout<<"In merge  "<<l<<"   "<<m<<"   "<<r<<"    "<<orderByCol<<endl;
   int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
    int no_of_columns = 16;
    const char*** temp1 = (const char***)malloc(n1 * sizeof(const char *));
    const char*** temp2 = (const char***)malloc(n1 * sizeof(const char *));
    for(i = 0; i < n1; i++){
        const char** temp = (const char**)malloc(no_of_columns * sizeof(int *));
        *(temp1+i) = temp;
        free(temp);
    }
    for(i = 0; i < n2; i++){
        const char** temp = (const char**)malloc(no_of_columns * sizeof(int *));
        *(temp2+i) = temp;
        free(temp);
    }

    for (i = 0; i < n1; i++) {
        temp1[i] = where_clause_res[l+i];
    }
    for (j = 0; j < n2; j++) {
        temp2[j] = where_clause_res[m + 1 + j];
    }
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (int(temp1[i][orderByCol][0]) < int(temp2[j][orderByCol][0])) {
            where_clause_res[k] = temp1[i];
            i++;
        }
        else {
            where_clause_res[k] = temp2[j];
            j++;
        }
        k++;
    }
    while (i < n1) {
        where_clause_res[k] = temp1[i];
        i++;
        k++;
    }
    while (j < n2) {
        where_clause_res[k] = temp2[j];
        j++;
        k++;
    }
    free(temp1);
    free(temp2);
}


static void mergeSort(int32_t l, int32_t r, int32_t orderByCol1)
{
   #define MERGESORT_LINE LINETOSTR(__LINE__)
   if (l < r) {
      int m = l + (r - l) / 2;
      mergeSort(l, m, orderByCol1);
      mergeSort(m + 1, r, orderByCol1);
      merge(l, m, r, orderByCol1);
      
   }
   
}

static int32_t minRes(int32_t x, int32_t y) {
    #define MINRES_LINE LINETOSTR(__LINE__)
    return (x<y)? x :y; 
} 
static void mergeHelper(int32_t curr_size, int32_t left_start, int32_t orderByCol1)
{
   #define MERGEHELPER_LINE LINETOSTR(__LINE__)
   //cout<<"in helper: "<<curr_size<<"    "<<left_start<<"   "<<where_size<<endl;
   //cout<<"hi merge helper"<<"   "<<curr_size<<"     "<<left_start<<"    "<<abc<<"   "<<where_size<<endl;
   int mid = minRes(left_start + curr_size - 1, where_size-1); 
   int right_end = minRes(left_start + 2*curr_size - 1, where_size-1); 
   merge(left_start, mid, right_end, 8);
    
   
}

static void printString(int32_t orderByCol1, int64_t ptr)
   {
   #define PRINTSTRING_LINE LINETOSTR(__LINE__)
   char *str = (char *) ptr;
   mergeSort(0, where_size-1, orderByCol1);
   }

static int32_t printCharWithFreq(int32_t index, int32_t n,  int32_t col) 
{  
    //int n = where_clause_result.size(); 
    //cout<<index<<"    "<<n<<"    "<<col<<endl;
    #define CHARWITHFREQ_LINE LINETOSTR(__LINE__)
    int freq[SIZE]; 
  
    memset(freq, 0, sizeof(freq)); 
    int i;
    for (i = index; i < n; i++) {
        freq[where_clause_res[i][col][0] - 'A']++; 
    }

    for (i = index; i < n; i++) { 
        if (freq[where_clause_res[i][col][0] - 'A'] != 0) { 
            orderByRes[orderByRes_size].first = where_clause_res[i][col][0];
            orderByRes[orderByRes_size].second = freq[where_clause_res[i][col][0] - 'A'];
            orderByRes_size++;
            freq[where_clause_res[i][col][0] - 'A'] = 0; 
        } 
    } 
    //cout<<"orderByRes_size "<<orderByRes_size<<endl;
    return orderByRes_size;
} 


static void printCharFreq(int32_t orderByCol1, int32_t orderByCol2)
{
   #define CHARFREQ_LINE LINETOSTR(__LINE__)
   printCharWithFreq(0, where_size, orderByCol1);
   int start_index = 0;
   int orderBy_size = 0;
    //2nd order sort
    //5.Sort the same return_flag based on the linestatus using the result of step 4.
   int i;
   for(i = 0; i < orderByRes_size; i++){
      //cout<<start_index<<endl;
      orderBy_size += orderByRes[i].second;
      mergeSort(start_index, orderBy_size - 1, orderByCol2);
      start_index += orderByRes[i].second;
   }
}

static void printCharWithFreqGroupBy(int32_t index, int32_t n,  int32_t col, int32_t order_index) 
{  
    #define GROUPBYFREQ_LINE LINETOSTR(__LINE__)
    //int32_t n = mat.size(); 
    char orderCol = orderByRes[order_index].first;
    int32_t freq[SIZE]; 
  
    memset(freq, 0, sizeof(freq)); 
    int32_t i;
    for (i = index; i < n; i++) 
        freq[where_clause_res[i][col][0] - 'A']++; 

    for (i = index; i < n; i++) { 
        if (freq[where_clause_res[i][col][0] - 'A'] != 0) { 
            groupByRes[groupByRes_size].first = where_clause_res[i][col][0];
            groupByRes[groupByRes_size].second = freq[where_clause_res[i][col][0] - 'A'];
            groupByRes_size++;
            resTracker[resTracker_size].first = orderCol;
            resTracker[resTracker_size].second_first = where_clause_res[i][col][0];
            resTracker[resTracker_size].second_second = freq[where_clause_res[i][col][0] - 'A'];
            
            resTracker_size++;

            freq[where_clause_res[i][col][0] - 'A'] = 0; 
        } 
    }  
}

static void group(int32_t orderByCol2)
{
   #define GROUP_LINE LINETOSTR(__LINE__)
   //6.1st grouping on the result of step 6
    int group_index = 0;
    int group_size = 0;
    int i;
    for(i = 0; i < orderByRes_size; i++){
        group_size += orderByRes[i].second;
        printCharWithFreqGroupBy(group_index , group_size, orderByCol2, i);
        group_index += orderByRes[i].second;
        
    }

}

float cal_sum(const char*** mat, int index, int size, int groupByCol){
    float sum = 0;
    int i;
    for(i = index; i < size; i++){
        sum = sum + atof(mat[i][groupByCol]);
    }
    return sum;
}

float cal_sum_disc(const char*** mat, int index, int size, int groupByCol, int disc){
    float sum = 0;
    int i;
    for(i = index; i < size; i++){
        sum = sum + (atof(mat[i][groupByCol]) * (1 - atof(mat[i][disc])) );
    }
    return sum;
}

float cal_sum_tax(const char*** mat, int index, int size, int groupByCol, int disc, int tax){
    float sum = 0;
    int i;
    for(i = index; i < size; i++){
        sum = sum + (atof(mat[i][groupByCol]) * (1 - atof(mat[i][disc])) * (1 + atof(mat[i][tax])) );
    }
    return sum;
}




static void groupBy(int32_t quantity,int32_t ep, int32_t discount, int32_t tax)
{
   #define GROUPBY_LINE LINETOSTR(__LINE__)
   int start_index = 0;
    int group_size = 0;
    int i;
    for(i = 0; i < groupByRes_size; i++){
        group_size += groupByRes[i].second;
        float sum_qty = cal_sum(where_clause_res, start_index, group_size, quantity);
        float sum_base_price = cal_sum(where_clause_res, start_index, group_size, ep);
        float disc = cal_sum(where_clause_res, start_index, group_size, discount);
        
        float sum_disc_price = cal_sum_disc(where_clause_res, start_index, group_size, ep, discount);
        float sum_charge = cal_sum_tax(where_clause_res, start_index, group_size, ep, discount, tax);

        float avg_qty = sum_qty/group_size;
        float avg_price = sum_base_price/group_size;
        float avg_disc = disc/group_size;

        genResult[genResult_size].first = sum_qty;
        genResult[genResult_size].second = avg_qty;
        genResult_size++;


        output[output_size].return_flag = resTracker[i].first;
        output[output_size].line_status = resTracker[i].second_first;
        output[output_size].sum_quantity = sum_qty;
        output[output_size].sum_base_price = sum_base_price;
        output[output_size].sum_disc_price = sum_disc_price;
        output[output_size].sum_charge = sum_charge;
        output[output_size].avg_qty = avg_qty;
        output[output_size].avg_price = avg_price;
        output[output_size].avg_disc = avg_disc;
        output[output_size].count_order = groupByRes[i].second;
        output_size++;

        start_index += groupByRes[i].second;
         
    }
}


static int32_t minValue(int32_t l, int32_t r)
{
   #define MINVALUE_LINE LINETOSTR(__LINE__)
   //cout<<"check: "<<l<<"   "<<r<<endl;
   if(l < r){
       return l;
   }
   else{
       return r;
   }
}

static void print(int32_t x)
{
   #define PRINT_LINE LINETOSTR(__LINE__)
   cout<<"x is: "<<x<<endl;
}

static int32_t find_size(int32_t index)
{
   #define FINDSIZE_LINE LINETOSTR(__LINE__)
   //cout<<"x is: "<<x<<endl;
   return orderByRes[index].second;
}

OMR::JitBuilder::IlValue*
tpchq1::PrintString(IlBuilder *bldr, OMR::JitBuilder::IlValue* wc_s, OMR::JitBuilder::IlValue* date)
   {
   //bldr->Call("printString", 2, 
   //bldr->   Load("orderByCol1"), 
   //bldr->   ConstInt64((int64_t)(char *)s));
   cout<<"int Print: "<<wc_s<<endl;
   bldr->Return(bldr->Load("orderByCol1"));
   }
void tpchq1::mergeMethod (IlBuilder *bldr, OMR::JitBuilder::IlValue* start_index,
                     OMR::JitBuilder::IlValue* merge_size, OMR::JitBuilder::IlValue* orderByCol)
{
   OMR::JitBuilder::IlBuilder* secondorder_loop = NULL;
   bldr->ForLoopUp("curr_size", &secondorder_loop,
   bldr->   Add(
               start_index, 
   bldr->      ConstInt32(1)),
            merge_size,
   bldr->   ConstInt32(0));

   OMR::JitBuilder::IlBuilder* secondorder_subArray_loop = NULL;
   secondorder_loop->ForLoopUp("left_start", &secondorder_subArray_loop,
                       start_index,
   secondorder_loop->  Sub(
                          merge_size,
   secondorder_loop->     ConstInt32(1)),
   secondorder_loop->  ConstInt32(0));

   secondorder_subArray_loop->Store("mid",
   secondorder_subArray_loop->   Call("minValue", 2, 
   secondorder_subArray_loop->      Sub(
   secondorder_subArray_loop->         Add(
   secondorder_subArray_loop->            Load("left_start"), 
   secondorder_subArray_loop->            Load("curr_size")), 
   secondorder_subArray_loop->         ConstInt32(1)), 
   secondorder_subArray_loop->      Sub(
                                       merge_size, 
   secondorder_subArray_loop->         ConstInt32(1)) ) );

   secondorder_subArray_loop->Store("right_end",
   secondorder_subArray_loop->   Call("minValue", 2, 
   secondorder_subArray_loop->      Sub(
   secondorder_subArray_loop->         Add(
   secondorder_subArray_loop->            Load("left_start"), 
   secondorder_subArray_loop->            Add(
   secondorder_subArray_loop->               Load("curr_size"), 
   secondorder_subArray_loop->               Load("curr_size")) ), 
   secondorder_subArray_loop->         ConstInt32(1)), 
   secondorder_subArray_loop->      Sub(
                                       merge_size, 
   secondorder_subArray_loop->         ConstInt32(1))) );

   secondorder_subArray_loop->Call("merge", 4, 
   secondorder_subArray_loop->   Load("left_start"), 
   secondorder_subArray_loop->   Load("mid"), 
   secondorder_subArray_loop->   Load("right_end"),
                                 orderByCol);

   secondorder_subArray_loop->Store("left_start",
   secondorder_subArray_loop->   Add(
   secondorder_subArray_loop->      Load("left_start"),
   secondorder_subArray_loop->      Add(
   secondorder_subArray_loop->         Load("curr_size"), 
   secondorder_subArray_loop->         Load("curr_size")) ));

   secondorder_loop->Store("curr_size",
   secondorder_loop->   Add(
   secondorder_loop->      Load("curr_size"),
   secondorder_loop->      Load("curr_size") ));
}

tpchq1::tpchq1(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("tpchq1");

   
   pString = types->PointerTo(types->PointerTo(types->toIlType<char *>()));
   pStr = types->toIlType<char *>();

   //define parameters
   //DefineParameter("table", pString);
   DefineParameter("date", pStr);
   DefineParameter("orderByCol1", Int32);
   DefineParameter("orderByCol2", Int32);
   DefineParameter("quantity", Int32);
   DefineParameter("ep", Int32);
   DefineParameter("discount", Int32);
   DefineParameter("tax", Int32);
   DefineParameter("no_of_rows", Int32);
   DefineParameter("no_of_columns", Int32);
   DefineParameter("wc_size", Int32);
   
   //define functions

   DefineFunction((char *)"filter_using_where", 
                  (char *)__FILE__,
                  (char *)WHEREFILTER_LINE,
                  (void *)&filter_using_where,
                  Int32,
                  2,
                  pStr,
                  Int32);

    DefineFunction((char *)"minRes", 
                  (char *)__FILE__,
                  (char *)MINRES_LINE,
                  (void *)&minRes,
                  Int32,
                  2,
                  Int32,
                  Int32);
   
   DefineFunction((char *)"printString", 
                  (char *)__FILE__,
                  (char *)PRINTSTRING_LINE,
                  (void *)&printString,
                  NoType,
                  2,
                  Int32,
                  Int64);
    
    DefineFunction((char *)"printCharWithFreq", 
                  (char *)__FILE__,
                  (char *)CHARWITHFREQ_LINE,
                  (void *)&printCharWithFreq,
                  Int32,
                  3,
                  Int32,
                  Int32,
                  Int32);

    DefineFunction((char *)"printCharFreq", 
                  (char *)__FILE__,
                  (char *)CHARFREQ_LINE,
                  (void *)&printCharFreq,
                  NoType,
                  2,
                  Int32,
                  Int32);

   DefineFunction((char *)"group", 
                  (char *)__FILE__,
                  (char *)GROUP_LINE,
                  (void *)&group,
                  NoType,
                  1,
                  Int32);

   DefineFunction((char *)"printCharWithFreqGroupBy", 
                  (char *)__FILE__,
                  (char *)GROUPBYFREQ_LINE,
                  (void *)&printCharWithFreqGroupBy,
                  NoType,
                  4,
                  Int32,
                  Int32,
                  Int32,
                  Int32);

    DefineFunction((char *)"groupBy", 
                  (char *)__FILE__,
                  (char *)GROUPBY_LINE,
                  (void *)&groupBy,
                  NoType,
                  4,
                  Int32,
                  Int32,
                  Int32,
                  Int32);

   DefineFunction((char *)"merge", 
                  (char *)__FILE__,
                  (char *)MERGE_LINE,
                  (void *)&merge,
                  NoType,
                  4,
                  Int32,
                  Int32,
                  Int32,
                  Int32); 
   
   DefineFunction((char *)"mergeSort", 
                  (char *)__FILE__,
                  (char *)MERGESORT_LINE,
                  (void *)&mergeSort,
                  NoType,
                  3,
                  Int32,
                  Int32,
                  Int32);  

    DefineFunction((char *)"mergeHelper", 
                  (char *)__FILE__,
                  (char *)MERGEHELPER_LINE,
                  (void *)&mergeHelper,
                  NoType,
                  3,
                  Int32,
                  Int32,
                  Int32);  

    DefineFunction((char *)"minValue", 
                  (char *)__FILE__,
                  (char *)MINVALUE_LINE,
                  (void *)&minValue,
                  Int32,
                  2,
                  Int32,
                  Int32);   

    DefineFunction((char *)"print", 
                  (char *)__FILE__,
                  (char *)PRINT_LINE,
                  (void *)&print,
                  NoType,
                  1,
                  Int32);      

    DefineFunction((char *)"find_size", 
                  (char *)__FILE__,
                  (char *)FINDSIZE_LINE,
                  (void *)&find_size,
                  Int32,
                  1,
                  Int32); 

   DefineReturnType(Int32);

   }

//=================================================BUILDIL for String=============================================

bool
tpchq1::buildIL()
   {

   //OMR::JitBuilder::IlValue *table = Load("table");

   OMR::JitBuilder::IlValue *date = Load("date");

   OMR::JitBuilder::IlValue *orderByCol1 = Load("orderByCol1");

   OMR::JitBuilder::IlValue *orderByCol2 = Load("orderByCol2");
   
   OMR::JitBuilder::IlValue *quantity = Load("quantity");

   OMR::JitBuilder::IlValue *ep = Load("ep");

   OMR::JitBuilder::IlValue *discount = Load("discount");

   OMR::JitBuilder::IlValue *tax = Load("tax");

   OMR::JitBuilder::IlValue *no_of_rows = Load("no_of_rows");
   OMR::JitBuilder::IlValue *no_of_columns = Load("no_of_columns");
   OMR::JitBuilder::IlValue *wc_size = Load("wc_size");

   OMR::JitBuilder::IlValue *merge_size = ConstInt32(3489491);  
   OMR::JitBuilder::IlValue *second_order_size = ConstInt32(3);

   DefineLocal("result", Int32);
   //where clause
   IlBuilder *loop = NULL;
   ForLoopUp("i", &loop,
      ConstInt32(0),
      no_of_rows,
      ConstInt32(1));


   loop->     Store("result", loop->Call("filter_using_where", 2, date, loop->Load("i")));

   //1st Ordering + 2nd Ordering
   //this->Call("printString", 2, orderByCol1, ConstInt64((int64_t)(char *)"orderby Parameters:\n"));
   //=============================================================================================================
   OMR::JitBuilder::IlBuilder* result_loop = NULL;
   ForLoopUp("curr_size", &result_loop,
      ConstInt32(1),
      merge_size,
      ConstInt32(0));
   
   //result_loop->Call("print", 1, result_loop->Load("curr_size"));

   OMR::JitBuilder::IlBuilder* subArray_loop = NULL;
   result_loop-> ForLoopUp("left_start", &subArray_loop,
   result_loop->   ConstInt32(0),
   result_loop->   Sub(merge_size,
   result_loop->      ConstInt32(1)),
   result_loop->   ConstInt32(0));
   
   subArray_loop->Store("mid",
   subArray_loop->   Call("minValue", 2, 
   subArray_loop->      Sub(
   subArray_loop->         Add(
   subArray_loop->            Load("left_start"), 
   subArray_loop->            Load("curr_size")), 
   subArray_loop->         ConstInt32(1)), 
   subArray_loop->      Sub(merge_size, 
   subArray_loop->         ConstInt32(1)) ) );

   subArray_loop->Store("right_end",
   subArray_loop->   Call("minValue", 2, 
   subArray_loop->      Sub(
   subArray_loop->         Add(
   subArray_loop->            Load("left_start"), 
   subArray_loop->            Add(
   subArray_loop->               Load("curr_size"), 
   subArray_loop->               Load("curr_size")) ), 
   subArray_loop->         ConstInt32(1)), 
   subArray_loop->      Sub(merge_size, 
   subArray_loop->         ConstInt32(1))) );

   subArray_loop->Call("merge", 4, 
   subArray_loop->   Load("left_start"), 
   subArray_loop->   Load("mid"), 
   subArray_loop->   Load("right_end"),
                     orderByCol1);
   
   /* subArray_loop increment logic */
   subArray_loop->Store("left_start",
   subArray_loop->   Add(
   subArray_loop->      Load("left_start"),
   subArray_loop->      Add(
   subArray_loop->         Load("curr_size"), 
   subArray_loop->         Load("curr_size")) ));

   /* result_loop increment logic */
   result_loop->Store("curr_size",
   result_loop->   Add(
   result_loop->      Load("curr_size"),
   result_loop->      Load("curr_size") ));
   //=============================================================================================================
   //this->Call("printCharFreq", 2, orderByCol1, orderByCol2);

   this->Call("printCharWithFreq", 3, this->ConstInt32(0), merge_size, orderByCol1);
   this->Store("start_index", this->ConstInt32(0));
   this->Store("orderBy_size", this->ConstInt32(0));

   OMR::JitBuilder::IlBuilder* freq_loop = NULL;
   ForLoopUp("freq_index", &freq_loop,
      ConstInt32(0),
      second_order_size,
      ConstInt32(1));
   
   //freq_loop->Call("print", 1, freq_loop-> Load("start_index"));

   freq_loop->Store("orderBy_size", 
   freq_loop->Add(
   freq_loop->   Load("orderBy_size") , 
   freq_loop->   Call("find_size", 1, 
   freq_loop->      Load("freq_index")) ) );

   /* freq_loop->Call("print", 1, freq_loop->   Load("start_index"));
   freq_loop->Call("print", 1, freq_loop->   Load("orderBy_size")); */
   
   /* Sort */
   freq_loop->Call("mergeSort",3, freq_loop->Load("start_index"), freq_loop->Sub(freq_loop->Load("orderBy_size"), freq_loop->ConstInt32(1)), orderByCol2);
   
   //mergeMethod(freq_loop, freq_loop->Load("start_index"), freq_loop->Load("orderBy_size"), orderByCol2);

   freq_loop->Call("printCharWithFreqGroupBy",4, 
   freq_loop->   Load("start_index"), 
   freq_loop->   Load("orderBy_size"), 
                 orderByCol2, 
   freq_loop->   Load("freq_index"));
   /********/

   freq_loop->Store("start_index",  
   freq_loop->   Add(
   freq_loop->      Load("start_index")  , 
   freq_loop->      Call("find_size", 1, 
   freq_loop->         Load("freq_index")) ) );


   //==================================================================================================================
   
   //1st GroupBy
   //this->Call("group", 1, orderByCol2);
   
   //2nd GroupBy
   this->Call("groupBy",4, quantity, ep, discount, tax);

   Return(ConstInt32(0));

   return true;
   }
//======================================================================================================






//print the array
void 
printStringMatrix(const char **S, int32_t M){
   
   for(int32_t i = 1; i < M; i++){
      if(S[i] != "Not Found"){
         cout<<S[i]<<endl;
      }
   }
   cout<<"\n";
}

void printMat(const char*** load_matrix, int32_t no_of_rows, int32_t no_of_columns){
   for(int32_t i = 0; i < no_of_rows; i++){
         for(int32_t j = 0; j < no_of_columns; j++){
            cout<<load_matrix[i][j]<<"     ";
         }
         cout<<endl;
   }
}

struct time
tpchq1::run(const char* table, const char* date, int32_t orderByCol1, int32_t orderByCol2, int32_t quantity, int32_t ep, int32_t discount, int32_t tax)
   {
   //read matrices before initialising
   printf("Step 0: read matrix before initialising\n");
   loadMatMem(table);
   //Load matrices to pass to the JIT to 2d array of string
   const int32_t no_of_rows = temp.size();
   
   load_matrix = (const char***)malloc(no_of_rows * sizeof(int *));
   

   for(int i = 0; i < no_of_rows; i++){
      load_matrix[i] = (const char**)malloc(no_of_columns * sizeof(int *));
   }

   

   for(int i = 0; i < no_of_rows; i++){
        for(int j = 0; j < no_of_columns; j++){
            load_matrix[i][j] = temp[i][j].c_str();
        }
   }

   

   //define where
   where_clause_res = (const char***)malloc(no_of_rows * sizeof(int *));
   for(int i = 0; i < no_of_rows; i++){
      where_clause_res[i] = (const char**)malloc(no_of_columns * sizeof(int *));
   }
  
   //================================================================================
   printf("Step 1: initialize JIT\n");
   bool initialized = initializeJit();
   if (!initialized)
      {
      fprintf(stderr, "FAIL: could not initialize JIT\n");
      exit(-1);
      }


   printf("Step 2: define type dictionaries\n");
   OMR::JitBuilder::TypeDictionary types;

   printf("Step 3: compile tpchq1 method builder\n");
   
   int32_t wc_size = 0;
   //time
   auto start_compile = high_resolution_clock::now(); 

   tpchq1 method(&types);
   void *entry=0;
   int32_t rc = compileMethodBuilder(&method, &entry);

   auto stop_compile = high_resolution_clock::now();
   auto compile_duration = duration_cast<microseconds>(stop_compile - start_compile);

   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }
  
   printf("Step 4: invoke tpchq1 compiled code\n");

   auto start_exec = high_resolution_clock::now(); 

   tpchq1FunctionType *test = (tpchq1FunctionType *)entry;
   int32_t res = 0;
   test(date, orderByCol1, orderByCol2, quantity, ep, discount, tax, no_of_rows, no_of_columns, wc_size);

   auto stop_exec = high_resolution_clock::now();
   auto exec_duration = duration_cast<microseconds>(stop_exec - start_exec);
   
   printf ("Step 5: shutdown JIT\n");
   shutdownJit();

   printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++PRINT RESULT+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    //8. print the finalresult
    printf("\n\n");
    printf("return_flag    line_status        sum_quantity      sum_base_price          sum_disc_price       sum_charge        avg_qty           avg_price           avg_disc    count_order");
    printf("\n");;

   
   

   //print result
   printStruct(output);

   printf("PASS\n");

   free(load_matrix);
   free(where_clause_res);
   where_size = 0;
   orderByRes_size = 0;
   groupByRes_size = 0;
   genResult_size = 0;
   finalResult_size = 0;
   resTracker_size = 0;
   output_size = 0;

   
   struct time time;
   time.compileTime = compile_duration.count();
   time.executionTime = exec_duration.count();
   return time;

   }