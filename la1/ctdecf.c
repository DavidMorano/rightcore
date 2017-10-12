/* ctdecf */

/* subroutines to convert an floating-value to a decimal string */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:
	This subroutine provides a rather flexible way to get a binary floating
	point variables converted to a decimal string (with decimal points and
	the such).

	Synopsis:

	int ctdecf(dbuf,dlen,dv,fcode,w,p,fill)
	char		dbuf[] ;
	int		dlen ;
	double		dv ;
	int		fcode ;
	int		w ;
	int		p ;
	int		fill ;

	Arguments:

	dbuf		caller supplied buffer
	dlen		caller supplied buffer length
	dv		double value to convert
	fcode		type of conversion to perform: e, f, g
	w		width
	p		precision
	fill		fill indicator (-1=no-fill, 0=zero-fill)

	Returns:

	>=0		length of buffer used by the conversion
	<0		error in the conversion


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>
#include	<floatingpoint.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	FORMAT_OCLEAN
#define	FORMAT_OCLEAN	(1<<0)		/* clean data */
#endif /* FORMAT_OCLEAN */
#ifndef	FORMAT_ONOOVERR
#define	FORMAT_ONOOVERR	(1<<1)		/* do not return error on overflow */
#endif /* FORMAT_ONOOVERR */

#define	DOFLOAT_STAGELEN	(310+MAXPREC+2)
#define	DOFLOAT_DEFPREC		MIN(4,MAXPREC)

#define	MAXPREC		41		/* maximum floating precision */
#define	TMPBUFLEN	(310+MAXPREC+2)	/* must be this large for floats */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexi(const char *,int) ;
#endif


/* local structures */

struct subinfo_flags {
	uint		ov:1 ;		/* overflow */
	uint		mclean:1 ;	/* mode: clean-up */
	uint		mnooverr:1 ;	/* mode: return-error */
} ;

struct subinfo {
	SUBINFO_FL	f ;		/* flags */
	char		*ubuf ;		/* user's buffer */
	int		ulen ;		/* buffer length */
	int		len ;		/* current usage count */
	int		mode ;		/* format mode */
} ;


/* forward references */

static int subinfo_start(SUBINFO *,char *,int,int) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_strw(SUBINFO *,const char *,int) ;
static int subinfo_char(SUBINFO *,int) ;
static int subinfo_float(SUBINFO *,int,double,int,int,int,char *) ;


/* local variables */


/* exported subroutines */


int ctdecf(char *dbuf,int dlen,double dv,int fcode,int w,int p,int fill)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		len = 0 ;
	char		tmpbuf[TMPBUFLEN + 1] ;

	if (fcode == 0) fcode = 'f' ;

	if ((rs = subinfo_start(sip,dbuf,dlen,0)) >= 0) {

	    rs = subinfo_float(sip,fcode,dv,w,p,fill,tmpbuf) ;

	    len = subinfo_finish(sip) ;
	    if (rs >= 0) rs = len ;
	} /* end if (subinfo) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (ctdecf) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,char *ubuf,int ulen,int mode)
{

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->ubuf = ubuf ;
	sip->ulen = ulen ;
	sip->mode = mode ;

	sip->f.mclean = (mode & FORMAT_OCLEAN) ;
	sip->f.mnooverr = (mode & FORMAT_ONOOVERR) ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		len = sip->len ;

	sip->ubuf[len] = '\0' ;
	if (sip->f.ov) {
	    if (! sip->f.mnooverr) rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_float(sip,fcode,v,width,prec,fill,buf)
SUBINFO		*sip ;
int		fcode ;
double		v ;
int		width, prec, fill ;
char		buf[] ;
{
	int		rs = SR_OK ;
	int		i, j ;
	int		dpp ;		/* (D)ecimal (P)oint (P)osition */
	int		remlen ;
	int		stagelen ;
	int		outlen ;
	int		f_sign ;
	int		f_leading ;
	int		f_varprec ;
	int		f_varwidth ;
	char		stage[DOFLOAT_STAGELEN + 1] ;

#if	CF_DEBUGS
	fprintf(stderr,"format/subinfo_float: fcode=%c width=%d prec=%d\n",
	    fcode,width,prec) ;
#endif

	f_varprec = FALSE ;
	if (prec < 0) {
	    prec = DOFLOAT_DEFPREC ;
	    f_varprec = TRUE ;
	}

	f_varwidth = FALSE ;
	if (width <= 0) {
	    width = DOFLOAT_STAGELEN ;
	    f_varwidth = TRUE ;
	}

	if (prec > MAXPREC) prec = MAXPREC ;

