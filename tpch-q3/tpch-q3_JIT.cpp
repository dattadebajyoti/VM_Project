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
#include "tpch-q3_JIT.hpp"
#include "readFile.cpp"


/* C++ libraries */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono> 
#include <vector>

using namespace std::chrono;

struct RESULT *result;
int total_result = 0;

struct GROUPRESULT *groupResult;
int32_t groupResult_size;

/**********************************************Helper functions*******************************************************************************/
void printResult()
{
    int32_t i;
    for(i = 0; i < total_result; i++){

        cout << setw(18) << left << result[i].l_orderkey<< " ";
        cout << setw(18) << left << result[i].l_extendedprice<< " ";
        cout << setw(18) << left << result[i].l_discount<< " ";
        cout << setw(18) << left << result[i].o_orderdate<< " ";
        cout << setw(18) << left << result[i].o_shippriority<< " ";

        cout<<endl;
    }
    cout<<"Total Rows: "<<total_result<<endl;
}


static char* substr(const char *src, int32_t m, int32_t n)
{
    #define SUBSTR_LINE LINETOSTR(__LINE__)

	int32_t len = n - m;
	char *dest = (char*)malloc(sizeof(char) * (len + 1));
	for (int32_t i = m; i < n && (*(src + i) != '\0'); i++)
	{
		*dest = *(src + i);
		dest++;
	}
	*dest = '\0';
	return dest - len;
}

/* return 0 for equal, 1 for smallerthan, 2 for greaterthan */
static int32_t compareDate(const char* d1, const char* d2) {
    #define COMPAREDATE_LINE LINETOSTR(__LINE__)

    int32_t year_d1  = atoi(substr(d1,0,4));
    int32_t month_d1 = atoi(substr(d1, 5, 7));
    int32_t day_d1   = atoi(substr(d1, 8, 10));
    //cout<<year_d1<<"  "<<month_d1<<"  "<<day_d1<<endl;

    int32_t year_d2  = atoi(substr(d2, 0, 4));
    int32_t month_d2 = atoi(substr(d2, 5, 7));;
    int32_t day_d2   = atoi(substr(d2, 8, 10));

    if (year_d1 < year_d2 || year_d1 <= year_d2 && month_d1 < month_d2 || year_d1 <= year_d2 && month_d1 <= month_d2 && day_d1 < day_d2) {
        return 1; //if d1 < d2
    }
    else if (year_d1 > year_d2 || year_d1 >= year_d2 && month_d1 > month_d2 || year_d1 >= year_d2 && month_d1 >= month_d2 && day_d1 > day_d2) {
        return 2; //if d1>d2
    }
    return 0; //if d1 == d2
}

/* merge function */
static void mergeRevenue(int32_t l, int32_t m, int32_t r) {
    #define MERGEREVENUE_LINE LINETOSTR(__LINE__)

    int32_t i, j, k;
    int32_t n1 = m - l + 1;
    int32_t n2 = r - m;

    struct GROUPRESULT *temp1 = (GROUPRESULT*)malloc(n1 * sizeof(GROUPRESULT));
    struct GROUPRESULT *temp2 = (GROUPRESULT*)malloc(n2 * sizeof(GROUPRESULT));

    for (i = 0; i < n1; i++) {
        temp1[i] = groupResult[l+i];
    }
    for (j = 0; j < n2; j++) {
        temp2[j] = groupResult[m + 1 + j];
    }
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (int(temp1[i].revenue) > int(temp2[j].revenue)) {
            groupResult[k] = temp1[i];
            i++;
        }
        else {
            groupResult[k] = temp2[j];
            j++;
        }
        k++;
    }
    while (i < n1) {
        groupResult[k] = temp1[i];
        i++;
        k++;
    }
    while (j < n2) {
        groupResult[k] = temp2[j];
        j++;
        k++;
    }
    free(temp1);
    free(temp2);
}

/* merge function */
static void merge(int32_t l, int32_t m, int32_t r) {
    #define MERGE_LINE LINETOSTR(__LINE__)

    int32_t i, j, k;
    int32_t n1 = m - l + 1;
    int32_t n2 = r - m;
    int32_t no_of_columns = 16;
    struct RESULT *temp1 = (RESULT*)malloc(n1 * sizeof(RESULT));
    struct RESULT *temp2 = (RESULT*)malloc(n2 * sizeof(RESULT));

    for (i = 0; i < n1; i++) {
        temp1[i] = result[l+i];
    }
    for (j = 0; j < n2; j++) {
        temp2[j] = result[m + 1 + j];
    }
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) {
        if (int(temp1[i].l_orderkey) < int(temp2[j].l_orderkey)) {
            result[k] = temp1[i];
            i++;
        }
        else {
            result[k] = temp2[j];
            j++;
        }
        k++;
    }
    while (i < n1) {
        result[k] = temp1[i];
        i++;
        k++;
    }
    while (j < n2) {
        result[k] = temp2[j];
        j++;
        k++;
    }
    free(temp1);
    free(temp2);
}

/* divide the result array into 2 equal parts */
void mergeSort(int32_t l, int32_t r) {
    if (l < r) {
        int32_t m = l + (r - l) / 2;
        mergeSort(l, m);
        mergeSort(m + 1, r);
        merge(l, m, r);
    }
}


