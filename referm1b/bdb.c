/* bdb */

/* Bibliographical DataBase */


#define	CF_DEBUGS	0


/* revision history:

	= 87/09/01, David A­D­ Morano

	This code module was originally written.


	= 98/09/01, David A­D­ Morano

	This module was changed to serve in the REFERM program.


*/


/*******************************************************************

	This code module contains subroutines used to add paramters
	to parameter lists and such for later access.

	All parameter names and values are stored in freshly 
	allocated memory.  The original storage for parameter
	names and values can be freed after they are stored
	using these routines.



*********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>

#include	<baops.h>
#include	<char.h>
#include	<vsystem.h>
#include	<hdb.h>
#include	<sbuf.h>
#include	<vecitem.h>

#include	"localmisc.h"
#include	"bdb.h"

#if	CF_DEBUGS
#include	"config.h"
#include	"defs.h"
#endif



/* local defines */

#define	BDB_MASTER	1

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#undef	CMDBUFLEN
#define	CMDBUFLEN	((MAXPATHLEN * 4) + 20)



/* external subroutines */

extern int	sibreak(const char *,int,const char *) ;


/* forward references */

int	bdbloads() ;
int	bdbload() ;
int	bdbfind() ;

static int	bdbloadem() ;
static int	bdbexists() ;


/* external variables */


/* exported subroutines */


int bdbinit(bdbp)
BDB		*bdbp ;
{


	bdbp->magic = 27 ;
	return hdb_start(&bdbp->lh,5,NULL,NULL) ;
}
/* end subroutine (bdbinit) */


/* load a string parameter into the string DB */
int bdbloads(bdbp,s,len)
BDB		*bdbp ;
char		*s ;
int		len ;
{
	int	rs, i, n = 0 ;

	char	*cp ;


#if	CF_DEBUGS
	debugprintf("bdbloads: entered\n") ;
#endif

	if (len < 0)
	    len = strlen(s) ;

#if	CF_DEBUGS
	debugprintf("bdbloads: s=%W len=%d\n",s,len,len) ;
#endif

	while ((i = sibreak(s,len," \t,:")) >= 0) {

#if	CF_DEBUGS
	    debugprintf("bdbloads: i=%d, about to call 'bdbload'\n",i) ;
#endif

	    if ((rs = bdbload(bdbp,s,i)) >= 0)
	        n += 1 ;

#if	CF_DEBUGS
	    debugprintf("bdbloads: returned from 'bdbload' rs=%d\n",rs) ;
#endif

	    s += (i + 1) ;
	    len -= (i + 1) ;

	}  /* end while */

	if (len > 0) {

	    if ((rs = bdbload(bdbp,s,len)) >= 0)
	        n += 1 ;

	}

#if	CF_DEBUGS
	debugprintf("bdbloads: exited OK\n") ;
#endif

	return n ;
}
/* end subroutine (bdbloads) */


