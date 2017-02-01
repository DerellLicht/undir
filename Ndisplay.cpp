//*****************************************************************
//                                                                 
//  NDISPLAY.CPP - NDIR display handlers                           
//                                                                 
//  Written by:   Daniel D. Miller  (the Derelict)                 
//                                                                 
//  Last update:  01/15/01 21:20                                   
//                                                                 
//  compile with  makefile                                         
//                                                                 
//*****************************************************************

// #include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
// #include <sys/types.h>
#include <sys/stat.h>
// #include <errno.h>
// Ndisplay.cpp  20  Warning 537: Repeated include file '\k26inc_user\time.h'
// Ndisplay.cpp  20  Warning 451: Header file '\k26inc_user\time.h' repeatedly included but does not have a standard include guard
#ifdef _lint
/* Used by other time functions.  */
struct tm
{
  int tm_sec;        /* Seconds. [0-60] (1 leap second) */
  int tm_min;        /* Minutes. [0-59] */
  int tm_hour;       /* Hours.   [0-23] */
  int tm_mday;       /* Day.     [1-31] */
  int tm_mon;        /* Month.   [0-11] */
  int tm_year;       /* Year  - 1900.  */
  int tm_wday;       /* Day of week.   [0-6] */
  int tm_yday;       /* Days in year.[0-365] */
  int tm_isdst;         /* DST.     [-1/0/1]*/

#ifdef   __USE_BSD
  long int tm_gmtoff;      /* Seconds east of UTC.  */
  __const char *tm_zone;   /* Timezone abbreviation.  */
#else
  long int __tm_gmtoff;    /* Seconds east of UTC.  */
  __const char *__tm_zone; /* Timezone abbreviation.  */
#endif
}; //lint !e770
extern struct tm *localtime(time_t *CLOCK);
#else
#include <time.h>
#endif

#include "ndata.hpp"
#include "i64tostr.hpp"

//lint -esym(522, dclreol, dgotoxy, ngotoxy)

//  from NDIR.CPP
// extern char const * const ShortVersion ;

static uchar attrclr;