static void printString(int32_t x)
{
   #define PRINTSTRING_LINE LINETOSTR(__LINE__)
   cout<<"x is: "<<x<<endl;
}

/* match string helper helper */
static int32_t matchString(const char* s1, const char* s2)
{
   #define MATCHSTRING_LINE LINETOSTR(__LINE__)
   if( (strcmp(s1, s2) == 0)){
       return 1;
   }
   else{
       return 0;
   }
}

/* swap */
static void swapResult(int32_t &val1, int32_t &val2)
{
   #define SWAP_LINE LINETOSTR(__LINE__)
   struct RESULT temp = result[val1];
   result[val1] = result[val2];
   result[val2] = temp;
} 

static void groupBy2nd_3rd()
{
   #define GROUPBY_LINE LINETOSTR(__LINE__)
   
   int32_t groupBy1_size = 0;
   int32_t groupBy2_size = 0;
   int32_t groupBy3_size = 0;

   int32_t group2_curr_size = 0;
   int32_t group3_curr_size = 0;
   
   int32_t group1_count;
   /**************************GroupBy1 o_orderkey********************************************/
   int32_t ijk = 0;
   int32_t r_index = 0;
   for(; r_index < total_result;){
       //cout<<"r_index: "<<r_index<<endl;
       //g[groupBy1_size].groupBy_value = result[r_index].l_orderkey;
       /* g[groupBy1_size].size = 1; */
       group1_count = 1;

       int32_t inner = r_index+1;
       for(; inner < total_result; inner++){
           //cout<<"inner: "<<inner<<endl;
           if(result[r_index].l_orderkey == result[inner].l_orderkey){
               /* g[groupBy1_size].size++; */
               group1_count++;
           }
           else{
               break;
           }
       }
       /***************************sort according to the groupBy****************************/
       
       group2_curr_size += /* g[groupBy1_size].size */group1_count;
       int32_t i_sort;
       for(i_sort  = r_index; i_sort < group2_curr_size; i_sort++){
           int32_t j_sort;
           for(j_sort = i_sort + 1; j_sort < group2_curr_size; j_sort++){
               if(compareDate(result[i_sort].o_orderdate, result[j_sort].o_orderdate) == 2){
                   struct RESULT temp = result[i_sort];
                   result[i_sort] = result[j_sort];
                   result[j_sort] = temp;
               }
           }
       }
       /********************************sorting complete*******************************************************/
       int32_t group2_count;
       /**************************GroupBy2 o_orderdate********************************************/
       int32_t r1_index = r_index;
       for(; r1_index < group2_curr_size;){
           
           //g2[groupBy2_size].groupBy_value = result[r1_index].o_orderdate;
           /* g2[groupBy2_size].size = 1; */
           group2_count = 1;

           int32_t inner2 = r1_index+1;
           for(; inner2 < group2_curr_size; inner2++){
               
               if(compareDate(result[r1_index].o_orderdate, result[inner2].o_orderdate) == 0){
                   /* g2[groupBy2_size].size++; */
                   group2_count++;
               }
               else{
                   break;
               }
           }
           

           /**************************GroupBy3 o_shippriority********************************************/
           group3_curr_size += /* g2[groupBy2_size].size */group2_count;
           int32_t i2_sort;
           for(i2_sort = r1_index; i2_sort < group3_curr_size; i2_sort++){
               int32_t j2_sort;
               for(j2_sort = i2_sort + 1; j2_sort < group3_curr_size; j2_sort++){
                   if(result[i2_sort].o_shippriority > result[j2_sort].o_shippriority){
                      struct RESULT temp = result[i_sort];
                      result[i2_sort] = result[j2_sort];
                      result[j2_sort] = temp;
                   }
               }
           }
           
           int32_t group3_count;
           int32_t r2_index = r1_index;
           for(; r2_index < group3_curr_size; ){
               
               /* g3[groupBy3_size].groupBy_value = result[r2_index].o_shippriority;
               g3[groupBy3_size].size = 1; */
               group3_count = 1;

               int32_t inner3 = r2_index+1;

               float sum = result[r2_index].l_extendedprice * (1 - result[r2_index].l_discount);

               for(; inner3 < group3_curr_size; inner3++){
                   if(result[r2_index].o_shippriority == result[inner3].o_shippriority){
                       sum = sum + (result[inner3].l_extendedprice * (1 - result[inner3].l_discount));
                       /* g3[groupBy3_size].size++; */
                       group3_count++;
                   }
                   else{
                      break;
                   }
                }
                groupResult[groupResult_size].l_orderkey = result[r2_index].l_orderkey;
                groupResult[groupResult_size].revenue = sum;
                groupResult[groupResult_size].o_orderdate = result[r2_index].o_orderdate;
                groupResult[groupResult_size].o_shippriority = result[r2_index].o_shippriority;
                groupResult_size++;

                groupBy3_size++;
                r2_index = inner3;
           }

           /**************************GroupBy3 complete**************************************************/
           
           groupBy2_size++;
           r1_index = inner2;
       }

       /*************************************GroupBy2 complete**************************************************/
       groupBy1_size++;
       r_index = inner;
       /*************************************GroupBy1 complete**************************************************/
   }
   
}

