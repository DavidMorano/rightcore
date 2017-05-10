/* fetchfrom */

/* fetch the envelope "from" header */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-01-06, David A­D­ Morano
	I modified from original to better parse the "from" 

	= 1996-06-18, David A­D­ Morano
        I finally added Walter Pitio's algorithm for displaying line numbers
        larger than 999.

*/

/* Copyright © 1994,1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will look for "From " or ">From " (for remote
	mail).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<mailmsg.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"
#include	"ds.h"


/* local defines */

#define	FROMWIDTH	18

#ifdef	NULL
#define	N	NULL
#else
#define	N	((char *) 0)
#endif


/* external subroutines */

extern int	getgecos() ;
extern int	white(const char *) ;

extern char	*strshrink(char *) ;


/* external variables */

extern struct mailbox	mb ;


/* forward references */


/* glabol variables */


/* local variables */


/* exported subroutines */


int fetchfrom(pip,mn,str,len)
struct proginfo	*pip ;
int		mn, len ;
char		str[] ;
{
	int	i, sl ;

	char	temp[LINEBUFLEN + 1], last[LINEBUFLEN + 1] ;
	char	sender[LINEBUFLEN + 1] ;
	char	*word, *c ;


	if (str == NULL)
		return SR_FAULT ;

	str[0] = '\0' ;

/* get the sender's name */

	if ((mn < 0) || (mn >= mb.total)) 
		return SR_INVALID ;

	if (curr.fp == NULL)
		return SR_FAULT ;

	fseek(curr.fp,messbeg[mn],0) ;

	sl = freadline(curr.fp,temp,LINEBUFLEN) ;

	temp[sl] = '\0' ;
	strcpy(last,temp) ;

/* find last "From" line (original sender) */

	while (ftell(curr.fp) < messend[mn]) {

	    sl = freadline(curr.fp,temp,LINEBUFLEN) ;

	    temp[sl] = '\0' ;
	    if ((strncmp(temp,"From ",5) != 0)  &&
	        (strncmp(temp,">From ",6) != 0)) 
		break ;

	    strcpy(last,temp) ;

	} /* end if */

	strtok(last," \t\n") ;	/* the "From" */

	c = strtok(0, " \t\n") ;

	if (c == NULL)
	    c = "" ;

	strcpy(str, c) ;	/* the sender login name */

/* find remote machine name if there is one */

	while ((word = strtok(0," \t\n")) != NULL) {

	    if (strcmp(word,"from") == 0) {	/* found "remote from" */

	        strcpy(sender,str) ;

	        c = strtok(0," \n") ;

	        if (c == NULL) c = "" ;

	        strcpy(str, c) ;      /* machine */

	        strcat(str, "!") ;

	        strcat(str, sender) ;

	        break ;
	    }

	} /* end while */

#ifdef	COMMENT
/* pad to proper field length */

	for (i = strlen(str) ; i < len ; i += 1)  
		str[i] = ' ' ;
#endif

	str[len] = '\0' ;
	return SR_OK ;
}
/* end subroutine (fetchfrom) */



