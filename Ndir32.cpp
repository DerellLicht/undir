//**************************************************************************************
//                                                                 
//  UNDIR32.CPP - Unix version of my NDIR utility
//                                                                 
//  Written by:   Daniel D. Miller  (the derelict)                 
//                                                                 
//  compile with makefile                                          
//                                                                 
//**************************************************************************************
//       Revision history
//    
// 1.09  02/08/05 09:07 
//    - Initialize strlist struct on allocation
//    - Fix field length in -2 option
//    
// 1.10  02/09/05 09:40 
//    Add support for Linux-specific colors
//    
// 1.11  02/16/05 12:34 
//    Try to make symlinks also show target filename
//    
// 1.12  02/18/05 15:35 
//    When generating new ini file, add new color options
//    
// 1.13  03/30/05 14:24 
//    Try to support filespecs
//    
// 1.14  04/05/05 11:20 
//    Now filespecs work, but directories don't; fixed.
//    *Still* doesn't support multiple directories
//    
// 1.15  04/07/05 12:30 
//    - Clean up file-read code
//    - Try to restore support for multiple directories
//    
// 1.16  04/19/05 17:56 
//    Generate fully-qualified paths for searching
//    
// 1.17  04/20/05 11:46 
//    Fix cluster-size management in tree listing
//    
// 1.18  04/25/05 11:46 
//    Build with flag to support 64-bit files 
//    
// 1.19  12/12/05 13:03 
//    More mucking around with paths to try to solve file/path
//    resolution differences between *nix and Windows/dos
//    
// 1.20  12/27/05 15:02 
//    Change how rows/columns are extracted from stty,
//    still trying to figure out why I still occasionally 
//    crash when running this from a dialup session...
//    
// 1.21  09/07/06 10:13 
//    - Restore drive summary (-i), with a linux context
//    - change get_disk_info() to use accurate info
//    
// 1.22  02/14/07 16:29 
//    Try to deal with "no matching files found" situation...
//    It generates an error and just returns nothing.
//    
// 1.23  11/30/07 16:15 
//    Resolve problem with resolving symlinks when filespecs are specified.
//    I have to use something other than realpath(), which resolves 
//    symlinks whether I want it to or not!!  (I don't)
//    
// 1.24  10/06/10 11:30
//    > fix bug in parsing ../.. path
//    > directory tree was not using correct chars for low_ascii
//    
// 1.25  10/10/13 09:11
//    Modify default path for ndir.ini; instead of using /etc, use user home directory
//**************************************************************************************

#define  VER_NUMBER "1.25"

//lint -esym(843, Version, ShortVersion)
//lint -esym(844, Version, ShortVersion)

char *Version = " UNDIR.EXE, Version " VER_NUMBER " " ;
char *ShortVersion = " UNDIR " VER_NUMBER " " ;

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>     //  getenv(), exit()
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
// #include <sys/types.h>
#include <termios.h>
#include <pwd.h>

#ifdef _lint
unsigned strlen(const char *STR);
#endif

#include "ndata.hpp"

//  per Jason Hood, this turns off MinGW's command-line expansion, 
//  so we can handle wildcards like we want to.
//  Not very useful in Linux...
// int _CRT_glob = 0 ;

//  cmd_line.cpp
extern void parse_command_string(char *cmdstr) ;

/***********************  qualify() constants  ************************/
#define  QUAL_WILDCARDS    0x01
#define  QUAL_NO_PATH      0x02
#define  QUAL_IS_FILE      0x04
#define  QUAL_INV_DRIVE    0x80

//  from CMD_LINE.CPP
extern void parse_command_args(int start, int argc, char** argv);

//  diskinfo.cpp
extern void display_drive_summary(void);

//***************  function prototypes  ***************
static void process_filespecs(void);
static void read_config_file(void);

//***********************************************************
// #include "strlist.hpp"
// strlist target ;  //  list of processed paths
#include "dirtree.hpp"
dirtree dlist ;

// void insert_target_filespec(char *fstr)
// {
//    // target.add_string(fstr) ;
//    dlist.add_file_entry(fstr) ;
// }