static void orderBy2nd_3rd()
{
   #define ORDERBY_LINE LINETOSTR(__LINE__)
   /* Second ordering */
   int32_t i_2ndOrder;

   int32_t end_2ndorder = 0;
   for(i_2ndOrder = 0; i_2ndOrder < groupResult_size; ){
       end_2ndorder++;
       int32_t j_2ndOrder;
       for(j_2ndOrder = i_2ndOrder+1; j_2ndOrder < groupResult_size; j_2ndOrder++){

           if(groupResult[i_2ndOrder].revenue == groupResult[j_2ndOrder].revenue){
               end_2ndorder++;
           }
       }
       
       int32_t i_2ndsort;
       for(i_2ndsort = i_2ndOrder; i_2ndsort < end_2ndorder; i_2ndsort++){
           int32_t j_2ndsort;
           for(j_2ndsort = i_2ndsort+1; j_2ndsort < end_2ndorder; j_2ndsort++){
               if(compareDate(groupResult[i_2ndsort].o_orderdate, groupResult[j_2ndsort].o_orderdate) == 2){
                   struct GROUPRESULT temp = groupResult[i_2ndsort];
                   groupResult[i_2ndsort] = groupResult[j_2ndsort];
                   groupResult[j_2ndsort] = temp;
               }
           }
       }

       i_2ndOrder = end_2ndorder;
   }
}

/* store helper */
static void storeResult(int32_t orderkey, float extendedprice, float discount, const char* orderdate, int32_t shippriority)
{
   #define STORERESULT_LINE LINETOSTR(__LINE__)

   result[total_result].l_orderkey      = orderkey;
   result[total_result].l_extendedprice = extendedprice;
   result[total_result].l_discount      = discount;
   result[total_result].o_orderdate     = orderdate;
   result[total_result].o_shippriority  = shippriority;

   total_result++;
   
}
/****************************************Define the method builder object********************************************************/

tpchq3::tpchq3(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("tpchq3");

   pStr = types->toIlType<char *>();

   StructTypeCUSTOMER      = types->LookupStruct("CUSTOMER");
   pStructTypeCUSTOMER     = types->PointerTo(StructTypeCUSTOMER);
   
   StructTypeORDERS        = types->LookupStruct("ORDERS");
   pStructTypeORDERS       = types->PointerTo(StructTypeORDERS);

   StructTypeLINEITEM      = types->LookupStruct("LINEITEM");
   pStructTypeLINEITEM     = types->PointerTo(StructTypeLINEITEM);
   
   /* define structs */
   DefineParameter("c",  pStructTypeCUSTOMER);
   DefineParameter("o",  pStructTypeORDERS);
   DefineParameter("l",  pStructTypeLINEITEM);
  
   DefineParameter("c_size", Int32);
   DefineParameter("o_size", Int32);
   DefineParameter("l_size", Int32);

   DefineParameter("c_mktsegment",  pStr);
   DefineParameter("o_orderdate",   pStr);
   DefineParameter("l_shipdate",    pStr);

   DefineFunction((char *)"printString", 
                  (char *)__FILE__,
                  (char *)PRINTSTRING_LINE,
                  (void *)&printString,
                  NoType,
                  1,
                  Int32);

    DefineFunction((char *)"compareDate", 
                  (char *)__FILE__,
                  (char *)COMPAREDATE_LINE,
                  (void *)&compareDate,
                  Int32,
                  2,
                  pStr,
                  pStr);

    DefineFunction((char *)"substr", 
                  (char *)__FILE__,
                  (char *)SUBSTR_LINE,
                  (void *)&substr,
                  pStr,
                  3,
                  pStr,
                  Int32,
                  Int32);

    DefineFunction((char *)"matchString", 
                  (char *)__FILE__,
                  (char *)MATCHSTRING_LINE,
                  (void *)&matchString,
                  Int32,
                  2,
                  pStr,
                  pStr);

    DefineFunction((char *)"storeResult", 
                  (char *)__FILE__,
                  (char *)STORERESULT_LINE,
                  (void *)&storeResult,
                  NoType,
                  5,
                  Int32,
                  Float,
                  Float,
                  pStr,
                  Int32);
   
   /* Define Return type */
   DefineReturnType(NoType);

   }

