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
#include "tpch-q2_JIT.hpp"
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

/**********************************************Helper functions*******************************************************************************/
/* substring function 2 compare 2 strings*/
int isSubstring(const char* s1, const char* s2, int s1_length, int s2_length) 
{ 
  
    /* A loop to slide pat[] one by one */
    int i;
    for (i = 0; i <= s2_length - s1_length; i++) { 
        int j; 
  
        /* For current index i, check for pattern match */
        for (j = 0; j < s1_length; j++) {
            if (s2[i + j] != s1[j]) {
                break; 
            }
        }
        if (j == s1_length) {
            return 1; 
        }
    } 
  
    return 0; 
} 

void printResult(struct RESULT result[], int total_result)
{
    int i;
    for(i = 0; i < total_result; i++){

        cout << setw(18) << left << result[i].s_acctbal<< " ";
        cout << setw(18) << left << result[i].s_name<< " ";
        cout << setw(18) << left << result[i].n_name<< " ";
        cout << setw(18) << left << result[i].p_partkey<< " ";
        cout << setw(18) << left << result[i].p_mfgr<< " ";
        cout << setw(18) << left << result[i].s_address<< " ";
        cout << setw(18) << left << result[i].s_phone<< " ";
        cout << setw(18) << left << result[i].s_comment<< " ";

        cout<<endl;
    }
    cout<<"Total Rows: "<<total_result<<endl;
}

/* Selection Sort to Order By s_name */
void selectionSortNation(struct RESULT mat[], int start_index, int n){
    int i, j, min_idx;  
    int MAX_LEN = 20;
    // One by one move boundary of unsorted subarray  
    char minStr[MAX_LEN];  
    for (i = start_index; i < n-1; i++)  
    {  
        // Find the minimum element in unsorted array  
        int min_idx = i;  
        strcpy(minStr, mat[i].s_name);  
        for (j = i + 1; j < n; j++)  
        {  
            // If min is greater than arr[j]  
            if (strcmp(minStr, mat[j].s_name) > 0)  
            {  
                // Make arr[j] as minStr and update min_idx  
                strcpy(minStr, mat[j].s_name);  
                min_idx = j;  
            }  
        }  
  
        // Swap the found minimum element with the first element  
        if (min_idx != i)  
        {    
            struct RESULT temp;
            temp = mat[i];
            mat[i] =  mat[min_idx];
            mat[min_idx] = temp;
        }  
    }  

}

/* Selection Sort to Order By Nation */
void selectionSort(struct RESULT mat[], int start_index, int n){
    int i, j, min_idx;  
    int MAX_LEN = 20;
    // One by one move boundary of unsorted subarray  
    char minStr[MAX_LEN];  
    for (i = start_index; i < n-1; i++)  
    {  
        // Find the minimum element in unsorted array  
        int min_idx = i;  
        strcpy(minStr, mat[i].n_name);  
        for (j = i + 1; j < n; j++)  
        {  
            // If min is greater than arr[j]  
            if (strcmp(minStr, mat[j].n_name) > 0)  
            {  
                // Make arr[j] as minStr and update min_idx  
                strcpy(minStr, mat[j].n_name);  
                min_idx = j;  
            }  
        }  
  
        // Swap the found minimum element with the first element  
        if (min_idx != i)  
        {    
            struct RESULT temp; 
            temp = mat[i];
            mat[i] =  mat[min_idx];
            mat[min_idx] = temp;
        }  
    }  

}

/* function to calculate the frequency of Part_key */
int calFrequencyPK(struct RESULT res[], int pk_freq[], int index, int freq_size){
    int i,c,j;
    int n_freq_size = 0;
    int a[freq_size];
    for(i = index; i < freq_size; i++){
        a[i] = 1;
    }
    //cout<<index<<"   "<<freq_size<<endl;
    for(i=index; i<freq_size; i++)
    {
        c=1;
        if(a[i] != 0)
		{
		    for(j = i+1; j < freq_size; j++)
     
            {
        	   if(res[i].p_partkey == res[j].p_partkey)
        	    {
                   //cout<<res[i].n_name<<"    "<<res[j].n_name<<endl;
			       c++;
			       //a[j]=-1;
                   a[j] = 0;
		        }
	       }
	       //b[i]=c;
           pk_freq[n_freq_size] = c;
           n_freq_size++;
		}   
    }

    return n_freq_size;
}

/* function to calculate the frequency of nation */
int calFrequencyNation(struct RESULT res[], int n_freq[], int index, int freq_size){
    int i,c,j;
    int n_freq_size = 0;
    int a[freq_size];
    for(i = index; i < freq_size; i++){
        a[i] = 1;
    }
    //cout<<index<<"   "<<freq_size<<endl;
    for(i=index; i<freq_size; i++)
    {
        c=1;
        if(a[i] != 0)
		{
		    for(j = i+1; j < freq_size; j++)
     
            {
        	   if(strcmp(res[i].n_name, res[j].n_name) == 0)
        	    {
                   //cout<<res[i].n_name<<"    "<<res[j].n_name<<endl;
			       c++;
			       //a[j]=-1;
                   a[j] = 0;
		        }
	       }
	       //b[i]=c;
           n_freq[n_freq_size] = c;
           n_freq_size++;
		}   
    }

    return n_freq_size;
}

/* function to calculate the frequency of accbal */
int calFrequencyAccBal(struct RESULT res[], int freq[], int total_result){
    int i,c,j;
    int freq_size = 0;
    float a[total_result];
    for(i = 0; i < total_result; i++){
        a[i] = res[i].s_acctbal;
    }
    for(i=0; i<total_result; i++)
    {
        c=1;
        if(a[i]!=-1)
		{
		    for(j=i+1; j<total_result && a[j] >= a[i]; j++)
     
            {
        	   if(a[i]==a[j])
        	    {
			       c++;
			       a[j]=-1;
		       }
	       }
	       //b[i]=c;
           freq[freq_size] = c;
           freq_size++;
		}   
    }

    return freq_size;
}