//*******************************************************************
//  standard functions for switching stdin/stdout between
//  buffered (default) and character modes.
//*******************************************************************

static int std_tty_init = 0 ;
static struct termios oldstdtio;

//*******************************************************************
static void enter_char_mode(void)
{
   struct termios newstdtio;
   /* next stop echo and buffering for stdin */
   tcgetattr (0, &oldstdtio);
   tcgetattr (0, &newstdtio);   /* get working stdtio */
   newstdtio.c_lflag &= ~(ICANON | ECHO);
   tcsetattr (0, TCSANOW, &newstdtio);

   //  turn off buffering for standard out!!  
   setbuf(stdout,NULL) ;   //  turn off output buffering for stdout

   std_tty_init = 1 ;
}

//*******************************************************************
static void exit_char_mode(void)
{
   if (!std_tty_init)
      return ;
   tcsetattr (0, TCSANOW, &oldstdtio); /* restore old tty setings */
}

//*****************************************************************
static char ininame[_MAX_PATH] ;

int main(int argc, char **argv)
   {
   // console_init(Version) ;
   enter_char_mode() ;

   //***********************************************************
   //  Check for NDIR environment variable
   //***********************************************************

   //  get program filename
   int startIdx = 1 ;
   // char exename[_MAX_PATH] ;

   //  for linux, we'll assume ndir.ini is in 
   //  either current directory or /etc
   struct stat st ;
   if (stat("./ndir.ini", &st) == 0) {
      sprintf(ininame, "./ndir.ini") ;
   } else {
      //  10/10/13 - we want to change this from /etc to user's home page
      // sprintf(ininame, "/etc/ndir.ini") ;
      
      struct passwd *mypwd = getpwuid(getuid());
      if (mypwd == NULL) {
         sprintf(ininame, "/etc/ndir.ini") ;
      } else {
         // printf("my home directory is %s\n", mypwd->pw_dir) ;
         sprintf(ininame, "%s/ndir.ini", mypwd->pw_dir) ;
      }
      
   }
   // printf("ininame=%s\n", ininame) ;
   // printf("argc=%d\n", argc) ;
   // getchar() ;

   //***********************************************************
   //  first read default settings
   //***********************************************************
   read_config_file() ;

   //***********************************************************
   //  override defaults with command line and environment vars
   //***********************************************************
   parse_command_args(startIdx, argc, argv) ;
   verify_flags() ;  //  this may add extensions if -x is given

   //***********************************************************
   //  Execute the requested command
   //***********************************************************
   display_logo() ;

   if (n.help)
      info(helptxt) ;
   else if (n.info)
      info(idtxt) ;
   else if (n.drive_summary)
      display_drive_summary() ;
   else 
      process_filespecs() ;

   error_exit(0, NULL) ;
   return 0 ;
   }

/**********************************************************************/
static void clear_file_list(void)
{
   if (ftop == NULL)
      return ;
   ffdata *fkill ;
   ffdata *ftemp = ftop ;
   ftop = NULL ;
   filecount = 0 ;
   while (ftemp != NULL) {
      fkill = ftemp ;
      ftemp = ftemp->next ;
      //  is there anything within ftemp 
      //  that I need to delete (filename, etc??)
      delete fkill ;
   }
}

/**********************************************************************/
/**                     File listing routines                        **/        
/**********************************************************************/
static void process_filespecs(void)
{
   char *p ;
   // unsigned i, j, k ;

   //  If no filespec was given, insert current path with *.*
   // if (tcount==0)
   if (dlist.get_dir_count() == 0) {
      // insert_target_filespec(p) ;
      // p = getcwd(NULL, 0) ;
      dlist.add_file_entry(0) ;
      // free(p) ;
   }

   // dlist.show_dirlist() ;

   //*************************************************************
   //  when tree listing is selected, it is assumed that each
   //  specified argument is a separate path, and that no
   //  wildcards nor specific filenames were provided.
   //  If such anomalies are presented, unpredictable results
   //  will occur.
   //*************************************************************
   if (n.tree == 1  ||  n.tree == 4  ||  n.tree == 5) {
      tree_listing() ;
   } else {
      // lfn_supported = 1 ;
      for (p = dlist.get_first_entry(); p != 0; p = dlist.get_next_entry()) {
         clear_file_list() ;

         //  Try to get disk info for current path.
         //  How do I determine which partition it is on??
         sprintf(base_path, "%s/", p) ;
         get_disk_info(base_path) ;

         if (dlist.has_files()) {
            file_list_listing() ;
         } else {
            file_listing() ;
         }
      }
   }
} /*  end  process_filespecs() */