/******************************Define struct***************************************************/
class StructArrayTypeDictionary : public OMR::JitBuilder::TypeDictionary
   {
   public:
   StructArrayTypeDictionary() :
      OMR::JitBuilder::TypeDictionary()
      {

      /* Declare CUSTOMER table */
      DEFINE_STRUCT(CUSTOMER);

      DEFINE_FIELD(CUSTOMER, custkey,    Int32);
      DEFINE_FIELD(CUSTOMER, name,       toIlType<const char *>());
      DEFINE_FIELD(CUSTOMER, address,    toIlType<const char *>());
      DEFINE_FIELD(CUSTOMER, nationkey,  Int32);
      DEFINE_FIELD(CUSTOMER, phone,      toIlType<const char *>());
      DEFINE_FIELD(CUSTOMER, acctbal,    Float);
      DEFINE_FIELD(CUSTOMER, mktsegment, toIlType<const char *>());
      DEFINE_FIELD(CUSTOMER, comment,    toIlType<const char *>());

      CLOSE_STRUCT(CUSTOMER);

      /* Declare ORDERS table */
      DEFINE_STRUCT(ORDERS);

      DEFINE_FIELD(ORDERS, orderkey,      Int32);
      DEFINE_FIELD(ORDERS, custkey,       Int32);
      DEFINE_FIELD(ORDERS, orderStatus,   toIlType<const char *>());
      DEFINE_FIELD(ORDERS, totalprice,    Float);
      DEFINE_FIELD(ORDERS, orderdate,     toIlType<const char *>());
      DEFINE_FIELD(ORDERS, orderpriority, toIlType<const char *>());
      DEFINE_FIELD(ORDERS, clerk,         toIlType<const char *>());
      DEFINE_FIELD(ORDERS, shippriority,  Int32);
      DEFINE_FIELD(ORDERS, comment,       toIlType<const char *>());

      CLOSE_STRUCT(ORDERS);

      /* Declare LINEITEM table */
      DEFINE_STRUCT(LINEITEM);

      DEFINE_FIELD(LINEITEM, orderkey,      Int32);
      DEFINE_FIELD(LINEITEM, partkey,       Int32);
      DEFINE_FIELD(LINEITEM, suppkey,       Int32);
      DEFINE_FIELD(LINEITEM, linenumber,    Int32);
      DEFINE_FIELD(LINEITEM, quantity,      Int32);
      DEFINE_FIELD(LINEITEM, extendedprice, Float);
      DEFINE_FIELD(LINEITEM, discount,      Float);
      DEFINE_FIELD(LINEITEM, tax,           Float);
      DEFINE_FIELD(LINEITEM, returnflag,    toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, linestatus,    toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, shipdate,      toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, commitdate,    toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, receiptdate,   toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, shipinstruct,  toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, shipmode,      toIlType<const char *>());
      DEFINE_FIELD(LINEITEM, comment,       toIlType<const char *>());

      CLOSE_STRUCT(LINEITEM);
      
      
      }
      
   };

void tpchq3::mergeSort(IlBuilder *b, OMR::JitBuilder::IlValue *left, OMR::JitBuilder::IlValue *right)
{
   IlBuilder *match=NULL;
   b->IfThen(&match, b->LessThan(left, right));
   match->Store("mid", 
   match->   Add(left, 
   match->      Div(
   match->         Sub(right, left), 
   match->         ConstInt32(2))));
   mergeSort(match, left, 
   match->   Load("mid"));

   mergeSort(match, 
   match->Add(
   match->   Load("mid"), 
   match->   ConstInt32(1)), right);

   match->Call("merge", 3, left, 
   match->   Load("mid"), right);
}

void tpchq3::matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt32(0));
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->EqualTo(one, two));
   twoSmaller->Store(resultName, twoSmaller->ConstInt32(1));
}
/******************************BUILDIL**********************************************/

