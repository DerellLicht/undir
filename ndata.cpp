//*****************************************************************
//                                                                 
//  NDATA.CPP - NDIR global data declarations                      
//                                                                 
//  Written by:  Daniel D. Miller  (the Derelict)                  
//                                                                 
//  Last update:  01/15/01 20:25                                   
//                                                                 
//  compile with makefile                                          
//                                                                 
//*****************************************************************

#include <stdio.h>

#include "ndata.hpp"

//*****************************************************************
//                  CONFIGURATION VARIABLES (new style)
//*****************************************************************
ndir_data n =
{
   //  default colors
   0x13, 0x02, 0x0A, 0x08, 0x01, 0x05, 0x0C, 0x04, 0x18, 0x06, 0x09, 
   0x11, 0x11, 0x11, 0x11, 0x12, //  new Linux colors

   //*********************************************************
   //                program state flags                      
   //*********************************************************
   0, //  uppercase = false
   1, //  sort by filename
   0, //  no reverse
   1, //  clearscreen = true
   1, //  pause on fullscreen = true
   2, //  display format = 4column
   0, //  Minimize display (make abbreviated header/footer)
   0, //  display normal files, not tree
   1, //  show all files = true
   1, //  list directories first = true
   0, //  show executables only = false
   0, //  display help screen = false
   0, //  display information screen = false
   0, //  list files horizontally = false
   0, //  use low-ASCII characters = false
   0, //  display in batch mode = false
   0, //  unused1
   1, //  display files in color = true
   0, //  showSHRfiles
   0, //  switch to EGA43/VGA50 line mode
   0, //  display disk_drive summary
   0, //  file-date option (MSDOS 7.00+ only)
   0, //  1 = force old DOS 8.3 filenames in MSDOS 7.00+
   0  //  0=normal tree, 1=show only one level of children
};

attrib_list attr_table[MAX_EXT] ;
unsigned attrib_count = 0 ;
uchar dtree_colors[MAX_DIR_ENTRY] ;

//*****************************************************************
//              END OF CONFIGURATION VARIABLES
//*****************************************************************

// int lfn_supported = 1 ; //  always true for 32-bit version
u64 diskbytes, diskfree ;
unsigned clbytes ;

//*****************************************************************
ffdata *ftop = NULL ;

char excl[20][_MAX_PATH] ; //  allocate dynamically??
// char volume_name[_MAX_PATH] ;

//  name of drive+path without filenames
char base_path[1024] ;
// unsigned base_len ;  //  length of base_path

// unsigned tcount = 0 ;   //  number of target filespecs
int  exclcount = 0 ;     //  number of exclusion filespecs
char leftstr[37], rightstr[37] ; //  used by batch mode
char tempstr[_MAX_PATH] ;
uchar tline, bline, vline, xline, dline ;
int  filecount = 0 ;

int  columns ;           //  number of display columns on screen
unsigned lines, linecnt ;

//*********************************************************
//  NDIR information screen
//*********************************************************
char *idtxt[] = {
   " ",
   "Copyright 1990, 1993-2005 by:",
   " ",
   "          Daniel D. Miller",
   "          4835 Coco Palm Drive",
   "          Fremont, CA  94538",
   " ",
   "          Email:    derelict@comcast.net",
   "          Website:  home.comcast.net/~derelict",
   " ",
   "This program, UNDIR.EXE, and its associated files, are hereby released as",
   "Freeware, though I retain the copyrights on them.  Please feel free to",
   "distribute copies to anyone who is (or might be) interested in them.",
   " ",
   "If you like this program, a letter, post card or Email telling me what you",
   "do or don't like would be very welcome, and will help me decide what ",
   "features to add in the future.",
   NULL } ;

//*********************************************************
//  NDIR help screen
//*********************************************************
char *helptxt[] = {
" ",
"USAGE:  NDIR <filespecs> -options or /options !<exclusions>",
" ",
" OPTIONS                      FUNCTIONS",
"   -a *     List ALL files (hidden, system, read-only, etc.).",
"   -c *     Clear screen before listing.",
"   -p *     Pause on full screen.",
"   -m *     Minimize size of header and footer.",
"   -w *     Use special colors for system/hidden/readonly files.",
"   -d       dir TREE: normal size display (work with -s, -n (default), -r).",
"   -d2      dir TREE: file/directory counts",
"   -d3      dir TREE: mixed size and file/directory counts",
"   -e       Sort by extension.",
"   -n        \"   by name.",
"   -s        \"   by file size, smallest first.",
"   -t        \"   by Date, oldest first.",
"   -z        \"   by DOS order (no sort).",
"   -r *     Reverse normal sort order.",
"   -1       Display one column,   with name/size/date/attr.",
"   -2          \"    two   \"   ,   with name/size/date.",
"   -4          \"    four  \"   ,   with name/size.",
"   -6          \"    six   \"   ,   with name only.",
"   -l *     Toggle long-filename enable flag (NDIR16 and MSDOS 7.00+ only)",
"   -k *     Toggle color mode.",
"   -j *     Use standard ASCII (for redirection). (forces -k)",
"   -5 *     Switch to EGA 43-line/VGA 50-line mode.",
"   -u *     List filenames in UPPERCASE.",
"   -oN      Date/Time display: 0=Last Write, 1=Last Access, 2=File Created",
"   -x *     List executables only (.EXE,.COM,.BAT).",
"   -i       Show summary of data for all disk mounts",
"   -v       Display registration/update information.",
"   -?       Display HELP screen.",
"   -g *     List directories FIRST.",
"   -h *     List files horizontally.",
"   -f *     List files only (No directories).",
"   -, *     Dir Tree: show only one level of subdirectories.",
" ",
"   -b       Batch mode;  files listed in one column.",
"            (This format can be redirected to a batch file)",
"   [\"string\"  specifies a string BEFORE each filename (Batch mode)",
"   ]\"string\"  specifies a string AFTER  each filename (Batch mode)",
" ",
"NOTE: items with a * after the flag are TOGGLES",
" ",
NULL } ;