//#########################################################################
//    INI-file handlers
//#########################################################################

//*********************************************************************
//  default file colors
static attrib_list attr_default_list[] = {
{ 0x02, ".1ST" }, { 0x0E, ".ARC" }, { 0x03, ".ASM" }, { 0x03, ".BAS" }, 
{ 0x04, ".BAT" }, { 0x03, ".C"   }, { 0x0C, ".COM" }, { 0x03, ".CPP" },
{ 0x02, ".DOC" }, { 0x0C, ".EXE" }, { 0x0D, ".H"   }, { 0x0D, ".HEX" }, 
{ 0x03, ".HPP" }, { 0x0D, ".INC" }, { 0x0D, ".LIB" }, { 0x0A, ".LST" },
{ 0x05, ".MAP" }, { 0x02, ".ME"  }, { 0x02, ".NOW" }, { 0x0D, ".OBJ" }, 
{ 0x03, ".PAS" }, { 0x02, ".TXT" }, {    0, "" }                 
} ;

//  default directory colors
static uchar const dir_default_list[MAX_DIR_ENTRY] = {
3, 4, 5, 6, 7, 8, 9, 0x0A,
3, 4, 5, 6, 7, 8, 9, 0x0A,
3, 4, 5, 6 } ;

static int write_default_ini_file(char *ini_str)
{
   FILE *ofile ;
   int j ;
   
   ofile = fopen(ini_str, "wt") ;
   if (ofile == 0) 
      return errno ;
   
   fprintf(ofile, "; Default configuration file for NDIR32.EXE\n") ;
   fprintf(ofile, "; This file was generated automatically by NDIR, \n") ;
   fprintf(ofile, "; but it will not be over-written.  \n") ;
   fprintf(ofile, "; Edit this file as you desire to re-configure NDIR.\n") ;
   fprintf(ofile, "; \n") ;
   fprintf(ofile, "; Lines beginning with semicolons, and blank lines, are ignored\n") ;
   fprintf(ofile, "\n") ;
   
   //  generate state flags
   fprintf(ofile, "ucase=%u     ; use upper-case for all names\n", n.ucase) ;
   fprintf(ofile, "sort=%u      ; 0=ext, 1=name, 2=size, 3=date/time, 4=none\n", n.sort) ;
   fprintf(ofile, "reverse=%u   ; reverse normal sort order\n", n.reverse) ;
   fprintf(ofile, "clear=%u     ; clear screen before display\n", n.clear) ;
   fprintf(ofile, "pause=%u     ; pause on full screen\n", n.pause) ;
   fprintf(ofile, "format=%u    ; output columns, 0=1, 1=2, 2=4, 3=6, 5=3\n", n.format) ;
   fprintf(ofile, "minimize=%u  ; minimize header/footer\n", n.minimize) ;
   fprintf(ofile, "show_all=%u  ; show S/H/R files\n", n.show_all) ;
   fprintf(ofile, "dir_first=%u ; list directories only\n", n.dir_first) ;
   fprintf(ofile, "exec_only=%u ; show executables only (exe,com,bat,btm)\n", n.exec_only) ;
   fprintf(ofile, "horz=%u      ; list files horizontally\n", n.horz) ;
   fprintf(ofile, "low_ascii=%u\n", n.low_ascii) ;
   fprintf(ofile, "color=%u\n", n.color) ;
   fprintf(ofile, "showSHRfiles=%u  ; use diff attrib for S/H/R files\n", n.showSHRfiles) ;
   fprintf(ofile, "ega_keep=%u  ; switch to 50-line mode\n", n.ega_keep) ;
   fprintf(ofile, "fdate_option=%u  ; 0=LastWriteTime (default), 1=LastAccessTime, 2=CreationTime\n", n.fdate_option) ;

   //  generate default colors
   fprintf(ofile, "\n") ;
   fprintf(ofile, "global display colors.\n") ;
   fprintf(ofile, "All colors can be decimal or hex (preceded by 0x)\n") ;
   fprintf(ofile, "colorlogo=%u\n", n.colorlogo) ;
   fprintf(ofile, "colornhead=%u\n", n.colornhead) ;
   fprintf(ofile, "colorxhead=%u\n", n.colorxhead) ;
   fprintf(ofile, "colorframe=%u\n", n.colorframe) ;
   fprintf(ofile, "colorattr=%u\n", n.colorattr) ;
   fprintf(ofile, "colorsize=%u\n", n.colorsize) ;
   fprintf(ofile, "colordate=%u\n", n.colordate) ;
   fprintf(ofile, "colortime=%u\n", n.colortime) ;
   fprintf(ofile, "colordir=%u\n", n.colordir) ;
   fprintf(ofile, "colorSHR=%u\n", n.colorSHR) ;
   fprintf(ofile, "colordefalt=%u\n", n.colordefalt) ;
   fprintf(ofile, "colorexec=%u\n", n.colorexec) ;
   fprintf(ofile, "colorlink=%u\n", n.colorlink) ;
   fprintf(ofile, "colorbdev=%u\n", n.colorbdev) ;
   fprintf(ofile, "colorcdev=%u\n", n.colorcdev) ;
   fprintf(ofile, "colorfifo=%u\n", n.colorfifo) ;

   //  generate colors for desired extentions
   // attrib_list attr_default_list[] = {
   fprintf(ofile, "\n") ;
   fprintf(ofile, "; assign colors to file extensions.\n") ;
   fprintf(ofile, "; You can add your extensions here, limited to 200 extentions\n") ;
   fprintf(ofile, "; Wildcards are supported!!\n") ;
   fprintf(ofile, "; two formats are supported:\n") ;
   fprintf(ofile, "; ext=color   and\n") ;
   fprintf(ofile, "; color:ext,ext,ext,ext\n") ;
   fprintf(ofile, "; Note that period is required for each extension\n") ;
   for (j=0; attr_default_list[j].ext[0] != 0; j++) {
      fprintf(ofile, "%s=%u\n", attr_default_list[j].ext, 
                                attr_default_list[j].attr ) ;
   }

   //  generate directory-tree colors
   fprintf(ofile, "\n") ;
   fprintf(ofile, "; assigned colors for levels in directory tree.\n") ;
   fprintf(ofile, "; Each line defines an additional level, limit=20\n") ;
   for (j=0; j<MAX_DIR_ENTRY; j++) {
      fprintf(ofile, ":%u\n", dir_default_list[j] ) ;
   }

   fclose(ofile) ;
   return 0;
}

