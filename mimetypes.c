/* mimetypes */

/* manage a MIME-type database */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-06-19, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module manages a MIME-type database. It reads in MIME-type
        formatted files with the 'mimetypes_file()' subroutine and provides
        other subroutines for database queries.

	Note on file parsing:

        The MIME content type field (the first field of the file) *must* be a
        single contiguous string without any white-space in it. This may be
        obvious but it should be observed when creating 'mimetypes' files. This
        code does not (currently) try to piece together a content type from
        pieces separated by white space (we can't do everything!).


*******************************************************************************/


#define	MIMETYPES_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>

#include	"mimetypes.h"


/* local defines */

#define	MIMETYPES_NUMKEYS	200	/* initial estimated number of keys */

#undef	LINEBUFLEN
#define	LINEBUFLEN	2048


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	exttypespec(const char *,int,const char **) ;

#ifdef	COMMENT
static int	cmpentry(const char *,const char *,int) ;
#endif


/* local variables */


/* exported subroutines */


int mimetypes_start(MIMETYPES *dbp)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("mimetypes_start: ent\n") ;
#endif

	rs = hdb_start(dbp,MIMETYPES_NUMKEYS,1,NULL,NULL) ;

	return rs ;
}
/* end subroutine (mimetypes_start) */


/* free up this whole object */
int mimetypes_finish(MIMETYPES *dbp)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, data ;
	int		rs ;
	int		rs1 ;

	if ((rs = hdb_curbegin(dbp,&cur)) >= 0) {
	    while (hdb_enum(dbp,&cur,&key,&data) >= 0) {
	        if (key.buf != NULL) {
	            rs1 = uc_free((void *) key.buf) ;
	            if (rs >= 0) rs = rs1 ;
	        }
	    } /* end while */
	    hdb_curend(dbp,&cur) ;
	} /* end if */

	rs1 = hdb_finish(dbp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mimetypes_finish) */


