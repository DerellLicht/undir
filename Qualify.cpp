/******************************************************************/
/*  qualify() modifies a string as required to generate a         */
/*  "fully-qualified" filename, which is a filename that          */
/*  complete drive specifier and path name.                       */
/*                                                                */
/*  input:  argptr: the input filename.                           */
/*                                                                */
/*  output: qresult, a bit-mapped unsigned int with the           */
/*                   following definitions:                       */
/*                                                                */
/*          bit 0 == 1 if wildcards are present.                  */
/*          bit 1 == 1 if no wildcards and path does not exist.   */
/*          bit 2 == 1 if no wildcards and path exists as a file. */
/*          bit 7 == 1 if specified drive is invalid.             */
/*                                                                */
/******************************************************************/
//  compile with:  cl /W3 /Zp1 /c qualify.cpp lfnfcnc.obj           
//                                         or lfnfcns.obj           
//                                         or lfnfcnl.obj           
//******************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>				  //  strlen()
#include <sys/stat.h>
#include <ctype.h>				  //  tolower()

/***********************  qualify() constants  ************************/
#define  QUAL_WILDCARDS    0x01
#define  QUAL_NO_PATH      0x02
#define  QUAL_IS_FILE      0x04
#define  QUAL_INV_DRIVE    0x80

static char path[260];

//***************************************************************
// static WIN32_FIND_DATA fffdata;

/******************************************************************/
unsigned qualify (char *argptr, int lfn_off)
{
   struct stat st ;
   return (stat(argptr, &st) == 0) ? 0 : QUAL_INV_DRIVE ;
}

