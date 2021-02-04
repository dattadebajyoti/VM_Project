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

#ifndef TPCHQ3_INCL
#define TPCHQ3_INCL

#include <cstdint>
#include <iostream>
#include <string.h>
#include <vector>
using namespace std;

#include "JitBuilder.hpp"

/* declare time struct to store time */
struct time {
    long int joincompileTime;
    long int joinexecutionTime;

    long int groupcompileTime;
    long int groupexecutionTime;

    long int orderbycompileTime;
    long int orderbyexecutionTime;
};

/* Customer table */
struct CUSTOMER{
    int32_t custkey;
    const char* name;
    const char* address;
    int32_t nationkey;
    const char* phone;
    float acctbal;
    const char* mktsegment;
    const char* comment;
};

/* orders table */
struct ORDERS{
    int32_t orderkey;
    int32_t custkey;
    const char* orderStatus;
    float totalprice;
    const char* orderdate;
    const char* orderpriority;
    const char* clerk;
    int32_t shippriority;
    const char* comment;
};

/* lineitem table */
struct LINEITEM{
    int32_t orderkey;
    int32_t partkey;
    int32_t suppkey;
    int32_t linenumber;
    int32_t quantity;
    float extendedprice;
    float discount;
    float tax;
    const char* returnflag;
    const char* linestatus;
    const char* shipdate;
    const char* commitdate;
    const char* receiptdate;
    const char* shipinstruct;
    const char* shipmode;
    const char* comment;
};

struct RESULT{
    int32_t l_orderkey;
    float l_extendedprice;
    float l_discount;
    const char* o_orderdate;
    int32_t o_shippriority;
};

struct groupByLorderkey{
    int32_t groupBy_value;
    int32_t size;
};

struct groupByOorderdate{
    const char* groupBy_value;
    int32_t size;
};

struct groupByOshippriority{
    int32_t groupBy_value;
    int32_t size;
    
};

struct GROUPRESULT{
    int32_t l_orderkey;
    float revenue;
    const char* o_orderdate;
    int32_t o_shippriority;
};

/* Declare Read file logic */
using vec = vector<string>;
using matrix = vector<vec>;
matrix readFile(string filename);
void printMatrix(const matrix& M);
void printFinalResult(struct GROUPRESULT *result, int32_t size);

void orderByRevenueDesc(struct GROUPRESULT *result, int32_t l, int32_t r);

typedef int32_t (tpchq3FunctionType)(CUSTOMER*, ORDERS*, LINEITEM*, int32_t, int32_t, int32_t, const char*, const char*, const char*);

class tpchq3 : public OMR::JitBuilder::MethodBuilder
   {
   private:

   OMR::JitBuilder::IlType *pStr;

   OMR::JitBuilder::IlType *StructTypeCUSTOMER;
   OMR::JitBuilder::IlType *pStructTypeCUSTOMER;

   OMR::JitBuilder::IlType *StructTypeORDERS;
   OMR::JitBuilder::IlType *pStructTypeORDERS;

   OMR::JitBuilder::IlType *StructTypeLINEITEM;
   OMR::JitBuilder::IlType *pStructTypeLINEITEM;
   

   public:

   tpchq3(OMR::JitBuilder::TypeDictionary *);
   virtual bool buildIL();

   void mergeSort(IlBuilder *b, OMR::JitBuilder::IlValue *left, OMR::JitBuilder::IlValue *right);

   static struct time  run(const char*, const char* , const char*, const char*, const char*, const char*);

   void matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   };

typedef int32_t (groupByFunctionType)(RESULT*, GROUPRESULT*, int32_t, int32_t);
class groupBy : public OMR::JitBuilder::MethodBuilder
   {
   private:

   OMR::JitBuilder::IlType *pStr;

   OMR::JitBuilder::IlType *StructTypeRESULT;
   OMR::JitBuilder::IlType *pStructTypeRESULT;

   OMR::JitBuilder::IlType *StructTypeGROUPRESULT;
   OMR::JitBuilder::IlType *pStructTypeGROUPRESULT;

   public:

   groupBy(OMR::JitBuilder::TypeDictionary *);
   virtual bool buildIL();

   void generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);
   void matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   };

typedef int32_t (orderByFunctionType)(RESULT*, GROUPRESULT*, int32_t, int32_t);
class orderBy : public OMR::JitBuilder::MethodBuilder
   {
   private:

   OMR::JitBuilder::IlType *pStr;

   OMR::JitBuilder::IlType *StructTypeRESULT;
   OMR::JitBuilder::IlType *pStructTypeRESULT;

   OMR::JitBuilder::IlType *StructTypeGROUPRESULT;
   OMR::JitBuilder::IlType *pStructTypeGROUPRESULT;

   public:

   orderBy(OMR::JitBuilder::TypeDictionary *);
   virtual bool buildIL();

   void generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);
   void matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   };
#endif // !defined(tpchq2_INCL)
