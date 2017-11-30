/* sysvarproc */

/* Ssytem-Variable-Process */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_PROCVARFILE	0		/* compile in 'procvarfile' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Process a 

	Synopsis:

	int sysvarprocget(vlp,fname)
	HDBSTR		*vlp ;
	const char	fname[] ;

	Arguments:

	vlp		pointer to hash-string object
	fname		file to process

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */

#include	<vsystem.h>
#include	<filebuf.h>
#include	<vecstr.h>
#include	<hdbstr.h>
#include	<field.h>
#include	<varmk.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(10 * MAXPATHLEN)
#endif

#define	BUFLEN		MAX((4 * MAXPATHLEN),LINEBUFLEN)

#define	WORDEXPORT	"export"

#define	TO_OPEN		5
#define	TO_READ		5
#define	TO_MKWAIT	20

#ifndef	DEFNVARS
#define	DEFNVARS	1000
#endif


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	vecstr_envfile(VECSTR *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	isalnumlatin(int) ;

extern char	*strnchr(const char *,int,int) ;


/* forward references */

static int	procaddvar(HDBSTR *,const char *,int) ;

#if	CF_PROCVARFILE
static int	procvarfile(HDBSTR *,const char *) ;
static int	hasweird(const char *,int) ;
#endif


/* local variables */

#if	CF_PROCVARFILE
static const uchar	fterms[32] = {
	0x00, 0x3A, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;
#endif /* CF_PROCVARFILE */

static const char	*wstrs[] = {
	"TZ",
	"LANG",
	"UMASK",
	"PATH",
	NULL
} ;

static const char	*pstrs[] = {
	"LC_",
	NULL
} ;


/* exported subroutines */


int sysvarprocget(HDBSTR *vlp,cchar fname[])
{
	vecstr		lvars ;
	int		rs ;
	int		rs1 ;

	if (vlp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if ((rs = vecstr_start(&lvars,10,0)) >= 0) {
	    int		i ;
	    int		f ;
	    cchar	*tp, *cp ;

	    if ((rs = vecstr_envfile(&lvars,fname)) >= 0) {

	        for (i = 0 ; vecstr_get(&lvars,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;

	            if ((tp = strchr(cp,'=')) == NULL) continue ;

	            f = (matstr(wstrs,cp,(tp - cp)) >= 0) ;
	            f = f || (matpstr(pstrs,10,cp,(tp - cp)) >= 0) ;
	            if (f) {
	                rs = procaddvar(vlp,cp,-1) ;
	            } /* end if */

	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (vecstr_envfile) */

	    rs1 = vecstr_finish(&lvars) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lvars) */

	return rs ;
}
/* end subroutine (sysvarprocget) */


int sysvarprocset(HDBSTR *vlp,cchar dbname[],mode_t om)
{
	HDBSTR_CUR	cur ;
	VARMK		svars ;
	const int	of = O_CREAT ;
	const int	n = DEFNVARS ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (vlp == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if ((rs = varmk_open(&svars,dbname,of,om,n)) >= 0) {
	    int		vl ;
	    cchar	*kp, *vp ;

	    if ((rs = hdbstr_curbegin(vlp,&cur)) >= 0) {

	        while (rs >= 0) {
	            rs1 = hdbstr_enum(vlp,&cur,&kp,&vp,&vl) ;
	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;

	            if (rs >= 0) {
	                c += 1 ;
	                rs = varmk_addvar(&svars,kp,vp,vl) ;
	            }

	        } /* end while */

	        hdbstr_curend(vlp,&cur) ;
	    } /* end if (cursor) */

	    rs1 = varmk_close(&svars) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (varmk) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysvarprocset) */


/* local subroutines */


#if	CF_PROCVARFILE

static int procvarfile(vlp,fname)
HDBSTR		*vlp ;
const char	fname[] ;
{
	const int	to = TO_READ ;
	int		rs ;
	int		cl ;
	int		kl ;
	int		c = 0 ;
	const char	*tp, *kp ;
	const char	*cp ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    FILEBUF	dfile, *dfp = &dfile ;
	    int	fd = rs ;

	    if ((rs = filebuf_start(dfp,fd,0L,BUFLEN,0)) >= 0) {
		FIELD		fsb ;
	        const int	llen = LINEBUFLEN ;
		const int	vlen = VBUFLEN ;
	        int		len ;
		int		vl ;
		const char	*vp ;
	        char		lbuf[LINEBUFLEN + 1] ;
		char		vbuf[VBUFLEN + 1] ;

	        while ((rs = filebuf_readline(dfp,lbuf,llen,to)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            cl = len ;
	            while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	                cp += 1 ;
	                cl -= 1 ;
	            }

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	                kl = field_get(&fsb,fterms,&kp) ;

	                if (kl == 6) {
	                    if (strncasecmp(WORDEXPORT,kp,kl) == 0) {
	                        kl = field_get(&fsb,fterms,&kp) ;
			    }
	                } /* end if (elimination of 'export') */

	                if ((kl > 0) && (! hasweird(kp,kl))) {
	                    const int	nrs = SR_NOTFOUND ;
	                    const void	*n = NULL ;

	                    if ((rs = hdbstr_fetch(vlp,kp,kl,n,n)) == nrs) {

	                        vp = vbuf ;
	                        vl = 0 ;
	                        if (fsb.term != '#')
	                            vl = field_sharg(&fsb,fterms,vbuf,vlen) ;

	                        if (vl >= 0) {
	                            c += 1 ;
	                            rs = hdbstr_add(vlp,kp,kl,vp,vl) ;
	                        }

	                    } /* end if (didn't have it already) */

	                } /* end if (have a variable keyname) */

	                field_finish(&fsb) ;
	            } /* end if (fields) */

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        filebuf_finish(dfp) ;
	    } /* end if (filebuf) */

	    u_close(fd) ;
	} /* end if (file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procvarfile) */


static int hasweird(cchar *sp,int sl)
{
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; (i != sl) && (sp[i] != '\0') ; i += 1) {
	    const int	ch = MKCHAR(sp[i]) ;
	    f = ((! isalnumlatin(ch)) && (ch != '_')) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (hasweird) */

#endif /* CF_PROCVARFILE */


static int procaddvar(HDBSTR *vlp,cchar *cp,int cl)
{
	int		rs = SR_OK ;
	int		kl, vl ;
	int		c = 0 ;
	const char	*tp ;
	const char	*kp, *vp ;

	kp = cp ;
	kl = cl ;
	vp = NULL ;
	vl = 0 ;
	if ((tp = strnchr(cp,cl,'=')) != NULL) {
	    vp = (tp + 1) ;
	    vl = -1 ;
	    kl = (tp - cp) ;
	}

	if ((rs = hdbstr_fetch(vlp,kp,kl,NULL,NULL)) == SR_NOTFOUND) {
	    c += 1 ;
	    rs = hdbstr_add(vlp,kp,kl,vp,vl) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procaddvar) */


