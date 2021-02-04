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
#include "tpch-q4_JIT.hpp"
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
int32_t groupResult_size = 0;

void printResult()
{
    int32_t i;
    for(i = 0; i < total_result; i++){

        cout << setw(18) << left << result[i].o_orderpriority<< " ";

        cout<<endl;
    }
    cout<<"Total Rows: "<<total_result<<endl;
}

void printFinalResult(struct GROUPRESULT *result, int32_t size){
    int32_t i;

    for(i = 0; i < size; i++){

        cout << setw(18) << left << result[i].o_orderpriority<< " ";
        cout << setw(18) << left << result[i].order_count<< " ";

        cout<<endl;
    }
    cout<<"Total Rows: "<<size<<endl;
}

/**********************************************Helper functions*******************************************************************************/


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

/* Interval-sql*/
static int32_t interval_function(const char* d1, const char* d2, const char* str, int32_t val) {
    #define INTERVAL_LINE LINETOSTR(__LINE__)

    int32_t year_d1  = atoi(substr(d1,0,4));
    int32_t month_d1 = atoi(substr(d1, 5, 7));
    int32_t day_d1   = atoi(substr(d1, 8, 10));

    //cout<<year_d1<<"  "<<month_d1<<"  "<<day_d1<<endl;

    int32_t year_d2  = atoi(substr(d2, 0, 4));
    int32_t month_d2 = atoi(substr(d2, 5, 7));;
    int32_t day_d2   = atoi(substr(d2, 8, 10));

    if(strcmp(str, "year") == 0)
    {
       year_d2  = year_d2 + val;
    }
    else if(strcmp(str, "month") == 0)
    {
       month_d2 = month_d2 + val;
    }
    else if(strcmp(str, "day") == 0)
    {
       day_d2   = day_d2 + val;
    }
    //cout<<year_d2<<"  "<<month_d2<<"  "<<day_d2<<endl;

    if (year_d1 < year_d2 || year_d1 <= year_d2 && month_d1 < month_d2 || year_d1 <= year_d2 && month_d1 <= month_d2 && day_d1 < day_d2) {
        return 1; //if d1 < d2
    }
    else if (year_d1 > year_d2 || year_d1 >= year_d2 && month_d1 > month_d2 || year_d1 >= year_d2 && month_d1 >= month_d2 && day_d1 > day_d2) {
        return 2; //if d1>d2
    }
    return 0; //if d1 == d2
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
        if (strcmp(temp1[i].o_orderpriority, temp2[j].o_orderpriority) < 0) {
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



/* store helper */
static void storeResult(const char* o_orderpriority, int32_t group_count)
{
   #define STORERESULT_LINE LINETOSTR(__LINE__)

   groupResult[groupResult_size].o_orderpriority = o_orderpriority;
   groupResult[groupResult_size].order_count = group_count;

   groupResult_size++;
   
}
/****************************************Define the method builder object********************************************************/

tpchq4::tpchq4(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("tpchq3");

   pStr = types->toIlType<char *>();
   
   StructTypeORDERS        = types->LookupStruct("ORDERS");
   pStructTypeORDERS       = types->PointerTo(StructTypeORDERS);

   StructTypeLINEITEM      = types->LookupStruct("LINEITEM");
   pStructTypeLINEITEM     = types->PointerTo(StructTypeLINEITEM);

   StructTypeRESULT      = types->LookupStruct("RESULT");
   pStructTypeRESULT     = types->PointerTo(StructTypeRESULT);

   
   /* define structs */
   DefineParameter("o",  pStructTypeORDERS);
   DefineParameter("l",  pStructTypeLINEITEM);

   DefineParameter("o_size", Int32);
   DefineParameter("l_size", Int32);

   DefineParameter("o_orderdate",  pStr);
   DefineParameter("interval",   Int32);
   DefineParameter("interval_str",    pStr);

   DefineParameter("r",        pStructTypeRESULT);
   DefineParameter("r_size",       Int32);

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

    DefineFunction((char *)"interval_function", 
                  (char *)__FILE__,
                  (char *)INTERVAL_LINE,
                  (void *)&interval_function,
                  Int32,
                  4,
                  pStr,
                  pStr,
                  pStr,
                  Int32);

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

      /* Declare RESULT Table */
      DEFINE_STRUCT(RESULT);

      DEFINE_FIELD(RESULT, o_orderpriority, toIlType<const char *>());

      CLOSE_STRUCT(RESULT);
      
      
      }
      
   };

void tpchq4::matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt32(0));
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->EqualTo(one, two));
   twoSmaller->Store(resultName, twoSmaller->ConstInt32(1));
}
/******************************BUILDIL**********************************************/

