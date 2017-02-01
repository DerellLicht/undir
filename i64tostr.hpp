//*****************************************************************************
//  i64tostr class - header file
//  This class takes a Microsoft __int64 or (unsigned __int64) variable
//  and converts it into a comma-delimited text string for displaying.
//  
//  Written by:   Daniel D. Miller
//  
//  Last Update:  12/16/98 05:11PM
//  
//*****************************************************************************
//  This class requires a 32-bit Microsoft C++ compiler
//  (Visual C++ version 4.00 or later)
//*****************************************************************************
#include <stdio.h>

class i64tostr {
private:
   char *outstr ;
   char i64str[40] ; //  temp string for creating converted string
public:
   i64tostr(unsigned long long uli) ;
   ~i64tostr(void) { delete outstr ; } ;
   void convert(unsigned long long indata) ;
   char *putstr(void) { return outstr ; } ;
};

