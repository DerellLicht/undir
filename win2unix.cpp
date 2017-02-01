#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned cols = 0 ;
static unsigned rows = 0 ;

//  eventually, of course, we want this to give real information...
//speed 9600 baud; rows 53; columns 90; line = 0;
unsigned get_window_cols(void)
{
   char str[128] ;
   char *fptr ;
   char *sptr ;

   if (cols == 0) {
      FILE *fd = popen("/bin/stty size", "r") ;
      if (fd == 0) 
         goto error_exit;
      fptr = fgets(str, sizeof(str), fd) ;
      pclose(fd) ;

      if (fptr == 0) 
         goto error_exit;
         
      sptr = strchr(fptr, ' ') ;
      if (sptr == 0)
         goto error_exit;
      *sptr++ = 0 ;
      rows = atoi(fptr) ;
      cols = atoi(sptr) ;
   }
error_exit:
   if (rows == 0)
       rows = 50 ;
   if (cols == 0)
       cols = 80 ;
   return cols ;
}

//************************************************************
unsigned get_window_rows(void)
{
   char str[128] ;
   char *fptr ;
   char *sptr ;

   if (rows == 0) {
      FILE *fd = popen("/bin/stty size", "r") ;
      if (fd == 0) 
         goto error_exit;
      fptr = fgets(str, sizeof(str), fd) ;
      pclose(fd) ;

      if (fptr == 0) 
         goto error_exit;
         
      sptr = strchr(fptr, ' ') ;
      if (sptr == 0)
         goto error_exit;
      *sptr++ = 0 ;
      rows = atoi(fptr) ;
      cols = atoi(sptr) ;
   }
error_exit:
   if (rows == 0)
       rows = 50 ;
   if (cols == 0)
       cols = 80 ;
   return rows ;
}

//************************************************************
unsigned is_redirected(void)
{
   return 0;
}

// unsigned _where_x(void)
// {
//    return 0;
// }

unsigned _where_y(void)
{
   return 0;
}

void dreturn(void)
{
   putchar('\r') ;
}

void dclreol(void)
{
   
}

void dgotoxy(unsigned x, unsigned y)
{
   
}  //lint !e715

void dclrscr(void)
{
   system("clear") ;
}

void dnewline(void)
{
   putchar('\n') ;
}

// # Below are the color init strings for the basic file types. A color init
// # string consists of one or more of the following numeric codes:
// # Attribute codes: 
// # 00=none 01=bold 04=underscore 05=blink 07=reverse 08=concealed
// # Text color codes:
// # 30=black 31=red 32=green 33=yellow 34=blue 35=magenta 36=cyan 37=white
// # Background color codes:
// # 40=black 41=red 42=green 43=yellow 44=blue 45=magenta 46=cyan 47=white
static unsigned char const ansi_fg[8] = { 30, 34, 32, 36, 31, 35, 33, 37 } ;
static unsigned char const ansi_bg[8] = { 40, 44, 42, 46, 41, 45, 43, 47 } ;

typedef struct dos_color_map_s {
   unsigned fgnd  : 3 ;
   unsigned bold  : 1 ;
   unsigned bgnd  : 3 ;
   unsigned blink : 1 ;
   unsigned unused : 24 ;
} dos_color_map_t ;

union dos2ansi {
   dos_color_map_t dm ;
   unsigned        raw ;
};

void set_text_attr(unsigned attr)
{
   // char astr[20] ;
   union dos2ansi d2a ;
   unsigned attrib ;

   d2a.raw = attr & 0xFF ;

   attrib = 0 ;
   if (d2a.dm.blink) attrib |= 5 ;
   if (d2a.dm.bold)  attrib |= 1 ;

   printf("\033[%u;%u;%um", attrib, 
   // printf("[%u;%u;%um", attrib, 
      ansi_bg[d2a.dm.bgnd], ansi_fg[d2a.dm.fgnd]) ;
   // if (d2a.dm.bgnd != 0) {
   //    printf("ESC[%u;%u;%um", attrib, 
   //       ansi_bg[d2a.dm.bgnd], ansi_fg[d2a.dm.fgnd]) ;
   //    getchar() ;
   // }
}

void dputc(char outchr)
{
   putchar(outchr) ;
}

void dputs(const char *outstr)
{
   printf("%s", outstr) ;
}

void dprints(unsigned y, unsigned x, char *str)
{
   printf("%s", str) ;
}  //lint !e715

void dputnchar (char chr, unsigned attr, unsigned count)
{
   char ostr[260] ;
   set_text_attr(attr) ;
   if (count > 250)
       count = 250 ;
      
   memset(ostr, chr, count) ;
   ostr[count] = 0 ;
   dputs(ostr) ;
}

