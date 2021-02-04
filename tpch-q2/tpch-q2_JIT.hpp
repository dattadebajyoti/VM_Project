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

#ifndef TPCHQ2_INCL
#define TPCHQ2_INCL

#include <cstdint>
#include <iostream>
#include <string.h>
#include <vector>
using namespace std;

#include "JitBuilder.hpp"

/* declare time struct to store time */
struct time {
    long int compileTime;
    long int executionTime;
    long int compileTime_mergeSort;
    long int executionTime_mergeSort;
};

/*  Declare tables  */

/* Declare PART table */
struct PART{
    int64_t partKey;
    const char* name;
    const char* mfgr;
    const char* brand;
    const char* type;
    int64_t  size;
    const char* container;
    float retailPrice;
    const char* comment;
};

/* Declare SUPPLIER table */
struct SUPPLIER{
    int64_t suppKey;
    const char* name;
    const char* address;
    int64_t nationKey;
    const char* phone;
    float acctBal;
    const char* comment;
};

/* Declare PARTSUPP table */
struct PARTSUPP{
    int64_t partKey;
    int64_t suppKey;
    int64_t availQty;
    float supplyCost;
    const char* comment;
};


/* Declare NATION table */
struct NATION{
    int64_t nationKey;
    const char* name;
    int64_t regionKey;
    const char* comment;
};

/* Declare REGION table */
struct TEST{
    int64_t regionKey;
    const char* name;
    const char* comment;
};


/* Declare struct to store the result */
struct RESULT{
    float s_acctbal;
	const char* s_name;
	const char* n_name;
	int64_t p_partkey;
	const char* p_mfgr;
	const char* s_address;
	const char* s_phone;
	const char* s_comment;
};


/* Declare Read file logic */
using vec = vector<string>;
using matrix = vector<vec>;
matrix readFile(const char* filename);
void printMatrix(const matrix& M);

typedef int32_t (tpchq2FunctionType)(PART* , int64_t, SUPPLIER*, int64_t, PARTSUPP*, int64_t, NATION*, int64_t, 
                                     int64_t, const char*, const char*, int64_t, TEST*, int64_t, RESULT*);

class tpchq2 : public OMR::JitBuilder::MethodBuilder
   {
   private:
   
   OMR::JitBuilder::IlType *pString;
   OMR::JitBuilder::IlType *pStr;

   OMR::JitBuilder::IlType *StructTypePart;
   OMR::JitBuilder::IlType *pStructTypePart;

   OMR::JitBuilder::IlType *StructTypeSupplier;
   OMR::JitBuilder::IlType *pStructTypeSupplier;
 
   OMR::JitBuilder::IlType *StructTypePartsupp;
   OMR::JitBuilder::IlType *pStructTypePartsupp;

   OMR::JitBuilder::IlType *StructTypeNation;
   OMR::JitBuilder::IlType *pStructTypeNation;

   OMR::JitBuilder::IlType *StructTypeTEST;
   OMR::JitBuilder::IlType *pStructTypeTEST;

   OMR::JitBuilder::IlType *StructTypeTestRES;
   OMR::JitBuilder::IlType *pStructTypeTestRES;
   

   public:

   tpchq2(OMR::JitBuilder::TypeDictionary *);
   virtual bool buildIL();

   static struct time  run(const char*, const char* , const char*, const char*, const char*, int64_t, const char*, const char*);

   void isSubstringJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2,
                              OMR::JitBuilder::IlValue *s1_length, OMR::JitBuilder::IlValue *s2_length);

   void CallSubStringJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2,
                              OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

    void CallSubStringJITHelper(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2,
                              OMR::JitBuilder::IlValue *one);

   void generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   void matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   void matchNameRegionJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two,
                              OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2);

   void matchSupplyCostJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   void matchkeyCostJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);

   void storeResultJIT(IlBuilder *b, OMR::JitBuilder::IlValue *res_size,  OMR::JitBuilder::IlValue *s_acctbal, OMR::JitBuilder::IlValue *s_name,
                       OMR::JitBuilder::IlValue *n_name, OMR::JitBuilder::IlValue  *p_partkey,
                       OMR::JitBuilder::IlValue *p_mfgr, OMR::JitBuilder::IlValue  *s_address,
                       OMR::JitBuilder::IlValue *s_phone, OMR::JitBuilder::IlValue *s_comment, const char* result_size);
   };

typedef int32_t (mergeSortFunctionType)(RESULT*, int64_t);

class mergeSort : public OMR::JitBuilder::MethodBuilder
   {
   private:

   OMR::JitBuilder::IlType *pStr;
   OMR::JitBuilder::IlType *StructTypeRES;
   OMR::JitBuilder::IlType *pStructTypeRES;

   public:
   mergeSort(OMR::JitBuilder::TypeDictionary *);
   virtual bool buildIL();
   void generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two);
   };

#endif // !defined(tpchq1_INCL)
