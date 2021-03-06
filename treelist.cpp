//*****************************************************************
//                                                                 
//  TREELIST.CPP: Read and display directory tree                  
//                                                                 
//  Written by:   Daniel D. Miller  (the Derelict)                 
//                                                                 
//  Last update:  02/07/03 18:38                                   
//                                                                 
//  compile with  makefile                                         
//                                                                 
//*****************************************************************

//  remove this define to remove all the debug messages
// #define  DESPERATE

// #include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ndata.hpp"
// #include "win2unix.hpp"
#include "i64tostr.hpp"

static char const * const dhdr =
	"                         쳐컴컴컴컴컴쩡컴컴컴컴컴탠컴컴컴컴컴컴컫컴컴컴컴컴컴컴";

static char const * const dhdrA =
   "                         +===========+===========+==============+==============";

static void sort_trees (void);

static char formstr[50];
static char levelstr[_MAX_PATH];

//  instantiate ultosc class as required
//lint -e747 
static i64tostr dsize (0);      //  dir size in bytes
// static i64tostr dssize (0);     //  dir size in clusters
// static i64tostr sdsize (0);     //  dir tree size in bytes
// static i64tostr sdssize (0);    //  dir tree size in clusters
//lint +e747 

//*****************************************************************
//  from FILELIST.CPP
extern void put_disk_summary (void);

//  from NDIR.CPP
// extern void get_volume_label(char dchar);

//**********************************************************
//  directory structure for directory_tree routines
//**********************************************************
struct dirs
{
	dirs *brothers;
	dirs *sons;
	char *name;
	uchar attrib;
   u64 dirsize;
   u64 dirsecsize;
   u64 subdirsize;
   u64 subdirsecsize;
	unsigned files;
	unsigned directs;
	unsigned subfiles;
	unsigned subdirects;
};

//************************************************************
//  the following object is a dummy point structure
//  which is used by merge_sort.  The main code must
//  allocate a strucure for this to point to.
//  
//  A global function pointer is also required by the
//  sort routine.  This will point to a function which
//  accepts two structure pointers as arguments, and
//  return:
//  
//     >0 if a > b
//    ==0 if a == b
//     <0 if a < b
//  
//************************************************************
static struct dirs *z = NULL;
static int (*tree_sort_fcn) (struct dirs * a, struct dirs * b);
static int tree_init_sort (void);

static dirs *top = NULL;

//**********************************************************
// static char dirpath[_MAX_PATH]; 
static char dir_path[_MAX_PATH];
static int level;

//***************  function prototypes  ***************
static int read_dir_tree (dirs * cur_node);
static void draw_dir_tree (void);
static void display_dir_tree (dirs * top);
static void printdirheader (void);
static void print_dir_end (void);
static dirs *new_dir_node (void);

//**********************************************************
#ifdef  DESPERATE
void debug_dump(char *fname, char *msg)
{
   FILE *fd ;
   fd = fopen("debug.txt", "at") ;
   if (fd == 0) {
      return ;
   }
   fseek(fd, 0, SEEK_END) ;
   fprintf(fd, "l%u %s: %s\n", level, fname, msg) ;
   fclose(fd) ;
}
#endif

//**********************************************************
static int build_dir_tree(char* tpath)
{
   char* strptr ;
   int base_len ;

   level = 0 ;

   strcpy(base_path, tpath) ;

   // printf("input:  [%s]\n", base_path) ;
   // int result ;
   // if ((result = qualify_filename(base_path)) < 0) {
   //    printf("%s: %s\n", base_path, strerror(-result)) ;
   //    error_exit(0, NULL) ;
   // }
   // printf("output: [%s]\n", base_path) ;
   get_disk_info(base_path) ;

   base_len = strlen(base_path) ;
   //  allocate struct for dir listing
   top = new_dir_node() ;

   //  derive root path name
   strptr = (char *) new char[base_len+1] ;
   // if (strptr == 0) {
   //    error_exit(ENOMEM, NULL) ;
   // }
   top->name = strptr ;
   strcpy(top->name, base_path) ;

   //  make sure base path ends with a slash
   if (base_path[base_len-1] != '/') {
      strcat(base_path, "/") ;
      base_len++ ;
   }
   strcpy(dir_path, base_path) ;

   //  call the recursive function
   read_dir_tree(top) ;

   return 0;   //lint !e438 Last value assigned to variable 'base_len' not used ??
}  