//*********************************************************************
struct ini_entry {
   char  *lvalue ;
   uchar *rvalue ;
};
static ini_entry const ndir_ini[] = {
{ "ucase",        &n.ucase },
{ "sort",         &n.sort },
{ "reverse",      &n.reverse },
{ "clear",        &n.clear },
{ "pause",        &n.pause },
{ "format",       &n.format },
{ "minimize",     &n.minimize },
{ "show_all",     &n.show_all },
{ "dir_first",    &n.dir_first },
{ "exec_only",    &n.exec_only },
{ "horz",         &n.horz },
{ "low_ascii",    &n.low_ascii },
{ "color",        &n.color },
{ "showSHRfiles", &n.showSHRfiles },
{ "ega_keep",     &n.ega_keep },
{ "lfn_off",      &n.lfn_off },
{ "fdate_option", &n.fdate_option },
{ "colorlogo",    &n.colorlogo },
{ "colornhead",   &n.colornhead },
{ "colorxhead",   &n.colorxhead },
{ "colorframe",   &n.colorframe },
{ "colorattr",    &n.colorattr },
{ "colorsize",    &n.colorsize },
{ "colordate",    &n.colordate },
{ "colortime",    &n.colortime },
{ "colorSHR",     &n.colorSHR },
{ "colordefalt",  &n.colordefalt },
{ "colordir",     &n.colordir },
{ "colorexec",    &n.colorexec },
{ "colorlink",    &n.colorlink },
{ "colorbdev",    &n.colorbdev },
{ "colorcdev",    &n.colorcdev },
{ "colorfifo",    &n.colorfifo },
{ 0, 0} } ;

