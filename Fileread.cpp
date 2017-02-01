//*****************************************************************
//                                                                 
//  FILEREAD.CPP - NDIR file-reading routines                      
//                                                                 
//  Written by:  Daniel D. Miller  (the Derelict)                  
//                                                                 
//  Last update:  01/15/01 21:20PM                                 
//                                                                 
//  compile with makefile                                          
//                                                                 
//*****************************************************************

// #define  _WIN32_WINNT   0x0400
// #include <windows.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>  //  readlink
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "ndata.hpp"
// #include "strlist.hpp"
// extern strlist target ;

#include "dirtree.hpp"
extern dirtree dlist ;

//  from nsort.cpp
extern void sort_filelist (void);

static char lfn_src[_MAX_PATH + 1];
static char lfn_dest[_MAX_PATH + 1];
//*****************  function prototypes  *****************
static void process_exclusions (void);
static void read_long_files (char *tname);

static ffdata *ftail = 0 ;

//***************************************************************
//  this search routine abandons the binary-search method        
//  used by the old procedure (commented out after this one),    
//  because my match_extension routine does not provide a        
//  greater_than/less_than result.  However, it recognizes       
//  wildcards, which is more important right now.                
//***************************************************************
static void getcolor (ffdata * fnew)
{
	unsigned j;
	attrib_list *aptr;

	for (j = 0; j < attrib_count; j++) {
		aptr = &attr_table[j];
      // if (strcmpiwc (fnew->ext, aptr->ext) != 0) {
      if (strcasecmp(fnew->ext, aptr->ext) == 0) {
         // printf("%s=%u\n", aptr->ext, aptr->attr) ;
			fnew->color = aptr->attr;
			return;
		}
	}
	fnew->color = n.colordefalt; //  if not found, assign default color
}

