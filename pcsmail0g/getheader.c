/* getheader */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
		David A.D. Morano

 
*
 * getheader( file )
 *
 * getheader collects the header fields from the named file
 * and puts them in corresponding strings in the global data space


*************************************************************************/



#include	<sys/types.h>
#include	<sys/unistd.h>
#include	<sys/stat.h>
#include	<sys/errno.h>
#include	<sys/param.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"header.h"



/* external subroutines */

extern int	mm_getfield() ;


/* external variables */

#if	CF_DEBUG
extern struct global	g ;
#endif


/* local data */

static char	headbuf[BUFSIZE] ;

struct hentry {
	int	type ;		/* type of header */
	char	*buf ;		/* buffer to hold value */
	int	blen ;		/* length of buffer */
} ;

static struct hentry	hinfo[] = {
	{ MH_TO,	headbuf,	(BUFSIZE - 2) },
	{ MH_SUBJECT,	subject,	(BUFSIZE - 2) },
	{ MH_FROM,	headbuf,	(BUFSIZE - 2) },
	{ MH_SENDER,	headbuf,	(BUFSIZE - 2) },
	{ MH_CC,	copyto,		(BUFSIZE - 2) },
	{ MH_BCC,	bcopyto,	(BUFSIZE - 2) },
	{ MH_KEYWORDS,	keys,		(BUFSIZE - 2) },
	{ MH_REFERENCE,	reference,	(BUFSIZE - 2) },
	{ MH_DATE,	headbuf,	(100 - 2) },
	{ MH_TITLE,	subject,	(BUFSIZE - 2) },
	{ -1,		NULL,		0 }
} ;



int getheader(file, from)
char	*file, *from ;
{
	struct ustat	stat_mm ;

	bfile	mmfile, *fp = &mmfile ;

	int	i, l, ml ;
	int	fd ;


#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"getheader: how are we doing ? \"%s\"\n",file) ;
#endif

/* force some header fields to always be updated if there is data available */

	from[0] = '\0' ;	/* because if was passed down ?? */
	headbuf[0] = '\0' ;	/* because it is shared w/ MH_TITLE */

/* open file for reading */

	if (bopen(fp,file,"r",0666) < 0) return BAD ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"getheader: OK 1\n") ;
#endif

	if (bcontrol(fp,BC_STAT,&stat_mm) < 0) goto badret ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"getheader: OK 3\n") ;
#endif

	for (i = 0 ; hinfo[i].type >= 0 ; i += 1) {

	    if ((*hinfo[i].buf == '\0') &&
	        ((l = mm_getfield(fp,(offset_t) 0,stat_mm.st_size,
	        headers[hinfo[i].type],hinfo[i].buf,hinfo[i].blen)) >= 0)) {

#if	CF_DEBUG
	        if (g.debuglevel > 0)
	            logfile_printf(&g.eh,"getheader: got a header \"%s: %s\"\n",
	                headers[hinfo[i].type],
	                hinfo[i].buf) ;
#endif

	        switch (hinfo[i].type) {

	        case MH_TO:
	            strcpy(realto,headbuf) ;

	            if (*recipient == '\0') strcpy(recipient,headbuf) ;

	            break ;

	        case MH_FROM:
	            if (l > 0) strcpy(from,headbuf) ;

	            break ;

	        case MH_SENDER:
	            strcpy(sentby,headbuf) ;

	            break ;

	        case MH_DATE:
	            if (l > 0) strcpy(date,headbuf) ;

	            break ;

	        } /* end switch */

	        headbuf[0] = '\0' ;
	    }

	} /* end for */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    logfile_printf(&g.eh,"getheader: OK 4\n") ;
#endif

	bclose(fp) ;

	return OK ;

badret:
	bclose(fp) ;

	return BAD ;
}
/* end subroutine (getheader) */


