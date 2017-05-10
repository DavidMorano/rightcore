/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz                                    *
 *									*
 ***********************************************************************/
/* definitions of global variables used by many routines.
  the external declarations are contained in the header file "defs.h"
  which is included into every source file.
*/
#define DEFS
#include "defs.h"

  /* message structure for random access into current mailbox */
   long  messbeg [MAXMESS+1];	/* byte ptrs to beginnings of messages */
   long  messend [MAXMESS+1];	/* byte ptrs to endings of messages */
   int   messord [MAXMESS+1]; 	/* message ordering (presort may reorder) */
   int   nummess;		/* number of messages */


 /* intermediate state during reading within a mailbox */
   int   messdel[MAXMESS+1];	/* 1 marks a to-be-deleted message, else 0 */


 /* pointers to state of current invocation */
   struct current  curr;


 /* global variables holding tokens for the logical expression search */
   int   isop [30];		/* 1 if token is operator, else 0 */
   int   etoken [30];		/* operator num  or  header num */
   char  *hvalpt [30];		/* ptr to value string of header in etoken */
   char  hvalues [400]; 	/* string space for value strings */
   int   numletok;		/* number of tokens */

   int	maxlines = DEF_MAXLINES;	/* size of the screen */

 /* logical expression tables */
   char operator [] =  "&|()" ;

char  *header[] = {
      	       "FROM:",
      	       "TO:",
      	       "DATE:",
      	       "SUBJECT:",
      	       "KEYWORDS:",
	       "REFERENCES:",
	       "MESSAGE-ID:",
	       "SENDER:",
   	       ""
} ;


 /* global flag for searching.  
   0 = no (suitable) match yet, 1 = first match, 2 = past first match .
 */
   int  firstmatch;

/* global markers for last message matched by range or logical expression.
  value is corresponding external messnum.
*/
   int  lastmatch;
   int  lastundeleted;



