/* ddb (unneeded, unfinished) */

/* domain data-base */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SAFE		1


/* revision history:

	= 1998-10-01, David A­D­ Morano

	I made up this idea for supporting multiple domains on the same
	machine at the same time!  We'll see where this idea leads.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine looks up the given username and returns the
	domainname for that user.  Per-user domain names are options
	and are administered through the file 'etc/udomains' located
	relative to the programroot directory that is optionally
	supplied.  If no programroot is supplied, then '/' is used.


******************************************************************************/ 


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<hdb.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


static int ddb_open(op,pr,fname)
DDB		*op ;
const char	pr[] ;
const char	fname[] ;
{
	int	rs ;


#if	CF_SAFE
	if (op == NULL)
		return SR_FAULT ;
#endif



	return rs ;
}
/* end subroutine (ddb_open) */


static int ddb_search(op,key,value)
DDB		*op ;
const char	key[] ;
char		value[] ;
{
	int	rs ;


#if	CF_SAFE
	if (op == NULL)
		return SR_FAULT ;
#endif


	return rs ;
}
/* end subroutine (ddb_search) */


static int ddb_close(op)
DDB		*op ;
{
	HDB_CUR	cur ;

	HDB_DATUM	key, value ;

	int	rs = SR_OK ;


#if	CF_SAFE
	if (op == NULL)
		return SR_FAULT ;
#endif

	hdb_curbegin(&op->db,&cur) ;

	while (hdb_enum(&op->db,&cur,&key,&value) >= 0) {

		if (value.buf != NULL)
			uc_free(value.buf) ;

	} /* end while */

	hdb_curend(&op->db,&cur) ;

	hdb_free(&op->db) ;

	vecstr_free(&keys) ;

	if (op->fname != NULL) {
	    uc_free(op->fname) ;
	    op->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (ddb_close) */


/* private subroutines */


int ddb_parse(op,fname)
DDB		*op ;
const char	fname[] ;
{
	bfile	udfile ;

	int	rs ;
	int	len ;
	int	sl, cl ;
	int	ml = 0 ;

	const char	*sp, *cp ;
	const char	*fname ;

	char	udfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("ddb_parse: username=%s\n",username) ;
#endif

	if ((pr != NULL) && (pr[0] != '\0')) {

		fname = udfname ;
		mkpath2(udfname,pr,UDOMASTDINFNAME) ;

	} else
		fname = UDOMASTDINFNAME ;

	domainname[0] = '\0' ;
	rs = bopen(&udfile,fname,"r",0666) ;

	if (rs < 0)
	    return rs ;

	ml = 0 ;
	while ((rs = breadline(&udfile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    linebuf[len] = '\0' ;

	    sp = linebuf ;
	    sl = len ;
	    cl = nextfield(sp,sl,&cp) ;

	    if ((cl <= 0) || (cp[0] == '#'))
	        continue ;

	    if (strncmp(username,cp,cl) == 0) {

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

#if	CF_DEBUGS
	        debugprintf("ddb_prase: remaining sl=%d sp=%s\n",sl,sp) ;
#endif

	        cl = nextfield(sp,sl,&cp) ;

	        if (cl > 0) {

#if	CF_DEBUGS
	            debugprintf("udomain: d=%s\n",cp) ;
#endif

	            ml = MIN(cl,MAXHOSTNAMELEN) ;
	            strwcpy(domainname,cp,ml) ;

	            break ;

	        } /* end if (got a domainname) */

	    } /* end if (username match) */

	} /* end while (reading lines) */

	bclose(&udfile) ;

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (ddb_parse) */



