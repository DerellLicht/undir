//*****************************************************************
//                                                                 
//  CMD_LINE.CPP - NDIR command_line parsing                       
//                                                                 
//  Written by:   Daniel D. Miller  (the Derelict)                 
//                                                                 
//  Last update:  01/16/02 16:18                                   
//                                                                 
//  compile with makefile                                          
//                                                                 
//*****************************************************************

#include <stdio.h>   //  debug only 
#include <string.h>
#include <limits.h>
#include <stdlib.h>
// #include <ctype.h>

#ifdef _lint
unsigned strlen(const char *STR);
#endif

#include "ndata.hpp"

#include "dirtree.hpp"
extern dirtree dlist ;

static unsigned multimedia_listing = 0;

//*************************************************************
//  Convert command-line switches into flags
//*************************************************************
static int update_switches (char *argstr)
{
   int slen = 1;
	char inchar = *argstr++;

	switch (inchar) {
		case 'a':
			n.show_all ^= 1;
			break;
		case 'b':
			n.batch ^= 1;
			break;
		case 'c':
			n.clear ^= 1;
			break;
		case 'd':
			if (*argstr == '2') {
				n.tree = 4;
				argstr++;
			}
			else if (*argstr == '3') {
				n.tree = 5;
				argstr++;
			}
			else {
				n.tree = 1;
			}
			break;
		case 'e':
			n.sort = 0;
			break;
		case 'f':
			n.tree = 2;
			break;
		case 'g':
			n.dir_first ^= 1;
			break;
		case 'h':
			n.horz ^= 1;
			break;
      case 'i':  n.drive_summary ^= 1;  break;
		case 'j':
			n.low_ascii ^= 1;
			break;
		case 'k':
			n.color ^= 1;
			break;					  //  redirection flag
		case 'l':
			n.lfn_off ^= 1;
			break;					  //  toggle long_filename flag
		case 'm':
			//  look for 'mm' switch
			if (*argstr == 'm') {
				multimedia_listing ^= 1;
            slen++ ;
			}
			else {
				n.minimize ^= 1;
			}
			break;
		case 'n':
			n.sort = 1;
			break;

		case 'o':
			if (*argstr == '1')
				n.fdate_option = FDATE_LAST_ACCESS;
			else if (*argstr == '2')
				n.fdate_option = FDATE_CREATE_TIME;
			else
				n.fdate_option = FDATE_LAST_WRITE;
			slen = 2;
			break;
		case 'p':
			n.pause ^= 1;
			break;
		case 'q':
			n.horz ^= 2;
			break;
		case 'r':
			n.reverse ^= 1;
			break;
		case 's':
			n.sort = 2;
			break;
		case 't':
			n.sort = 3;
			break;
		case 'u':
			n.ucase ^= 1;
			break;
		case 'v':
			n.info = 1;
			break;
		case 'w':
			n.showSHRfiles ^= 1;
			break;
		case 'x':
			n.exec_only ^= 1;
			break;
			// case 'y':
		case 'z':
			n.sort = 4;
			break;

		case '1':
			n.format = 0;
			break;
		case '2':
			n.format = 1;
			break;
		case '4':
			n.format = 2;
			break;
		case '6':
			n.format = 3;
			break;
		case '5':
			n.ega_keep ^= 1;
			break;

		case ',':
			n.tree_short ^= 1;
			break;

		case '?':
// nputs(0x13, "I see question mark") ;
// ncrlf() ;
// _getch() ;         
			n.help = 1;
			break;

		default:
			break;					  //  make lint happy
	}									  /* end SWITCH      */
   return slen;   //lint !e438 Last value assigned to variable 'argstr' not used
}

//**********************************************************
void parse_command_string (char *cmdstr)
{
	char *extptr;
   char *fptr ;
	int slen;
   char real_path[1024] ;

	switch (*cmdstr) {
		case '-':
      // case '/':   //  can't use this in *nix !!
			cmdstr++;				  //  skip the switch char
			while (1) {
				if (*cmdstr == 13 || *cmdstr == 0)
					break;

				slen = update_switches (cmdstr);
				cmdstr += slen;
			}
			break;

		case '[':
			strcpy (leftstr, ++cmdstr);
			break;

		case ']':
			strcpy (rightstr, ++cmdstr);
			break;

		case '!':
			strcpy (tempstr, ++cmdstr);

			//  process exclusion extentions...
			extptr = strrchr (tempstr, '.');
			if (extptr != 0 && strlen (extptr) <= 4) {
				strcpy (excl[exclcount], extptr);
				exclcount++;
			}
			break;

		default:
         //*************************************************************************
         //  filename handling is a little tricky under linux...
         //  if "." or ".." are present, they may have been explicitly
         //  entered by the user, or they may have been collected by
         //  the shell in response to a directory scan.
         //  In the latter case, we don't want to actually process the entries, 
         //  but in the former case, we don't want to throw them out!!
         //*************************************************************************
         //  10/06/10 11:28 Note: 
         //  Well, I don't really understand the preceding discussion,
         //  but the method we were using before was failing on ../..
         //  so I'm going back to realpath() for now.
         //*************************************************************************
         // printf ("in:%s\n", cmdstr) ;
         if (*cmdstr == '.') {
            realpath(cmdstr, real_path) ;
            // printf("in:  [%s]\n", cmdstr) ;
            // printf("out: [%s]\n", real_path) ;
            // getchar() ;
            cmdstr = real_path ;
         }

         fptr = strrchr(cmdstr, '/') ;
         if (fptr != 0) {
            fptr++ ; // skip past the slash
            if (strcmp(fptr, ".") == 0  ||  strcmp(fptr, "..") == 0) 
               break;
         }

			// cmdstr = copy_tempstr(cmdstr, tempstr) ;
         dlist.add_file_entry(cmdstr) ;
         // insert_target_filespec (tempstr);
			break;
   }  //  switch (*cmdstr)
}