/* Merge Function */
static void merge(int64_t l, int64_t m, int64_t r) 
{ 
    #define MERGE_LINE LINETOSTR(__LINE__)
    //cout<<l<<"   "<<m<<"   "<<r<<endl;
    int i, j, k; 
    int n1 = m - l + 1; 
    int n2 =  r - m; 

    struct RESULT L[n1], R[n2]; 
 
    for (i = 0; i < n1; i++) 
        L[i] = result[l + i]; 
    for (j = 0; j < n2; j++) 
        R[j] = result[m + 1+ j]; 
  
    i = 0; 
    j = 0; 
    k = l; 
    while (i < n1 && j < n2) 
    { 
        if (L[i].s_acctbal > R[j].s_acctbal) 
        { 
            result[k] = L[i]; 
            i++; 
        } 
        else
        { 
            result[k] = R[j]; 
            j++; 
        } 
        k++; 
    } 

    while (i < n1) 
    { 
        result[k] = L[i]; 
        i++; 
        k++; 
    } 

    while (j < n2) 
    { 
        result[k] = R[j]; 
        j++; 
        k++; 
    } 
}

void mergeSortAlgorithm(struct RESULT mat[], int l, int r) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSortAlgorithm(mat, l, m);
        mergeSortAlgorithm(mat, m + 1, r);
        merge(l, m, r);
    }
}

/**********************************OrderBY********************************************/
    
static void orderBy(int64_t total_result)
{
   #define ORDERBY_LINE LINETOSTR(__LINE__)
   /*******************1st Order by s_accBal********************************************/
   int i;
   //selectionSort(result, total_result);

   /*****************2nd order sort based on accbal to nation wise**********************/
   int freq[total_result];
   int freq_size = calFrequencyAccBal(result, freq, total_result);

   /* var to iterate over freq array */
   int count_fre_size = 0;
   /* var to vary the sorting size */
   int freq_tot = 0;
   /* var to iterate over the results */
   i = 0;
   while(i < total_result){
       freq_tot += freq[count_fre_size];
       /* sort only if the size is greate than 1 */
       if(freq[count_fre_size] > 1)
       {
            //cout<<"test bdw: "<<i<<" and "<<freq_tot-1<<endl;
            selectionSort(result, i, freq_tot);
            
            /*******************3rd Order by s_name********************************************/
            int n_freq[freq[count_fre_size]];
            int n_freq_size = calFrequencyNation(result, n_freq, i, freq_tot);
            //cout<<n_freq_size<<endl;
            int n_index = i;
            int count_nfre_size = 0;
            int nfreq_tot = i;
            while(n_index < n_freq_size){
                nfreq_tot += n_freq[count_nfre_size];
                if(n_freq[count_nfre_size] > 1)
                {
                    selectionSortNation(result, n_index, nfreq_tot);

                    /*******************4th Order by p_partkey********************************************/
                    int pk_freq[n_freq[count_nfre_size]];
                    int pk_freq_size = calFrequencyPK(result, pk_freq, n_index, nfreq_tot);
                    //cout<<pk_freq_size<<endl;
                    int pk_index = n_index;
                    int count_pkfre_size = 0;
                    int pkfreq_tot = n_index;
                    while(pk_index < pk_freq_size){
                        pkfreq_tot += pk_freq[count_pkfre_size];
                        if(pk_freq[count_pkfre_size] > 1)
                        { 
                            mergeSortAlgorithm(result, pk_index, pkfreq_tot-1);
                        }
                        pk_index += pk_freq[count_pkfre_size];
                        count_pkfre_size++;
                    }

                }
                n_index += n_freq[count_nfre_size];
                count_nfre_size++;
            }
       }
       /* change the start index */
       i = i + freq[count_fre_size];
       count_fre_size++;
   }
}


static void printString(int32_t x)
{
   #define PRINTSTRING_LINE LINETOSTR(__LINE__)
   cout<<"x is: "<<x<<endl;
}

static void printCharArray(char* x)
{
   #define PRINTCHARARRAY_LINE LINETOSTR(__LINE__)
   cout<<"x is: "<<x<<endl;
}

/* Call substring helper */
static int64_t CallSubString(const char* s1, const char* s2, int64_t size1, int64_t size2)
{
   #define CALLSUBSTRING_LINE LINETOSTR(__LINE__)
   if( (isSubstring(s1, s2, strlen(s1), strlen(s2)) == 1) && (size1 == size2) ){
       return 1;
   }
   else{
       return 0;
   }
}

/* match string helper helper */
static int64_t matchString(const char* s1, const char* s2)
{
   #define MATCHSTRING_LINE LINETOSTR(__LINE__)
   if( (strcmp(s1, s2) == 0)){
       return 1;
   }
   else{
       return 0;
   }
}

static int64_t minValue(int64_t l, int64_t r)
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

void tpchq2::matchkeyCostJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt64(0));
   IlBuilder *match=NULL;
   b->IfThen(&match, b->EqualTo(one, two));
   match->Store(resultName, match->ConstInt64(1));
}

/* matching strings using bitwise operators */
static int64_t isLikeTIN(const char *s)
{
   #define ISLIKETIN_LINE LINETOSTR(__LINE__)
   int32_t w = 0;
   char c = *s;
   while(c != 0){
       w = (w << 8) | c;
       s++;
       c = *s;
   }
   w = w<<8;
   return (w == (int32_t)0x54494E00); // 0x54=='T', 0x49=="I", 0x4E=='N'
}

static void storeResult(float s_acctbal, const char* s_name, const char* n_name, int64_t p_partkey, 
                        const char* p_mfgr, const char* s_address, const char* s_phone, const char* s_comment)
{
   #define STORERESULT_LINE LINETOSTR(__LINE__)

   result[total_result].s_acctbal   = s_acctbal;
   result[total_result].s_name      = s_name;
   result[total_result].n_name      = n_name;
   result[total_result].p_partkey   = p_partkey;
   result[total_result].p_mfgr      = p_mfgr;
   result[total_result].s_address   = s_address;
   result[total_result].s_phone     = s_phone;
   result[total_result].s_comment   = s_comment;

   total_result++;
   
}

void tpchq2::storeResultJIT(IlBuilder *b, OMR::JitBuilder::IlValue *res_size, OMR::JitBuilder::IlValue *s_acctbal, OMR::JitBuilder::IlValue *s_name,
                       OMR::JitBuilder::IlValue *n_name, OMR::JitBuilder::IlValue  *p_partkey,
                       OMR::JitBuilder::IlValue *p_mfgr, OMR::JitBuilder::IlValue  *s_address,
                       OMR::JitBuilder::IlValue *s_phone, OMR::JitBuilder::IlValue *s_comment, const char* result_size)
{
   
}