bool
tpchq3::buildIL()
   {
   
   OMR::JitBuilder::IlValue *customer_size = Load("c_size");
   OMR::JitBuilder::IlValue *orders_size = Load("o_size");
   OMR::JitBuilder::IlValue *lineitem_size = Load("l_size");
   
   OMR::JitBuilder::IlBuilder* c_loop = NULL;
   ForLoopUp("c_index", &c_loop,
      ConstInt32(0),
      customer_size,
      ConstInt32(1));

   c_loop->Store("c_element",
   c_loop->   IndexAt(pStructTypeCUSTOMER,
   c_loop->      Load("c"),
   c_loop->      Load("c_index")));
   
   c_loop->Store("res_mktsegment",
   c_loop->   Call("matchString", 2, 
   c_loop->      LoadIndirect("CUSTOMER", "mktsegment",
   c_loop->         Load("c_element")), 
   c_loop->      Load("c_mktsegment")) );
   //c_loop->Call("printString", 1, c_loop->Load("res_mktsegment"));

   OMR::JitBuilder::IlBuilder *mktsegment_Match = NULL;
   c_loop->IfThen(&mktsegment_Match,
   c_loop->   EqualTo(
   c_loop->      Load("res_mktsegment"),
   c_loop->      ConstInt32(1)));

   OMR::JitBuilder::IlBuilder* o_loop = NULL;
   mktsegment_Match->ForLoopUp("o_index", &o_loop,
   mktsegment_Match->   ConstInt32(0),
                        orders_size,
   mktsegment_Match->   ConstInt32(1));

   o_loop->Store("o_element",
   o_loop->   IndexAt(pStructTypeORDERS,
   o_loop->      Load("o"),
   o_loop->      Load("o_index")));

   matchkeyJIT(o_loop, "matchCustKey", 
   o_loop->   LoadIndirect("CUSTOMER", "custkey",
   o_loop->      Load("c_element")),
   o_loop->   LoadIndirect("ORDERS", "custkey",
   o_loop->      Load("o_element")));

   OMR::JitBuilder::IlBuilder *custkey_Match = NULL;
   o_loop->IfThen(&custkey_Match,
   o_loop->   EqualTo(
   o_loop->      Load("matchCustKey"),
   o_loop->      ConstInt32(1)));

   custkey_Match->Store("matchOrderdate", 
   custkey_Match->   Call("compareDate", 2,
   custkey_Match->      LoadIndirect("ORDERS", "orderdate",
   custkey_Match->         Load("o_element")),
   custkey_Match->      Load("o_orderdate")));

   OMR::JitBuilder::IlBuilder *orderdate_Match = NULL;
   custkey_Match->IfThen(&orderdate_Match,
   custkey_Match->   EqualTo(
   custkey_Match->      Load("matchOrderdate"),
   custkey_Match->      ConstInt32(1)));
   //custkey_Match->Call("printString", 1, custkey_Match->Load("matchOrderdate"));

   OMR::JitBuilder::IlBuilder* l_loop = NULL;
   orderdate_Match->ForLoopUp("l_index", &l_loop,
   orderdate_Match->   ConstInt32(0),
                       lineitem_size,
   orderdate_Match->   ConstInt32(1));

   l_loop->Store("l_element",
   l_loop->   IndexAt(pStructTypeLINEITEM,
   l_loop->      Load("l"),
   l_loop->      Load("l_index")));

   matchkeyJIT(l_loop, "matchOrderkey", 
   l_loop->   LoadIndirect("LINEITEM", "orderkey",
   l_loop->      Load("l_element")),
   l_loop->   LoadIndirect("ORDERS", "orderkey",
   l_loop->      Load("o_element")));

   OMR::JitBuilder::IlBuilder *orderkey_Match = NULL;
   l_loop->IfThen(&orderkey_Match,
   l_loop->   EqualTo(
   l_loop->      Load("matchOrderkey"),
   l_loop->      ConstInt32(1)));

   orderkey_Match->Store("matchShipdate", 
   orderkey_Match->   Call("compareDate", 2,
   orderkey_Match->      LoadIndirect("LINEITEM", "shipdate",
   orderkey_Match->         Load("l_element")),
   orderkey_Match->      Load("l_shipdate")));

   OMR::JitBuilder::IlBuilder *shipdate_Match = NULL;
   orderkey_Match->IfThen(&shipdate_Match,
   orderkey_Match->   EqualTo(
   orderkey_Match->      Load("matchShipdate"),
   orderkey_Match->      ConstInt32(2)));
   
  
   shipdate_Match->Call("storeResult", 5,
   shipdate_Match->   LoadIndirect("LINEITEM", "orderkey",
   shipdate_Match->      Load("l_element")),
   shipdate_Match->   LoadIndirect("LINEITEM", "extendedprice",
   shipdate_Match->      Load("l_element")),
   shipdate_Match->   LoadIndirect("LINEITEM", "discount",
   shipdate_Match->      Load("l_element")),
   shipdate_Match->   LoadIndirect("ORDERS",   "orderdate",
   shipdate_Match->      Load("o_element")),
   shipdate_Match->   LoadIndirect("ORDERS",   "shippriority",
   shipdate_Match->      Load("o_element")) );
   
   Return(ConstInt32(0));

   return true;
   }


groupBy::groupBy(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("groupBy");

   pStr = types->toIlType<char *>();

   StructTypeRESULT      = types->LookupStruct("RESULT");
   pStructTypeRESULT     = types->PointerTo(StructTypeRESULT);
   
   StructTypeGROUPRESULT        = types->LookupStruct("GROUPRESULT");
   pStructTypeGROUPRESULT       = types->PointerTo(StructTypeGROUPRESULT);
   
   /* define structs */
   DefineParameter("r",        pStructTypeRESULT);
   DefineParameter("group_r",  pStructTypeGROUPRESULT);
  
   DefineParameter("r_size",       Int32);
   DefineParameter("group_r_size", Int32);

   DefineFunction((char *)"printString", 
                  (char *)__FILE__,
                  (char *)PRINTSTRING_LINE,
                  (void *)&printString,
                  NoType,
                  1,
                  Int32);

    DefineFunction((char *)"merge", 
                  (char *)__FILE__,
                  (char *)MERGE_LINE,
                  (void *)&merge,
                  NoType,
                  3,
                  Int32,
                  Int32,
                  Int32);

    DefineFunction((char *)"compareDate", 
                  (char *)__FILE__,
                  (char *)COMPAREDATE_LINE,
                  (void *)&compareDate,
                  Int32,
                  2,
                  pStr,
                  pStr);

    DefineFunction((char *)"swapResult", 
                  (char *)__FILE__,
                  (char *)SWAP_LINE,
                  (void *)&swapResult,
                  NoType,
                  2,
                  Int32,
                  Int32);

    DefineFunction((char *)"groupBy2nd_3rd", 
                  (char *)__FILE__,
                  (char *)GROUPBY_LINE,
                  (void *)&groupBy2nd_3rd,
                  NoType,
                  0);

   
   /* Define Return type */
   DefineReturnType(NoType);

   }