//**********************************************************
void parse_command_args (int startIdx, int argc, char **argv)
{
	char *argvptr, *spcptr;

	//  see if we have an environment variable to deal with
	if (startIdx == 0) {
		argvptr = argv[0];
		while (1) {
			spcptr = strchr (argvptr, ' ');
			if (spcptr == 0) {
				parse_command_string (argvptr);
				break;
			}

			//  fall thru here if space is found
			*spcptr++ = 0;			  //  NULL-term first string, point to next string
			parse_command_string (argvptr);
			argvptr = spcptr;
		}
		startIdx = 1;
	}

	//  deal with normal command-line arguments
	for (int j = startIdx; j < argc; j++) {
		argvptr = argv[j];

      //  fix an obscure bug under win32 and 4DOS7;
      //  for some reason, "/?" is getting changed to "/~" ...
      //  Not needed in undir ??
      // if (strcmp(argvptr, "/~") == 0) {
      //    *(argvptr+1) = '/' ;
      // }
      // printf("%u: %s\n", j, argvptr) ;
      parse_command_string (argvptr);
	}
   // getchar() ;
}

//************************************************************
//  Spontaneous Assembly library video types
//************************************************************
void verify_flags (void)
{
	// lines = (unsigned) (int) (sinfo.srWindow.Bottom - sinfo.srWindow.Top + 1) ;
	lines = get_window_rows ();

	//  used on exit, to see if we should restore 25-line mode
   // curlines = lines;

//   if (is_redirected ()) {
//       //  This bypasses the html-redirection operation,
//       //  which doesn't work with Cygwin/Bash shells
//       n.color = 0 ;
//       // redirected = 0 ;
//       // n.minimize = 0;
//       // n.pause = 0;
//       // n.low_ascii = 1;
//   }

   //  
   if (multimedia_listing) {
      n.format = 0 ;
   }

	//****************************************************
	//  For batch mode, standard ASCII, or help/identify
	//  requests, force "no color" mode.
	//****************************************************
	// if (
	//     n.low_ascii 
	//     || n.batch 
	//     || n.help 
	//     || n.info
	//     )
	//    {
	//    n.color = 0 ;
	//    }

	/************************************************/
	/*  Test for, and correct, inconsistent flags.  */

	/************************************************/
   if (n.tree == 1 || n.tree == 4 || n.tree == 5  ||  n.drive_summary)
		n.exec_only = 0;

	//******************************************************
	//  at this point, if exec_only is set, stuff the       
	//  executable filespec into target[]                   
	//******************************************************
	//  The drawback to this technique is that the user     
	//  cannot apply -x to multiple drives simultaneously.  
   //******************************************************
   //  Linux note:  this option probably doesn't 
   //  mean much anyway... disable for now
	//******************************************************
   // if (n.exec_only) {
   //   insert_target_filespec ("*.exe");
   //   insert_target_filespec ("*.com");
   //   insert_target_filespec ("*.bat");
   //   insert_target_filespec ("*.btm");
   // }

	//*********************************************
	// If -3 or -5 was used, set 43/50-line mode
	//*********************************************
   // if (n.ega_keep) {
   //   set_lines (50);
   //   curlines = lines;
   //   lines = 50;
   // }

	/*  Set 'dir tree' conditions  */
   if (n.tree == 1 || n.tree == 4 || n.tree == 5)
		n.minimize = 0;

	/* If not 'find all'  then don't use attr bits = 0x27  */
	// findattr = IFF (n.show_all)  THENN 0xF7  ELSSE 0x10 ;

	if (n.horz & 2) {
		n.horz &= ~1;
		n.reverse = 0;
		n.sort = 0;
	}

	//****************************************************
	//  fix up the format specifiers
	//****************************************************
	if (n.tree == 1 || n.tree == 4) {
		columns = 0;
	}
	else if (n.horz == 2) {
		columns = 1;
		n.sort = 0;					  //  force sort-by-extension
	}
	else {
		switch (n.format) {
			case 0:
				columns = 1;
				break;
			case 1:
				columns = 2;
				break;
			case 2:
				columns = 4;
				break;
			case 3:
				columns = 6;
				break;
			case 5:
				columns = 3;
				break;
			default:
				n.format = 2;
				columns = 4;
				break;
		}
	}

	//*************************************
	//  Initialize line-drawing variables
	//*************************************
	if (n.low_ascii) {
		tline = bline = xline = crosschar;
		vline = altvlinechar;
		dline = altdvlchar;
	}
	else {
		tline = topchar;
		bline = bottomchar;
		vline = vlinechar;
		xline = xlinechar;
		dline = dvlinechar;
	}
}
