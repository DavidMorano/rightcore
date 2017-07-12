/* eigendb */

/* store eigen words in a database */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module stores eigen words in a database for later convenient
	access.


*******************************************************************************/


#define	EIGENDB_MASTER		0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<strpack.h>
#include	<hdb.h>
#include	<filebuf.h>
#include	<char.h>
#include	<naturalwords.h>
#include	<localmisc.h>

#include	"eigendb.h"


/* local defines */

#define	EIGENDB_DEFENT		150
#define	EIGENDB_CHUCKSIZE	720
#define	EIGENDB_MAXFILESIZE	(40 * 1024 * 1024)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	KEYBUFLEN
#ifdef	NATURALWORDLEN
#define	KEYBUFLEN	NATURALWORDLEN
#else
#define	KEYBUFLEN	80
#endif
#endif

#define	TO_READ		5


/* external subroutines */

extern int	sncpylc(char *,int,const char *) ;
extern int	iceil(int,int) ;
extern int	hasuc(const char *,int) ;
extern int	isalnumlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

int		eigendb_addword(EIGENDB *,const char *,int) ;

static int	eigendb_fileparse(EIGENDB *,const char *) ;
static int	eigendb_fileparseread(EIGENDB *,int,int) ;
static int	eigendb_fileparsemap(EIGENDB *,int,int) ;


/* local variables */


/* exported subroutines */


