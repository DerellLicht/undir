//****************************************************************
//  build a list of directories, not necessarily related to
//  each other, with optional list of files in each directory.
//****************************************************************

// #include <limits.h>  //  PATH_MAX

//****************************************************************
//  internal structs
//****************************************************************

typedef struct file_entry_s {
   struct file_entry_s *next ;
   char name[PATH_MAX] ;
} file_entry_t, *file_entry_p ;

typedef struct dir_entry_s {
   struct dir_entry_s *next ;
   file_entry_p   ftop ;
   file_entry_p   ftail ;
   char dfpath[PATH_MAX] ;
   unsigned fcount ;
} dir_entry_t, *dir_entry_p ;

//***********************************************************************
class dirtree {
private:
   dir_entry_p dirtop ;
   dir_entry_p dirtail ;
   dir_entry_p iter ;
   file_entry_p fiter ;
   char cwdir[PATH_MAX] ;
   char *curpath ;
   unsigned dircount ;
   char *get_entry(void);
   char *get_file_entry(void);
   void expand_path(char *inpath, char *outpath);

public:
   dirtree(void) ;
   ~dirtree(void);
   int add_file_entry(char *dfentry) ;
   unsigned get_dir_count(void) ;
   void show_dirlist(void) ;  //  debug
   char *get_first_entry(void) ;
   char *get_next_entry(void) ;
   unsigned has_files(void) ;
   char *get_first_file(void);
   char *get_next_file(void);
} ;