//*****************************************************************
//  no filename may be specified here...
//*****************************************************************
// #include "strlist.hpp"
// extern strlist target ;
#include "dirtree.hpp"
extern dirtree dlist ;

void tree_listing (void)
{
	if (z == 0)
		tree_init_sort ();

   for (char *p=dlist.get_first_entry(); p != 0; p=dlist.get_next_entry()) {
   // for (p=target.get_first_string(); p != 0; p=target.get_next_string()) {
      printf("building tree from %s\n", p) ;

   // for (unsigned l = 0; l < tcount; l++) {
		//  check for validity of long_filename functions
      // dname[0] = *target[l];
      // lfn_supported = 1 - n.lfn_off;
      // lfn_supported = 1 ;

		//  read and build the dir tree
      build_dir_tree (p) ;

      //  sort the tree list
      sort_trees ();

      //  now display the resulting directory tree
      draw_dir_tree ();
   }
}

//*********************************************************
//  "waiting" pattern generator
//*********************************************************
// static unsigned lrow, lcol, dircount ;
// 
// static void pattern_init(char *lstr)
// {
//    dircount = 0 ;
//    lrow = _where_y() ;
//    if (lstr == 0) {
//       lcol = 0 ;
//    } else {
//       dprints(lrow, 0, lstr) ;
//       lcol = strlen(lstr) ;
//    }
// }
// 
// static void pattern_reset(void)
// {
//    dgotoxy(0, lrow) ;
//    dclreol() ;
//    dgotoxy(0, lrow) ;
// }
// 
// static void pattern_update(void)
// {
//    static char pucount[20] ;
// 
//    dircount++ ;
//    sprintf(pucount, "%u", dircount) ;
//    dprints(lrow, lcol, pucount) ;
// }