//*********************************************************
static int read_one_file(char *d_path, char *d_name)
{
   ffdata *ftemp;
   char *p ;
   char dsrcpath[1024] ;
   // int slen = 0 ;
   // struct dirent *dp;
   struct stat st ;
   // char tempfile[_MAX_PATH + 1] ;
   sprintf(dsrcpath, "%s%s", d_path, d_name) ;

   // strcpy(tempfile, d_name) ;
   if (lstat(dsrcpath, &st) != 0) {
      // printf("%s: failing lstat\n", dsrcpath) ;
      return -ENOENT ;
      // error_exit(errno, d_name) ;
   }
   //  interesting... stat() is *not* telling
   //  me about symbolic links!!
   // if (strcmp(dp->d_name, "vi") == 0) {
   //    printf("%04X %s\n", (unsigned) st.st_mode, tempfile) ;
   //    getchar() ;
   // }
   // tempfile[slen] = 0 ;

   // printf("%9ld %04X %s\n", fdata.file_size, fdata.attrib, fdata.cFileName) ;
   filecount++;

   // printf("found file: %s, len %d\n", d_name, strlen(d_name)) ;
   // getchar() ;
   //****************************************************
   //  allocate and initialize the structure
   //****************************************************
   ftemp = new ffdata;
   // if (ftemp == NULL) {
   //    error_exit (ENOMEM, NULL);   //  does not return
   //    // return;  //  only to make lint happy
   // }

   //  filename is complete filename with extension
   ftemp->filename = (char *) new char[strlen(d_name)+1] ;
   if (ftemp->filename == 0) {
      error_exit (ENOMEM, NULL);   //  does not return
      // return ; //  only to make lint happy
   }
   strcpy(ftemp->filename, d_name) ;

   //  name is body of filename without extension
   ftemp->name = (char *) new char[strlen(d_name)+1] ;
   if (ftemp->name == 0) {
      error_exit (ENOMEM, NULL);   //  does not return
      // return ; //  only to make lint happy
   }
   strcpy(ftemp->name, d_name) ;
   //  try to separate out extension
   ftemp->ext[0] = 0 ;
   p = strrchr(ftemp->name, '.') ;
   if (p != 0) {
      if (strlen(p) < MAX_EXT_SIZE) {
         strcpy(ftemp->ext, p) ;
         *p = 0 ; //  strip ext off of 'name' 
      }
   }
   // printf("%s=%s=%s%s\n", d_name, ftemp->filename, ftemp->name, ftemp->ext) ;
   // getchar() ;

   // ftemp->attrib = (uchar) fdata.dwFileAttributes;
   ftemp->attrib = st.st_mode ;

//        The following flags are defined for the st_mode field:
// 
//        S_IFMT     0170000   bitmask for the file type bitfields
//        S_IFSOCK   0140000   socket
//        S_IFLNK    0120000   symbolic link
//        S_IFREG    0100000   regular file
//        S_IFBLK    0060000   block device
//        S_IFDIR    0040000   directory
//        S_IFCHR    0020000   character device
//        S_IFIFO    0010000   fifo
//        S_ISUID    0004000   set UID bit
//        S_ISGID    0002000   set GID bit (see below)
//        S_ISVTX    0001000   sticky bit (see below)
//        S_IRWXU      00700   mask for file owner permissions
//        S_IRUSR      00400   owner has read permission
//        S_IWUSR      00200   owner has write permission
//        S_IXUSR      00100   owner has execute permission
//        S_IRWXG      00070   mask for group permissions
//        S_IRGRP      00040   group has read permission
//        S_IWGRP      00020   group has write permission
//        S_IXGRP      00010   group has execute permission
//        S_IRWXO      00007   mask for permissions for others (not in group)
//        S_IROTH      00004   others have read permission
//        S_IWOTH      00002   others have write permisson
//        S_IXOTH      00001   others have execute permission
   typedef struct fattr_bits_s {
      unsigned perms  :  9 ;
      unsigned setids :  3 ;
      unsigned types  :  4 ;
      unsigned unused : 16 ;
   } fattr_bits_t ;
   typedef union fattribs_s {
      fattr_bits_t fb ;
      unsigned raw ;
   } fattribs_t ;
   fattribs_t fattr ;

   fattr.raw = ftemp->attrib ;
   // printf("%03o ", fattr.fb.types) ;
   switch (fattr.fb.types) {
   case 014:  
      ftemp->dirflag = 's' ; 
      ftemp->color = n.colordefalt;  //  for now, no socket color
      break ;
   case 012:  
      ftemp->dirflag = 'l' ; 
      ftemp->color = n.colorlink;

      //  note: readlink requires full path and filename...
      sprintf(lfn_src, "%s%s", d_path, d_name) ;
      bzero(lfn_dest, sizeof(lfn_dest)) ;
      if (readlink(lfn_src, lfn_dest, sizeof(lfn_dest)) > 0) {
         // printf("[%s -> %s]\n", lfn_src, lfn_dest) ;
         int rllen = strlen(lfn_dest) ;
         ftemp->linktgt = (char *) new char[sizeof(lfn_dest)] ;
         if (ftemp->linktgt != 0) {
            bzero(ftemp->linktgt, rllen+1) ;
            strcpy(ftemp->linktgt, lfn_dest) ;
            // printf("{%s -> %s}\n", ftemp->filename, ftemp->linktgt) ;
         }
      } else {
         perror(lfn_dest) ;
         getchar() ;
      }
      break ;
   case 06:   
      ftemp->dirflag = 'b' ; 
      ftemp->color = n.colorbdev;
      break ;
   case 04:   
      ftemp->dirflag = 'd' ; 
      ftemp->color = n.colordir;
      break ;
   case 02:   
      ftemp->dirflag = 'c' ; 
      ftemp->color = n.colorcdev;
      break ;
   case 01:   
      ftemp->dirflag = 'f' ; 
      ftemp->color = n.colorfifo;
      break ;
   default:   
      ftemp->dirflag = '-' ; 
      if (fattr.fb.perms & 0111) {
         ftemp->color = n.colorexec;
      } else {
         getcolor (ftemp);
      }
      break ;
   }

   //  convert file time
   //luconv.l = IFF (n.fdate_option == 0)
   // ftemp->ft = IFF (n.fdate_option == 0)
   //       THENN fdata.ftLastAccessTime
   //       ELSSE fdata.ftCreationTime ;
   // ftemp->ft = fdata.ftLastWriteTime ;
   if (n.fdate_option == FDATE_LAST_ACCESS)
      ftemp->ft = st.st_atime ;
   // else if (n.fdate_option == FDATE_CREATE_TIME)
   //    ftemp->ft = st.st_mtime ;
   else
      ftemp->ft = st.st_mtime ;

   //  convert file size
   // i64tol iconv;
   // iconv.u[0] = fdata.nFileSizeLow;
   // iconv.u[1] = fdata.nFileSizeHigh;
   ftemp->fsize = st.st_size ;
   // ftemp->fsize = (symlink) ? 4 : st.st_size ;

   // ftemp->filename = new char[strlen ((char *) fdata.cFileName) + 1];
   // strcpy (ftemp->filename, (char *) fdata.cFileName);

   //  find and extract the file extension, if valid
   // ftemp->name[0] = 0 ; //  don't use name at all
   // ftemp->name = new char[strlen (ftemp->filename) + 1];
   // if (ftemp->name == NULL)
   //    error_exit (ENOMEM, NULL);   //  does not return

   // strcpy (ftemp->name, ftemp->filename);
   // strptr = strrchr (ftemp->name, '.');
   // if (strptr != 0 && strlen (strptr) <= MAX_EXT_SIZE) {
   //    strcpy (ftemp->ext, strptr);
   //    *strptr = 0;        //  NULL-term name field
   // }
   // else
   //    ftemp->ext[0] = 0;  //  no extension found

   // ftemp->dirflag = ftemp->attrib & FILE_ATTRIBUTE_DIRECTORY;
   // ftemp->dirflag = (st.st_mode & S_IFDIR) ? 1 : 0 ;
   ftemp->next = NULL;

   //****************************************************
   //  add the structure to the file list
   //****************************************************
   if (ftop == NULL) {
      ftop = ftemp;
   }
   else {
      ftail->next = ftemp;
   }
   ftail = ftemp;
   return 0;
}

