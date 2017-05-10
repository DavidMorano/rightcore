/* bdb */

/* Bibliographical DataBase */


#define	CF_DEBUG	1	/* compile-time */


/* revision history:

	- 87/09/10, David A­D­ Morano
	This code module was originally written.

	- 98/09/10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This code module contains subroutines used to add paramters
	to parameter lists and such for later access.

	All parameter names and values are stored in freshly 
	allocated memory.  The original storage for parameter
	names and values can be freed after they are stored
	using these routines.


*******************************************************************************/


#define	BDB_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>

#include	<baops.h>
#include	<char.h>
#include	<vsystem.h>
#include	<hdb.h>
#include	<sbuf.h>
#include	<vecelem.h>
#include	<localmisc.h>

#include	"bdb.h"

#if	CF_DEBUG
#include	"config.h"
#include	"defs.h"
#endif


/* local defines */

#define	BDB_MAGIC	27

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#undef	CMDBUFLEN
#define	CMDBUFLEN	((MAXPATHLEN * 4) + 20)


/* external subroutines */

extern int	sibreak(const char *,int,const char *) ;

extern char	*strupper() ;


/* forward references */

int	bdbloads() ;
int	bdbload() ;
int	bdbfind() ;

static int	bdbloadem() ;
static int	bdbexists() ;


/* external variables */

#if	CF_DEBUG
extern struct global	g ;
#endif


/* exported subroutines */


/* initialize a Bibliography DB */
int bdb_init(bdbp)
bdb	*bdbp ;
{


	bdbp->magic = BDB_MAGIC ;
	return hdbinit(&bdbp->lh,5,NULL) ;
}
/* end subroutine (bdb_init) */


/* load a string parameter into the DB */
int bdb_loads(bdbp,s,len)
bdb	*bdbp ;
char		*s ;
int		len ;
{
	int	rs, i, n = 0 ;

	char	*cp ;


#if	CF_DEBUG
	eprintf("bdbloads: entered\n") ;
#endif

	if (len < 0)
	    len = strlen(s) ;

#if	CF_DEBUG
	eprintf("bdbloads: s=%W len=%d\n",s,len,len) ;
#endif

	while ((i = sibreak(s,len," \t,:")) >= 0) {

#if	CF_DEBUG
	    eprintf("bdbloads: i=%d, about to call 'bdbload'\n",i) ;
#endif

	    if ((rs = bdb_load(bdbp,s,i)) >= 0)
	        n += 1 ;

#if	CF_DEBUG
	    eprintf("bdbloads: returned from 'bdbload' rs=%d\n",rs) ;
#endif

	    s += (i + 1) ;
	    len -= (i + 1) ;

	}  /* end while */

	if (len > 0) {

	    if ((rs = bdb_load(bdbp,s,len)) >= 0)
	        n += 1 ;

	}

#if	CF_DEBUG
	eprintf("bdbloads: exited OK\n") ;
#endif

	return n ;
}
/* end subroutine (bdb_loads) */