/* load a single parameter into the "params" DB */
int bdbload(bdbp,s,ulen)
BDB		*bdbp ;
char		s[] ;
int		ulen ;
{
	BDB_VALUE	*rp ;

	int	len = ulen ;
	int	rs ;
	int	f_didit = FALSE ;

	char	*sp, *cp ;


#if	CF_DEBUGS
	debugprintf("bdbload: entered, s=%s\n",s) ;
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

#if	CF_DEBUGS
	debugprintf("bdbload: non-zero len value, v=\"%s\"\n",
	    sp) ;
#endif

/* does this bibliography exist */

	if (bdbexists(sp,len) < 0)
	    return BAD ;


/* do we have one of these named parameters already ? */

	if ((rs = bdbfind(bdbp,sp,len,&rp)) < 0) {

#if	CF_DEBUGS
	    debugprintf("bdbload: not already present\n") ;
#endif

	    if ((rs = bdbloadem(bdbp,sp,len)) >= 0)
	        f_didit = TRUE ;

#if	CF_DEBUGS
	    debugprintf("bdbload: stored rs %d\n",rs) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("bdbload: exiting rs=%d\n",rs) ;
#endif

	return ((f_didit) ? 0 : rs) ;
}
/* end subroutine (bdbload) */


/* find a paramter by name */
int bdbfind(bdbp,name,len,rpp)
BDB		*bdbp ;
char		name[] ;
char		len ;
BDB_VALUE	**rpp ;
{
	HDB_DATUM	key, value ;

	int		rs ;


#if	CF_DEBUGS
	debugprintf("bdbfind: entered, n=%s\n",name) ;
#endif

	key.buf = name ;
	key.len = (len >= 0) ? len : strlen(name) ;

	rs = hdb_fetch(&bdbp->lh,key,NULL,&value) ;

	if (rs < 0)
	    return rs ;

#if	CF_DEBUGS
	debugprintf("bdbfind: got it\n") ;
#endif

	if (rpp != NULL)
	    *rpp = (struct bdb_value *) value.buf ;

	return value.len ;
}
/* end subroutine (bdbfind) */


int bdbfree(bdbp)
BDB		*bdbp ;
{
	HDB_CUR	keycursor ;
	HDB_DATUM	key, value ;


	hdb_curbegin(&bdbp->lh,&keycursor) ;

	while (hdb_enum(&bdbp->lh,&keycursor,&key,&value) >= 0) {

	    if (key.buf != NULL) free(key.buf) ;

	    if (value.buf != NULL) free(value.buf) ;

	}

	hdb_curend(&bdbp->lh,&keycursor) ;

	return hdb_finish(&bdbp->lh) ;
}
/* end subroutine (bdbfree) */


int bdbcount(bdbp)
BDB		*bdbp ;
{


	return hdb_count(&bdbp->lh) ;
}
/* end subroutine (bdbcount) */


/* get the entry under the current cursor */
int bdbgetname(bdbp,cp,rpp)
BDB		*bdbp ;
BDB_CUR	*cp ;
char		**rpp ;
{
	HDB_DATUM	key, value ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("bdbgetkey: entered\n") ;
#endif

	if ((rs = hdb_enum(&bdbp->lh,(HDB_CUR *) cp,&key,&value)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("bdbgetkey: loop\n") ;
#endif

	    rs = -1 ;
	    if ((key.buf != NULL) && (key.len > 0)) {

	        if (rpp != NULL)
	            *rpp = key.buf ;

	        rs = key.len ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("bdbgetkey: exiting rs %d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdbgetname) */


/* delete the entry under the current cursor */
int bdbdelcursor(bdbp,curp)
BDB		*bdbp ;
BDB_CUR	*curp ;
{
	HDB_CUR	*hcurp = (HDB_CUR *) curp ;
	int	rs ;


#if	CF_DEBUGS
	debugprintf("bdbdelcursor: magic=%d\n",bdbp->magic) ;
	debugprintf("bdbdelcursor: hdbdelcursor\n") ;
#endif

	rs = hdb_delcur(&bdbp->lh,hcurp,0) ;

#if	CF_DEBUGS
	debugprintf("bdbdelcursor: past hdbdelcursor\n") ;
#endif

	return rs ;
}
/* end subroutine (bdbdelcursor) */


/* make a query */
int bdbquery(bdbp,keys,vep)
BDB		*bdbp ;
char		keys[] ;
VECITEM		*vep ;
{
	HDB_DATUM	key, value ;

	HDB_CUR	cur ;

	bfile	*fpa[3], in, out ;

	SBUF	result ;

	int	n = 0, rs ;
	int	buflen, child_stat ;

	char	cmdbuf[CMDBUFLEN + 1] ;
	char	buf[BDB_BUFLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("bdbquery: entered\n") ;
#endif

	if (vep == NULL)
	    return SR_FAULT ;

	sbuf_start(&result,buf,BDB_BUFLEN) ;

	fpa[0] = NULL ;
	fpa[1] = &out ;
	fpa[2] = NULL ;

	hdb_curbegin(&bdbp->lh,&cur) ;

	while ((rs = hdb_enum(&bdbp->lh,&cur,&key,&value)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("bdbquery: top loop\n") ;
#endif

	    bufprintf(cmdbuf,CMDBUFLEN,"%s -p %W %s",
	        "glkbib",key.buf,key.len,keys) ;

	    if ((rs = bopencmd(fpa,cmdbuf)) >= 0) {

	        int	f_start = TRUE ;
	        int	len, ll ;

	        char	*lp ;


	        while ((len = breadline(&out,linebuf,LINEBUFLEN)) > 0) {

	            if (linebuf[0] == '\n') {

/* end an entry ? */

	                if (! f_start) {

	                    f_start = TRUE ;
	                    buflen = sbuf_getlen(&result) ;

	                    if (vecitem_find(vep,buf,buflen) < 0)
	                        vecitem_add(vep,buf,buflen) ;

	                    sbuf_finish(&result) ;

	                } else
	                    continue ;

	            } else {

	                f_start = FALSE ;
	                ll = sfshrink(linebuf,len,&lp) ;

	                if (ll > 0)
	                    sbuf_strw(&result,lp,ll) ;

	            } /* end if */

	        } /* end while (reading lines) */

	        bclose(&out) ;

	        waitpid((pid_t) rs,&child_stat,WUNTRACED) ;

	    } /* end if (spawned child) */

	} /* end while (looping through indivdual databases) */

	hdb_curend(&bdbp->lh,&cur) ;

	n = vecitem_count(vep) ;

	return n ;
}
/* end subroutine (bdbquery) */



/* FUNNY EXTRA SUBROUTINES */



int bdbcurbegin(op,cp)
BDB		*op ;
BDB_CUR	*cp ;
{


	return hdb_curbegin(&op->lh,(HDB_CUR *) cp) ;
}
/* end subroutine (bdbcurbegin) */


int bdbcurend(op,cp)
BDB		*op ;
BDB_CUR	*cp ;
{


	return hdb_curend(&op->lh,(HDB_CUR *) cp) ;
}
/* end subroutine (bdbcurend) */



/* INTERNAL SUBROUTINES */



static int bdbloadem(dbp,name,len)
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

	value.len = sizeof(struct bdb_value) ;
	if ((rs = uc_malloc(value.len,&value.buf)) < 0) {

	    free(key.buf) ;

	    return rs ;
	}

	((struct bdb_value *) value.buf)->f.open = FALSE ;
	return hdb_store(&dbp->lh,key,value) ;
}
/* end subroutine (bdbloadem) */


/* does this bibliography exist ? */

static char	*suffix[] = {
	".ia",
	".i",
	NULL,
} ;


static int bdbexists(s,len)
char		s[] ;
int		len ;
{
	char	buf[MAXPATHLEN + 1] ;
	char	**spp = suffix ;


#if	CF_DEBUGS
	    debugprintf("bibexists: entered\n") ;
#endif

	strwcpy(buf,s,MIN(len,MAXPATHLEN)) ;

	if (u_access(buf,R_OK) >= 0)
	    return OK ;

	while (*spp != NULL) {

#if	CF_DEBUGS
	        debugprintf("bibexists: top loop >%s<\n",*spp) ;
#endif

	    bufprintf(buf,MAXPATHLEN,"%W%s",
	        s,len,*spp) ;

#if	CF_DEBUGS
	        debugprintf("bibexists: buf=%s\n",buf) ;
#endif

	    if (u_access(buf,R_OK) >= 0)
	        return OK ;

	    spp += 1 ;

#if	CF_DEBUGS
	        debugprintf( "bibexists: bl progname=%s\n", g.progname) ;
#endif

	} /* end while */

#if	CF_DEBUGS
	    debugprintf( "bibexists: ol progname=%s\n", g.progname) ;
#endif

#if	CF_DEBUGS
	    debugprintf("bibexists: exiting\n") ;
#endif

	return BAD ;
}
/* end subroutine (bdbexists) */



