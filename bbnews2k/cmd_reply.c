/* cmd_reply */

/* lets the user type a reply to the specified message */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1994-01-01, Jishnu Mukerji
	Originally written.

	= 1994-02-01, David A­D­ Morano
        I wrote this from scratch when I took over the code. The previous code
        was a mess (still is in many places!).

	= 1998-11-22, David A­D­ Morano
        I wrote this from scratch when I took over the code. The previous code

*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 			Jishnu Mukerji
 *                      David A.D. Morano


	This subroutine was modified several times by J.Mukerji between
	4.1.84 and 8.1.84 to do the following :
 *									   
 * (i)	Interface with PCSMAIL using a file rather than the command line.   
 * (ii)	Pass a copy of the original message to PCSMAIL so that it is avail- 
 *	able for displaying in the +edit mode				   
 *	This is done only if the editor specified in the environment	   
 *	variable ED can make good use of it.				   


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdio.h>

#include	<localmisc.h>

#include	"artlist.h"
#include	"headerkeys.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define NOTFOUND	2
#define	CMDLEN		((2 * BUFSIZE) + 12)


/* external variables */

struct proginfo		g ;


/* exported subroutines */


int cmd_reply(pip,ap,ngdir,afname)
struct proginfo	*pip ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	afname[] ;
{
	FILE	*tfd ;

	int	rs = SR_OK ;
	int	titflag = 0 ;
	int	ret = 0 ;
	int	l1, l2 ;

	char	tempfile[BUFSIZE + 1] ;
	char	to[BUFSIZE + 1], subj[BUFSIZE + 1], oldsubj[BUFSIZE + 1] ;
	char	ccto[BUFSIZE + 1], recipto[BUFSIZE + 1] ;
	char	messid[BUFSIZE + 1], reference[BUFSIZE + 1] ;
	char	command[CMDLEN + 1] ;
	char	*cp, *cp1, *cp2, *cp3 ;
	char	*tmp1, *tmp2 ;
	char	*toname, *name, *last, *subjpt, *spt, *comment ;


	comment = NULL ;

/* get recipient (= old from).  select out only the last 
	   name to insure proper translation . */

	to[0] = '\0' ;
	ret = getfield(afname,HK_REPLYTO,to) ;

	if (to[0] == '\0')
	    ret = getfield(afname,HK_FROM,to) ;

	getfield(afname,HK_MESSAGEID,messid) ;

	getfield(afname,HK_REFERENCES,reference) ;

	if (ret == 1) 
		goto ret0 ;

	if (ret == NOTFOUND) {

	    bprintf(pip->ofp,
	        "can't find who the article was from -- aborting\n") ;

	    goto ret0 ;
	}

/* get rid of leading spaces */

	for (toname = to; *toname == ' '; toname++) ;

/* We have to deal with the new SMTP from line which is of the form
   From: some string <address>
   here.
*/

	if (strchr( toname, '<' ) != NULL ) {

/* This is an SMTP mail header, so do the right thing for it */

	    comment = strtok( toname, "<") ;
	    last = strtok( 0, ">") ;

	} else {

/* FROM: address (comment string) */

	    if (strchr(toname,')') != NULL) {

	        last = strtok( toname, "(" ) ;

	        comment = strtok( 0, ")" ) ;

	    } else
	        last = strtok(toname," ") ;

	}

/* get rid of trailing blanks */

	for (name = &to[strlen(to)-1]; *name == ' '; *name-- = '\0') ;

/* strip last of leading and trailing blanks */

	for ( ; *last == ' '; last++ ) ;

	for (name = last; *name; *name++) ;

	for (name-- ; *name == ' '; name-- ) ;

	*(++name) = '\0' ;

/* get subject */

	oldsubj[0] = '\0' ;
	getfield(afname,"subject",oldsubj) ;

	if (oldsubj[0] == '\0')
	    getfield(afname,"TITLE:",oldsubj) ;

	tmp1 = oldsubj+strspn(oldsubj," ") ;

	if (strncmp ("re:", tmp1 ,3) != 0) {
	    strcpy (subj,"re: ") ;

	} else
	    strcpy (subj,"");		/* already has re: */

	strcat(subj, tmp1) ;

	strcpy(tempfile,"/tmp/replyXXXXXX") ;

	mktemp(tempfile) ;

	if ((tfd = fopen( tempfile, "w" )) == NULL) {

	    bprintf(pip->ofp,
	        "unable to create reply message !\n") ;

	    goto ret0 ;
	}

	chown(tempfile, getuid(), getgid()) ;

	chmod(tempfile, 0640 ) ;

/* put reply message template in reply file		*/

	if (comment != NULL ) {
	    fprintf(tfd,"TO: %s <%s>\n",comment,last) ;

	} else
	    fprintf(tfd,"TO: %s\n",last) ;

	if (*subj) 
		fprintf(tfd,"SUBJECT: %s\n",subj) ;

	if ((*reference != NULL) || (*messid != NULL))
	    fprintf(tfd,"REFERENCES: %s %s\n",reference, messid) ;

	fclose(tfd) ;

	l1 = sfdirname(pip->prog_mailer,-1,&cp1) ;

	l2 = sfbasename(pip->prog_mailer,-1,&cp2) ;

	cp3 = "" ;
	if ((l2 > 1) && (*cp2 != 'o') && (*cp2 != 'n')) {

	    if ((pip->prefix != NULL) && (pip->prefix[0] != '\0')) {

	        if (pip->f.newprogram) 
			cp3 = "n" ;

	    }
	}

	bufprintf(command,CMDLEN,"%t%s%s%t -verify f=%s re=",
	    ((l1 > 0) ? cp1 : ""),l1,
	    ((l1 > 0) ? "/" : ""),
	    cp3,
	    cp2,l2,
	    tempfile) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

		debugprintf("reply: CMD> %s\n",command) ;

		sleep(5) ;

	}
#endif /* CF_DEBUG */

	system(command) ;

	unlink(tempfile) ;

ret0:
	return rs ;
}
/* end subroutine (cmd_reply) */