/****************************************Define the method builder object********************************************************/

tpchq2::tpchq2(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("tpchq2");
   
   //pString = types->PointerTo(types->PointerTo(types->toIlType<char *>()));
   pStr = types->toIlType<char *>();

   StructTypePart      = types->LookupStruct("PART");
   pStructTypePart     = types->PointerTo(StructTypePart);
   
   StructTypeSupplier  = types->LookupStruct("SUPPLIER");
   pStructTypeSupplier = types->PointerTo(StructTypeSupplier);

   StructTypePartsupp  = types->LookupStruct("PARTSUPP");
   pStructTypePartsupp = types->PointerTo(StructTypePartsupp);

   StructTypeNation    = types->LookupStruct("NATION");
   pStructTypeNation   = types->PointerTo(StructTypeNation);
   
   /* Declare the region struct */
   StructTypeTEST    = types->LookupStruct("TEST");
   pStructTypeTEST   = types->PointerTo(StructTypeTEST);
   
   /* Declare the result struct */
   StructTypeTestRES      = types->LookupStruct("RESULT");
   //pStructTypeTestRES     = types->PointerTo(StructTypeTestRES);


   /* define structs */
   DefineParameter("p",         pStructTypePart);
   DefineParameter("part_size", Int64);

   DefineParameter("s",         pStructTypeSupplier);
   DefineParameter("s_size",    Int64);

   DefineParameter("ps",        pStructTypePartsupp);
   DefineParameter("ps_size",   Int64);

   DefineParameter("n",         pStructTypeNation);
   DefineParameter("n_size",    Int64);

   /* Arguments passed by user */
   DefineParameter("p_size",         Int64);
   DefineParameter("region",         pStr);
   DefineParameter("like_substring", pStr);
   
   DefineParameter("r_size",  Int64);
   DefineParameter("t",       pStructTypeTEST);

   
   DefineParameter("test_result_size", Int64);
   //DefineParameter("test_result",      pStructTypeTestRES);

   /* Define functions */
   DefineFunction((char *)"printString", 
                  (char *)__FILE__,
                  (char *)PRINTSTRING_LINE,
                  (void *)&printString,
                  NoType,
                  1,
                  Int64);

    DefineFunction((char *)"isLikeTIN", 
                  (char *)__FILE__,
                  (char *)ISLIKETIN_LINE,
                  (void *)&isLikeTIN,
                  Int64,
                  1,
                  pStr);

    DefineFunction((char *)"matchString", 
                  (char *)__FILE__,
                  (char *)MATCHSTRING_LINE,
                  (void *)&matchString,
                  Int64,
                  2,
                  pStr,
                  pStr);

    DefineFunction((char *)"storeResult", 
                  (char *)__FILE__,
                  (char *)STORERESULT_LINE,
                  (void *)&storeResult,
                  NoType,
                  8,
                  Float,
                  pStr,
                  pStr,
                  Int64,
                  pStr,
                  pStr,
                  pStr,
                  pStr);
   
   
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

      /* Declare PART table */
      DEFINE_STRUCT(PART);

      DEFINE_FIELD(PART, partKey, Int64);
      DEFINE_FIELD(PART, name, toIlType<const char *>());
      DEFINE_FIELD(PART, mfgr, toIlType<const char *>());
      DEFINE_FIELD(PART, brand, toIlType<const char *>());
      DEFINE_FIELD(PART, type, toIlType<const char *>());
      DEFINE_FIELD(PART, size, Int64);
      DEFINE_FIELD(PART, container, toIlType<const char *>());
      DEFINE_FIELD(PART, retailPrice, Float);
      DEFINE_FIELD(PART, comment, toIlType<const char *>());

      CLOSE_STRUCT(PART);
      
      /* Declare SUPPLIER table */
      DEFINE_STRUCT(SUPPLIER);

      DEFINE_FIELD(SUPPLIER, suppKey, Int64);
      DEFINE_FIELD(SUPPLIER, name, toIlType<const char *>());
      DEFINE_FIELD(SUPPLIER, address, toIlType<const char *>());
      DEFINE_FIELD(SUPPLIER, nationKey, Int64);
      DEFINE_FIELD(SUPPLIER, phone, toIlType<const char *>());
      DEFINE_FIELD(SUPPLIER, acctBal, Float);
      DEFINE_FIELD(SUPPLIER, comment, toIlType<const char *>());

      CLOSE_STRUCT(SUPPLIER);
      
      /* Declare PARTSUPP table */
      DEFINE_STRUCT(PARTSUPP);

      DEFINE_FIELD(PARTSUPP, partKey, Int64);
      DEFINE_FIELD(PARTSUPP, suppKey, Int64);
      DEFINE_FIELD(PARTSUPP, availQty, Int64);
      DEFINE_FIELD(PARTSUPP, supplyCost, Float);
      DEFINE_FIELD(PARTSUPP, comment, toIlType<const char *>());

      CLOSE_STRUCT(PARTSUPP);

      /* Declare NATION table */
      DEFINE_STRUCT(NATION);

      DEFINE_FIELD(NATION, nationKey, Int64);
      DEFINE_FIELD(NATION, name, toIlType<const char *>());
      DEFINE_FIELD(NATION, regionKey, Int64);
      DEFINE_FIELD(NATION, comment, toIlType<const char *>());

      CLOSE_STRUCT(NATION);

      /* Declare REGION table */
      
      DEFINE_STRUCT(TEST);
      DEFINE_FIELD(TEST, regionKey, Int64);
      DEFINE_FIELD(TEST, name, toIlType<const char *>());
      DEFINE_FIELD(TEST, comment, toIlType<const char *>());
      CLOSE_STRUCT(TEST);
      
      /* Declare RESULT table */
      DEFINE_STRUCT(RESULT);

      DEFINE_FIELD(RESULT, s_acctbal, Float);
      DEFINE_FIELD(RESULT, s_name, toIlType<const char *>());
      DEFINE_FIELD(RESULT, n_name, toIlType<const char *>());
      DEFINE_FIELD(RESULT, p_partkey, Int64);
      DEFINE_FIELD(RESULT, p_mfgr, toIlType<const char *>());
      DEFINE_FIELD(RESULT, s_address, toIlType<const char *>());
      DEFINE_FIELD(RESULT, s_phone, toIlType<const char *>());
      DEFINE_FIELD(RESULT, s_comment, toIlType<const char *>());

      CLOSE_STRUCT(RESULT);
      
      }
      
   };