/* load a single parameter into the DB */
int bdb_load(bdbp,s,ulen)
bdb	*bdbp ;
char		s[] ;
int		ulen ;
{
	BDB_VALUE	*rp ;

	int	len = ulen ;
	int	rs ;
	int	f_didit = FALSE ;

	char	*sp, *cp ;


#if	CF_DEBUG
	eprintf("bdbload: entered, s=%s\n",s) ;
#endif

/* clean up the key a little */

	sp = s ;
	while (CHAR_ISWHITE(*sp) || (*sp == ',')) {

	    sp += 1 ;
	    len -= 1 ;

	} /* end while */

	cp = sp ;
	while ((len > 0) && 
	    (CHAR_ISWHITE(sp[len - 1]) || (sp[len - 1] == ',')))
	    sp[--len] = '\0' ;

	if (len <= 0)
	    return BAD ;

#if	CF_DEBUG
	eprintf("bdbload: non-zero len value, v=\"%s\"\n",
	    sp) ;
#endif

/* does this bibliography exist */

	if (bdbexists(sp,len) < 0)
	    return BAD ;


/* do we have one of these named parameters already ? */

	if ((rs = bdb_find(bdbp,sp,len,&rp)) < 0) {

#if	CF_DEBUG
	    eprintf("bdbload: not already present\n") ;
#endif

	    if ((rs = bdb_loadem(bdbp,sp,len)) >= 0)
	        f_didit = TRUE ;

#if	CF_DEBUG
	    eprintf("bdbload: stored rs %d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUG
	eprintf("bdbload: exiting rs=%d\n",rs) ;
#endif

	return ((f_didit) ? 0 : rs) ;
}
/* end subroutine (bdb_load) */


/* find a paramter by name (path of the bibliography) */
int bdb_find(bdbp,name,len,rpp)
bdb		*bdbp ;
char			name[] ;
char			len ;
struct bdb__value	**rpp ;
{
	HDB_DATUM	key, value ;

	int		rs ;


#if	CF_DEBUG
	eprintf("bdbfind: entered, n=%s\n",name) ;
#endif

	key.buf = name ;
	key.len = (len >= 0) ? len : strlen(name) ;

	if ((rs = hdbfetch(&bdbp->lh,key,NULL,&value)) < 0)
	    return rs ;

#if	CF_DEBUG
	eprintf("bdbfind: got it\n") ;
#endif

	if (rpp != NULL)
	    *rpp = (struct bdb__value *) value.buf ;

	return value.len ;
}
/* end subroutine (bdb_find) */


int bdb_free(bdbp)
bdb	*bdbp ;
{
	HDB_CURSOR	keycursor ;
	HDB_DATUM	key, value ;


	hdbnullcursor(NULL,&keycursor) ;

	while (hdbenum(&bdbp->lh,&keycursor,&key,&value) >= 0) {

	    if (key.buf != NULL) free(key.buf) ;

	    if (value.buf != NULL) free(value.buf) ;

	}

	return hdbfree(&bdbp->lh) ;
}
/* end subroutine (bdb_free) */


int bdb_count(bdbp)
bdb	*bdbp ;
{


	return hdbcount(&bdbp->lh) ;
}
/* end subroutine (bdb_count) */


/* get the entry under the current cursor */
int bdb_getname(bdbp,cp,rpp)
bdb		*bdbp ;
BDB_CURSOR	*cp ;
char		**rpp ;
{
	HDB_DATUM	key, value ;

	int	rs ;


#if	CF_DEBUG
	eprintf("bdbgetkey: entered\n") ;
#endif

	if ((rs = hdbenum(&bdbp->lh,(HDB_CURSOR *) cp,&key,&value)) >= 0) {

#if	CF_DEBUG
	    eprintf("bdbgetkey: loop\n") ;
#endif

	    rs = -1 ;
	    if ((key.buf != NULL) && (key.len > 0)) {

	        if (rpp != NULL)
	            *rpp = key.buf ;

	        rs = key.len ;
	    }

	} /* end if */

#if	CF_DEBUG
	eprintf("bdbgetkey: exiting rs %d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdb_getname) */


/* delete the entry under the current cursor */
int bdb_delcur(bdbp,cp)
bdb		*bdbp ;
BDB_CURSOR	*cp ;
{
	int	rs ;


#if	CF_DEBUG
	eprintf("bdbdelcursor: magic=%d\n",bdbp->magic) ;
	eprintf("bdbdelcursor: hdbdelcursor\n") ;
#endif

	rs = hdbdelcursor(&bdbp->lh,(HDB_CURSOR *) cp) ;

#if	CF_DEBUG
	eprintf("bdbdelcursor: past hdbdelcursor\n") ;
#endif

	return rs ;
}
/* end subroutine (bdb_delcur) */


/* make a query */
int bdb_query(bdbp,keys,vep)
bdb	*bdbp ;
char	keys[] ;
VECELEM	*vep ;
{
	HDB_DATUM	key, value ;

	HDB_CURSOR	cur ;

	bfile	*fpa[3], in, out ;

	sbuf	result ;

	int	n = 0, rs ;
	int	buflen, child_stat ;

	char	cmdbuf[CMDBUFLEN + 1] ;
	char	buf[BDB_BUFLEN + 1] ;
	char	linebuf[LINELEN + 1] ;


#if	CF_DEBUG
	eprintf("bdbquery: entered\n") ;
#endif

	if (vep == NULL)
	    return SR_FAULT ;

	sbuf_init(&result,buf,BDB_BUFLEN) ;

	fpa[0] = NULL ;
	fpa[1] = &out ;
	fpa[2] = NULL ;

	hdbcurbegin(&bdbp->lh,&cur) ;

	while ((rs = hdbenum(&bdbp->lh,&cur,&key,&value)) >= 0) {

#if	CF_DEBUG
	    eprintf("bdbquery: top loop\n") ;
#endif

	    bufprintf(cmdbuf,CMDBUFLEN,"%s -p %W %s",
	        "glkbib",key.buf,key.len,keys) ;

	    if ((rs = bopencmd(fpa,cmdbuf)) >= 0) {

	        int	f_start = TRUE ;
	        int	len, ll ;

	        char	*lp ;


	        while ((len = bgetline(&out,linebuf,LINELEN)) > 0) {

	            if (linebuf[0] == '\n') {

/* end an entry ? */

	                if (! f_start) {

	                    f_start = TRUE ;
	                    buflen = sbuf_getlen(&result) ;

	                    if (vecelemfind(vep,buf,buflen) < 0) {

				n += 1 ;
	                        vecelemadd(vep,buf,buflen) ;

				}

	                    sbuf_free(&result) ;

	                } else
	                    continue ;

	            } else {

	                f_start = FALSE ;
	                ll = sfshrink(linebuf,len,&lp) ;

	                if (ll > 0)
	                    sbuf_strn(&result,lp,ll) ;

	            } /* end if */

	        } /* end while (reading lines) */

	        bclose(&out) ;

	        waitpid((pid_t) rs,&child_stat,WUNTRACED) ;

	    } /* end if (spawned child) */

	} /* end while (looping through indivdual databases) */

	hdbcurend(&bdbp->lh,&cur) ;

#ifdef	COMMENT
	n = vecelemcount(vep) ;
#endif

	return n ;
}
/* end subroutine (bdb_query) */


int bdb_curbegin(bdbp,cp)
BDB		*bdbp ;
BDB_CURSOR	*cp ;
{


	return hdb_curbegin(&bdbp->lh,(HDB_CURSOR *) cp) ;
}
/* end subroutine (bdb_curbegin) */


int bdb_curend(bdbp,cp)
BDB		*bdbp ;
BDB_CURSOR	*cp ;
{


	return hdb_curend(&bdbp->lh,(HDB_CURSOR *) cp) ;
}
/* end subroutine (bdb_curend) */



/* INTERNAL SUBROUTINES */



static int bdb_loadem(dbp,name,len)
BDB		*dbp ;
char		name[] ;
int		len ;
{
	HDB_DATUM	key, value ;

	int	rs ;


	if ((dbp == NULL) || (name == NULL))
	    return SR_FAULT ;

	if (len < 0)
	    len = strlen(name) ;

	if (len <= 0)
	    return SR_INVALID ;

	if ((rs = uc_malloc(len,&key.buf)) < 0)
	    return rs ;

	key.len = len ;
	strwcpy(key.buf,name,len) ;

	value.len = sizeof(struct bdb__value) ;
	if ((rs = uc_malloc(value.len,&value.buf)) < 0) {

	    free(key.buf) ;

	    return rs ;
	}

	((struct bdb__value *) value.buf)->f.open = FALSE ;
	return hdbstore(&dbp->lh,key,value) ;
}
/* end subroutine (bdb_loadem) */


/* does this bibliography exist ? */

static char	*suffix[] = {
	".ia",
	".i",
	NULL,
} ;


static int bdb_exists(s,len)
char	s[] ;
int	len ;
{
	char	buf[MAXPATHLEN + 1] ;
	char	**spp = suffix ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    eprintf("bdb_exists: entered\n") ;
#endif

	strwcpy(buf,s,MIN(len,MAXPATHLEN)) ;

	if (u_access(buf,R_OK) >= 0)
	    return OK ;

	while (*spp != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        eprintf("bdb_exists: top loop >%s<\n",*spp) ;
#endif

	    bufprintf(buf,MAXPATHLEN,"%W%s",
	        s,len,*spp) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        eprintf("bdb_exists: buf=%s\n",buf) ;
#endif

	    if (u_access(buf,R_OK) >= 0)
	        return OK ;

	    spp += 1 ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        eprintf( "bdb_exists: bl progname=%s\n", g.progname) ;
#endif

	} /* end while */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    eprintf( "bdb_exists: ol progname=%s\n", g.progname) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    eprintf("bdb_exists: exiting\n") ;
#endif

	return BAD ;
}
/* end subroutine (bdb_exists) */



