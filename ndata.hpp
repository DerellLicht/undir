//************************************************************
//    NDATA.HPP - Header file for NDIR directory lister.      
//                                                            
//    Written by:  Daniel D. Miller                           
//                                                            
//    Last Update: 05/31/02 16:57                             
//                                                            
//************************************************************

#ifndef NDATA_HPP
#define NDATA_HPP

extern char *Version ;
extern char *ShortVersion;

#include "win2unix.hpp"

typedef  unsigned long long   u64 ;
typedef  unsigned char  uchar ;

//*********************************************************
#define  MAX_EXT  200
#define  MAX_EXT_SIZE   6
struct attrib_list {
   uchar  attr ;
   char  ext[MAX_EXT_SIZE+1] ;
} ;
extern attrib_list attr_table[MAX_EXT] ;

extern unsigned attrib_count ;

#define  MAX_DIR_ENTRY     20
extern uchar dtree_colors[MAX_DIR_ENTRY] ;

//*********************************************************
// union i64tol {
//    u64 i ;
//    ulong u[2] ;
// };

//*****************************************************************
//  CONFIGURATION VARIABLES
//*****************************************************************
struct ndir_data {
   uchar colorlogo   ;
   uchar colornhead  ;
   uchar colorxhead  ;
   uchar colorframe  ;
   uchar colorattr   ;
   uchar colorsize   ;
   uchar colordate   ;
   uchar colortime   ;
   uchar colorSHR ;
   uchar colordefalt ;
   uchar colordir    ;
   //  next four attribs are used only on Linux
   uchar colorlink   ;
   uchar colorbdev   ;
   uchar colorcdev   ;
   uchar colorfifo   ;
   uchar colorexec   ;

   uchar ucase ;
   uchar sort ;        //  0=ext, 1=name, 2=size, 3=date/time, 4=none,
   uchar reverse ;     //  reverse normal sort order
   uchar clear ;
   uchar pause ;
   uchar format ;      //  0=1col, 1=2col, 2=4col, 3=6col, 4=dirtree, 5=3col
   uchar minimize ;
   uchar tree ;        //  0=all, 1=dir tree, 2=files only, 3=unused
   uchar show_all ;
   uchar dir_first ;
   uchar exec_only ;
   uchar help ;
   uchar info ;
   uchar horz ;
   uchar low_ascii ;
   uchar batch ;
   uchar unused1 ;
   uchar color ;
   uchar showSHRfiles ;
   uchar ega_keep ;
   uchar drive_summary ;   //  not supported in linux version
   uchar fdate_option ; //  0 = LAST_MODIFY, 1 = FILE_CREATE (MSDOS 7.00+ only)
   uchar lfn_off ;      //  0 = force DOS 8.3 filenames (MSDOS 7.00+ only)
   uchar tree_short ;   //  0=normal tree, 1=show only one level of children
};
extern ndir_data n ;

//  fdate_option flags
#define  FDATE_LAST_WRITE     0
#define  FDATE_LAST_ACCESS    1
#define  FDATE_CREATE_TIME    2

//*****************************************************************
//              END OF CONFIGURATION VARIABLES
//*****************************************************************

//***************************************************************
// extern WIN32_FIND_DATA fdata ;

// extern char fsn_bfr[32] ;  //  buffer for name of lfn file system
// extern char *dname ;       //  used by is_lfn_supported() call

// extern int lfn_supported ; //  always true for 32-bit version

extern u64 diskbytes, diskfree ;
extern unsigned clbytes ;

//************************************************************
#define topchar       0xD1   /*  Ñ  */
#define bottomchar    0xCF   /*  Ï  */
#define vlinechar     0xB3   /*  ³  */
#define xlinechar     0xD8   /*  Ø  */
#define dvlinechar    0xCD   /*  Í  */
#define crosschar     0x2B   /*  +  */
#define altvlinechar  0x7C   /*  |  */
#define altdvlchar    0x3D   /*  =  */

extern uchar tline, bline, vline, xline, dline ;

/************************************************************/
struct ffdata {
//    uchar          attrib ;
   unsigned       attrib ;
   unsigned       ft ;
   long long      fsize ;
   char           *filepath ;
   char           *filename ;
   char           *name ;  //  short filename
   char           ext[MAX_EXT_SIZE+1] ;
   char           *linktgt ;  //  if filename is a symlink, this contains name
   uchar          color ;
   uchar          dirflag ;   //  actually, file type
   struct ffdata  *next ;
   } ;
extern ffdata *ftop ;

//**********************************************************
// extern char* target[20] ;
// extern target_list_p target ;
// extern target_list_p tgtail ;

extern char excl[20][_MAX_PATH] ;         //  20 * 260
// extern char volume_name[_MAX_PATH] ;

//*********************************************************
//                NSORT.CPP
//*********************************************************
enum sort_modes {
SORT_EXT   = 0,
SORT_NAME     ,
SORT_SIZE     ,
SORT_DATE     ,
SORT_EXT_REV  ,
SORT_NAME_REV ,
SORT_SIZE_REV ,
SORT_DATE_REV ,
SORT_INV_TYPE } ;

//************************************************************
//  ntypes.cpp
//************************************************************
extern char *idtxt[] ;
extern char *helptxt[] ;

//  name of drive+path without filenames
extern char base_path[1024] ;
// extern unsigned base_len ;  //  length of base_path

//************************************************************
//  ndata.cpp
//************************************************************
extern char tempstr[_MAX_PATH] ;
extern int  filecount ;             //  number of files found
extern int  exclcount ;             //  number of exclusion filespecs
extern char leftstr[37], rightstr[37] ; //  used by batch mode

extern unsigned lines ;
extern int columns ;                //  number of display columns on screen
extern unsigned linecnt ;           //  non-color display line counter

//*****************  function prototypes**********************

//  NDIR.CPP
void error_exit(int errcode, char* outstr);

//  CMD_LINE.CPP
void verify_flags(void);

//  FILELIST.CPP
void display_files(void);
int  get_disk_info(char *dstr);

//  FILEREAD.CPP
void file_listing(void);
void file_list_listing (void);

//  TREELIST.CPP
void tree_listing(void);

//  NDISPLAY.CPP
void info(char *data[]);
void display_logo(void);
void print1(ffdata *fptr);
// void print2(ffdata *fptr);
// void print3(ffdata *fptr);
// void print4(ffdata *fptr);
// void print6(ffdata *fptr);
void ngotoxy(int x, int y);
void ncrlf(void);
void nputc(uchar attr, const uchar outchr);
void nputs(uchar attr, const char* outstr);
void nput_char(uchar attr, char chr, int count);
void nput_line(uchar attr, char chr);

#endif