void tpchq2::generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, one);
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->LessThan(one, two));
   twoSmaller->Store(resultName, two);
}

void tpchq2::matchkeyJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt64(0));
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->EqualTo(one, two));
   twoSmaller->Store(resultName, twoSmaller->ConstInt64(1));
}

/* Match Like string and Partkey */
void tpchq2::CallSubStringJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2,
                              OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt64(0));
   matchkeyJIT(b, "matchSize", one, two);
   IlBuilder *matchKey=NULL;
   b->IfThen(&matchKey, b->EqualTo(b->Load("matchSize"), b->ConstInt64(1)));
   //b->IfThen(&matchKey, b->EqualTo(one, b->ConstInt64(37)));

   matchKey->Store("findLikeString", matchKey->Call("isLikeTIN", 1, s2));
   IlBuilder *matchString=NULL;
   matchKey->IfThen(&matchString, matchKey->EqualTo(matchKey->Load("findLikeString"), matchKey->ConstInt64(1)));
   matchString->Store(resultName, matchString->ConstInt64(1));
}

/* Match Like string and Partkey */
void tpchq2::CallSubStringJITHelper(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2,
                              OMR::JitBuilder::IlValue *one)
{
   b->Store(resultName, b->ConstInt64(0));
   IlBuilder *matchKey=NULL;
   b->IfThen(&matchKey, b->EqualTo(one, b->ConstInt64(37)));

   matchKey->Store("findLikeString", matchKey->Call("isLikeTIN", 1, s2));
   IlBuilder *matchString=NULL;
   matchKey->IfThen(&matchString, matchKey->EqualTo(matchKey->Load("findLikeString"), matchKey->ConstInt64(1)));
   matchString->Store(resultName, matchString->ConstInt64(1));
}

/* Match region and nation key and match region == AFRICA */
void tpchq2::matchNameRegionJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two,
                              OMR::JitBuilder::IlValue *s1, OMR::JitBuilder::IlValue *s2)
{
   b->Store(resultName, b->ConstInt64(0));
   matchkeyJIT(b, "matchKey", one, two);
   IlBuilder *matchKey=NULL;
   b->IfThen(&matchKey, b->EqualTo(b->Load("matchKey"), b->ConstInt64(1)));

   matchKey->Store("isStringMatched", matchKey->Call("matchString", 2, s1, s2));
   IlBuilder *matchString=NULL;
   matchKey->IfThen(&matchString, matchKey->EqualTo(matchKey->Load("isStringMatched"), matchKey->ConstInt64(1)));
   matchString->Store(resultName, matchString->ConstInt64(1));
}


/* Match suppycost */
void tpchq2::matchSupplyCostJIT(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, b->ConstInt64(0));
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->LessOrEqualTo(one, two));
   twoSmaller->Store(resultName, twoSmaller->ConstInt64(1));
}
/******************************BUILDIL**********************************************/

