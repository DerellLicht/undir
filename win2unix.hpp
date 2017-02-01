#ifndef _WIN2UNIX_HPP
#define _WIN2UNIX_HPP

#define  _strcmpi strcasecmp

// typedef  char  TCHAR ;

// #define  O_BINARY    0

// typedef unsigned  DWORD ;

#ifndef _MAX_PATH
#define _MAX_PATH  260
#endif

// typedef struct _WIN32_FIND_DATA { // wfd 
//     DWORD dwFileAttributes; 
//     unsigned ftCreationTime; 
//     unsigned ftLastAccessTime; 
//     unsigned ftLastWriteTime; 
//     DWORD    nFileSizeHigh; 
//     DWORD    nFileSizeLow; 
//     DWORD    dwReserved0; 
//     DWORD    dwReserved1; 
//     TCHAR    cFileName[_MAX_PATH]; 
//     TCHAR    cAlternateFileName[14]; 
// } WIN32_FIND_DATA; 

unsigned get_window_cols(void);
unsigned get_window_rows(void);
unsigned is_redirected(void);
// unsigned _where_x(void);
unsigned _where_y(void);
void dreturn(void);
void dclreol(void);
void dnewline(void);
void dgotoxy(unsigned x, unsigned y);
void dclrscr(void);
void set_text_attr(unsigned attr);
void dputc(char outchr);
void dputnchar (char chr, unsigned attr, unsigned count);
void dputs(const char *outstr);
void dprints(unsigned y, unsigned x, char *str);

#endif