//**********************************************************
//  recursive routine to read directory tree
//**********************************************************
static int read_dir_tree(dirs* cur_node)
{
   dirs *dtail = 0 ;
   dirs *dtemp ;
   int slen ;
   unsigned cut_dot_dirs ;
   unsigned long clusters, file_clusters ;
   DIR *dirp;                   /* pointer to directory */
   struct dirent *dp;
   struct stat statbuf ; 
   char pathname[_MAX_PATH] ;

   //  lstat on our path is giving incorrect (and absurd!!)
   //  cluster size.  How about we try /etc/inittab ??
   // if (level == 0) {
   //    // if (lstat(dir_path, &statbuf) != 0) {
   //    if (stat("/etc/inittab", &statbuf) != 0) {
   //       error_exit(errno, NULL) ; //  never returns
   //    }
   //    clbytes = statbuf.st_blksize ;
   //    printf("cluster size=%u\n", clbytes) ;
   // }

   //  first, build tree list for current level
   level++ ;

   dirp = opendir(dir_path);
   if (dirp == NULL) {
      error_exit(errno, NULL) ; //  never returns
      // perror(dir_path);
      // exit(1) ;
   }

   //  loop on find_next
   while ((dp = readdir(dirp)) != NULL) {
      sprintf(pathname, "%s%s", dir_path, dp->d_name) ;
      if (lstat(pathname, &statbuf) != 0) {
         perror(pathname);
         continue;
      }

      //  process directory
      if ((S_ISDIR(statbuf.st_mode)) != 0) {
         //  skip '.' and '..', but NOT .ncftp (for example)
         if (dp->d_name[0] != '.')  
            cut_dot_dirs = 0 ;
         else if (dp->d_name[1] == 0)
            cut_dot_dirs = 1 ;
         else if (dp->d_name[1] == '.'  &&  dp->d_name[2] == 0)
            cut_dot_dirs = 1 ;
         else 
            cut_dot_dirs = 0 ;

         cur_node->directs++;
         cur_node->subdirects++;

         if (cut_dot_dirs)
            continue;

         dtemp = new_dir_node() ;
         if (cur_node->sons == NULL) 
             cur_node->sons = dtail = dtemp ;
         else {
            dtail->brothers = dtemp ;  //lint !e613
            dtail = dtail->brothers ;  //lint !e613
         }

         dtail->name = new char[strlen(dp->d_name)+1] ;
         if (dtail->name == 0) {
            error_exit(ENOMEM, NULL) ; //  never returns
         }
         strcpy(dtail->name, dp->d_name) ;
         dtail->attrib = (uchar) statbuf.st_mode ;
      }

      //  we found a normal file
      else {
         //  convert file size
         cur_node->dirsize     += statbuf.st_size ;
         cur_node->subdirsize  += statbuf.st_size ;

         clusters = statbuf.st_size / clbytes ; //lint !e573 Signed-unsigned mix with divide
         if ((statbuf.st_size % clbytes) > 0)   //lint !e573 Signed-unsigned mix with divide
            clusters++ ;

         file_clusters = clusters * clbytes ;
         cur_node->dirsecsize    += file_clusters ;
         cur_node->subdirsecsize += file_clusters ;

         cur_node->files++;
         cur_node->subfiles++;
      }
   }

   // FindClose(handle) ;
   closedir(dirp) ;

   //  next, build tree lists for subsequent levels (recursive)
   dirs* ktemp = cur_node->sons ;
   while (ktemp != NULL) {
      //  form next search path before recursing into read_dir_tree(),
      //  but prepare to restore the dir_path when done...
      slen = strlen(dir_path) ;
      strcat(dir_path, ktemp->name) ;
      strcat(dir_path, "/") ;
      read_dir_tree(ktemp) ;
      dir_path[slen] = 0 ; //  remove our name from path

      cur_node->subdirsize    += ktemp->subdirsize ;
      cur_node->subdirsecsize += ktemp->subdirsecsize ;
      ktemp = ktemp->brothers ;
   }

   //  restore the level number
   level-- ;
   return 0;
}

//**********************************************************
//  allocate struct for dir listing                         
//  NOTE:  It is assumed that the caller will               
//         initialize the name[], ext[], attrib fields!!    
//**********************************************************
static dirs *new_dir_node (void)
{
	//lint -esym(613,dtemp) 
	dirs *dtemp = new dirs;
   // if (dtemp == NULL)
   //    error_exit (ENOMEM, NULL);
	memset ((char *) dtemp, 0, sizeof (struct dirs));	//lint !e668
	// if (n.tree == 1) {
	dtemp->dirsecsize = clbytes;
	dtemp->subdirsecsize = clbytes;
	// dtemp->directs = 1 ;
	// }
	// else {
	//   dtemp->dirsecsize = 1;
	//   dtemp->subdirsecsize = 1;
	// }
	return dtemp;
}

//*********************************************************
static void draw_dir_tree (void)
{
	printdirheader ();
	display_dir_tree (top);
   print_dir_end ();
}

//**********************************************************
static void display_tree_filename (char *lstr, char *frmstr)
{
	int slen = strlen (lstr);

	//  if directory name, etc., is too long, make separate line
	if (slen > 37) {
		sprintf (tempstr, "%-64s", lstr);	//  write filename
		nputs (dtree_colors[level], tempstr);

		//  insert blank line
		nputc (n.colorframe, vline);
		ncrlf ();

		sprintf (tempstr, "%-26s", frmstr);
		nputs (dtree_colors[level], tempstr);	//  spaces
	}
	else if (slen > 25) {
		sprintf (tempstr, "%-37s", lstr);	//  write filename
		nputs (dtree_colors[level], tempstr);

		//  insert blank line
		nputc (n.colorframe, vline);
		sprintf (tempstr, "%26s", "");
		nputs (dtree_colors[level], tempstr);	//  spaces

		nputc (n.colorframe, vline);
		ncrlf ();
		sprintf (tempstr, "%-26s", frmstr);
		nputs (dtree_colors[level], tempstr);	//  spaces
	}
	else {
		sprintf (tempstr, "%-26s", lstr);
		nputs (dtree_colors[level], tempstr);
	}
}

