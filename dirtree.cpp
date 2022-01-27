//****************************************************************
//  build a list of directories, not necessarily related to
//  each other, with optional list of files in each directory.
//****************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#include "dirtree.hpp"

//lint -esym(613, dirtree::dirtail)
//lint -esym(418, perror)
//lint -esym(1714, dirtree::show_dirlist)

//***********************************************************************
dirtree::dirtree(void)
{
   dircount = 0 ;
   curpath = 0 ;
   dirtop = 0 ;
   dirtail = 0 ;
   char *p = getcwd(cwdir, sizeof(cwdir)) ;
   if (p == 0)
      cwdir[0] = 0 ;
}

//***********************************************************************
dirtree::~dirtree(void)
{
   dirtop  = 0 ;
   dirtail = 0 ;
   iter    = 0 ;
   fiter   = 0 ;
   curpath = 0 ;
}

//***********************************************************************
//  This function will:
//  - resolve all multiple-slash entries to single-slash entries.
//  - resolve all /./ to /
//  - resolve all ../ to references without ..
//  Unlike realpath(), it does *not* resolve symbolic links.
//***********************************************************************
void dirtree::expand_path(char *inpath, char *outpath)
{
   //  work on the destination copy.
   //  Make sure we *always* are dealing with a fully-qualified path!!
   if (*inpath == '/') {
      strcpy(outpath, inpath) ;
   } else {
      sprintf(outpath, "%s/%s", cwdir, inpath) ;
   }
   char *hd = outpath ;
   char *tl ;
   // char *pre ;
   // unsigned plen ;

   //  - resolve all multiple-slash entries to single-slash entries.
   while (1) {
      tl = strstr(hd, "//") ;
      if (tl == 0) 
         break;
      // plen = strlen(tl) ;
      strcpy(tl+1, tl+2) ;
   }
   
   //  - resolve all /./ to /
   while (1) {
      tl = strstr(hd, "/./") ;
      if (tl == 0) 
         break;
      // plen = strlen(tl) ;
      strcpy(tl+1, tl+3) ;
   }

   //  - resolve all .. to references without ..
   while (1) {
      // printf("hd=%s\n", hd) ;
      tl = strstr(hd, "/..") ;
      if (tl == 0) 
         break;
      //  if path starts with /../ it is invalid; 
      //  just exit without doing any more work.
      unsigned plen = (unsigned) (tl - hd) ;
      if (plen == 0)
         break;
      //  okay, we have a valid path, but we need to find the
      //  next higher path component
      char *pre = tl - 1 ;
      while (1) {
         if (*pre == '/')
            break;
         pre-- ;
      }
      strcpy(pre, tl+3) ;
   }
   //  dilemma - we need to expand the path *before* we can resolve .. references,
   //  but realpath will destroy symlinks, which we need to preserve.
   // realpath(inpath, outpath) ;
}