/* fill up extra field width which may be specified (for some reason) */

	while ((rs >= 0) && (width > DOFLOAT_STAGELEN)) {
	    rs = subinfo_char(sip,' ') ;
	    width -= 1 ;
	} /* end while */

	if (rs >= 0) {

/* do the floating decimal conversion */

	    switch (fcode) {
	    case 'e':
	        econvert(v, prec, &dpp,&f_sign,buf) ;
	        break ;
	    case 'f':
	        fconvert(v, prec, &dpp,&f_sign,buf) ;
	        break ;
	    case 'g':
	        {
	            int	trailing = (width > 0) ;
	            gconvert(v, prec, trailing,buf) ;
	        }
	        break ;
	    } /* end switch */

#if	CF_DEBUGS
	    fprintf(stderr,"format/subinfo_float: xconvert() b=>%s<\n",buf) ;
#endif

	    remlen = width ;
	    stagelen = prec + dpp ;
	    i = DOFLOAT_STAGELEN ;
	    j = stagelen ;

/* output any characters after the decimal point */

	    outlen = MIN(stagelen,prec) ;
	    while ((remlen > 0) && (outlen > 0)) {

	        if ((! f_varprec) || (buf[j - 1] != '0')) {
	            f_varprec = FALSE ;
	            stage[--i] = buf[--j] ;
	            remlen -= 1 ;
	            outlen -= 1 ;
	        } else {
	            j -= 1 ;
	            outlen -= 1 ;
	        }

	    } /* end while */

/* output any needed zeros after the decimal point */

	    outlen = -dpp ;
	    while ((remlen > 0) && (outlen > 0)) {

	        if ((! f_varprec) || (outlen == 1)) {
	            stage[--i] = '0' ;
	            remlen -= 1 ;
	        }

	        outlen -= 1 ;

	    } /* end while */

/* output a decimal point */

	    if (remlen > 0) {
	        stage[--i] = '.' ;
	        remlen -= 1 ;
	    }

/* output any digits from the float conversion before the decimal point */

	    outlen = dpp ;
	    f_leading = (outlen > 0) ;
	    while ((remlen > 0) && (outlen > 0)) {
	        stage[--i] = buf[--j] ;
	        remlen -= 1 ;
	        outlen -= 1 ;
	    }

/* output any leading zero digit if needed */

	    if ((! f_leading) && (remlen > 0)) {
	        stage[--i] = '0' ;
	        remlen -= 1 ;
	    }

/* output any leading fill zeros if called for */

	    while ((! f_varwidth) && (fill == 0) && (remlen > 1)) {
	        stage[--i] = '0' ;
	        remlen -= 1 ;
	    }

/* output any sign if called for */

	    if (f_sign && (remlen > 0)) {
	        stage[--i] = '-' ;
	        remlen -= 1 ;
	    }

/* output any leading fill zeros if called for */

	    while ((! f_varwidth) && (fill == 0) && (remlen > 0)) {
	        stage[--i] = '0' ;
	        remlen -= 1 ;
	    }

/* output any leading blanks */

	    while ((! f_varwidth) && (remlen > 0)) {
	        stage[--i] = ' ' ;
	        remlen -= 1 ;
	    }

/* copy the stage buffer to the output buffer */

#if	CF_DEBUGS
	    fprintf(stderr,"format/subinfo_float: stage=>%s<\n",(stage+i)) ;
#endif

	    while ((rs >= 0) && (i < DOFLOAT_STAGELEN)) {
	        rs = subinfo_char(sip,stage[i++]) ;
	    }

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (subinfo_float) */


static int subinfo_char(SUBINFO *sip,int ch)
{
	int		rs = SR_OK ;
	char		buf[1] ;

	if (ch != 0) {
	    buf[0] = ch ;
	    rs = subinfo_strw(sip,buf,1) ;
	}

	return rs ;
}
/* end subroutine (subinfo_char) */


static int subinfo_strw(SUBINFO *sip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		ml = 0 ;

	if (! sip->f.ov) {
	    int	rlen = (sip->ulen - sip->len) ;

	    if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	    fprintf(stderr,"format/subinfo_strw: sl=%u\n",sl) ;
#endif

	    if (sl > rlen) sip->f.ov = TRUE ;

	    ml = MIN(sl,rlen) ;
	    if (ml > 0) {
	        char	*bp = (sip->ubuf + sip->len) ;
	        memcpy(bp,sp,ml) ;
	        sip->len += ml ;
	    }

	    if (sip->f.ov) rs = SR_OVERFLOW ;

	} else
	    rs = SR_OVERFLOW ;

#if	CF_DEBUGS
	fprintf(stderr,"format/subinfo_strw: ret rs=%d ml=%u\n",rs,ml) ;
#endif

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (subinfo_strw) */


