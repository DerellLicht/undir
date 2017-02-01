//****************************************************************************
//  diskparm.cpp - linux version of ndir drive_summary
//                                                                 
//  Written by:   Daniel D. Miller  (the derelict)                 
//                                                                 
//  compile with makefile                                          
//****************************************************************************
// #  read-only disk
// > cat /proc/mounts
// /dev/root / ext2 ro 0 0
// /proc /proc proc rw 0 0
// /dev/ram0 /var ext2 rw 0 0
// /dev/ram1 /tmp ext2 rw 0 0
// /dev/ram2 /dev ext2 rw 0 0
// /dev/pts /dev/pts devpts rw 0 0
// automount(pid389) /misc autofs rw 0 0
// 
// #  read-write disk
// ~ >cat /proc/mounts
// /dev/root / ext2 rw 0 0
// /proc /proc proc rw 0 0
// none /dev/pts devpts rw 0 0
//****************************************************************************

// #include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/vfs.h>

#include "ndata.hpp"
#include "i64tostr.hpp"

//****************************************************************************
typedef struct mount_s {
   struct mount_s *next ;
   char  devname[40] ;
   char  mountpt[40] ;
   char  filesys[20] ;
   int   readonly ;
   int   first_num ;
   int   second_num ;
   int   mountpt_len ;  //  length of name
} mount_t, *mount_p ;

static mount_p mount_top = 0 ;
static mount_p mount_tail ;

//****************************************************************************
static int read_proc_mounts(void)
{
   int mcount = 0 ;
   char *hd, *tl ;
   mount_p mtemp ;
   FILE *fd = fopen("/proc/mounts", "rt") ;
   if (fd == 0) {
      // perror("/proc/mounts") ;
      return -(int)errno;
   }
   while (fgets(tempstr, sizeof(tempstr), fd) != 0) {
      if (tempstr[0] != '/')
         continue;

      mtemp = (mount_p) new mount_t ;
      // if (mtemp == 0)
      //    return -(int)ENOMEM;
      bzero((char *) mtemp, sizeof(mount_t)) ;

      //   /dev/pts /dev/pts devpts rw 0 0
      hd = tempstr ;
      tl = strchr(hd, ' ') ;
      *tl++ = 0 ; //lint !e613
      strncpy(mtemp->devname, hd, sizeof(mtemp->devname)) ;

      hd = tl ;
      tl = strchr(hd, ' ') ;
      *tl++ = 0 ; //lint !e613
      strncpy(mtemp->mountpt, hd, sizeof(mtemp->mountpt)) ;
      mtemp->mountpt_len = strlen(mtemp->mountpt) ;

      hd = tl ;
      tl = strchr(hd, ' ') ;
      *tl++ = 0 ; //lint !e613
      strncpy(mtemp->filesys, hd, sizeof(mtemp->filesys)) ;

      mtemp->readonly = (strncasecmp(tl, "ro", 2) == 0) ? 1 : 0 ;

      //  add struct to list
      if (mount_top == 0)
          mount_top = mtemp ;
      else
          mount_tail->next = mtemp ;
      mount_tail = mtemp ;
      mcount++ ;
   }
   fclose(fd) ;

   return mcount ;
}