static void parse_ini_line(char *iniptr)
{
   char *eqptr ;
   int j ;

   eqptr = strchr(iniptr, '=') ;
   if (eqptr == 0)
      return ;
   *eqptr++ = 0 ; //  NULL-terminate lvalue, point to rvalue
   for (j=0; ndir_ini[j].lvalue != 0; j++) {
      //  if we find the lvalue (pointed to by iniptr) in the 
      //  lookup table, assigned the rvalue (pointed to by eqptr)
      //  to the variable stored
      if (strcasecmp(iniptr, ndir_ini[j].lvalue) == 0) {
         *(ndir_ini[j].rvalue) = (uchar) strtoul(eqptr, 0, 0) ;
         // printf("found %s=%u\n", ndir_ini[j].lvalue, (int) *(ndir_ini[j].rvalue)) ;
         break;
      }
   }
}

//*********************************************************************
static void parse_color_entry(char *iniptr)
{
   char *eqptr ;
   char *hdptr ;
   char *tlptr ;
   uchar atr ;
   attrib_list *aptr ;

   //  check for multiple-color-entry forms...
   // 0x32:.com,.bat,.btm,.sys
   hdptr = strchr(iniptr, ':') ;
   //  if colon *is* found, parse multi-attribute entry
   if (hdptr != 0) {
      //  strip newline from string
      // puts(iniptr) ;
      *hdptr++ = 0 ; //  terminate attribute, point to first extension
      atr = (uchar) strtoul(iniptr, 0, 0) ;
      if (atr == 0)
         return ;
      
      //14:.arc,.tgz,.tar,.gz,.z,.zip,.bz2,.rar,.7z
      while (1) {
         //  make sure we don't overrun our table
         if (attrib_count >= MAX_EXT)
            break;
         if (*hdptr != '.') {
            // printf("exit on no dot\n") ;
            break;
         }
         tlptr = strchr(hdptr, ',') ;
         //  assume we're at end of line
         if (tlptr == 0) {
            aptr = &attr_table[attrib_count++] ;
            strncpy(aptr->ext, hdptr, MAX_EXT_SIZE) ;
            aptr->attr = atr ;
            // printf("!%s=%u\n", aptr->ext, aptr->attr) ;
            break;
         } 
         *tlptr++ = 0 ;
         aptr = &attr_table[attrib_count++] ;
         strncpy(aptr->ext, hdptr, MAX_EXT_SIZE) ;
         aptr->attr = atr ;
         // printf("%s=%u\n", aptr->ext, aptr->attr) ;

         hdptr = tlptr ;
      }
      
   }
   //  if no colon found, parse single-attribute entry
   else {
      //  make sure we don't overrun our table
      if (attrib_count >= MAX_EXT)
         return ;

      eqptr = strchr(iniptr, '=') ;
      if (eqptr == 0)
         return ;
      *eqptr++ = 0 ; //  NULL-terminate lvalue, point to rvalue

      //.ARC=14
      aptr = &attr_table[attrib_count++] ;
      strncpy(aptr->ext, iniptr, MAX_EXT_SIZE) ;
      aptr->attr = (uchar) strtoul(eqptr, 0, 0) ;
   }
}

//*********************************************************************
static void parse_dir_color_entry(char *iniptr)
{
   static int dcIdx = 0 ;
   iniptr++ ;  //  skip colon flag
   if (dcIdx < MAX_DIR_ENTRY) {
      dtree_colors[dcIdx++] = (uchar) strtoul(iniptr, 0, 0) ;
   }
}

