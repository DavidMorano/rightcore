/* vecpstr_envfile */

/* process an environment file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) an environment file and put any
        newly encountered environment variablee into the string list (supplied).
	Variables found that already exist are ignored.

	Synopsis:

	int vecpstr_envfile(vlp,fname)
	VECPSTR		*vlp ;
	const char	fname[] ;

	Arguments:

	vlp		vecpstr-string object to accumulate results
	fname		file to process

	Returns:

	>=0		count of environment variables
	<0		bad


	- Implementation note:

        Although this function is (overall) rather simple, I put some slightly
	cleaver thought into parsing the lines. 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */

#include	<vsystem.h>
#include	<vecpstr.h>
#include	<filebuf.h>
#include	<field.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#define	ENVLINEBUFLEN	(5*LINEBUFLEN)

#define	SUBINFO		struct subinfo

#define	WORDEXPORT	"export"

#define	TO_OPEN		5
#define	TO_READ		-1


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	vstrkeycmp(const char **,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,cchar *) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* external variables */


/* local structures */

struct subinfo {
	VECPSTR		*vlp ;
	const char	*a ;		/* allocation */
	char		*lbuf ;
	char		*ebuf ;
	int		llen ;
	int		elen ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,VECPSTR *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_line(SUBINFO *,cchar *,int) ;
static int	subinfo_var(SUBINFO *,FIELD *,cuchar *,cchar *,int) ;


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x02, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* external subroutines */


int vecpstr_envfile(VECPSTR *vlp,cchar *fname)
{
	SUBINFO		si, *sip = &si ;
	const int	to = TO_READ ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (vlp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = subinfo_start(sip,vlp)) >= 0) {
	    if ((rs = uc_opene(fname,O_RDONLY,0666,TO_OPEN)) >= 0) {
	        FILEBUF		dfile, *dfp = &dfile ;
	        const int	fd = rs ;

	        if ((rs = filebuf_start(dfp,fd,0L,0,0)) >= 0) {
	            const int	llen = sip->llen ;
	            int		len ;
	            int		cl ;
	            const char	*cp ;
	            char	*lbuf = sip->lbuf ;
		    void	*n = NULL ;

	            while ((rs = filebuf_readlines(dfp,lbuf,llen,to,n)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

#if	CF_DEBUGS
	                debugprintf("vecpstr_envfile: line=>%t<\n",
				lbuf,len) ;
#endif

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        rs = subinfo_line(sip,cp,cl) ;
	                        c += rs ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = filebuf_finish(dfp) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (filebuf) */

	        rs1 = u_close(fd) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file) */
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("vecpstr_envfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_envfile) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,VECPSTR *vlp)
{
	const int	llen = ENVLINEBUFLEN ;
	int		size ;
	int		rs ;
	char		*bp ;
	sip->vlp = vlp ;
	size = (2*(llen+1)) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    sip->a = bp ;
	    sip->lbuf = bp ;
	    sip->llen = llen ;
	    bp += (llen+1) ;
	    sip->ebuf = bp ;
	    sip->elen = llen ;
	}
	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (sip->a != NULL) {
	    rs1 = uc_free(sip->a) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->a = NULL ;
	}
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_line(SUBINFO *sip,const char *lp,int ll)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    int		fl ;
	    const uchar	*ft = fterms ;
	    const char	*fp ;

	    if ((fl = field_get(&fsb,ft,&fp)) > 0) {
	        if ((strncasecmp(WORDEXPORT,fp,fl) == 0) && (fl == 6)) {
	            fl = field_get(&fsb,ft,&fp) ;
	        }
	    } /* end if (variable key-name) */

	    if (fl > 0) {
	        rs = subinfo_var(sip,&fsb,ft,fp,fl) ;
	        c = rs ;
	    }

	    field_finish(&fsb) ;
	} /* end if (fields) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_line) */


static int subinfo_var(SUBINFO *sip,FIELD *fsp,const uchar *ft,cchar *kp,int kl)
{
	vecpstr		*vlp = sip->vlp ;
	const int	klen = (sip->elen -1) ;
	int		rs = SR_OK ;
	int		c = 0 ;
	char		*kbuf = sip->ebuf ;

#if	CF_DEBUGS
	debugprintf("vecpstr_enfile/subinfo_var: ent\n") ;
	debugprintf("vecpstr_enfile/subinfo_var: k=>%t<\n",kp,kl) ;
	debugprintf("vecpstr_enfile/subinfo_var: elen=%d\n",sip->elen) ;
#endif

	if (kl <= klen) {
	    const int	rsn = SR_NOTFOUND ;
	    const int	vlen = sip->elen ;
	    int		(*vs)(cchar **,cchar **) = vstrkeycmp ;
	    int		vl ;
	    char	*vbuf = sip->ebuf ;
	    char	*vp = strdcpy1w(kbuf,klen,kp,kl) ;

	    if ((rs = vecpstr_finder(vlp,kbuf,vs,NULL)) == rsn) {
	        int	fl ;
	 	rs = SR_OK ;
	        *vp++ = '=' ;
	        *vp = '\0' ;
	        vl = (vp-vbuf) ;
	        while ((fl = field_sharg(fsp,ft,vp,(vlen-vl))) >= 0) {
	            if (fl > 0) {
	                vp += fl ;
	                vl += fl ;
	            }
	            if (fsp->term == '#') break ;
	        } /* end while */
		if (fl != rsn) rs = fl ;
	        *vp = '\0' ;
	        if (rs >= 0) {
	            c += 1 ;
	            rs = vecpstr_add(vlp,vbuf,vl) ;
	        }
	    } /* end if (didn't have it already) */

	} /* end if (not-overflow) */

#if	CF_DEBUGS
	debugprintf("vecpstr_enfile/subinfo_var: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_var) */