// *************************************************************************
//                                Disk Drive Summary                        
// =========================================================================
// file sys    total space         free space  [Cluster Size] mount point  
// ========  ===============  ===============  ==========================  
// ext2        8,290,459,648    6,799,368,192  [ 4096] /
// proc                    0                0  [ 1024] /proc
// ext2           16,735,232       16,624,640  [ 1024] /var
// ext2            4,058,112        3,989,504  [ 1024] /tmp
// ext2            1,010,688          961,536  [ 1024] /dev
// devpts                  0                0  [ 1024] /dev/pts
//*****************************************************************
//  this function determines the following information:
//    - What file system is in use??
//    - Total partition size in bytes
//    - Free partition space in bytes
//    - Cluster size for active partition
//*****************************************************************
int get_disk_info(char *dstr)
{
   int result ;
   mount_p mtemp ;
   mount_p mroot = 0 ;
   struct statfs st ;
   // struct statfs myfs ;

   if (mount_top == 0) {
      result = read_proc_mounts() ;
      if (result < 0) {
         sprintf(tempstr, "read /proc/mounts: %s\n", strerror(-result)) ;
         nputs (n.colordefalt, tempstr);
         diskbytes = 0 ;   
         diskfree = 0 ;    
         clbytes = 4096 ;  //  cluster size
         return 0 ;
      }
   }

   //********************************************
   //  now, look up the target path
   //********************************************
   diskbytes = 0 ;
   for (mtemp=mount_top; mtemp != 0; mtemp = mtemp->next) {
      // sprintf(tempstr, "compare %s vs %s [%d]\n", dstr, mtemp->mountpt, mtemp->mountpt_len) ;
      // nputs (n.colordefalt, tempstr);

      //  don't compare vs root path on first pass...
      if (mtemp->mountpt_len == 1) {
         mroot = mtemp ;
      } else {
         if (strncmp(dstr, mtemp->mountpt, mtemp->mountpt_len) == 0) {
            result = statfs(mtemp->mountpt, &st) ;
            if (result != 0) {
               perror(mtemp->mountpt) ;
               diskbytes = 0 ;   
               diskfree = 0 ;    
               clbytes = 4096 ;  //  cluster size
               break;
            }
            // nputs (n.colordefalt, "found something??\n");
            diskbytes = (u64) st.f_bsize * (u64) st.f_blocks ; //lint !e571
            diskfree  = (u64) st.f_bsize * (u64) st.f_bfree ;  //lint !e571
            clbytes   = st.f_bsize ;
            break;
         }
      }
      // nputs (n.colordefalt, "trying next...\n");
   }
   //  if we didn't find our target path, get data from root path...
   if (diskbytes == 0) {
      //  unless, of course, that wasn't found either
      if (mroot == 0) {
         // nputs (n.colordefalt, "no root info??\n");
         diskbytes = 0 ;   
         diskfree = 0 ;    
         clbytes = 4096 ;  //  cluster size
         return 0;
      }
      // nputs (n.colordefalt, "using default root info\n");
      result = statfs(mroot->mountpt, &st) ;
      if (result != 0) {
         perror(mroot->mountpt) ;
         diskbytes = 0 ;   
         diskfree = 0 ;    
         clbytes = 4096 ;  //  cluster size
         return 0;
      }
      diskbytes = (u64) st.f_bsize * (u64) st.f_blocks ; //lint !e571
      diskfree  = (u64) st.f_bsize * (u64) st.f_bfree ;  //lint !e571
      clbytes   = st.f_bsize ;
   }
   return 0;
}

//****************************************************************************
//   struct statfs {
//      long    f_type;     /* type of filesystem (see below) */
//      long    f_bsize;    /* optimal transfer block size */
//      long    f_blocks;   /* total data blocks in file system */
//      long    f_bfree;    /* free blocks in fs */
//      long    f_bavail;   /* free blocks avail to non-superuser */
//      long    f_files;    /* total file nodes in file system */
//      long    f_ffree;    /* free file nodes in fs */
//      fsid_t  f_fsid;     /* file system id */
//      long    f_namelen;  /* maximum length of filenames */
//      long    f_spare[6]; /* spare for later */
//   };
//****************************************************************************
void display_drive_summary(void)
{
   int result ;
   mount_p mtemp ;
   u64 frees1, totals1;
   struct statfs st ;
   // struct stat st ; //  doesn't provide useful info about mounts
   i64tostr diskavail (0);      //lint !e747 
   i64tostr disktotal (0);      //lint !e747 

   //  draw header
   ncrlf() ;
   nput_line (n.colorframe, '*');
   nputs (n.colornhead,
      "                               Disk Drive Summary                              \n\r");
   nput_line (n.colorframe, '=');

   nputs (n.colornhead,
      "file sys    total space         free space  [Cluster Size] mount point  \n");
   nputs (n.colornhead,
      "========  ===============  ===============  ==========================  \n");

   result = read_proc_mounts() ;
   if (result < 0) {
      sprintf(tempstr, "read /proc/mounts: %s\n", strerror(-result)) ;
      nputs (n.colordefalt, tempstr);
      return ;
   }
   // printf("found %d entries in /proc/mounts\n", result) ;
   for (mtemp=mount_top; mtemp != 0; mtemp = mtemp->next) {
      // printf("%s mounted on %s, %s\n",
      //    mtemp->devname, mtemp->mountpt, (mtemp->readonly) ? "readonly" : "read-write") ;
      result = statfs(mtemp->mountpt, &st) ;
      if (result != 0) {
         perror(mtemp->mountpt) ;
         continue;
      }
      totals1 = (u64) st.f_bsize * (u64) st.f_blocks ;   //lint !e571
      frees1  = (u64) st.f_bsize * (u64) st.f_bfree ;    //lint !e571
      disktotal.convert (totals1);
      diskavail.convert (frees1);
      sprintf (tempstr, "%-8s  %15s  %15s  [%5u] %s", mtemp->filesys,
            disktotal.putstr (), diskavail.putstr (),
            (unsigned) st.f_bsize, mtemp->mountpt) ;
      nputs (n.colordefalt, tempstr);
      ncrlf() ;
   }
}