/* Define result type structs */
class resultTypeDictionary : public OMR::JitBuilder::TypeDictionary
   {
   public:
   resultTypeDictionary() :
      OMR::JitBuilder::TypeDictionary()
      {
      /* Declare RESULT Table */
      DEFINE_STRUCT(RESULT);

      DEFINE_FIELD(RESULT, l_orderkey, Int32);
      DEFINE_FIELD(RESULT, l_extendedprice, Float);
      DEFINE_FIELD(RESULT, l_discount, Float);
      DEFINE_FIELD(RESULT, o_orderdate, toIlType<const char *>());
      DEFINE_FIELD(RESULT, o_shippriority, Int32);

      CLOSE_STRUCT(RESULT);
      
      DEFINE_STRUCT(GROUPRESULT);

      DEFINE_FIELD(GROUPRESULT, l_orderkey, Int32);
      DEFINE_FIELD(GROUPRESULT, revenue, Float);
      DEFINE_FIELD(GROUPRESULT, o_orderdate, toIlType<const char *>());
      DEFINE_FIELD(GROUPRESULT, o_shippriority, Int32);

      CLOSE_STRUCT(GROUPRESULT);


      }
   };


void groupBy::generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, one);
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->LessThan(two, one));
   twoSmaller->Store(resultName, two);
}

void groupBy::matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt32(0));
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->EqualTo(one, two));
   twoSmaller->Store(resultName, twoSmaller->ConstInt32(1));
}

bool
groupBy::buildIL()
   {
   
   OMR::JitBuilder::IlValue *r_size = Load("r_size");
   //OMR::JitBuilder::IlValue *group_r_size = Load("group_r_size");

   OMR::JitBuilder::IlBuilder* result_loop = NULL;
   ForLoopUp("curr_size", &result_loop,
      ConstInt32(1),
      r_size,
      ConstInt32(0));

   OMR::JitBuilder::IlBuilder* subArray_loop = NULL;
   result_loop-> ForLoopUp("left_start", &subArray_loop,
   result_loop->   ConstInt32(0),
   result_loop->   Sub(r_size,
   result_loop->      ConstInt32(1)),
   result_loop->   ConstInt32(0));

   generateMinValue(subArray_loop, "mid",
   subArray_loop-> Sub(
   subArray_loop->    Add(
   subArray_loop->       Load("left_start"),
   subArray_loop->       Load("curr_size")),
   subArray_loop->    ConstInt32(1)),
   subArray_loop-> Sub(r_size,
   subArray_loop->    ConstInt32(1)) );
   
   generateMinValue(subArray_loop, "right_end",
   subArray_loop-> Sub(
   subArray_loop->    Add(
   subArray_loop->       Load("left_start"), 
   subArray_loop->       Add(
   subArray_loop->          Load("curr_size"), 
   subArray_loop->          Load("curr_size")) ), 
   subArray_loop->       ConstInt32(1)),
   subArray_loop-> Sub(r_size,
   subArray_loop->    ConstInt32(1)) );

   subArray_loop->Call("merge", 3, 
   subArray_loop->   Load("left_start"), 
   subArray_loop->   Load("mid"), 
   subArray_loop->   Load("right_end"));

   subArray_loop->Store("left_start",
   subArray_loop->   Add(
   subArray_loop->      Load("left_start"),
   subArray_loop->      Add(
   subArray_loop->         Load("curr_size"), 
   subArray_loop->         Load("curr_size")) ));

   result_loop->Store("curr_size",
   result_loop->   Add(
   result_loop->      Load("curr_size"),
   result_loop->      Load("curr_size") ));
   
   /******************************Start groupby 1+2+3 ***************************************/
   Call("groupBy2nd_3rd", 0);
  
   Return(ConstInt32(0));

   return true;
   }

orderBy::orderBy(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("orderBy");

   pStr = types->toIlType<char *>();

   StructTypeRESULT      = types->LookupStruct("RESULT");
   pStructTypeRESULT     = types->PointerTo(StructTypeRESULT);
   
   StructTypeGROUPRESULT        = types->LookupStruct("GROUPRESULT");
   pStructTypeGROUPRESULT       = types->PointerTo(StructTypeGROUPRESULT);
   
   /* define structs */
   DefineParameter("r",        pStructTypeRESULT);
   DefineParameter("group_r",  pStructTypeGROUPRESULT);
  
   DefineParameter("r_size",       Int32);
   DefineParameter("group_r_size", Int32);

   DefineFunction((char *)"printString", 
                  (char *)__FILE__,
                  (char *)PRINTSTRING_LINE,
                  (void *)&printString,
                  NoType,
                  1,
                  Int32);

    DefineFunction((char *)"mergeRevenue", 
                  (char *)__FILE__,
                  (char *)MERGEREVENUE_LINE,
                  (void *)&mergeRevenue,
                  NoType,
                  3,
                  Int32,
                  Int32,
                  Int32);

    DefineFunction((char *)"orderBy2nd_3rd", 
                  (char *)__FILE__,
                  (char *)ORDERBY_LINE,
                  (void *)&orderBy2nd_3rd,
                  NoType,
                  0);

   
   /* Define Return type */
   DefineReturnType(NoType);

   }

void orderBy::generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, one);
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->LessThan(two, one));
   twoSmaller->Store(resultName, two);
}

void orderBy::matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt32(0));
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->EqualTo(one, two));
   twoSmaller->Store(resultName, twoSmaller->ConstInt32(1));
}