//**********************************************************
static void display_size(u64 dlen, unsigned slen, unsigned attr)
{
   if (dlen > (u64) 999999999) {
      dlen /= 1000 ;
      dsize.convert (dlen);
      sprintf (tempstr, "%*s", (int) (slen-1), dsize.putstr ());
      nputs (attr, tempstr);
      nputc(attr ^ 0x08, 'K') ;
   } else {
      dsize.convert (dlen);
      sprintf (tempstr, "%*s", (int) slen, dsize.putstr ());
      nputs (attr, tempstr);
   }
}

//**********************************************************
//  recursive routine to display directory tree
//  do all subroutines, then go to next.
//**********************************************************
static void display_dir_tree (dirs * ktop)
{
	dirs *ktemp = ktop;
	if (ktop == NULL)
		return;

	//  next, build tree lists for subsequent levels (recursive)
	while (ktemp != NULL) {
		//  first, build tree list for current level
		if (level == 0) {
			formstr[0] = (char) NULL;
		}
		else {
			if (ktemp->brothers == (struct dirs *) NULL) {
            formstr[level - 1] = (char) (n.low_ascii) ? '\\' : '�';   //lint !e743 
				formstr[level] = (char) NULL;
			}
			else {
            formstr[level - 1] = (char) (n.low_ascii) ? '+' : '�';   //lint !e743 
				formstr[level] = (char) NULL;
			}
		}
		sprintf (levelstr, "%s%s", formstr, ktemp->name);

		//*****************************************************************
		//                display data for this level                      
		//*****************************************************************
		display_tree_filename (levelstr, formstr);
		switch (n.tree) {
         //  show file/directory sizes only
			case 1:
				if (ktemp->dirsize != ktemp->subdirsize ||
					ktemp->dirsecsize != ktemp->subdirsecsize) {
               // dsize.convert  (ktemp->dirsize);
               // dssize.convert (ktemp->dirsecsize);
               // sdsize.convert (ktemp->subdirsize);

					//  now, print the normal directory
               // sprintf (tempstr, "%11s", dsize.putstr ());
               // nputs (dtree_colors[level], tempstr);

					// sprintf(tempstr, "%11s %14s", dssize_ptr, sdsize_ptr) ;
               // sprintf (tempstr, "%11s %14s", dssize.putstr (),
               //   sdsize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               display_size(ktemp->dirsize, 11, dtree_colors[level]) ;
               nputc (n.colorframe, vline);
               display_size(ktemp->dirsecsize, 11, dtree_colors[level]) ;
               nputc (dtree_colors[level], ' ');
               display_size(ktemp->subdirsize, 14, dtree_colors[level]) ;
               nputc (n.colorframe, vline);
            }

				/*  no subdirectories are under this one  */
				else {
					//  now, print the normal directory
					nputs (dtree_colors[level], "           ");
					nputc (n.colorframe, vline);
               nputs (dtree_colors[level], "            ");
					// sprintf(tempstr, "            %14s", sdsize_ptr) ;
               // sdsize.convert (ktemp->subdirsize);
               // sprintf (tempstr, "%14s", sdsize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               display_size(ktemp->subdirsize, 14, dtree_colors[level]) ;
               nputc (n.colorframe, vline);
            }                   /* end  else !(ktemp->nsdi) */

            // sprintf(tempstr, "%14s", sdssize_ptr) ;
            // sdssize.convert (ktemp->subdirsecsize);
            // sprintf (tempstr, "%14s", sdssize.putstr ());
            // nputs (dtree_colors[level], tempstr);
            display_size(ktemp->subdirsecsize, 14, dtree_colors[level]) ;
				break;

         //  show file/directory counts only
			case 4:
				if ((ktemp->files == ktemp->subfiles) &&
					(ktemp->directs == ktemp->subdirects)) {
					//  now, print the normal directory
					nputs (dtree_colors[level], "           ");
					nputc (n.colorframe, vline);

               // sdsize.convert ((unsigned long long) ktemp->files);
               // sprintf (tempstr, "            %12s  ", sdsize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               nputs (dtree_colors[level], "            ");
               display_size((u64) ktemp->files, 12, dtree_colors[level]) ;

               nputs (dtree_colors[level], "  ");
					nputc (n.colorframe, vline);
					// sprintf(tempstr, "%14s", sdssize_ptr) ;
               // sdssize.convert ((unsigned long long) ktemp->directs);
               // sprintf (tempstr, "%10s", sdssize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               display_size((u64) ktemp->directs, 10, dtree_colors[level]) ;
				}

				/*  no subdirectories are under this one  */
				else {
					//  now, print the normal directory
               // dsize.convert ((unsigned long long) ktemp->files);
               // sprintf (tempstr, "%9s  ", dsize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               display_size((u64) ktemp->files, 9, dtree_colors[level]) ;
               nputs (dtree_colors[level], "  ");
					nputc (n.colorframe, vline);

               // dssize.convert ((unsigned long long) ktemp->directs);
               // sdsize.convert ((unsigned long long) ktemp->subfiles);
               // sprintf (tempstr, "%9s   %12s  ", dssize.putstr (),
               //   sdsize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               display_size((u64) ktemp->directs, 9, dtree_colors[level]) ;
               nputs (dtree_colors[level], "   ");
               display_size((u64) ktemp->subfiles, 12, dtree_colors[level]) ;
               nputs (dtree_colors[level], "  ");
					nputc (n.colorframe, vline);

               // sdssize.convert ((unsigned long long) ktemp->subdirects);
               // sprintf (tempstr, "%10s", sdssize.putstr ());
               // nputs (dtree_colors[level], tempstr);
               display_size((u64) ktemp->subdirects, 10, dtree_colors[level]) ;
				}						  /* end  else !(ktemp->nsdi) */
				break;

         //  show mixed size, file counts, directory counts
			case 5:
            //  now, print the normal directory
            // dsize.convert ((unsigned long long) ktemp->subfiles);
            // sprintf (tempstr, "%9s  ", dsize.putstr ());
            // nputs (dtree_colors[level], tempstr);
            display_size((u64) ktemp->subfiles, 9, dtree_colors[level]) ;
            nputs (dtree_colors[level], "  ");
            nputc (n.colorframe, vline);

            // dssize.convert ((unsigned long long) ktemp->subdirects);
            // sdsize.convert ((unsigned long long) ktemp->dirsecsize);
            // sprintf (tempstr, "%9s   %13s ", dssize.putstr (),
            //   sdsize.putstr ());
            // nputs (dtree_colors[level], tempstr);
            display_size((u64) ktemp->subdirects, 9, dtree_colors[level]) ;
            nputs (dtree_colors[level], "   ");
            display_size(ktemp->dirsecsize, 13, dtree_colors[level]) ;
            nputs (dtree_colors[level], " ");
            nputc (n.colorframe, vline);

            // sdssize.convert ((unsigned long long) ktemp->subdirsecsize);
            // sprintf (tempstr, "%14s", sdssize.putstr ());
            // nputs (dtree_colors[level], tempstr);
            display_size(ktemp->subdirsecsize, 14, dtree_colors[level]) ;
				break;

			default:
				break;				  // make lint happy
		}
		ncrlf ();

		//  build tree string for deeper levels
		if (level > 0) {
			if (ktemp->brothers == NULL)
				formstr[level - 1] = ' ';
			else
            formstr[level - 1] = (n.low_ascii) ? '|' : '�'; //lint !e743 
		}								  //  if level > 1

		//  process any sons
		level++;
		if (!n.tree_short || level < 2) {
			display_dir_tree (ktemp->sons);
		}
		formstr[--level] = (char) NULL;

		//  goto next brother
		ktemp = ktemp->brothers;
	}									  //  while not done listing directories
}