//***********************************************************************
//  the input to this function is a non-expanded filename argument,
//  which may contain any combination of file/path specifications.
//***********************************************************************
int dirtree::add_file_entry(char *dfentry)
{
   char dftemp[PATH_MAX] ;
   dir_entry_p dtemp ;
   file_entry_p ftemp ;
   struct stat st ;

   if (dfentry == 0) {
      if (cwdir[0] == 0) {
         strcpy(dftemp, "/") ;
      } else {
         strcpy(dftemp, cwdir) ;
      }
   // } else if (*dfentry == '/') {
   //    strcpy(dftemp, dfentry) ;
   } else {
      //  the problem with realpath() is that, when processing a symlink,
      //  it returns the target (which throws away symlink data).
      //  All I *want* it to do is resolve . and .. references.
      // realpath(dfentry, dftemp) ;
      expand_path(dfentry, dftemp) ;
   }
   // printf("%s=%s\n", dfentry, dftemp) ;
   // getchar() ;
   //  if expanded filename is a directory, 
   //  process as a directory, not a file.
   int result = stat(dftemp, &st) ;
   // if (stat(dftemp, &st) != 0) {
   //    printf("%s: stat failed\n", dftemp) ;
   //    perror(dftemp) ;
   //    getchar() ;
   //    return -errno ;
   // }
   //  process directory entry
   if (result == 0  &&  S_ISDIR(st.st_mode)) {
      // process new path
      if (curpath == 0  ||  strcmp(curpath, dftemp) != 0) {
         dtemp = new dir_entry_t ;
         // if (dtemp == 0) { //  this will abort automatically, we hope
         //    puts("out of memory") ;
         //    return -ENOMEM ;
         // }
         memset((char *) dtemp, 0, sizeof(dir_entry_t)) ;
         strncpy(dtemp->dfpath, dftemp, PATH_MAX) ;

         //  add new struct to list
         if (dirtop == 0) 
            dirtop = dtemp ;
         else 
            dirtail->next = dtemp ;
         dirtail = dtemp ;
         curpath = dtemp->dfpath ;
         dircount++ ;
      } else {
         printf("%s/%s: duplicate path??\n", dfentry, dftemp) ;
         return -EINVAL ;
      }
   } 
   //  process filename entry
   //  02/14/07 16:32 - we also fall through here 
   //  if the filespec does not exist...
   //  We're *hoping* to generate a "No matching files found" message,
   //  instead of defaulting to full listing of current directory.
   else {
      char *fptr = strrchr(dftemp, '/') ;
      if (fptr == 0) 
      {
         perror(dfentry) ; //lint !e668
         return -errno;
      }
      *fptr++ = 0 ;

      //  process the directory portion
      if (curpath == 0  ||  strcmp(curpath, dftemp) != 0) {
         dtemp = new dir_entry_t ;
         // if (dtemp == 0) {
         //    puts("out of memory") ;
         //    return -ENOMEM ;
         // }
         memset((char *) dtemp, 0, sizeof(dir_entry_t)) ;
         strncpy(dtemp->dfpath, dftemp, PATH_MAX) ;

         //  add new struct to list
         if (dirtop == 0) 
            dirtop = dtemp ;
         else 
            dirtail->next = dtemp ;
         dirtail = dtemp ;
         curpath = dtemp->dfpath ;
         dircount++ ;
      }
      //  add name to path list
      ftemp = new file_entry_t ;
      // if (ftemp == 0) {
      //    puts("out of memory") ;
      //    return -ENOMEM ;
      // }
      memset((char *) ftemp, 0, sizeof(file_entry_t)) ;
      strncpy(ftemp->name, fptr, PATH_MAX) ;

      if (dirtail->ftop == 0) 
         dirtail->ftop = ftemp ;
      else
         dirtail->ftail->next = ftemp ;
      dirtail->ftail = ftemp ;
      dirtail->fcount++ ;
   }

   return 0;
}

//***********************************************************************
void dirtree::show_dirlist(void)
{
   dir_entry_p dtemp ;
   for (dtemp=dirtop; dtemp != 0; dtemp = dtemp->next) {
      printf("%s: %u files\n", dtemp->dfpath, dtemp->fcount) ;
   }
   // getchar() ;
}

//***********************************************************************
unsigned dirtree::get_dir_count(void)
{
   return dircount ;
}

//***********************************************************************
char *dirtree::get_entry(void)
{
   return (iter == 0) ? 0 : iter->dfpath ;
}

//***********************************************************************
char *dirtree::get_next_entry(void)
{
   if (iter == 0) 
      return 0;
   iter = iter->next ;
   return get_entry();
}

//***********************************************************************
char *dirtree::get_first_entry(void)
{
   iter = dirtop ;
   return get_entry();
}

//***********************************************************************
unsigned dirtree::has_files(void)
{
   return (iter == 0) ? 0 : iter->fcount ;
}

//***********************************************************************
char *dirtree::get_file_entry(void)
{
   return (fiter == 0) ? 0 : fiter->name ;
}

//***********************************************************************
char *dirtree::get_next_file(void)
{
   if (fiter == 0) 
      return 0;
   fiter = fiter->next ;
   return get_file_entry();
}

//***********************************************************************
char *dirtree::get_first_file(void)
{
   if (iter == 0)
      return 0;
   fiter = iter->ftop ;
   return get_file_entry();
}


