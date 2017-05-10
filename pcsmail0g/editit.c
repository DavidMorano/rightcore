/* editit */


#define	DEBUG	0


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
 *									*


 * editit() does the necessary footwork for the edit command


*************************************************************************/



#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<bfile.h>

#include	"config.h"
#include	"header.h"
#include	"smail.h"




/*PAS-JM 2/8/85*//*begin*/
/*	Don't check for editor type.  Lots of users have their own names
 *	for editors.  Many might accept two file names.
 *	Therefore use whatever editor the user has in ED (unless it's
 *	a null value).  Use option "original_edit" to decide whether to
 *	pass both file names to the editor.
 *						- PAS
 */


/* external subroutines */

extern int	table_has_changed() ;

extern char	*getenv() ;


/* external variables */

extern struct global	g ;


/* local static variables */

static char	*editor ;	/* pointer for name of editor	PAS	*/




editit()
{
	bfile	*fpa[3] ;

	pid_t	pid ;

	int	child_stat, i ;


#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	errprintf("editit: called w/ subject \"%s\"\n",subject) ;
#endif

/* Check for null	value in ED - PAS */

	if (((editor = getenv("ED")) != NULL) && (*editor != '\0'))
	    strcpy(syscom,editor) ;

	else if (((editor = getenv("EDITOR")) != NULL) && (*editor != '\0'))
	    strcpy(syscom,editor) ;

	else 
	    strcpy(syscom, "ed") ;

	printf("editing message ...\n") ;

	sprintf(syscom + strlen(syscom)," %s %s",
	    tempfile, (orig_edit && isoriginal) ? origfile : "") ;

/*PAS-JM 2/8/85*//*end*/

#ifdef	COMMENT
	system(syscom) ;
#else
	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	pid = bopencmd(fpa,syscom) ;

	if (pid > 0) {

		errprintf("editit: waiting for child \n") ;

		waitpid(pid,&child_stat,0) ;

		errprintf("editit: child returned\n") ;

	} else {

		logfile_printf("could not execute user's editor\n") ;

		bprintf(g.efp,
			"%s: could not execute user's editor program\n",
			g.progname) ;

	}
#endif

/* scan message for changes */

	getfield(tempfile,HS_TO,recipient) ;

	getfield(tempfile,HS_CC,copyto) ;

	getfield(tempfile,HS_BCC,bcopyto) ;

	getfield(tempfile,HS_FROM,syscom) ;

	getfield(tempfile,HS_SUBJECT,subject) ;

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	errprintf("editit: subject \"%s\"\n",subject) ;
#endif

/* parse and rewrite the address in place */

	tonames = getnames(recipient) ;

	ccnames = getnames(copyto) ;

	bccnames = getnames(bcopyto) ;

	isedit = 0 ;
	if (strcmp(from,syscom) != 0) {

	    if (*sentby == '\0') strcpy(sentby, from) ;

	    strcpy(from, syscom ) ;

	}

	*realto = '\0' ;

/* re-read translation tables if necessary */

	if (table_has_changed()) regettable() ;

}
/* end subroutine (editit) */