bool
tpchq4::buildIL()
   {
   OMR::JitBuilder::IlValue *orders_size = Load("o_size");
   OMR::JitBuilder::IlValue *lineitem_size = Load("l_size");
   
   OMR::JitBuilder::IlBuilder* o_loop = NULL;
   ForLoopUp("o_index", &o_loop,
      ConstInt32(0),
      orders_size,
      ConstInt32(1));

   o_loop->Store("o_element",
   o_loop->   IndexAt(pStructTypeORDERS,
   o_loop->      Load("o"),
   o_loop->      Load("o_index")));
   
   o_loop->Store("matchOrderdate", 
   o_loop->   Call("compareDate", 2,
   o_loop->      LoadIndirect("ORDERS", "orderdate",
   o_loop->         Load("o_element")),
   o_loop->      Load("o_orderdate")));
   //c_loop->Call("printString", 1, c_loop->Load("res_mktsegment"));

   OMR::JitBuilder::IlBuilder *orderdate_Match = NULL;
   o_loop->IfThen(&orderdate_Match,
   o_loop->   NotEqualTo(
   o_loop->      Load("matchOrderdate"),
   o_loop->      ConstInt32(1)));

   orderdate_Match->Store("matchInterval", 
   orderdate_Match->   Call("interval_function", 4,
   orderdate_Match->      LoadIndirect("ORDERS", "orderdate",
   orderdate_Match->         Load("o_element")),
   orderdate_Match->      Load("o_orderdate"),
   orderdate_Match->      Load("interval_str"),
   orderdate_Match->      Load("interval")));

   OMR::JitBuilder::IlBuilder *interval_Match = NULL;
   orderdate_Match->IfThen(&interval_Match,
   orderdate_Match->   EqualTo(
   orderdate_Match->      Load("matchInterval"),
   orderdate_Match->      ConstInt32(1)));

   interval_Match->Store("count_match",
   interval_Match->   ConstInt32(0));

   OMR::JitBuilder::IlBuilder* l_loop = NULL;
   interval_Match->ForLoopUp("l_index", &l_loop,
   interval_Match->   ConstInt32(0),
                        lineitem_size,
   interval_Match->   ConstInt32(1));

   l_loop->Store("l_element",
   l_loop->   IndexAt(pStructTypeLINEITEM,
   l_loop->      Load("l"),
   l_loop->      Load("l_index")));

   matchkeyJIT(l_loop, "matchOrderKey", 
   l_loop->   LoadIndirect("LINEITEM", "orderkey",
   l_loop->      Load("l_element")),
   l_loop->   LoadIndirect("ORDERS", "orderkey",
   l_loop->      Load("o_element")));

   OMR::JitBuilder::IlBuilder *orderKey_Match = NULL;
   l_loop->IfThen(&orderKey_Match,
   l_loop->   EqualTo(
   l_loop->      Load("matchOrderKey"),
   l_loop->      ConstInt32(1)));

   orderKey_Match->Store("matchLinedate", 
   orderKey_Match->   Call("compareDate", 2,
   orderKey_Match->      LoadIndirect("LINEITEM", "commitdate",
   orderKey_Match->         Load("l_element")),
   orderKey_Match->      LoadIndirect("LINEITEM", "receiptdate",
   orderKey_Match->         Load("l_element")) ) );

   OMR::JitBuilder::IlBuilder *lineDate_Match = NULL;
   orderKey_Match->IfThen(&lineDate_Match,
   orderKey_Match->   EqualTo(
   orderKey_Match->      Load("matchLinedate"),
   orderKey_Match->      ConstInt32(1)));

   lineDate_Match->Store("count_match", 
   lineDate_Match->   Add(
   lineDate_Match->      Load("count_match"), 
   lineDate_Match->      ConstInt32(1)));

   OMR::JitBuilder::IlBuilder *check_count_match = NULL;
   interval_Match->IfThen(&check_count_match,
   interval_Match->   GreaterOrEqualTo(
   interval_Match->      Load("count_match"),
   interval_Match->      ConstInt32(1)));

   //check_count_match->Call("printString", 1, check_count_match->Load("count_match"));

   check_count_match->Store("r_element",
   check_count_match->   IndexAt(pStructTypeRESULT,
   check_count_match->      Load("r"),
   check_count_match->      Load("r_size")));

   check_count_match->StoreIndirect("RESULT", "o_orderpriority",
   check_count_match->   Load("r_element"),
   check_count_match->   LoadIndirect("ORDERS", "orderpriority",
   check_count_match->      Load("o_element")));

   check_count_match->Store("r_size", 
   check_count_match->   Add(
   check_count_match->      Load("r_size"), 
   check_count_match->      ConstInt32(1)));
   
   Return(Load("r_size"));

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
                  2,
                  pStr,
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

      DEFINE_FIELD(RESULT, o_orderpriority, toIlType<const char *>());

      CLOSE_STRUCT(RESULT);
      
      DEFINE_STRUCT(GROUPRESULT);

      DEFINE_FIELD(GROUPRESULT, o_orderpriority, toIlType<const char *>());
      DEFINE_FIELD(GROUPRESULT, order_count, Int32);

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

   /****************************Group*******************************/

   Store("group_count", ConstInt32(0));

   OMR::JitBuilder::IlBuilder* group_loop = NULL;
   ForLoopUp("r_index", &group_loop,
      ConstInt32(0),
      r_size,
      ConstInt32(0));

   group_loop->Store("outer_element",
   group_loop->   IndexAt(pStructTypeRESULT,
   group_loop->      Load("r"),
   group_loop->      Load("r_index")));

   group_loop->Store("group_count", 
   group_loop->   ConstInt32(1));

   group_loop->Store("innerVal", 
   group_loop->   Add(
   group_loop->      Load("r_index"), 
   group_loop->      ConstInt32(1)));


   OMR::JitBuilder::IlBuilder* inner_loop = NULL;
   group_loop-> ForLoopUp("inner", &inner_loop,
   group_loop->   Add(
   group_loop->      Load("r_index"), 
   group_loop->      ConstInt32(1)),
                  r_size,
   group_loop->   ConstInt32(0));

   inner_loop->Store("inner_element",
   inner_loop->   IndexAt(pStructTypeRESULT,
   inner_loop->      Load("r"),
   inner_loop->      Load("inner")));

   inner_loop->Store("matchOrderpriority", 
   inner_loop->   Call("matchString", 2,
   inner_loop->      LoadIndirect("RESULT", "o_orderpriority",
   inner_loop->         Load("outer_element")),
   inner_loop->      LoadIndirect("RESULT", "o_orderpriority",
   inner_loop->         Load("inner_element"))));

   OMR::JitBuilder::IlBuilder *match_orderpriority = NULL;
   OMR::JitBuilder::IlBuilder *nomatch_orderpriority = NULL;
   inner_loop->IfThenElse(&match_orderpriority, &nomatch_orderpriority,
   inner_loop->   EqualTo(
   inner_loop->      Load("matchOrderpriority"),
   inner_loop->      ConstInt32(1)));

   match_orderpriority->Store("group_count", 
   match_orderpriority->   Add(
   match_orderpriority->      Load("group_count"), 
   match_orderpriority->      ConstInt32(1)));   

   match_orderpriority->Store("inner", 
   match_orderpriority->   Add(
   match_orderpriority->      Load("inner"), 
   match_orderpriority->      ConstInt32(1)));  

   match_orderpriority->Store("innerVal", 
   match_orderpriority->   Add(
   match_orderpriority->      Load("innerVal"), 
   match_orderpriority->      ConstInt32(1)));

   nomatch_orderpriority->Store("inner", r_size);

   
   //Store element
   group_loop->Store("g_element",
   group_loop->   IndexAt(pStructTypeGROUPRESULT,
   group_loop->      Load("group_r"),
   group_loop->      Load("group_r_size")));

   group_loop->StoreIndirect("GROUPRESULT", "o_orderpriority",
   group_loop->   Load("g_element"),
   group_loop->   LoadIndirect("RESULT", "o_orderpriority",
   group_loop->      Load("outer_element")));

   group_loop->StoreIndirect("GROUPRESULT", "order_count",
   group_loop->   Load("g_element"),
   group_loop->   Load("group_count"));

   group_loop->Store("group_r_size", 
   group_loop->   Add(
   group_loop->      Load("group_r_size"), 
   group_loop->      ConstInt32(1)));
   
   //group_loop->Call("printString", 1, group_loop->Load("innerVal"));
   group_loop->Store("r_index", group_loop->Load("innerVal"));
  
   Return(Load("group_r_size"));

   return true;
   }