bool
orderBy::buildIL()
   {
   
   //OMR::JitBuilder::IlValue *r_size = Load("r_size");
   OMR::JitBuilder::IlValue *group_r_size = Load("group_r_size");

   OMR::JitBuilder::IlBuilder* result_loop = NULL;
   ForLoopUp("curr_size", &result_loop,
      ConstInt32(1),
      group_r_size,
      ConstInt32(0));

   OMR::JitBuilder::IlBuilder* subArray_loop = NULL;
   result_loop-> ForLoopUp("left_start", &subArray_loop,
   result_loop->   ConstInt32(0),
   result_loop->   Sub(group_r_size,
   result_loop->      ConstInt32(1)),
   result_loop->   ConstInt32(0));

   generateMinValue(subArray_loop, "mid",
   subArray_loop-> Sub(
   subArray_loop->    Add(
   subArray_loop->       Load("left_start"),
   subArray_loop->       Load("curr_size")),
   subArray_loop->    ConstInt32(1)),
   subArray_loop-> Sub(group_r_size,
   subArray_loop->    ConstInt32(1)) );
   
   generateMinValue(subArray_loop, "right_end",
   subArray_loop-> Sub(
   subArray_loop->    Add(
   subArray_loop->       Load("left_start"), 
   subArray_loop->       Add(
   subArray_loop->          Load("curr_size"), 
   subArray_loop->          Load("curr_size")) ), 
   subArray_loop->       ConstInt32(1)),
   subArray_loop-> Sub(group_r_size,
   subArray_loop->    ConstInt32(1)) );

   subArray_loop->Call("mergeRevenue", 3, 
   subArray_loop->   Load("left_start"), 
   subArray_loop->   Load("mid"), 
   subArray_loop->   Load("right_end"));

   subArray_loop->Store("left_start",
   subArray_loop->   Add(
   subArray_loop->      Load("left_start"),
   subArray_loop->      Add(
   subArray_loop->         Load("curr_size"), 
   subArray_loop->         Load("curr_size")) ));

   result_loop->Store("curr_size",
   result_loop->   Add(
   result_loop->      Load("curr_size"),
   result_loop->      Load("curr_size") ));
   
   /******************************Start groupby 1+2+3 ***************************************/
   Call("orderBy2nd_3rd", 0);
  
   Return(ConstInt32(0));

   return true;
   }