bool
tpchq2::buildIL()
   {
   
   OMR::JitBuilder::IlValue *part_size = Load("part_size");
   OMR::JitBuilder::IlValue *ps_size = Load("ps_size");
   OMR::JitBuilder::IlValue *s_size = Load("s_size");
   OMR::JitBuilder::IlValue *n_size = Load("n_size");
   OMR::JitBuilder::IlValue *t_size = Load("r_size");

   OMR::JitBuilder::IlBuilder* p_loop = NULL;
   ForLoopUp("p_index", &p_loop,
      ConstInt64(0),
      part_size,
      ConstInt64(1));
    
   //p_loop->Call("printString", 1, part_size);
   
   p_loop->Store("p_element",
   p_loop->   IndexAt(pStructTypePart,
   p_loop->      Load("p"),
   p_loop->      Load("p_index")));

   CallSubStringJIT(p_loop, "subStr_res", 
   p_loop->   Load("like_substring"),
   p_loop->   LoadIndirect("PART", "type",
   p_loop->      Load("p_element")),
   p_loop->   LoadIndirect("PART", "size",
   p_loop->      Load("p_element")),
   p_loop->   Load("p_size") );

   /* CallSubStringJITHelper(p_loop, "subStr_res", 
   p_loop->   Load("like_substring"),
   p_loop->   LoadIndirect("PART", "type",
   p_loop->      Load("p_element")),
   p_loop->   LoadIndirect("PART", "size",
   p_loop->      Load("p_element")) ); */

   OMR::JitBuilder::IlBuilder *subStrMatch = NULL;
   p_loop->IfThen(&subStrMatch,
   p_loop->   EqualTo(
   p_loop->      Load("subStr_res"),
   p_loop->      ConstInt64(1)));

   OMR::JitBuilder::IlBuilder* ps_loop = NULL;
   subStrMatch->ForLoopUp("ps_index", &ps_loop,
   subStrMatch->   ConstInt64(0),
                   ps_size,
   subStrMatch->   ConstInt64(1));

   ps_loop->Store("ps_element",
   ps_loop->   IndexAt(pStructTypePartsupp,
   ps_loop->      Load("ps"),
   ps_loop->      Load("ps_index")));

   matchkeyJIT(ps_loop, "match_partKey",
   ps_loop->   LoadIndirect("PART", "partKey",
   ps_loop->      Load("p_element")),
   ps_loop->   LoadIndirect("PARTSUPP", "partKey",
   ps_loop->      Load("ps_element")) );

   OMR::JitBuilder::IlBuilder *keyMatch = NULL;
   ps_loop->IfThen(&keyMatch,
   ps_loop->   EqualTo(
   ps_loop->      Load("match_partKey"),
   ps_loop->      ConstInt64(1)));
   //ps_loop->Call("printString", 1, ps_size);

   /*******************************Sub query***************************************************/
   
   DefineLocal("findMinSupplycost", Float);
   /* keyMatch->Store("findMinSupplycost", 
   keyMatch->   ConstFloat(10000)); */
   
   keyMatch->Store("flag", 
   keyMatch->   ConstInt64(0));

   OMR::JitBuilder::IlBuilder* sub_ps_loop = NULL;
   keyMatch->ForLoopUp("sub_ps_index", &sub_ps_loop,
   keyMatch->   ConstInt64(0),
                ps_size,
   keyMatch->   ConstInt64(1));

   sub_ps_loop->Store("sub_ps_element",
   sub_ps_loop->   IndexAt(pStructTypePartsupp,
   sub_ps_loop->      Load("ps"),
   sub_ps_loop->      Load("sub_ps_index")));

   matchkeyJIT(sub_ps_loop, "sub_match_partKey",
   sub_ps_loop->   LoadIndirect("PART", "partKey",
   sub_ps_loop->      Load("p_element")),
   sub_ps_loop->   LoadIndirect("PARTSUPP", "partKey",
   sub_ps_loop->      Load("sub_ps_element")) );
   
   
   OMR::JitBuilder::IlBuilder *subkeyMatch = NULL;
   sub_ps_loop->IfThen(&subkeyMatch,
   sub_ps_loop->   EqualTo(
   sub_ps_loop->      Load("sub_match_partKey"),
   sub_ps_loop->      ConstInt64(1)));

   OMR::JitBuilder::IlBuilder* sub_s_loop = NULL;
   subkeyMatch->ForLoopUp("sub_s_index", &sub_s_loop,
   subkeyMatch->   ConstInt64(0),
                   s_size,
   subkeyMatch->   ConstInt64(1));

   sub_s_loop->Store("sub_s_element",
   sub_s_loop->   IndexAt(pStructTypeSupplier,
   sub_s_loop->      Load("s"),
   sub_s_loop->      Load("sub_s_index")));

   matchkeyJIT(sub_s_loop, "sub_match_suppKey",
   sub_s_loop->   LoadIndirect("SUPPLIER", "suppKey",
   sub_s_loop->      Load("sub_s_element")),
   sub_s_loop->   LoadIndirect("PARTSUPP", "suppKey",
   sub_s_loop->      Load("sub_ps_element")) );
   
   OMR::JitBuilder::IlBuilder *sub_suppKeyMatch = NULL;
   sub_s_loop->IfThen(&sub_suppKeyMatch,
   sub_s_loop->   EqualTo(
   sub_s_loop->      Load("sub_match_suppKey"),
   sub_s_loop->      ConstInt64(1)));

   OMR::JitBuilder::IlBuilder* sub_n_loop = NULL;
   sub_suppKeyMatch->ForLoopUp("sub_n_index", &sub_n_loop,
   sub_suppKeyMatch->   ConstInt64(0),
                        n_size,
   sub_suppKeyMatch->   ConstInt64(1));

   sub_n_loop->Store("sub_n_element",
   sub_n_loop->   IndexAt(pStructTypeNation,
   sub_n_loop->      Load("n"),
   sub_n_loop->      Load("sub_n_index")));

   matchkeyJIT(sub_n_loop, "sub_match_nationKey",
   sub_n_loop->   LoadIndirect("SUPPLIER", "nationKey",
   sub_n_loop->      Load("sub_s_element")),
   sub_n_loop->   LoadIndirect("NATION", "nationKey",
   sub_n_loop->      Load("sub_n_element")) );
   
   OMR::JitBuilder::IlBuilder *sub_nationKeyMatch = NULL;
   sub_n_loop->IfThen(&sub_nationKeyMatch,
   sub_n_loop->   EqualTo(
   sub_n_loop->      Load("sub_match_nationKey"),
   sub_n_loop->      ConstInt64(1)));
   //sub_n_loop->Call("printString", 1, partKey);
   
   OMR::JitBuilder::IlBuilder* sub_t_loop = NULL;
   sub_nationKeyMatch->ForLoopUp("sub_t_index", &sub_t_loop,
   sub_nationKeyMatch->   ConstInt64(0),
                          t_size,
   sub_nationKeyMatch->   ConstInt64(1));

   sub_t_loop->Store("sub_t_element",
   sub_t_loop->   IndexAt(pStructTypeTEST,
   sub_t_loop->      Load("t"),
   sub_t_loop->      Load("sub_t_index")));

   matchNameRegionJIT(sub_t_loop, "sub_match_regionKey", 
   sub_t_loop->   LoadIndirect("TEST", "regionKey",
   sub_t_loop->      Load("sub_t_element")), 
   sub_t_loop->   LoadIndirect("NATION", "regionKey",
   sub_t_loop->      Load("sub_n_element")),
   sub_t_loop->   LoadIndirect("TEST", "name",
   sub_t_loop->      Load("sub_t_element")),
   sub_t_loop->   Load("region"));
   
   OMR::JitBuilder::IlBuilder *sub_regionKeyMatch = NULL;
   sub_t_loop->IfThen(&sub_regionKeyMatch,
   sub_t_loop->   EqualTo(
   sub_t_loop->      Load("sub_match_regionKey"),
   sub_t_loop->      ConstInt64(1)));

   matchkeyJIT(sub_regionKeyMatch, "sub_flag",
   sub_regionKeyMatch->   Load("flag"),
   sub_regionKeyMatch->   ConstInt64(0) );

   OMR::JitBuilder::IlBuilder *sub_flagMatch = NULL;
   sub_regionKeyMatch->IfThen(&sub_flagMatch,
   sub_regionKeyMatch->   EqualTo(
   sub_regionKeyMatch->      Load("sub_flag"),
   sub_regionKeyMatch->      ConstInt64(1)));
   
   sub_flagMatch->Store("findMinSupplycost",
   sub_flagMatch->      LoadIndirect("PARTSUPP", "supplyCost",
   sub_flagMatch->         Load("sub_ps_element")));

   sub_flagMatch->Store("flag",
   sub_flagMatch->   ConstInt64(1));

   matchSupplyCostJIT(sub_regionKeyMatch, "sub_suppCost", 
   sub_regionKeyMatch->   LoadIndirect("PARTSUPP", "supplyCost",
   sub_regionKeyMatch->      Load("sub_ps_element")),
   sub_regionKeyMatch->   Load("findMinSupplycost"));
   
   OMR::JitBuilder::IlBuilder *sub_suppcostMatch = NULL;
   sub_regionKeyMatch->IfThen(&sub_suppcostMatch,
   sub_regionKeyMatch->   EqualTo(
   sub_regionKeyMatch->      Load("sub_suppCost"),
   sub_regionKeyMatch->      ConstInt64(1)));

   sub_suppcostMatch->Store("findMinSupplycost",
   sub_suppcostMatch->      LoadIndirect("PARTSUPP", "supplyCost",
   sub_suppcostMatch->         Load("sub_ps_element")));
   
   
   /***************************Sub query end******************************************/

   matchkeyCostJIT(keyMatch, "match_sub_suppCost", 
   keyMatch->   Load("findMinSupplycost"), 
   keyMatch->   LoadIndirect("PARTSUPP", "supplyCost",
   keyMatch->      Load("ps_element")));

   OMR::JitBuilder::IlBuilder *subkeyCostMatch = NULL;
   keyMatch->IfThen(&subkeyCostMatch,
   keyMatch->   EqualTo(
   keyMatch->      Load("match_sub_suppCost"),
   keyMatch->      ConstInt64(1)));
   
   OMR::JitBuilder::IlBuilder* s_loop = NULL;
   subkeyCostMatch->ForLoopUp("s_index", &s_loop,
   subkeyCostMatch->   ConstInt64(0),
                   s_size,
   subkeyCostMatch->   ConstInt64(1));

   s_loop->Store("s_element",
   s_loop->   IndexAt(pStructTypeSupplier,
   s_loop->      Load("s"),
   s_loop->      Load("s_index")));

   matchkeyJIT(s_loop, "match_suppKey",
   s_loop->   LoadIndirect("SUPPLIER", "suppKey",
   s_loop->      Load("s_element")),
   s_loop->   LoadIndirect("PARTSUPP", "suppKey",
   s_loop->      Load("ps_element")) );

   OMR::JitBuilder::IlBuilder *suppKeyMatch = NULL;
   s_loop->IfThen(&suppKeyMatch,
   s_loop->   EqualTo(
   s_loop->      Load("match_suppKey"),
   s_loop->      ConstInt64(1)));
   
   OMR::JitBuilder::IlBuilder* n_loop = NULL;
   suppKeyMatch->ForLoopUp("n_index", &n_loop,
   suppKeyMatch->   ConstInt64(0),
                    n_size,
   suppKeyMatch->   ConstInt64(1));

   n_loop->Store("n_element",
   n_loop->   IndexAt(pStructTypeNation,
   n_loop->      Load("n"),
   n_loop->      Load("n_index")));

   matchkeyJIT(n_loop, "match_nationKey",
   n_loop->   LoadIndirect("SUPPLIER", "nationKey",
   n_loop->      Load("s_element")),
   n_loop->   LoadIndirect("NATION", "nationKey",
   n_loop->      Load("n_element")) );

   OMR::JitBuilder::IlBuilder *nationKeyMatch = NULL;
   n_loop->IfThen(&nationKeyMatch,
   n_loop->   EqualTo(
   n_loop->      Load("match_nationKey"),
   n_loop->      ConstInt64(1)));


   OMR::JitBuilder::IlBuilder* t_loop = NULL;
   nationKeyMatch->ForLoopUp("t_index", &t_loop,
   nationKeyMatch->   ConstInt64(0),
                      t_size,
   nationKeyMatch->   ConstInt64(1));

   t_loop->Store("t_element",
   t_loop->   IndexAt(pStructTypeTEST,
   t_loop->      Load("t"),
   t_loop->      Load("t_index")));

   matchNameRegionJIT(t_loop, "match_regionKey", 
   t_loop->   LoadIndirect("TEST", "regionKey",
   t_loop->      Load("t_element")),
   t_loop->   LoadIndirect("NATION", "regionKey",
   t_loop->      Load("n_element")), 
   t_loop->   LoadIndirect("TEST", "name",
   t_loop->      Load("t_element")),
   t_loop->   Load("region"));

   OMR::JitBuilder::IlBuilder *regionKeyMatch = NULL;
   t_loop->IfThen(&regionKeyMatch,
   t_loop->   EqualTo(
   t_loop->      Load("match_regionKey"),
   t_loop->      ConstInt64(1)));
   
   
   regionKeyMatch->Call("storeResult", 8,
   regionKeyMatch->      LoadIndirect("SUPPLIER", "acctBal",
   regionKeyMatch->         Load("s_element")), 
   regionKeyMatch->      LoadIndirect("SUPPLIER", "name",
   regionKeyMatch->         Load("s_element")),
   regionKeyMatch->      LoadIndirect("NATION", "name",
   regionKeyMatch->         Load("n_element")), 
   regionKeyMatch->      LoadIndirect("PART", "partKey",
   regionKeyMatch->         Load("p_element")),
   regionKeyMatch->      LoadIndirect("PART", "mfgr",
   regionKeyMatch->         Load("p_element")), 
   regionKeyMatch->      LoadIndirect("SUPPLIER", "address",
   regionKeyMatch->         Load("s_element")),
   regionKeyMatch->      LoadIndirect("SUPPLIER", "phone",
   regionKeyMatch->         Load("s_element")), 
   regionKeyMatch->      LoadIndirect("SUPPLIER", "comment",
   regionKeyMatch->         Load("s_element")));

   /* regionKeyMatch->StoreIndirect("RESULT", "s_acctbal",
   regionKeyMatch->   Load("s_element"),
   regionKeyMatch->   Load("i")); */

   Return(ConstInt64(0));

   return true;
   }