//*********************************************************
//                 File-listing routine
//*********************************************************
void file_list_listing(void)
{
   //  dirtree method: this must iterate over files
   //  in current directory element
   char *p ;
   for (p=dlist.get_first_file(); p != 0; p=dlist.get_next_file()) {
      // printf("found %s\n", p) ;
      read_one_file(base_path, p) ;
   }

   //***********************************************
   process_exclusions ();

   //***********************************************
   //              Sort directory data
   //***********************************************
   if (filecount > 0) {
      sort_filelist ();

      // if (n.dir_first)
      //    // sort_elements(sort_dir) ;
      //    ftop = sort_elements(sort_dir, ftop) ;
   }

   //  now do the file-listing...
   display_files ();
}

//*********************************************************
//                 File-listing routine
//*********************************************************
void file_listing (void)
{
   read_long_files(base_path) ;
   // printf("filecount=%u\n", filecount) ;
   // getchar() ;

   //***********************************************
   process_exclusions ();

   //***********************************************
   //              Sort directory data
   //***********************************************
   if (filecount > 0) {
      sort_filelist ();

      // if (n.dir_first)
      //    // sort_elements(sort_dir) ;
      //    ftop = sort_elements(sort_dir, ftop) ;
   }

   //  now do the file-listing...
   display_files ();
}

//*********************************************************
//  This loops thru all files in one subdirectory,
//  calling update_filelist() to add files to the
//  global linked list.
//*********************************************************
static void read_long_files (char *tname)
{
   DIR *hdl ;
   struct dirent *dp;
   unsigned cut_dot_dirs ;
   char tempfile[_MAX_PATH + 1] ;

   // handle = FindFirstFile (target[i], &fdata);
   hdl = opendir(tname) ;
   if (hdl == NULL)
      return ;

   strcpy(tempfile, tname) ;
   //  make sure there's a slash at end
   int slen = strlen(tempfile) ;
   if (tempfile[slen-1] != '/') {
       tempfile[slen++] = '/' ;
       tempfile[slen] = 0 ;
   }

   /* Get each directory entry from hdl and print its name */
   while ((dp = readdir(hdl)) != NULL) {
      // printf( "%s\n", dp->d_name );

      //  skip '.' and '..', but NOT .ncftp (for example)
      //  this is a more efficient method than
      //  if (strcmp(name, ".") == 0  ||  strcmp(name, "..") == 0)
      if (dp->d_name[0] != '.')  
         cut_dot_dirs = 0 ;
      else if (dp->d_name[1] == 0)
         cut_dot_dirs = 1 ;
      else if (dp->d_name[1] == '.'  &&  dp->d_name[2] == 0)
         cut_dot_dirs = 1 ;
      else 
         cut_dot_dirs = 0 ;

      if (!cut_dot_dirs) {
         read_one_file(tempfile, dp->d_name) ;
      }
   }
   //  Release the open directory 
   closedir(hdl);
}

//*************************************************************
//  Compare file list against exclusion list, then remove 
//  excluded files.  For now, this will seek only extensions.
//  
//  Also, this should be deleting the separate 
//  name-string allocations!!
//*************************************************************
static void process_exclusions (void)
{
	for (int i = 0; i < exclcount; i++) {
		ffdata *ftemp = ftop;
		ffdata *fprev = NULL;

		while (ftemp != NULL) {
			//  if we have a match, delete the second argument
         // if (strcmpiwc (ftemp->ext, excl[i]) != 0) {
         if (strcasecmp(ftemp->ext, excl[i]) != 0) {
				if (fprev == NULL) {
					ftop = ftop->next;
					delete ftemp;
					ftemp = ftop;
				}
				else {
					fprev->next = ftemp->next;
					delete ftemp;
					ftemp = fprev->next;
				}
				filecount--;
			}

			//  otherwise, just move to the next file
			else {
				fprev = ftemp;
				ftemp = ftemp->next;
			}
		}
	}
}
