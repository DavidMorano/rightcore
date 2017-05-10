/* defs */

/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz, Jishnu Mukerji                    
			David A.D. Morano
 *									

 ***********************************************************************/


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include <stdio.h>
#include <string.h>

#include	<bfile.h>
#include	<logfile.h>


/* this is the global header file which is included into every 
  source file.   the declarations here are defined in "defs.c".
*/


#define A_READ	04		/* for access()		*/


struct global {
	bfile		*efp ;
	logfile	lh ;
	char		*progname ;
	char		*progroot ;
	char		*prog_mailer ;
} ;



/* maximum size of mailbox */

#define  MAXMESS  	1000
#define  MAXLINES  	maxlines
#define	 DEF_MAXLINES	24
#define  LINELEN   	500


struct current {
	char mailbox[100];	/* name of current mailbox */
	FILE *fp;		/* ioptr to its actual file */
	int msgno;		/* current message pointer */
} ;

#ifndef DEFS

  /* message structure for random access into current mailbox */

   extern long  messbeg [];	/* byte ptrs to beginnings of messages */
   extern long  messend [];	/* byte ptrs to endings of messages */
   extern int   messord []; 	/* message ordering (presort may reorder) */
   extern int   nummess;	/* number of messages */
   extern int	maxlines;	/* number of lines on screen */

 /* intermediate state during reading within a mailbox */
   extern int   messdel[];	/* 1 marks a to-be-deleted message, else 0 */


 /* pointers to state of current invocation */
   extern struct current  curr;


 /* global variables holding tokens for the logical expression search */
   extern int   isop [];		/* 1 if token is operator, else 0 */
   extern int   etoken [];		/* operator num  or  header num */
   extern char  *hvalpt [];        /*ptr to value string of header in etoken */
   extern char  hvalues [];        /* string space for value strings */
   extern int   numletok;		/* number of tokens */


 /* logical expression tables */
   extern char operator [];

   extern char  *header [];


 /* global flag for searching.  
   0 = no (suitable) match yet, 1 = first match, 2 = past first match .
 */
   extern int  firstmatch;

/* global markers for last message matched by range or logical expression.
  value is corresponding external messnum.
*/
   extern int  lastmatch;
   extern int  lastundeleted;
#endif

 /* non-integer function returns */
   char * maildir ();           /* returns mailbox directory (in boxname.c) */

#define  USERNAME   (cuserid(0))         /* login id of current user */

#define  COLS       80			 /* width of line */


#endif /* DEFS_INCLUDE */