int eigendb_open(EIGENDB *op,cchar *fname)
{
	const int	n = EIGENDB_DEFENT ;
	const int	chucksize = EIGENDB_CHUCKSIZE ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("eigendb_open: fname=%s\n",fname) ;
#endif

	if ((rs = strpack_start(&op->packer,chucksize)) >= 0) {
	    if ((rs = hdb_start(&op->db,n,1,NULL,NULL)) >= 0) {
	        op->magic = EIGENDB_MAGIC ;
	        if (fname != NULL) {
		    if (fname[0] != '\0') {
	                rs = eigendb_fileparse(op,fname) ;
		    } else
			rs = SR_INVALID ;
#if	CF_DEBUGS
	debugprintf("eigendb_open: _fileparse() rs=%d\n",rs) ;
#endif
	        }
	        if (rs < 0) {
	            hdb_finish(&op->db) ;
	        }
	    } /* end if (hdb-start) */
	    if (rs < 0) {
	        strpack_finish(&op->packer) ;
	        op->magic = 0 ;
	    }
	} /* end if (strpack-start) */

#if	CF_DEBUGS
	debugprintf("eigendb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (eigendb_open) */


/* close this DB */
int eigendb_close(EIGENDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	rs1 = hdb_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strpack_finish(&op->packer) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (eigendb_close) */


int eigendb_addfile(EIGENDB *op,cchar fname[])
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	rs = eigendb_fileparse(op,fname) ;

	return rs ;
}
/* end subroutine (eigendb_addfile) */


/* add a word to the DB */
int eigendb_addword(EIGENDB *op,cchar wp[],int wl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (wp == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	if (wl < 0)
	    wl = strlen(wp) ;

	if (wl > 0) {
	    HDB_DATUM	key, value ;
	    char	keybuf[KEYBUFLEN + 1] ;
	    if (hasuc(wp,wl)) {
	        if (wl > KEYBUFLEN) wl = KEYBUFLEN ;
	        strwcpylc(keybuf,wp,wl) ;
	        wp = keybuf ;
	    }
	    key.buf = wp ;
	    key.len = wl ;
	    if ((rs = hdb_fetch(&op->db,key,NULL,NULL)) == SR_NOTFOUND) {
		const char	*ep ;
	        if ((rs = strpack_store(&op->packer,wp,wl,&ep)) >= 0) {
		    c = 1 ;
	            key.buf = ep ;
	            key.len = wl ;
		    value.buf = ep ;
		    value.len = wl ;
	            rs = hdb_store(&op->db,key,value) ;
		} /* end if (strpack-store) */
	    } /* end if (hdb-fetch) */
	} /* end if (non-zero word) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (eigendb_addword) */


int eigendb_exists(EIGENDB *op,cchar wp[],int wl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (wp == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	if (wl < 0)
	    wl = strlen(wp) ;

	if (wl > 0) {
	    HDB_DATUM	key ;
	    char	keybuf[KEYBUFLEN + 1] ;
	    if (hasuc(wp,wl)) {
	        if (wl > KEYBUFLEN) wl = KEYBUFLEN ;
	        strwcpylc(keybuf,wp,wl) ;
	        wp = keybuf ;
	    }
	    key.buf = (char *) wp ;
	    key.len = wl ;
	    rs = hdb_fetch(&op->db,key,NULL,NULL) ;
	} else {
	    rs = SR_INVALID ;
	}

	return rs ;
}
/* end subroutine (eigendb_exists) */


int eigendb_count(EIGENDB *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	rs = hdb_count(&op->db) ;

	return rs ;
}
/* end subroutine (eigendb_count) */


int eigendb_curbegin(EIGENDB *op,EIGENDB_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	return hdb_curbegin(&op->db,cp) ;
}
/* end subroutine (eigendb_curbegin) */


int eigendb_curend(EIGENDB *op,EIGENDB_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	return hdb_curend(&op->db,cp) ;
}
/* end subroutine (eigendb_curend) */


/* enumerate */
int eigendb_enum(EIGENDB *op,EIGENDB_CUR *curp,cchar **rpp)
{
	HDB_DATUM	key ;
	HDB_DATUM	value ;
	int		rs ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != EIGENDB_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if ((rs = hdb_enum(&op->db,curp,&key,&value)) >= 0) {
	    len = key.len ;
	    if (rpp != NULL)
	        *rpp = (const char *) key.buf ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (eigendb_enum) */


/* private subroutines */


static int eigendb_fileparse(EIGENDB *op,cchar fname[])
{
	const mode_t	om = 0666 ;
	const int	oflags = O_RDONLY ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("eigendb_fileparse: fname=%s\n",fname) ;
#endif

	if ((rs = uc_open(fname,oflags,om)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if (! S_ISDIR(sb.st_mode)) {
	            const int	mfsize = EIGENDB_MAXFILESIZE ;
	            const int	fsize = sb.st_size ;
		    if (fsize > 0) {
	                if (S_ISREG(sb.st_mode) && (fsize <= mfsize)) {
	                    rs = eigendb_fileparsemap(op,fd,fsize) ;
	                } else {
	                    rs = eigendb_fileparseread(op,fd,fsize) ;
			}
		    }
	        } else
	            rs = SR_ISDIR ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (open-file) */

#if	CF_DEBUGS
	debugprintf("eigendb_fileparse: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (eigendb_fileparse) */


static int eigendb_fileparseread(EIGENDB *op,int fd,int fsize)
{
	FILEBUF		fb ;
	const int	to = TO_READ ;
	int		rs ;
	int		bufsize = 0 ;
	int		c = 0 ;

	if (fsize >= 0)
	    bufsize = iceil(fsize,1024) ;

	if ((rs = filebuf_start(&fb,fd,0L,bufsize,0)) >= 0) {
	    const int	llen = MAXPATHLEN ;
	    int		len ;
	    int		sl, cl ;
	    int		f_bol, f_eol ;
	    const char	*tp, *sp ;
	    const char	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

/* read the file */

	    f_bol = TRUE ;
	    while ((rs = filebuf_readline(&fb,lbuf,llen,to)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;
	        if (f_eol) len -= 1 ;

	        if (! f_eol) {
	            while ((len > 0) && (! CHAR_ISWHITE(lbuf[len - 1]))) {
	                len -= 1 ;
		    }
	        }

	        if ((tp = strnchr(lbuf,len,'#')) != NULL) {
	            len = (tp - lbuf) ;
		}

	        if ((len > 0) && f_bol) {

	            sp = lbuf ;
	            sl = len ;
	            while ((cl = nextfield(sp,sl,&cp)) > 0) {

	                c += 1 ;
	                rs = eigendb_addword(op,cp,cl) ;

	                sl -= ((cp + cl) - sp) ;
	                sp = (cp + cl) ;

	                if (rs < 0) break ;
	            } /* end while (words) */

	        } /* end if (line w/ feasible data) */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (lines) */

	    filebuf_finish(&fb) ;
	} /* end if (filebuf-finish) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (eigendb_fileparseread) */


static int eigendb_fileparsemap(EIGENDB *op,int fd,int fsize)
{
	size_t		msize ;
	int		rs ;
	int		mprot, mflags ;
	int		ml ;
	int		len ;
	int		ll, cl ;
	int		c = 0 ;
	const char	*mp ;
	const char	*tp ;
	const char	*lp ;
	const char	*cp ;
	void		*mdata ;

#if	CF_DEBUGS
	debugprintf("eigendb_fileparsemap: fsize=%u\n",fsize) ;
#endif

	msize = (size_t) fsize ;
	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	if ((rs = u_mmap(NULL,msize,mprot,mflags,fd,0L,&mdata)) >= 0) {
	    mp = (const char *) mdata ;
	    ml = msize ;

	    while ((tp = strnpbrk(mp,ml,"\n#")) != NULL) {

	        lp = mp ;
	        ll = (tp - mp) ;
	        len = ((tp + 1) - mp) ;
	        if (*tp == '#') {
	            if ((tp = strnchr((tp+1),(mp+ml-(tp+1)),'\n')) != NULL) {
	                len = ((tp + 1) - mp) ;
		    }
	        }

	        while ((cl = nextfield(lp,ll,&cp)) > 0) {

	            c += 1 ;
	            rs = eigendb_addword(op,cp,cl) ;

#if	CF_DEBUGS
	debugprintf("eigendb_fileparsemap: _addword() rs=%d\n",rs) ;
#endif
	            ll -= ((cp + cl) - lp) ;
	            lp = (cp + cl) ;

	            if (rs < 0) break ;
	        } /* end while (words) */

	        mp += len ;
	        ml -= len ;

	        if (rs < 0) break ;
	    } /* end while (lines) */

	    u_munmap(mdata,msize) ;
	} /* end if (map-file) */

#if	CF_DEBUGS
	debugprintf("eigendb_fileparsemap: rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (eigendb_fileparsemap) */


