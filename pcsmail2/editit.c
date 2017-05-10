/* editit */


#define	CF_DEBUG	0
#define	F_SYSTEM3	0


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *									*


 * editit() does the necessary footwork for the edit command


*************************************************************************/



#include	<sys/types.h>
#include	<sys/wait.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"header.h"
#include	"defs.h"




/*PAS-JM 2/8/85*//*begin*/
/*	Don't check for editor type.  Lots of users have their own names
 *	for editors.  Many might accept two file names.
 *	Therefore use whatever editor the user has in ED (unless it's
 *	a null value).  Use option "original_edit" to decide whether to
 *	pass both file names to the editor.
 *						- PAS
 */


/* external subroutines */

extern pid_t	u_waitpid() ;

extern int	table_has_changed() ;

extern char	*getenv() ;


/* external variables */

extern struct global	g ;


/* local static variables */




void editit(editfname)
char	editfname[] ;
{
	bfile	*fpa[3] ;

	pid_t	pid ;

	int	child_stat ;
	int	i ;

	char	hv_from[HVLEN + 1] ;
	char	buf[BUFLEN + 1] ;


#if	CF_DEBUG && 0
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"editit: called w/ subject \"%s\"\n",subject) ;
#endif

/* Check for null	value in ED - PAS */

	bprintf(g.ofp,"editing message ...\n") ;

#if	CF_DEBUG
	if (g.f.debug) {

		logfile_printf(&g.eh,"editit: editor=%s\n",g.prog_editor) ;

	}
#endif

	sprintf(syscom,"%s %s %s",
	    g.prog_editor,editfname, (orig_edit && isoriginal) ? origfile : "") ;

#if	CF_DEBUG
	logfile_printf(&g.lh,"edit_cmd=\"%s\"\n",syscom) ;
#endif

#if	F_SYSTEM3
	system(syscom) ;
#else
	for (i = 0 ; i < 3 ; i += 1) fpa[i] = NULL ;

	pid = bopencmd(fpa,syscom) ;

	if (pid > 0) {

		logfile_printf(&g.eh,"editit: waiting for child pid=%d\n",
			pid) ;

		u_waitpid(pid,&child_stat,0) ;

		logfile_printf(&g.eh,"editit: child returned\n") ;

	} else {

		logfile_printf(&g.lh,"could not execute user's editor\n") ;

		bprintf(g.efp,
			"%s: could not execute user's editor program\n",
			g.progname) ;

	}
#endif

#if	CF_DEBUG && 0
		sprintf(buf,"cp %s /home/dam/rje/edit1.out",editfname) ;

		system(buf) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	fileinfo(editfname,"editit") ;
#endif

/* scan message for changes */

	getfield(editfname,HS_TO,recipient) ;

	getfield(editfname,HS_CC,copyto) ;

	getfield(editfname,HS_BCC,bcopyto) ;

	getfield(editfname,HS_FROM,hv_from) ;

	getfield(editfname,HS_SUBJECT,subject) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"editit: subject \"%s\"\n",subject) ;
#endif

/* parse and rewrite the address in place */

	tonames = getnames(recipient) ;

	ccnames = getnames(copyto) ;

	bccnames = getnames(bcopyto) ;

	isedit = 0 ;
	if (strcmp(from,hv_from) != 0) {

	    if (*sentby == '\0') strcpy(sentby, from) ;

	    strcpy(from, hv_from) ;

	}

	*realto = '\0' ;

/* re-read translation tables if necessary */

	if (table_has_changed()) regettable() ;

}
/* end subroutine (editit) */