/**************************************************Entry for exectution*************************************************/
struct time
tpchq4::run(const char* orders, const char* lineitem, const char* o_orderdate, int32_t interval, const char* interval_str)
{
   /* 1. Load the tables into the memory -- Time not considered for loading tables in memory*/
   matrix ordersTemp, lineitemTemp;
   int32_t i, j;
   
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

   printf("Step 3: compile tpchq4 method builder\n");

   /*******************compile tpchq4*********************/

   auto join_start_compile = high_resolution_clock::now(); 

   tpchq4 method(&joinmethodTypes);
   void *entry=0;
   int32_t rc = compileMethodBuilder(&method, &entry);

   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   auto join_stop_compile = high_resolution_clock::now();
   auto join_compile_duration = duration_cast<microseconds>(join_stop_compile - join_start_compile);

   /*******************tpchq4 compilation done**********************/
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


   /*******************invoke**************************************/
   printf("Step 5: invoke tpchq4 compiled code\n");

   auto join_start_exec = high_resolution_clock::now(); 

   tpchq4FunctionType *join = (tpchq4FunctionType *)entry;

   total_result = join(o, l, orders_size, lineitem_size, o_orderdate, interval, interval_str, result, total_result);
   
   auto join_stop_exec = high_resolution_clock::now();
   auto join_exec_duration = duration_cast<microseconds>(join_stop_exec - join_start_exec);

   printf("Step 6: invoke groupBy compiled code\n");

   groupResult = (GROUPRESULT*) malloc(total_result * sizeof(GROUPRESULT));

   auto groupBy_start_exec = high_resolution_clock::now(); 

   groupByFunctionType *groupBy = (groupByFunctionType *)groupEntry;
   groupResult_size = groupBy(result, groupResult, total_result, groupResult_size);

   auto groupBy_stop_exec = high_resolution_clock::now();
   auto groupBy_exec_duration = duration_cast<microseconds>(groupBy_stop_exec - groupBy_start_exec);

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
   time.orderbycompileTime = 0;
   time.orderbyexecutionTime = 0;

   free(o);
   free(l);
   total_result = 0;
   groupResult_size = 0;

   return time;
}