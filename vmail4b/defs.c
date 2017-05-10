/* defs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Definitions of global variables used by many routines. the external
        declarations are contained in the file "config.h" which is included into
        every source file.


*******************************************************************************/


#define	DEFS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"


/* global data */

struct proginfo		g ;

struct mailbox		mb ;


/*  structure for random access into current mailbox */

offset_t	messbeg[MAXMESS+1] ;	/* byte ptrs to beginnings of messages */
offset_t	messend[MAXMESS+1] ;	/* byte ptrs to endings of messages */

int   messord [MAXMESS+1] ;	/* message ordering (presort may reorder) */
int   messlen [MAXMESS+1] ;	/* message length in lines */
int   mlength[MAXMESS+1] ;	/* message length in bytes */
int   nummess ;			/* number of messages */


/* intermediate state during reading within a mailbox */

int   messdel[MAXMESS+1] ;	/* 1 marks a to-be-deleted , else 0 */


/* pointers to state of current invocation */

struct current  curr ;


/* global variables holding screen parameters */

int cursline ;			/* line of --> cursor */
int firstmsg ;			/* first msg displayed */


/* global holding file pointers to pages */

long pages[MAXPAGE] ;

/* global variables holding tokens for the logical expression search */

int   isop [30] ;		/* 1 if token is operator, else 0 */
int   etoken [30] ;		/* operator num  or   num */
char  *hvalpt [30] ;		/* ptr to value string of  in etoken */
char  hvalues [400] ;	/* string space for value strings */
int   numletok ;		/* number of tokens */


/* logical expression tables */

char	operator[] = "&|()" ;

char  *header[] = {
	"FROM:",
	"TO:",
	"DATE:",
	"SUBJECT:",
	"KEYWORDS:",
	"",
} ;


/* global search string */

char searchstr[100] = { '\0'} ;