/*****************************************************************/
static void printdirheader (void)
{
   // int slen, freelen;

	nputs (n.colornhead, "Directory of ");
	sprintf (tempstr, "%s", base_path);
	nputs (n.colorxhead, tempstr);

   // slen = _where_x ();
   // freelen = 80 - (17 + strlen (volume_name));
   // if (slen < freelen) {
   //   nput_char (n.colornhead, ' ', (freelen - slen));
   // }
   // else {
   //   ncrlf ();
   // }
   // nputs (n.colornhead, "Volume label is ");
   // nputs (n.colorxhead, volume_name);
	ncrlf ();

	//**************************************
	//  Heading line 1
	//**************************************
	memset (&tempstr[0], dline, 79);
	tempstr[79] = 0;
	tempstr[25] = tempstr[49] = tline;
	nputs (n.colorframe, tempstr);
	ncrlf ();

	switch (n.tree) {
		case 1:
			//**************************************
			//  Heading line 2
			//**************************************
			nput_char (n.colornhead, ' ', 25);
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "   size of requested   ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "    total size,  including");
			ncrlf ();

			//**************************************
			//  Heading line 3
			//**************************************
			nput_char (n.colornhead, ' ', 25);
			// nputs (n.colornhead, "Subdirectory names       ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "       directory       ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "     lower subdirectories");
			ncrlf ();
			break;

		case 4:
			//**************************************
			//  Heading line 2
			//**************************************
			nput_char (n.colornhead, ' ', 25);
			nputc (n.colorframe, vline);
			nputs (n.colornhead, " files and directories ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "   files and directories ");
			ncrlf ();

			//**************************************
			//  Heading line 3
			//**************************************
			nput_char (n.colornhead, ' ', 25);
			nputc (n.colorframe, vline);
			nputs (n.colornhead, " in current directory  ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, " in current and lower dirs");
			ncrlf ();
			break;

		case 5:
			//**************************************
			//  Heading line 2
			//**************************************
			nput_char (n.colornhead, ' ', 25);
			nputc (n.colorframe, vline);
			nputs (n.colornhead, " files and directories ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "    files and directories");
			ncrlf ();

			//**************************************
			//  Heading line 3
			//**************************************
			nput_char (n.colornhead, ' ', 25);
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "   cumulative counts   ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "       cumulative sizes   ");
			ncrlf ();
			break;

		default:
			break;					  // make lint happy
	}

	//**************************************
	//  Heading line 4
	//**************************************
   nputs (n.colorframe, (n.low_ascii) ? dhdrA : dhdr);
   // if (n.low_ascii)
   //    nputs (n.colorframe, dhdrA);
   // else
   //    nputs (n.colorframe, dhdr);
	ncrlf ();

	//**************************************
	//  Heading line 5
	//**************************************
	nputs (n.colornhead, "Subdirectory names       ");
	nputc (n.colorframe, vline);
	switch (n.tree) {
		case 1:
			nputs (n.colornhead, "  in bytes ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, " disk space");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "   in bytes   ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "  disk space");
			ncrlf ();
			break;

		case 4:
			nputs (n.colornhead, "    files  ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "directories");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "       files  ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, " directories");
			ncrlf ();
			break;

		case 5:
			nputs (n.colornhead, "    files  ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "directories");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "  disk space  ");
			nputc (n.colorframe, vline);
			nputs (n.colornhead, "  disk space");
			ncrlf ();
			break;

		default:
			break;					  // make lint happy
	}

	//**************************************
	//  Heading line 6
	//**************************************
	memset (&tempstr[0], dline, 79);
	tempstr[79] = 0;
	tempstr[25] = tempstr[49] = bline;
	tempstr[37] = tempstr[64] = xline;
	nputs (n.colorframe, tempstr);
	ncrlf ();
}

//*********************************************************
static void print_dir_end (void)
{
	//  draw divider line for bottom of data
	memset (&tempstr[0], dline, 79);
	tempstr[79] = 0;
	tempstr[37] = tempstr[64] = bline;
	nputs (n.colorframe, tempstr);
	ncrlf ();

	//  now show disk totals
   put_disk_summary ();
}

//****************************************************
//  allocate a dummy structure for merge_sort()
//****************************************************
static int tree_init_sort (void)
{
	z = new dirs;
	if (z == NULL)
      error_exit (ENOMEM, NULL);
	z->sons = NULL;
	z->brothers = NULL;
   return 0;
}

//****************************************************
// static void free_tree_structs(void)
//    {
//    if (z != NULL)  delete z ;
//    }

//*********************************************************
static int tree_sort_name (struct dirs *a, struct dirs *b)
{
	return (_strcmpi (a->name, b->name));
}

//*********************************************************
static int tree_sort_name_rev (struct dirs *a, struct dirs *b)
{
	return (_strcmpi (b->name, a->name));
}

//*********************************************************
static int tree_sort_size (struct dirs *a, struct dirs *b)
{
	if (a->subdirsecsize > b->subdirsecsize)
		return (1);
	else if (b->subdirsecsize > a->subdirsecsize)
		return (-1);
	else
		return (0);
}

//*********************************************************
static int tree_sort_size_rev (struct dirs *a, struct dirs *b)
{
	if (b->subdirsecsize > a->subdirsecsize)
		return (1);
	else if (a->subdirsecsize > b->subdirsecsize)
		return (-1);
	else
		return (0);
}

//*********************************************************
//  This routine merges two sorted linked lists.
//*********************************************************
static struct dirs *tree_merge (struct dirs *a, struct dirs *b)
{
	struct dirs *c;
	c = z;

	do {
		int x = tree_sort_fcn (a, b);
		if (x <= 0) {
			c->brothers = a;
			c = a;
			a = a->brothers;
		}
		else {
			c->brothers = b;
			c = b;
			b = b->brothers;
		}
	}
	while ((a != NULL) && (b != NULL));

	if (a == NULL)
		c->brothers = b;
	else
		c->brothers = a;
	return z->brothers;
}

//*********************************************************
//  This routine recursively splits linked lists
//  into two parts, passing the divided lists to
//  merge() to merge the two sorted lists.
//*********************************************************
static struct dirs *tree_merge_sort (struct dirs *c)
{
	struct dirs *a, *b, *prev;
	int pcount = 0;
	int j = 0;

	if ((c != NULL) && (c->brothers != NULL)) {
		a = c;
		while (a != NULL) {
			pcount++;
			a = a->brothers;
		}
		a = c;
		b = c;
		prev = b;
		while (j < pcount / 2) {
			j++;
			prev = b;
			b = b->brothers;
		}
		prev->brothers = NULL;	  //lint !e771

		return tree_merge (tree_merge_sort (a), tree_merge_sort (b));
	}
	return c;
}

//*********************************************************
//  recursive routine that sorts the sons of each brother
//*********************************************************
static struct dirs *tree_sort_walk (struct dirs *t)
{
	struct dirs *dptr = t;
	while (dptr != 0) {
		dptr->sons = tree_sort_walk (dptr->sons);
		dptr = dptr->brothers;
	}
	return tree_merge_sort (t);
}

//*********************************************************
//  This intermediate function is used because I want
//  merge_sort() to accept a passed parameter,
//  but in this particular application the initial
//  list is global.  This function sets up the global
//  comparison-function pointer and passes the global
//  list pointer to merge_sort().
//*********************************************************
static void sort_trees (void)
{
	// tree_sort_fcn = current_sort ;

	if (n.reverse) {
		if (n.sort == SORT_SIZE)
			tree_sort_fcn = tree_sort_size_rev;
		else
			tree_sort_fcn = tree_sort_name_rev;
	}

	//  normal sort
	else {
		if (n.sort == SORT_SIZE)
			tree_sort_fcn = tree_sort_size;
		else
			tree_sort_fcn = tree_sort_name;
	}

	//  now, sort the data
	top = tree_sort_walk (top);
}