/***************************************MergeSort methodbuilder********************************************************/
mergeSort::mergeSort(OMR::JitBuilder::TypeDictionary *types)
   : OMR::JitBuilder::MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("mergeSort");

   pStr = types->toIlType<char *>();

   StructTypeRES      = types->LookupStruct("RESULT");
   pStructTypeRES     = types->PointerTo(StructTypeRES);

   DefineParameter("res",      pStructTypeRES);
   DefineParameter("res_size", Int64);

   DefineFunction((char *)"printString", 
                  (char *)__FILE__,
                  (char *)PRINTSTRING_LINE,
                  (void *)&printString,
                  NoType,
                  1,
                  Int64);
    
    DefineFunction((char *)"minValue", 
                  (char *)__FILE__,
                  (char *)MINVALUE_LINE,
                  (void *)&minValue,
                  Int64,
                  2,
                  Int64,
                  Int64);

    DefineFunction((char *)"merge", 
                  (char *)__FILE__,
                  (char *)MERGE_LINE,
                  (void *)&merge,
                  NoType,
                  3,
                  Int64,
                  Int64,
                  Int64);

    DefineFunction((char *)"orderBy", 
                  (char *)__FILE__,
                  (char *)ORDERBY_LINE,
                  (void *)&orderBy,
                  NoType,
                  1,
                  Int64);

   DefineReturnType(NoType);
   }