static char monthstr[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

//*********************************************************
//  this handles full-screen in NON-COLOR mode.
//*********************************************************
static void testpause (void)
{
	if (!n.pause)
		return;
	if (is_redirected ())
		return;

	if (++linecnt >= lines - 1) {
		nputs (n.colornhead, "Press any key to continue (or ESC to exit)");
      // unsigned inkey = get_scode ();
      unsigned inkey = getchar();
		if (inkey == 27) {
         // if ((curlines != lines) && (!(n.ega_keep)))
         //   set_lines (25);
         error_exit (0, NULL);
		}

		if (n.color) {
			dreturn ();				  // CR only!! 
			dclreol ();
		}
		else {
			printf ("\n");
		}
		linecnt = 1;
	}
}

/*****************************************************************/
void ngotoxy (int x, int y)
{
	if (n.color)
		dgotoxy (x, y);
}

//**************************************************
void info (char *data[])
{
	unsigned j = 0;

	while (data[j] != NULL) {
		nputs (n.colordefalt, data[j++]);
		ncrlf ();
	}
}

//**************************************************
//  linecnt = number of lines written (non-color).
//**************************************************
void display_logo (void)
{
	if (n.batch)
		return;

	if (n.clear && !is_redirected ())
		dclrscr ();

	linecnt = 0;

	if (n.minimize) {
		ngotoxy (0, _where_y ());
		nputs (n.colorlogo, ShortVersion);
		// ncrlf ();
	}
	else {
		ngotoxy ((80 - strlen (Version)) / 2, _where_y ());
		nputs (n.colorlogo, Version);
		ncrlf ();
	}
	// nputs(n.colordefalt, ShortVersion) ;
	set_text_attr (n.colordefalt);
}

//*********************************************************************
// union ul2uc {
//    unsigned       ul ;
//    unsigned short us[2] ;
//    unsigned char  uc[4] ;
// };

//*********************************************************************
// unsigned short get2bytes(unsigned char *p)
// {
//    union ul2uc uconv ;
//    
//    uconv.uc[1] = *p++ ;
//    uconv.uc[0] = *p ;
//    return uconv.us[0] ;
// }

//*********************************************************************
// unsigned get4bytes(unsigned char *p)
// {
//    union ul2uc uconv ;
//    
//    uconv.uc[3] = *p++ ;
//    uconv.uc[2] = *p++ ;
//    uconv.uc[1] = *p++ ;
//    uconv.uc[0] = *p ;
//    return uconv.ul ;
// }

//************************************************************************
void print1 (ffdata * fptr)
{
   // char mlstr[30] ;
   // parse_time outdt;
   // int show_normal_info ;
	uchar SHRattr = fptr->attrib & 7;
	// outdt.dtime[0] = fptr->ftime ;
	// outdt.dtime[1] = fptr->fdate ;
   // FILETIME lft;
   // FileTimeToLocalFileTime (&(fptr->ft), &lft);
   // FileTimeToDosDateTime (&lft, &(outdt.dtime[1]), &(outdt.dtime[0]));
   struct tm *ftm = localtime((time_t *) &fptr->ft) ;

   int secs  = ftm->tm_sec ;
   int mins  = ftm->tm_min ;
   int hour  = ftm->tm_hour ;
   int day   = ftm->tm_mday ;
   int month = ftm->tm_mon + 1 ;
   long year = 1900 + ftm->tm_year ;

	//  detect non-standard file attributes and display specially.
   // if ((fptr->attrib) > 0x3F) {
   //   sprintf (attr, " 0x%2X ", fptr->attrib);
   //   attrclr = 0x0C;
   // }
   // else {
   //   sprintf (attr, "%5s ", attrstr[fptr->attrib]);
   //   attrclr = n.colorattr;
   // }
   // char attrstr[] = "-adshr";
   // attr[0] = (fptr->attrib & FILE_ATTRIBUTE_ARCHIVE)   ? 'a' : '_' ;
   // attr[1] = (fptr->attrib & FILE_ATTRIBUTE_DIRECTORY) ? 'd' : '_' ;
   // attr[2] = (fptr->attrib & FILE_ATTRIBUTE_SYSTEM)    ? 's' : '_' ;
   // attr[3] = (fptr->attrib & FILE_ATTRIBUTE_HIDDEN)    ? 'h' : '_' ;
   // attr[4] = (fptr->attrib & FILE_ATTRIBUTE_READONLY)  ? 'r' : '_' ;
   // attr[5] = '\0';

   char attr[12];
   attr[0] = fptr->dirflag ;
   attr[1] = (fptr->attrib & S_IRUSR) ? 'r' : '-' ;
   attr[2] = (fptr->attrib & S_IWUSR) ? 'w' : '-' ;
   attr[3] = (fptr->attrib & S_IXUSR) ? 'x' : '-' ;
   attr[4] = (fptr->attrib & S_IRGRP) ? 'r' : '-' ;
   attr[5] = (fptr->attrib & S_IWGRP) ? 'w' : '-' ;
   attr[6] = (fptr->attrib & S_IXGRP) ? 'x' : '-' ;
   attr[7] = (fptr->attrib & S_IROTH) ? 'r' : '-' ;
   attr[8] = (fptr->attrib & S_IWOTH) ? 'w' : '-' ;
   attr[9] = (fptr->attrib & S_IXOTH) ? 'x' : '-' ;
   attr[10] = ' ' ;
   attr[11] = 0 ;
   // sprintf(attr, "0x%06o ", fptr->attrib) ;

   attrclr = n.colorattr;

   // mlstr[0] = 0 ;
   // show_normal_info = 1 ;
	//  display directory entry
   if (fptr->dirflag == 'd') {
      nputs (attrclr, attr);
		sprintf (tempstr, "%14s ", "");
		nputs (n.colorsize, tempstr);
      //sprintf(tempstr, "%02d-%02d-%04lu ", month, day, year);
      sprintf (tempstr, "%3s %02d, %04lu ", monthstr[month - 1], day,
         year);
      nputs (n.colordate, tempstr);
      sprintf (tempstr, "%02d:%02d:%02d ", hour, mins, secs);
      nputs (n.colortime, tempstr);
      sprintf (tempstr, "[%s]", fptr->filename);

		//  display filename in appropriate color...
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (n.colordir, tempstr);
	}

	//  display file entry
	else {
      i64tostr fsize (fptr->fsize);
      // char *p ;
      // unsigned idx ;

      //  display normal file listing
      nputs (attrclr, attr);

      //  debug: show attribute in binary form
      // sprintf (tempstr, "%04X ", fptr->attrib);
      // nputs (attrclr, tempstr);

      //  show file size
      sprintf (tempstr, "%14s ", fsize.putstr ());
      nputs (n.colorsize, tempstr);

      sprintf (tempstr, "%3s %02d, %04lu ", monthstr[month - 1], day, year);
      nputs (n.colordate, tempstr);
      sprintf (tempstr, "%02d:%02d:%02d ", hour, mins, secs);
      nputs (n.colortime, tempstr);

      //  format filename as required
      if (fptr->dirflag == 'l'  &&  fptr->linktgt != 0) {
         sprintf (tempstr, "%s -> %s ", fptr->filename, fptr->linktgt);
      } else {
         sprintf (tempstr, "%s ", fptr->filename);
      }

      //  display filename in appropriate color...
      if (SHRattr != 0 && n.showSHRfiles)
         nputs (n.colorSHR | SHRattr, tempstr);
      else
         nputs (fptr->color, tempstr);
   }
}

//*********************************************************
//  from FILELIST.CPP
//  global vars for file size calculations
//*********************************************************
extern char tempfmtstr[6];		  //  for forming strings of the form %nnld

//*********************************************************
void lfn_print2 (ffdata * fptr)
{
	uchar SHRattr = fptr->attrib & 7;

   struct tm *ftm = localtime((time_t *) &fptr->ft) ;

   int mins  = ftm->tm_min ;
   int hour  = ftm->tm_hour ;
   int day   = ftm->tm_mday ;
   int month = ftm->tm_mon + 1 ;
   long year = 1900 + ftm->tm_year ;

   u64 fsize = fptr->fsize;

   if (fptr->dirflag == 'd') {
      nputs (n.colordir, " [DIR] ");
		sprintf (tempstr, "%02d-%3s-%02d ", day, monthstr[month - 1],
			(int) (year % 100));
		nputs (n.colordate, tempstr);
		sprintf (tempstr, "%02d:%02d ", hour, mins);
		nputs (n.colortime, tempstr);

		sprintf (tempstr, tempfmtstr, fptr->filename);
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (n.colordir, tempstr);
	}
	else {
		//  print file size
		if (fsize > 99999999L) {
#ifdef _MSC_VER
			sprintf (tempstr, "%5I64u", fsize / 1000000L);
#else
			sprintf (tempstr, "%5llu", fsize / 1000000L);
#endif
			nputs (n.colorsize, tempstr);
			nputs (n.colorsize ^ 0x08, "M ");
		}
		else if (fsize > 999999L) {
#ifdef _MSC_VER
			sprintf (tempstr, "%5I64u", fsize / 1000L);
#else
			sprintf (tempstr, "%5llu", fsize / 1000L);
#endif
			nputs (n.colorsize, tempstr);
			nputs (n.colorsize ^ 0x08, "K ");
		}
		else {
#ifdef _MSC_VER
			sprintf (tempstr, "%6I64u ", fsize);
#else
			sprintf (tempstr, "%6llu ", fsize);
#endif
			nputs (n.colorsize, tempstr);
		}

		sprintf (tempstr, "%02d-%3s-%02d ", day, monthstr[month - 1],
			(int) (year % 100));
		nputs (n.colordate, tempstr);
		sprintf (tempstr, "%02d:%02d ", hour, mins);
		nputs (n.colortime, tempstr);

		//  generate filename
		sprintf (tempstr, tempfmtstr, fptr->filename);
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (fptr->color, tempstr);
	}
}

/*****************************************************************/
void lfn_print4 (ffdata * fptr)
{
   u64 fsize = fptr->fsize;
	uchar SHRattr = fptr->attrib & 7;

   if (fptr->dirflag == 'd') {
      nputs (n.colordir, " [DIR] ");
		sprintf (tempstr, tempfmtstr, fptr->filename);
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (n.colordir, tempstr);
	}
	else {
		//  print file size
		if (fsize > 99999999L) {
#ifdef _MSC_VER
			sprintf (tempstr, "%5I64u", fsize / 1000000L);
#else
			sprintf (tempstr, "%5llu", fsize / 1000000L);
#endif
			nputs (n.colorsize, tempstr);
			nputs (n.colorsize ^ 0x08, "M ");
		}
		else if (fsize > 999999L) {
#ifdef _MSC_VER
			sprintf (tempstr, "%5I64u", fsize / 1000L);
#else
			sprintf (tempstr, "%5llu", fsize / 1000L);
#endif
			nputs (n.colorsize, tempstr);
			nputs (n.colorsize ^ 0x08, "K ");
		}
		else {
#ifdef _MSC_VER
			sprintf (tempstr, "%6I64u ", fsize);
#else
			sprintf (tempstr, "%6llu ", fsize);
#endif
			nputs (n.colorsize, tempstr);
		}

		//  generate filename
		sprintf (tempstr, tempfmtstr, fptr->filename);
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (fptr->color, tempstr);
	}
}

/*****************************************************************/
void lfn_print6 (ffdata * fptr)
{
	uchar SHRattr = fptr->attrib & 7;
   if (fptr->dirflag == 'd') {
		sprintf (tempstr, tempfmtstr, fptr->filename);
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (n.colordir, tempstr);
	}
	else {
		sprintf (tempstr, tempfmtstr, fptr->filename);
		if (SHRattr != 0 && n.showSHRfiles)
			nputs (n.colorSHR | SHRattr, tempstr);
		else
			nputs (fptr->color, tempstr);
	}
}

/******************************************************************/
void ncrlf (void)
{
	if (n.color)
      dnewline ();
	else
		printf ("\n");
	testpause ();
}

/******************************************************************/
void nputc (uchar attr, const uchar outchr)
{
	if (n.color) {
      set_text_attr (attr);
      dputc (outchr);
	}
	else
		printf ("%c", outchr);
}

/******************************************************************/
void nputs (uchar attr, const char *outstr)
{
	if (n.color) {
      set_text_attr (attr);
      dputs (outstr);
	}
	else
      printf ("%s", outstr);
}

/******************************************************************/
void nput_char (uchar attr, char chr, int count)
{
	if (n.color) {
      dputnchar (chr, attr, count);
	}
	else {
		for (int j = 0; j < count; j++)
			putchar (chr);
	}
}

/******************************************************************/
void nput_line (uchar attr, char chr)
{
   int j, wincols = get_window_cols() - 1 ;
	if (n.color) {
      dputnchar (chr, attr, wincols);
	}
	else {
      for (j = 0; j < wincols; j++)
			putchar (chr);
	}
	ncrlf ();
}

