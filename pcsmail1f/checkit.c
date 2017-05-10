/* checkit */


#define	CF_DEBUG	0


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
		David A.D. Morano
 *									*
*
 * checkit() does the footwork for the "check" command.


*************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"header.h"



/* external variables */

extern struct global	g ;




void checkit()
{

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: called and about to do a 'getfield TO'\n") ;
#endif

	getfield(tempfile,HS_TO,recipient);

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: recipient %s\n",recipient) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: about to 'getnames(recipient)'\n") ;
#endif

	tonames = getnames(recipient);

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: about to 'getfield CC'\n") ;
#endif

	getfield(tempfile,HS_CC,copyto);

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: about to 'getnames(copyto)'\n") ;
#endif

	ccnames = getnames(copyto);

#ifdef	COMMENT
	if (ccnames == 0) {

		getfield(tempfile,HS_CC,copyto);

		ccnames = getnames(copyto);

	}
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: about to 'getfield BCC'\n") ;
#endif

	getfield(tempfile,HS_BCC,bcopyto);

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: about to 'getnames(bcopyto)'\n") ;
#endif

	bccnames = getnames(bcopyto);

/* Check TO:, BCC:, CC: list */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	logfile_printf(&g.eh,"checkit: about to call 'checkreclist'\n") ;
#endif

	checkreclist(1) ;

} 
/* end wubroutine */