class resultTypeDictionary : public OMR::JitBuilder::TypeDictionary
   {
   public:
   resultTypeDictionary() :
      OMR::JitBuilder::TypeDictionary()
      {
      /* Declare RESULT Table */
      DEFINE_STRUCT(RESULT);

      DEFINE_FIELD(RESULT, s_acctbal, Float);
      DEFINE_FIELD(RESULT, s_name, toIlType<const char *>());
      DEFINE_FIELD(RESULT, n_name, toIlType<const char *>());
      DEFINE_FIELD(RESULT, p_partkey, Int64);
      DEFINE_FIELD(RESULT, p_mfgr, toIlType<const char *>());
      DEFINE_FIELD(RESULT, s_address, toIlType<const char *>());
      DEFINE_FIELD(RESULT, s_phone, toIlType<const char *>());
      DEFINE_FIELD(RESULT, s_comment, toIlType<const char *>());

      CLOSE_STRUCT(RESULT);
      }
   };

void mergeSort::generateMinValue(IlBuilder *b, char *resultName, OMR::JitBuilder::IlValue *one, OMR::JitBuilder::IlValue *two)
{
   b->Store(resultName, one);
   IlBuilder *twoSmaller=NULL;
   b->IfThen(&twoSmaller, b->LessThan(two, one));
   twoSmaller->Store(resultName, two);
}

/*BuildIL for merge sort*/
bool
mergeSort::buildIL()
   {
   OMR::JitBuilder::IlValue *res_size = Load("res_size");
   
   //result_loop->Call("printString", 1, result_loop->Load("curr_size"));
   OMR::JitBuilder::IlBuilder* result_loop = NULL;
   ForLoopUp("curr_size", &result_loop,
      ConstInt64(1),
      res_size,
      ConstInt64(0));
   
   //result_loop->Call("printString", 1, result_loop->Load("curr_size"));
   OMR::JitBuilder::IlBuilder* subArray_loop = NULL;
   result_loop-> ForLoopUp("left_start", &subArray_loop,
   result_loop->   ConstInt64(0),
   result_loop->   Sub(res_size,
   result_loop->      ConstInt64(1)),
   result_loop->   ConstInt64(0));

   //subArray_loop->Call("printString", 1, subArray_loop->Load("left_start"));

   generateMinValue(subArray_loop, "mid",
   subArray_loop-> Sub(
   subArray_loop->    Add(
   subArray_loop->       Load("left_start"),
   subArray_loop->       Load("curr_size")),
   subArray_loop->    ConstInt64(1)),
   subArray_loop-> Sub(res_size,
   subArray_loop->    ConstInt64(1)) );
   //subArray_loop->Call("printString", 1, subArray_loop->Load("mid"));

   generateMinValue(subArray_loop, "right_end",
   subArray_loop-> Sub(
   subArray_loop->    Add(
   subArray_loop->       Load("left_start"), 
   subArray_loop->       Add(
   subArray_loop->          Load("curr_size"), 
   subArray_loop->          Load("curr_size")) ), 
   subArray_loop->       ConstInt64(1)),
   subArray_loop-> Sub(res_size,
   subArray_loop->    ConstInt64(1)) );
   //subArray_loop->Call("printString", 1, subArray_loop->Load("mid"));

   subArray_loop->Call("merge", 3, 
   subArray_loop->   Load("left_start"), 
   subArray_loop->   Load("mid"), 
   subArray_loop->   Load("right_end"));
   
   /* subArray_loop increment logic */
   subArray_loop->Store("left_start",
   subArray_loop->   Add(
   subArray_loop->      Load("left_start"),
   subArray_loop->      Add(
   subArray_loop->         Load("curr_size"), 
   subArray_loop->         Load("curr_size")) ));
   //subArray_loop->Call("printString", 1, subArray_loop->Load("left_start"));

   /* result_loop increment logic */
   result_loop->Store("curr_size",
   result_loop->   Add(
   result_loop->      Load("curr_size"),
   result_loop->      Load("curr_size") ));

   Call("orderBy",1, res_size);

   Return(ConstInt64(0));
   return true;
   }