/* read a whole file into the database */
int mimetypes_file(MIMETYPES *dbp,cchar *fname)
{
	bfile		mfile, *mfp = &mfile ;
	int		rs ;
	int		ctl ;
	int		size ;
	int		cl, fl ;
	int		c = 0 ;
	uchar		terms[32] ;
	const char	*fp ;
	const char	*cp ;
	const char	*ctp ;

#if	CF_DEBUGS
	debugprintf("mimetypes_file: ent\n") ;
#endif

	if (dbp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mimetypes_file: ent, fname=%s\n",fname) ;
#endif

	fieldterms(terms,FALSE," \t#") ;

	if ((rs = bopen(mfp,fname,"r",0666)) >= 0) {
	    HDB_DATUM	key, data ;
	    FIELD	fb ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(mfp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

#if	CF_DEBUGS
	        debugprintf("mimetypes_file: line> %s\n",lbuf) ;
#endif

	        if (len == 0)
	            continue ;

/* extract the typespec (MIME content type) */

	        ctl = exttypespec(lbuf,len,&ctp) ;

#if	CF_DEBUGS
	        debugprintf("mimetypes_file: exttypespec() ctp=%t ctl=%d\n",
	            ctp,ctl,ctl) ;
#endif

	        if (ctl <= 0)
	            continue ;

/* get the file suffixes for this MIME content type */

	        cp = ctp + ctl ;
	        cl = len - (cp - lbuf) ;
	        if ((rs = field_start(&fb,cp,cl)) >= 0) {

	            while ((fl = field_get(&fb,terms,&fp)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("mimetypes_file: field> %t\n",fp,fl) ;
#endif

	                if (fl > 0) {

	                    key.buf = fp ;
	                    key.len = fl ;
	                    if (hdb_fetch(dbp,key,NULL,&data) < 0) {
	                        char	*bkp, *bvp ;

#if	CF_DEBUGS
	                        debugprintf("mimetypes_file: key=%t val=%t\n",
	                            key.buf,key.len,ctp,ctl) ;
#endif

	                        size = key.len + 1 + ctl + 1 ;
	                        rs = uc_malloc(size,&bkp) ;
	                        if (rs < 0) break ;

	                        bvp = strwcpy(bkp,key.buf,key.len) + 1 ;

	                        key.buf = bkp ;
	                        strwcpy(bvp,ctp,ctl) ;

	                        data.len = ctl ;
	                        data.buf = bvp ;

	                        rs = hdb_store(dbp,key,data) ;
	                        if (rs < 0) {
	                            uc_free(bkp) ;
	                            break ;
	                        }

	                        c += 1 ;

	                    } /* end if */

	                } /* end if */

	                if (fb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while (reading fields) */

	            field_finish(&fb) ;
	        } /* end if (field) */

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    bclose(mfp) ;
	} /* end if (bfile) */

#if	CF_DEBUGS
	{
	    HDB_CUR	cur ;
	    debugprintf("mimetypes_file: dumping key-val pairs\n") ;
	    hdb_curbegin(dbp,&cur) ;
	    while (hdb_enum(dbp,&cur,&key,&data) >= 0) {
	        debugprintf("mimetypes_file: key=%t val=%t\n",
	            key.buf,key.len,data.buf,data.len) ;
	    }
	    hdb_curend(dbp,&cur) ;
	} /* end block */
#endif /* CF_DEBUGS */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mimetypes_file) */


/* find a MIME type for the given extension */
int mimetypes_find(MIMETYPES *dbp,char *typespec,cchar *ext)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*tp ;

#if	CF_DEBUGS
	debugprintf("mimetypes_get: ent\n") ;
#endif

	if (ext == NULL) return SR_FAULT ;

	if (typespec != NULL) typespec[0] = '\0' ;

	if ((tp = strrchr(ext,'.')) == NULL) {
	    tp = ext ;
	} else
	    tp += 1 ;

	if (tp[0] != '\0') {
	    HDB_DATUM	key, data ;
	    key.len = -1 ;
	    key.buf = tp ;
	    if ((rs = hdb_fetch(dbp,key,NULL,&data)) >= 0) {
	        if (typespec != NULL) {
	            const int	typelen = MIMETYPES_TYPELEN ;
	            rs = snwcpy(typespec,typelen,data.buf,data.len) ;
	            len = rs ;
	        }
	    }
	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("mimetypes_get: ret rs=%d len=%d\n",rs,data.len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mimetypes_find) */


/* initialize a cursor */
int mimetypes_curbegin(MIMETYPES *dbp,HDB_CUR *cp)
{

	return hdb_curbegin(dbp,cp) ;
}
/* end subroutine (mimetypes_curbegin) */


/* free up a cursor */
int mimetypes_curend(MIMETYPES *dbp,HDB_CUR *cp)
{

	return hdb_curend(dbp,cp) ;
}
/* end subroutine (mimetypes_curend) */


/* enumerate all of the key-val pairs */
int mimetypes_enum(MIMETYPES *dbp,MIMETYPES_CUR *curp,char *ext,char *ts)
{
	HDB_DATUM	key, val ;
	int		rs ;

	if (dbp == NULL) return SR_FAULT ;
	if (ext == NULL) return SR_FAULT ;

	ext[0] = '\0' ;
	if (ts != NULL) ts[0] = '\0' ;

	if ((rs = hdb_enum(dbp,curp,&key,&val)) >= 0) {
	    const int	ml = MIN(key.len,MIMETYPES_TYPELEN) ;
	    strwcpy(ext,key.buf,ml) ;
	    rs = MIN(val.len,MIMETYPES_TYPELEN) ;
	    if (ts != NULL) strwcpy(ts,val.buf,rs) ;
	}

	return rs ;
}
/* end subroutine (mimetypes_enum) */


/* fetch the next val by extension and a possible cursor */
int mimetypes_fetch(MIMETYPES *dbp,MIMETYPES_CUR *curp,char *ext,char *ts)

{
	HDB_DATUM	key, val ;
	int		rs ;

	if (dbp == NULL) return SR_FAULT ;
	if ((ext == NULL) || (ts == NULL)) return SR_FAULT ;

	key.buf = ext ;
	key.len = -1 ;
	ts[0] = '\0' ;
	if ((rs = hdb_fetch(dbp,key,curp,&val)) >= 0) {
	    const int	ml = MIN(val.len,MIMETYPES_TYPELEN) ;
	    const char	*mp = (const char *) val.buf ;
	    rs = (strwcpy(ts,mp,ml) - ts) ;
	}

	return rs ;
}
/* end subroutine (mimetypes_fetch) */


/* OLDER API: find a MIME type given an extension */
int mimetypes_get(MIMETYPES *dbp,char *typespec,cchar *ext)
{

	return mimetypes_find(dbp,typespec,ext) ;
}
/* end subroutine (mimetypes_get) */


/* private subroutines */


/* extract the typespec from this buffer */
static int exttypespec(cchar *tbuf,int tlen,cchar **rpp)
{
	int		cl ;
	const char	*sp, *cp ;

	cp = tbuf ;
	cl = tlen ;
	while (CHAR_ISWHITE(*cp) && (cl > 0)) {
	    cp += 1 ;
	    cl -= 1 ;
	}

	if (*cp == '#')
	    return -1 ;

	sp = cp ;
	while (*cp && (cl > 0) && (! CHAR_ISWHITE(*cp)) && (*cp != '#')) {
	    cp += 1 ;
	    cl -= 1 ;
	}

	*rpp = sp ;
	return (cp - sp) ;
}
/* end subroutine (exttypespec) */


#ifdef	COMMENT

static int cmpentry(cchar *s1,cchar *s2,int len)
{
	int		rc = 0 ;

	if (len > 0) {
	    rc = (s2[len] - s1[len]) ;
	    if (rc == 0) rc = strncmp(s1,s2,len) ;
	}

	return rc ;
}
/* end subroutine (cmpentry) */
#endif /* COMMENT */


