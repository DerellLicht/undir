//*****************************************************************************
//  i64tostr class                                                             
//  This class takes a Microsoft __int64 or (unsigned __int64) variable        
//  and converts it into a comma-delimited text string for displaying.         
//                                                                             
//  Written by:   Daniel D. Miller                                             
//                                                                             
//  Last Update:  01/13/99 04:21PM                                             
//                                                                             
//  compile with:  cl /c /O2 /G3 /W3 i64tostr.cpp                              
//*****************************************************************************
//  This class requires a 32-bit Microsoft C++ compiler
//  (Visual C++ version 4.00 or later)
//*****************************************************************************
// #include <stdio.h>
#include <stdlib.h>  //  _ui64toa() 
#include <string.h>
#include "i64tostr.hpp"

//*****************************************************************************
i64tostr::i64tostr(unsigned long long uli) 
   // outstr(new char[30])
   {
   outstr = new char[30] ;
   i64str[0] = 0 ;
   // if (outstr == 0) {
   //    exit(1) ;
   // }
   convert(uli) ;
   }

//*****************************************************************************
void i64tostr::convert(unsigned long long uli)
   {
   int slen, inIdx, j ;
   char *strptr ;

   // _ui64toa(uli, i64str, 10) ;
   sprintf(i64str, "%llu", uli) ;
   slen = strlen(i64str) ;
   inIdx = --slen ;//  convert byte-count to string index 

   //  put NULL at end of temp string
   strptr = outstr + 29 ;
   *strptr-- = 0 ;   //  make sure there's a NULL-terminator

   for (j=0; j<slen; j++)
      {
      *strptr-- = i64str[inIdx--] ;
      if ((j+1) % 3 == 0)
         *strptr-- = ',' ;
      }
   *strptr = i64str[inIdx] ;

   strcpy(outstr, strptr) ;
   }