/**************************************************Entry for exectution*************************************************/
struct time
tpchq2::run(const char* partTable, const char* supplierTable, const char* partSuppTable, const char* nationTable, const char* regionTable, 
                   int64_t p_size, const char* region, const char* like_substring)
{
    /* 1. Load the tables into the memory -- Time not considered for loading tables in memory*/
   matrix partTemp, supplierTemp, partSuppTemp, nationTemp, regionTemp;
   int i, j;

   partTemp = readFile(partTable);
   /* Declare PART table array */
   struct PART *p = (PART*)malloc(partTemp.size() * sizeof(PART)); 
   int64_t PART_size = partTemp.size();
   for(i = 0; i < partTemp.size(); i++){
       j = 0;
       
       p[i].partKey     = atoi(partTemp[i][j++].c_str());
       p[i].name        = partTemp[i][j++].c_str();
       p[i].mfgr        = partTemp[i][j++].c_str();
       p[i].brand       = partTemp[i][j++].c_str();
       p[i].type        = partTemp[i][j++].c_str();
       p[i].size        = atoi(partTemp[i][j++].c_str());
       p[i].container   = partTemp[i][j++].c_str();
       p[i].retailPrice = atof(partTemp[i][j++].c_str());
       p[i].comment     = partTemp[i][j++].c_str();

       j = 0;
   }

   supplierTemp = readFile(supplierTable);
   /* Declare SUPPLIER table array*/
   struct SUPPLIER *s = (SUPPLIER*)malloc(supplierTemp.size() * sizeof(SUPPLIER));
   int64_t SUPPLIER_size = supplierTemp.size();
   for(i = 0; i < supplierTemp.size(); i++){
       j = 0;

       s[i].suppKey     = atoi(supplierTemp[i][j++].c_str());
       s[i].name        = supplierTemp[i][j++].c_str();
       s[i].address     = supplierTemp[i][j++].c_str();
       s[i].nationKey   = atoi(supplierTemp[i][j++].c_str());
       s[i].phone       = supplierTemp[i][j++].c_str();
       s[i].acctBal     = atof(supplierTemp[i][j++].c_str());
       s[i].comment     = supplierTemp[i][j++].c_str();

       j = 0;
   }


   
   partSuppTemp = readFile(partSuppTable);
   /* Declare PARTSUPP table array*/
   struct PARTSUPP *ps = (PARTSUPP*)malloc(partSuppTemp.size() * sizeof(PARTSUPP));
   int64_t PARTSUPP_size = partSuppTemp.size();
   for(i = 0; i < partSuppTemp.size(); i++){
       j = 0;

       ps[i].partKey    = atoi(partSuppTemp[i][j++].c_str());
       ps[i].suppKey    = atoi(partSuppTemp[i][j++].c_str());
       ps[i].availQty   = atoi(partSuppTemp[i][j++].c_str());
       ps[i].supplyCost = atof(partSuppTemp[i][j++].c_str());
       ps[i].comment    = partSuppTemp[i][j++].c_str();

       j = 0;
   }

   
   
   nationTemp = readFile(nationTable);
   /* Declare NATION table array*/
   struct NATION *n = (NATION*)malloc(nationTemp.size() * sizeof(NATION));
   int64_t NATION_size = nationTemp.size();
   for(i = 0; i < nationTemp.size(); i++){
       j = 0;
 
       n[i].nationKey   = atoi(nationTemp[i][j++].c_str());
       n[i].name        = nationTemp[i][j++].c_str();
       n[i].regionKey   = atoi(nationTemp[i][j++].c_str());;
       n[i].comment     = nationTemp[i][j++].c_str();

       j = 0;
   }


   
   regionTemp = readFile(regionTable);
   /* Declare REGION table array*/
   //struct REGION *r = (REGION*)malloc(regionTemp.size() * sizeof(REGION));
   struct TEST *t = (TEST*)malloc(regionTemp.size() * sizeof(TEST));
   int64_t REGION_size = regionTemp.size();
   for(i = 0; i < regionTemp.size(); i++){
       j = 0;

       t[i].regionKey   = atoi(regionTemp[i][j++].c_str());
       t[i].name        = regionTemp[i][j++].c_str();
       t[i].comment     = regionTemp[i][j++].c_str();

       j = 0;
   }


   /* for(int i = 0; i < regionTemp.size(); i++){
       cout<<t[i].regionKey<< "   "<<t[i].name<<"   "<<t[i].comment<<endl;
   } */

   //printMatrix(partSuppTemp);

   int p_index;
   int s_index;
   int ps_index;
   int n_index;
   int r_index;

   /* Define struct RESULT to store the result */
   result = (RESULT*)malloc(PARTSUPP_size * sizeof(RESULT));
   
   /*********************************Using JIT API***************************************************/  

   printf("Step 1: initialize JIT\n");
   bool initialized = initializeJit();
   if (!initialized)
      {
      fprintf(stderr, "FAIL: could not initialize JIT\n");
      exit(-1);
      }

   printf("Step 2: define type dictionaries\n");
   StructArrayTypeDictionary methodTypes;

   /* result type dictionary */
   resultTypeDictionary resMethodTypes;

   printf("Step 3: compile tpchq2 method builder\n");
   
   /*******************compile tpchq2*********************/

   auto start_compile = high_resolution_clock::now(); 

   tpchq2 method(&methodTypes);
   void *entry=0;
   int32_t rc = compileMethodBuilder(&method, &entry);

   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   auto stop_compile = high_resolution_clock::now();
   auto compile_duration = duration_cast<microseconds>(stop_compile - start_compile);
   /*******************tpchq2 compilation done**********************/

   /*******************Mergesort compile*******************************/

   auto start_compile_mergeSort = high_resolution_clock::now(); 

   mergeSort mergeMethod(&resMethodTypes);
   void *mergeEntry = 0;
   rc = compileMethodBuilder(&mergeMethod, &mergeEntry);
   if (rc != 0)
      {
      
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   auto stop_compile_mergeSort = high_resolution_clock::now();
   auto compile_duration_mergeSort = duration_cast<microseconds>(stop_compile_mergeSort - start_compile_mergeSort);

   /*******************MergeSort compilation done**********************/
  
   printf("Step 4: invoke tpchq2 compiled code\n");
   
   /*******************invoke tpchq2**************************************/
   auto start_exec = high_resolution_clock::now(); 

   tpchq2FunctionType *test = (tpchq2FunctionType *)entry;
   test(p, PART_size, s, SUPPLIER_size, ps, PARTSUPP_size, n, NATION_size, p_size, region, like_substring, REGION_size, t, total_result, result);
   
   auto stop_exec = high_resolution_clock::now();
   auto exec_duration = duration_cast<microseconds>(stop_exec - start_exec);
   /*******************tpchq2 Execution Completed**************************************/
   
   /*******************invoke MergeSort**************************************/
   auto start_exec_mergeSort = high_resolution_clock::now();  

   mergeSortFunctionType *mergeSortTest = (mergeSortFunctionType *)mergeEntry;
   mergeSortTest(result, total_result);
   
   auto stop_exec_mergeSort = high_resolution_clock::now();
   auto exec_duration_mergeSort = duration_cast<microseconds>(stop_exec_mergeSort - start_exec_mergeSort);
   /*******************MergeSort Execution done******************************/
   
   printf ("Step 5: shutdown JIT\n");
   shutdownJit();

   /*******************************JIT task Completed*********************************************************************/
   
   /* Print the final results */
   //printResult(result, total_result);

   /* Free the memory allocated */
   free(p);
   free(ps);
   free(s);
   free(n);
   free(t);
   free(result);
   total_result = 0;
   
   /* Store the time recorded */
   struct time time;
   time.compileTime = compile_duration.count();
   time.executionTime = exec_duration.count();
   time.compileTime_mergeSort = compile_duration_mergeSort.count();
   time.executionTime_mergeSort = exec_duration_mergeSort.count();
   return time;
}