//*********************************************************************
static int read_ini_file(char const * const ini_str)
{
   FILE *ofile ;
   int slen ;
   char *strptr ;
   static char line[_MAX_PATH] ;

// printf("reading %s\n", ini_str) ;
   ofile = fopen(ini_str, "rt") ;
   if (ofile == 0) 
      return errno ;

   while (fgets(line, sizeof(line), ofile) != 0) {
      //  strip off newline char
      slen = strlen(line) ;
      strptr = &line[slen-1] ;
      while (1) {
         if (slen == 0)
            break;
         if (*strptr != '\n'  &&  *strptr != '\r') 
            break;
         *strptr-- = 0 ;   //  strip off newline
         slen-- ;
      }

      //  next, find and strip off comments
      strptr = strchr(line, ';') ;
      if (strptr != 0)
         *strptr-- = 0 ;

      //  skip blank lines
      slen = strlen(line) ;
      if (slen == 0)
         continue;
      strptr = &line[slen-1] ;

      //  then strip off tailing spaces
      while (slen > 0  &&  *strptr == ' ') {
         *strptr-- = 0 ;
         slen-- ;
      }
      if (slen == 0)
         continue;

      //  now we should have a simple line in field=value format.
      //  See if we can parse it...

      //  see whether we're dealing with an extention-color entry,
      //  or a flags entry
      if (line[0] == '!') {
         parse_command_string(line) ;
      } else if (line[0] == '.') {
         parse_color_entry(line) ;
      } else if (line[0] >= '0'  &&  line[0] <= '9') {
         parse_color_entry(line) ;
      } else if (line[0] == ':') {
         parse_dir_color_entry(line) ;
      } else {
// printf("line=%s", line) ;
         parse_ini_line(line) ;
      }
   }
   
   fclose(ofile) ;
   return 0;
}

//*********************************************************************
static char const * const local_ini_name = ".\\ndir.ini" ;
static char ini_path[_MAX_PATH] ;

static void read_config_file(void)
{
   int result ;

   //  search for existing file.
   //  1. look in current directory
   //  2. if not found, search location of executable
   //  3. if not found, generate default file in location of executable
   // printf("seek local ini=%s\n", local_ini_name) ;
   result = read_ini_file(local_ini_name) ;
   if (result == 0) {
      return ;
   }

   //  If search for local file failed, try location of executable,
   //  if that isn't the local directory.

   //  If global INI filename isn't present, give up on search.
   //  This will usually mean that we are running under WinNT 4.0,
   //  and the executable is already in the current directory.
   //  Just write the file in the current directory.
   // printf("ininame=%s\n", ininame) ;
   // getchar() ;
   if (ininame[0] == 0) {
      strcpy(ini_path, local_ini_name) ;
   } 
   //  If global INI filename IS present, try to load it
   else {
      result = read_ini_file(ininame) ;
      if (result == 0) {
         return ;
      }
      strcpy(ini_path, ininame) ;
   }

   //  If we couldn't open any existing INI files,
   //  generate default file in appropriate location.
   result = write_default_ini_file(ini_path) ;
   if (result != 0) {
      // perror(ini_path) ;
      sprintf (tempstr, "path [%s]\n", ini_path);
      nputs (0xA, tempstr);
      sprintf (tempstr, "FindFirst: %s\n", strerror(errno));
      nputs (0xA, tempstr);
   }
   //  try to read again, after writing defaults
   read_ini_file(ini_path) ;
}

//*********************************************************
void error_exit(int errcode, char* outstr)
{
   char msg[80] ;
   if (errcode == 0)
      sprintf(msg, " ") ;
   else if (outstr == NULL) 
      sprintf(msg, "%s\n", strerror(errcode)) ;
   else 
      sprintf(msg, "%s: %s\n", outstr, strerror(errcode)) ;
   nputs(7, msg) ;

   // restore_console_attribs();
   exit_char_mode() ;

   exit(errcode) ;
}