/**************************************************Entry for exectution*************************************************/
struct time
tpchq3::run(const char* customer, const char* orders, const char* lineitem, const char* c_mktsegment, const char* o_orderdate, 
               const char* l_shipdate)
{
   /* 1. Load the tables into the memory -- Time not considered for loading tables in memory*/
   matrix customerTemp, ordersTemp, lineitemTemp;
   int32_t i, j;

   customerTemp = readFile(customer);
   /* Declare PART table array */
   struct CUSTOMER *c = (CUSTOMER*)malloc(customerTemp.size() * sizeof(CUSTOMER)); 
   int32_t customer_size = customerTemp.size();
   for(i = 0; i < customer_size; i++){
       j = 0;
       
       c[i].custkey     = atoi(customerTemp[i][j++].c_str());
       c[i].name        = customerTemp[i][j++].c_str();
       c[i].address     = customerTemp[i][j++].c_str();
       c[i].nationkey   = atoi(customerTemp[i][j++].c_str());
       c[i].phone       = customerTemp[i][j++].c_str();
       c[i].acctbal     = atof(customerTemp[i][j++].c_str());
       c[i].mktsegment  = customerTemp[i][j++].c_str();
       c[i].comment     = customerTemp[i][j++].c_str();

       j = 0;
   }
   
   ordersTemp = readFile(orders);
   /* Declare PART table array */
   struct ORDERS *o = (ORDERS*)malloc(ordersTemp.size() * sizeof(ORDERS));
   int32_t orders_size = ordersTemp.size();
   for(i = 0; i < orders_size; i++){
       j = 0;

       o[i].orderkey       = atoi(ordersTemp[i][j++].c_str());
       o[i].custkey        = atoi(ordersTemp[i][j++].c_str());
       o[i].orderStatus    = ordersTemp[i][j++].c_str();
       o[i].totalprice     = atof(ordersTemp[i][j++].c_str());;
       o[i].orderdate      = ordersTemp[i][j++].c_str();;
       o[i].orderpriority  = ordersTemp[i][j++].c_str();;
       o[i].clerk          = ordersTemp[i][j++].c_str();;
       o[i].shippriority   = atoi(ordersTemp[i][j++].c_str());
       o[i].comment        = ordersTemp[i][j++].c_str();;

       j = 0;
   }

   lineitemTemp = readFile(lineitem);
   /* Declare PART table array */
   struct LINEITEM *l = (LINEITEM*)malloc(lineitemTemp.size() * sizeof(LINEITEM));
   int32_t lineitem_size = lineitemTemp.size();
   for(i = 0; i < lineitem_size; i++){
       j = 0;

       l[i].orderkey        = atoi(lineitemTemp[i][j++].c_str());
       l[i].partkey         = atoi(lineitemTemp[i][j++].c_str());
       l[i].suppkey         = atoi(lineitemTemp[i][j++].c_str());
       l[i].linenumber      = atoi(lineitemTemp[i][j++].c_str());
       l[i].quantity        = atoi(lineitemTemp[i][j++].c_str());
       l[i].extendedprice   = atof(lineitemTemp[i][j++].c_str());
       l[i].discount        = atof(lineitemTemp[i][j++].c_str());
       l[i].tax             = atof(lineitemTemp[i][j++].c_str());
       l[i].returnflag      = lineitemTemp[i][j++].c_str();
       l[i].linestatus      = lineitemTemp[i][j++].c_str();
       l[i].shipdate        = lineitemTemp[i][j++].c_str();
       l[i].commitdate      = lineitemTemp[i][j++].c_str();
       l[i].receiptdate     = lineitemTemp[i][j++].c_str();
       l[i].shipinstruct    = lineitemTemp[i][j++].c_str();
       l[i].shipmode        = lineitemTemp[i][j++].c_str();
       l[i].comment         = lineitemTemp[i][j++].c_str();

       j = 0;
   }

   result = (RESULT*)malloc(lineitem_size * sizeof(RESULT));
   
   /*********************************Using JIT API***************************************************/  

   printf("Step 1: initialize JIT\n");
   bool initialized = initializeJit();
   if (!initialized)
      {
      fprintf(stderr, "FAIL: could not initialize JIT\n");
      exit(-1);
      }

   printf("Step 2: define type dictionaries\n");
   StructArrayTypeDictionary joinmethodTypes;

   /* result type dictionary */
   resultTypeDictionary groupMethodTypes;

   resultTypeDictionary orderMethodTypes;

   printf("Step 3: compile tpchq3 method builder\n");

   /*******************compile tpchq2*********************/

   auto join_start_compile = high_resolution_clock::now(); 

   tpchq3 method(&joinmethodTypes);
   void *entry=0;
   int32_t rc = compileMethodBuilder(&method, &entry);

   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   auto join_stop_compile = high_resolution_clock::now();
   auto join_compile_duration = duration_cast<microseconds>(join_stop_compile - join_start_compile);

   /*******************tpchq3 compilation done**********************/
   printf("Step 4: compile groupBy method builder\n");

   /* Compile groupBy */
   auto start_compile_groupBy = high_resolution_clock::now(); 
   
   groupBy groupByMethod(&groupMethodTypes);
   void *groupEntry = 0;
   rc = compileMethodBuilder(&groupByMethod, &groupEntry);
   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   auto stop_compile_groupBy = high_resolution_clock::now();
   auto compile_duration_groupBy = duration_cast<microseconds>(stop_compile_groupBy - start_compile_groupBy);
   /* GroupBy compile complete */

   printf("Step 5: compile groupBy method builder\n");

   /* Compile orderBy */
   auto start_compile_orderBy = high_resolution_clock::now(); 
   
   orderBy orderByMethod(&orderMethodTypes);
   void *orderEntry = 0;
   rc = compileMethodBuilder(&orderByMethod, &orderEntry);
   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   auto stop_compile_orderBy = high_resolution_clock::now();
   auto compile_duration_orderBy = duration_cast<microseconds>(stop_compile_orderBy - start_compile_orderBy);
   /* orderBy compile complete */

   /*******************invoke**************************************/
   printf("Step 6: invoke tpchq3 compiled code\n");

   auto join_start_exec = high_resolution_clock::now(); 

   tpchq3FunctionType *join = (tpchq3FunctionType *)entry;
   //join(c, o, l, customer_size, orders_size, lineitem_size);
   join(c, o, l, customer_size, orders_size, lineitem_size, c_mktsegment, o_orderdate, l_shipdate);
   
   auto join_stop_exec = high_resolution_clock::now();
   auto join_exec_duration = duration_cast<microseconds>(join_stop_exec - join_start_exec);

   printf("Step 7: invoke groupBy compiled code\n");

   groupResult = (GROUPRESULT*) malloc(total_result * sizeof(GROUPRESULT));
   groupResult_size = 0;

   auto groupBy_start_exec = high_resolution_clock::now(); 

   groupByFunctionType *groupBy = (groupByFunctionType *)groupEntry;
   groupBy(result, groupResult, total_result, groupResult_size);

   auto groupBy_stop_exec = high_resolution_clock::now();
   auto groupBy_exec_duration = duration_cast<microseconds>(groupBy_stop_exec - groupBy_start_exec);

   printf("Step 7: invoke groupBy compiled code\n");

   auto orderBy_start_exec = high_resolution_clock::now(); 

   orderByFunctionType *orderBy = (orderByFunctionType *)orderEntry;
   orderBy(result, groupResult, total_result, groupResult_size);

   auto orderBy_stop_exec = high_resolution_clock::now();
   auto orderBy_exec_duration = duration_cast<microseconds>(orderBy_stop_exec - orderBy_start_exec);

   /*******************tpchq3 Execution Completed**************************************/

   printf ("Step 7: shutdown JIT\n");
   shutdownJit();

   //printMatrix(lineitemTemp);
   //printResult();
   printFinalResult(groupResult, groupResult_size);

   struct time time;

   time.joincompileTime    = join_compile_duration.count();
   time.joinexecutionTime  = join_exec_duration.count();
   time.groupcompileTime   = compile_duration_groupBy.count();
   time.groupexecutionTime = groupBy_exec_duration.count();
   time.orderbycompileTime = compile_duration_orderBy.count();
   time.orderbyexecutionTime = orderBy_exec_duration.count();

   return time;
}