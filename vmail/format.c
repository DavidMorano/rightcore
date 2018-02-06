/* format */

/* subroutine to format string output */
/* last modified %G% version %I% */


#define	CF_DEBUGZ	0	/* turn on debugging */
#define	CF_DEBUGZZ	0	/* extra */
#define	CF_FLOAT	1	/* do you want floating output conversions? */
#define	CF_LIMITS	1	/* do you have a 'limits.h' include? */
#define	CF_LPRINT	0	/* the local-print subroutines */
#define	CF_LSPRINTF	1	/* use internal 'lsprintf(3c)' */
#define	CF_LVSPRINTF	0	/* local 'vsprintf(3s)'? */
#define	CF_LSNPRINTF	0	/* local 'snprintf(3s)'? */
#define	CF_LVSNPRINTF	0	/* local 'vsnprintf(3s)'? */
#define	CF_SUNCC	0	/* using Sun CC -- complains on 'lsprintf()' */
#define	CF_SPECIALHEX	1	/* perform special HEX function */
#define	CF_RESERVE	0	/* compile in 'subinfo_reserve()' */
#define	CF_CLEANSTR	1	/* clean strings */
#define	CF_BINARYMIN	1	/* perform binary conversion minimally */
#define	CF_BINCHAR	0	/* compile in 'binchar()' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        Of course, this subroutine was inspired by the UNIX® equivalent, but
        this is my own version for a. when I don't have the UNIX® libraries
        around, and b. to customize it to what I want!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used by 'printf()' type routines to format an output
	string from a format specification.  Floating point support is optional
	at compile time by using the compile time switch "CF_FLOAT".

	Synopsis:

	int format(ubuf,ulen,mode,fmt,ap)
	char		ubuf[] ;
	int		ulen ;
	int		mode ;
	const char	fmt[] ;
	va_list		ap ;

	Arguments:

	ubuf		call buffer to receive result
	ulen		length of caller buffer
	mode		formatting mode (options)
	fmt		the format string
	ap		argument-pointer (see STDARG)


	Returns:

	<0		error (if mode allows errors to be returned)
	>=0		resulting length of filled call result buffer

	Note on formatting 'modes':

	bits	description
	-----------------------------------
	1<<0	clean up bad strings (with bad characters) by substitution
	1<<1	return error on overflow


	Other options:

	There are several compile time options available through the
	switches located at the top of this file.  If you have a
	'limits.h' include file, you can optionally select to have it
	used instead of the default limits.

	The following nonstandard additions are supported:

	%t		counted byte string output like 'u_write(2)'

	Not ethat all of the following format-code additions have been
	removed since the C-89 standard (adding the 'C' format-code):

	%x.yH		terminal cursor positioning, 2 integer arguments (r.c)
	%xA		the usual cursor movements
	%xB
	%xC
	%xD
	%xK		x=0,1,2
	%xJ		x=0,1,2

	Not implemented yet:

	%{string}
	%[string]	%[3A]
			%[*ERASE]


*******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<wchar.h>
#include	<stdarg.h>

#if	CF_LIMITS
#include	<limits.h>
#endif

#if	CF_FLOAT && LOCAL_SOLARIS
#include	<floatingpoint.h>
#endif

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"format.h"


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	FMTSPEC		struct fmtspec
#define	FMTSPEC_FL	struct fmtspec_flags

#define	STRDATA		struct strdata

#undef	FD_WRITE
#define	FD_WRITE	4

#undef	CH_BADSUB
#define	CH_BADSUB	'¿'

#ifndef	SWUCHAR
#define	SWUCHAR(ch)	((ch) & 0xff)
#endif

#define	MAXLEN		(MAXPATHLEN + 40)

#define	NDF		"format.nd"

#define MAXDECDIG_I	10		/* decimal digits in 'int' */
#define MAXDECDIG_UI	10		/* decimal digits in 'uint' */
#define MAXDECDIG_L	10		/* decimal digits in 'long' */
#define MAXDECDIG_UL	10		/* decimal digits in 'ulong' */
#define MAXDECDIG_L64	19		/* decimal digits in 'long64' */
#define MAXDECDIG_UL64	20		/* decimal digits in 'ulong64' */

#define MAXDECDIG	MAXDECDIG_I	/* decimal digits in 'int' */

#define MAXOCTDIG	11		/* maximum octal digits in a long */

/* largest normal length positive integer */

#if	defined(CF_LIMITS)
#if	(! CF_LIMITS)

#ifndef	INT_MIN
#define	INT_MIN		(-2147483647-1)	/* min value of an "int" */
#define	INT_MAX		2147483647	/* max value of an "int" */
#define	UINT_MAX	4294967295U	/* max value of an "unsigned int" */
#endif

#endif /* (! CF_LIMITS) */
#endif

#ifndef	LONG_MIN
#define	LONG_MIN	(-9223372036854775807L-1LL)
#endif
#ifndef	LONG_MAX
#define	LONG_MAX	9223372036854775807LL
#endif
#ifndef	ULONG_MAX
#define	ULONG_MAX	18446744073709551615ULL
#endif

/* largest power of 10 that can fit in whatever is unsigned form */

#define SHORT_MAXPOW10	10000
#define USHORT_MAXPOW10	10000
#define INT_MAXPOW10	1000000000
#define UINT_MAXPOW10	1000000000
#define	LONG_MAXPOW10	1000000000000000000LL
#define	ULONG_MAXPOW10	10000000000000000000LL

/* BUFLEN must be large enough for both large floats and binaries */
#define	MAXPREC		41		/* maximum floating precision */
#define	BUFLEN		MAX((310+MAXPREC+2),((8*sizeof(LONG))+1))
#define	BLANKSIZE	16		/* number of blanks in 'blanks' */
#define	DEBUGBUFLEN	1024

#ifndef	NULLPOINTER
#define	NULLPOINTER	"¯NP¯"
#endif

#ifndef	NULL
#define	NULL		0
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	strlinelen(const char *,int,int) ;
extern int	hasprintbad(const char *,int) ;
extern int	isprintbad(int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGZ
extern int	zprint(cchar *,cchar *,int) ;
extern int	zprintf(cchar *,cchar *,...) ;
extern int	mkhexstr(char *,int,void *,int) ;
extern int	hexblock(const char *,void *,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		ov:1 ;		/* overflow */
	uint		mclean:1 ;	/* mode: clean-up */
	uint		mnooverr:1 ;	/* mode: return-error */
} ;

struct subinfo {
	SUBINFO_FL	f ;	/* flags */
	char		*ubuf ;		/* user's buffer */
	int		ulen ;		/* buffer length */
	int		len ;		/* current usage count */
	int		mode ;		/* format mode */
} ;

struct fmtspec_flags {
	uint		alternate:1 ;	/* alternate form */
	uint		zerofill:1 ;	/* zero fill on left */
	uint		plus:1 ;	/* leading plus sign */
	uint		minus:1 ;	/* leading minus sign */
	uint		left:1 ;	/* left justified */
	uint		thousands:1 ;	/* group by thousands */
	uint		space:1 ;	/* leading space */
	uint		isdigit:1 ;	/* is a digit-type character */
} ;

struct fmtspec {
	FMTSPEC_FL	f ;
	int		fcode ;		/* the format "code" */
	int		width ;		/* <0 turns off */
	int		prec ;		/* <0 turns off */
	int		lenmod ;	/* length modifier */
} ;

struct strdata {
	const wint_t	*lsp ;
	const wchar_t	*wsp ;
	const char	*sp ;
	int		sl ;
	int		f_wint ;
	int		f_wchar ;
} ;

enum lenmods {
	lenmod_none,
	lenmod_half,
	lenmod_long,
	lenmod_longlong,
	lenmod_longdouble,
	lenmod_wide,
	lenmod_overlast

} ;


/* forward references */

#if	CF_BINCHAR
static ULONG	rshiftul(ULONG,int) ;
#endif

static int subinfo_start(SUBINFO *,char *,int,int) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_strw(SUBINFO *,const char *,int) ;
static int subinfo_char(SUBINFO *,int) ;
static int subinfo_blanks(SUBINFO *,int) ;
static int subinfo_cleanstrw(SUBINFO *,const char *,int) ;
static int subinfo_emit(SUBINFO *,FMTSPEC *,const char *,int) ;
static int subinfo_fmtstr(SUBINFO *,FMTSPEC *,STRDATA *) ;

#if	CF_RESERVE
static int subinfo_reserve(SUBINFO *,int) ;
#endif

#if	CF_BINCHAR
static int	binchar(ULONG,int) ;
#endif

#if	CF_FLOAT && LOCAL_SOLARIS
static int	subinfo_float(SUBINFO *,int,double,int,int,int,char *) ;
#endif

#if	CF_CLEANSTR
static int	hasourbad(cchar *sp,int sl) ;
#endif /* CF_CLEANSTR */

static int	isourbad(int ch) ;

#if	CF_LPRINT
#if	((CF_LSPRINTF || CF_LVSPRINTF || CF_LSNPRINTF || CF_LVSNPRINTF))
#if	(! CF_SUNCC)
static int	lsprintf(char *,const char *,...) ;
#endif
#endif
#endif /* CF_LPRINT */


/* local variables */

static const char	digtable_hi[] = "0123456789ABCDEF" ;
static const char	digtable_lo[] = "0123456789abcdef" ;
static const char	blanks[] = "                " ;
static const char	*nullpointer = NULLPOINTER ;


/* exported subroutines */


int format(char *ubuf,int ulen,int mode,cchar *fmt,va_list ap)
{
	SUBINFO		si, *sip = &si ;
	FMTSPEC		fs, *fsp = &fs ;
	const int	tlen = BUFLEN ;
	int		rs = SR_OK ;
	int		fmtlen = 0 ;
	const char	*tp ;
#if	CF_DEBUGZ
	const int	dlen = DEBUGBUFLEN ;
	char		dbuf[DEBUGBUFLEN+1] ;
#endif
	char		tbuf[BUFLEN+1] ;	/* must be > MAXDECDIG */

#if	CF_DEBUGZ
	if (fmt != NULL) {
	    int dl = strlinelen(fmt,-1,70) ;
	    strdcpy1(dbuf,dlen,fmt) ;
	    dbuf[dl] = '\0' ;
	    zprintf(NDF,"format: ent dl=%d\n",dl) ;
	    zprintf(NDF,"format: fmt=>%s<\n",dbuf) ;
	} else {
	    zprintf(NDF,"format: ent fmt=>%s<\n",nullpointer) ;
	}
#endif /* CF_DEBUGZ */

	if (ubuf == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (fmt[0] == '\0') return SR_INVALID ;

	if ((rs = subinfo_start(sip,ubuf,ulen,mode)) >= 0) {

/* go through the loops */

	    while ((rs >= 0) && *fmt && ((tp = strchr(fmt,'%')) != NULL)) {
	        int	bl = 0 ;
	        int	nfmt ;
	        int	fcode ;
	        char	*bp = tbuf ;

	        if ((tp-fmt) > 0) {
	            rs = subinfo_cleanstrw(sip,fmt,(tp-fmt)) ;
	        }
	        fmt = (tp+1) ;
	        if (rs < 0) break ;

	        if (fmt[0] == '\0') break ;

/* create the format specification that we will use later */

	        {
	            int	f_continue = FALSE ;
#if	CF_DEBUGZ && CF_DEBUGZZ
	            int	f_special = FALSE ;
#endif

	            cchar	*fp = fmt ;

	            memset(fsp,0,sizeof(FMTSPEC)) ;
	            fsp->width = -1 ;
	            fsp->prec = -1 ;

	            f_continue = TRUE ;
	            while (f_continue) {
	                const int	ch = MKCHAR(fp[0]) ;
	                switch (ch) {
	                case '-':
	                    fsp->f.left = TRUE ;
	                    break ;
	                case '+':
	                    fsp->f.plus = TRUE ;
	                    break ;
	                case '\'':
	                    fsp->f.thousands = TRUE ;
	                    break ;
	                case '0':
	                    fsp->f.zerofill = TRUE ;
	                    break ;
	                case '#':
	                    fsp->f.alternate = TRUE ;
	                    break ;
	                case ' ':
	                    fsp->f.space = TRUE ;
	                    break ;
	                default:
	                    f_continue = FALSE ;
	                    break ;
	                } /* end switch */
	                if (f_continue) {
#if	CF_DEBUGZ && CF_DEBUGZZ
	                    f_special = TRUE ;
#endif
	                    fp += 1 ;
	                }
	            } /* end while */

#if	CF_DEBUGZ && CF_DEBUGZZ
	            if (f_special) {
	                zprintf(NDF,"format: f_left=%u\n",fsp->f.left) ;
	                zprintf(NDF,"format: f_plus=%u\n",fsp->f.plus) ;
	                zprintf(NDF,"format: f_thousands=%u\n",
	                    fsp->f.thousands) ;
	                zprintf(NDF,"format: f_zerofill=%u\n",
	                    fsp->f.zerofill) ;
	                zprintf(NDF,"format: f_alternate=%u\n",
	                    fsp->f.alternate) ;
	                zprintf(NDF,"format: f_space=%u\n",fsp->f.space) ;
	            }
#endif /* CF_DEBUGZ */

/* now comes a digit string which may be a '*' */

	            {
	                int	width = -1 ;
	                if (*fp == '*') {
	                    width = (int) va_arg(ap,int) ;
	                    fp += 1 ;
	                    if (width < 0) {
	                        width = -width ;
	                        fsp->f.left = (! fsp->f.left) ;
	                    }
	                } else if ((*fp >= '0') && (*fp <= '9')) {
	                    width = 0 ;
	                    while ((*fp >= '0') && (*fp <= '9')) {
	                        width = width * 10 + (*fp++ - '0') ;
	                    }
	                } /* end if (width) */

	                if (width >= 0) fsp->width = width ;

	            } /* end if (width) */

/* maybe a decimal point followed by more digits (or '*') */

	            if (*fp == '.') {
	                int	prec = -1 ;
	                fp += 1 ;
	                if (*fp == '*') {
	                    prec = (int) va_arg(ap,int) ;
	                    fmt += 1 ;
	                } else { /* the default if nothing is zero-precision */
	                    prec = 0 ; /* default if nothing specified */
	                    while ((*fp >= '0') && (*fp <= '9')) {
	                        prec = prec * 10 + (*fp++ - '0') ;
	                    }
	                }
	                if (prec >= 0) fsp->prec = prec ;

	            } /* end if (a precision was specified) */

/* check for a format length-modifier */

	            {
	                const int	ch = MKCHAR(*fp) ;

	                switch (ch) {
	                case 'h':
	                    fsp->lenmod = lenmod_half ;
	                    fp += 1 ;
	                    break ;
	                case 'l':
	                    fsp->lenmod = lenmod_long ;
	                    fp += 1 ;
	                    break ;
	                case 'L':
	                    fsp->lenmod = lenmod_longlong ;
	                    fp += 1 ;
	                    break ;
	                case 'D':
	                    fsp->lenmod = lenmod_longdouble ;
	                    fp += 1 ;
	                    break ;
	                case 'w':
	                    fsp->lenmod = lenmod_wide ;
	                    fp += 1 ;
	                    break ;
	                } /* end switch */

	                if (*fp == 'l') {
	                    enum lenmods m = fsp->lenmod ;
	                    if (m == lenmod_long) {
	                        fsp->lenmod = lenmod_longlong ;
	                    } else {
	                        fsp->lenmod = lenmod_long ;
	                    }
	                    fp += 1 ;
	                }

	            } /* end block (length specifier) */

/* finally the format code itself */

	            fsp->fcode = MKCHAR(*fp++) ;
	            nfmt = (fp - fmt) ;
	        } /* end block (loading up the FMTSPEC object) */
	        if (nfmt == 0) break ;

	        fmt += nfmt ;
	        fcode = fsp->fcode ;

#if	CF_DEBUGZ
	        {
	            int	fc = (isprintlatin(fcode) ? fcode : '¿') ;
	            zprintf(NDF,"format: switch fcode=%c (\\x%04X)\n",
	                fc,fcode) ;
	        }
#endif /* CF_DEBUGZ */

	        switch (fcode) {

	        case 0:
	            break ;

	        case '%':
	            fcode = 0 ;
	            rs = subinfo_char(sip,'%') ;
	            break ;

		case '_':
		    {
			tbuf[0] = ' ' ;
			bp = tbuf ;
			bl = 1 ;
		    }
		    break ;
/* handle "c"haracter type numbers */
	        case 'C':
	        case 'c':
	            {
	                int	ch ;
	                int	f_wchar = FALSE ;
	                int	f_wint = FALSE ;

#if	CF_DEBUGZ
	                zprintf(NDF,"format: got a character\n") ;
#endif

	                if (fsp->fcode != 'C') {
	                    switch (fsp->lenmod) {
	                    case lenmod_wide:
	                        f_wchar = TRUE ;
	                        break ;
	                    case lenmod_long:
	                        f_wint = TRUE ;
	                        break ;
	                    }
	                } else {
	                    f_wchar = TRUE ;
			}

	                if (f_wint) {
	                    ch = (int) va_arg(ap,wint_t) ;
	                } else if (f_wchar) {
	                    ch = (int) va_arg(ap,wchar_t) ;
	                } else {
	                    ch = (int) va_arg(ap,int) ;
	                }

/* see if we can handle the general case easily */

	                ch &= 0xff ;
	                if ((fsp->width <= 1) && (fsp->prec <= 1)) {
	                    fcode = 0 ;
	                    if (sip->f.mclean) {
	                        if ((ch != '\n') && isourbad(ch)) {
	                            ch = CH_BADSUB ;
	                        }
	                    }
	                    rs = subinfo_char(sip,ch) ;
	                } else {
	                    tbuf[0] = ch ;
	                    bp = tbuf ;
	                    bl = 1 ;
	                } /* end if */

	            } /* end block */
	            break ;

/* end of "c"haracter type arguments */

/* handle "s"tring type arguments */
	        case 'S':
	        case 's':
	        case 't':
	            {
	                STRDATA	sd ;

	                memset(&sd,0,sizeof(STRDATA)) ;
	                sd.sl = -1 ;

	                if (fsp->fcode == 'S') {
	                    sd.f_wchar = TRUE ;
	                } else {
	                    switch (fsp->lenmod) {
	                    case lenmod_long:
	                        sd.f_wint = TRUE ;
	                        break ;
	                    case lenmod_wide:
	                        sd.f_wchar = TRUE ;
	                        break ;
	                    } /* end switch */
	                } /* end if (wide or narrow) */

	                if (sd.f_wint) {
	                    sd.lsp = (const wint_t *) va_arg(ap,wint_t *) ;
	                } else if (sd.f_wchar) {
	                    sd.wsp = (const wchar_t *) va_arg(ap,wchar_t *) ;
	                } else {
	                    sd.sp = (const char *) va_arg(ap,char *) ;
	                }

	                if (fsp->fcode == 't') {
	                    sd.sl = (int) va_arg(ap,int) ;
	                }

#ifdef	COMMENT /* null-pointer check (done later) */
	                {
	                    const void *p = sd.sp ;
	                    if (sd.f_wint) {
	                        p = sd.lsp ;
	                    } else if (sd.f_wchar) {
	                        p = sd.wsp ;
	                    }
	                    if ((p == NULL) && (sd.sl != 0)) {
	                        sd.lsp = NULL ;
	                        sd.wsp = NULL ;
	                        sd.sp = nullpointer ;
	                        sd.sl = -1 ;
	                    }
	                } /* end block */
#endif /* COMMENT */

#if	CF_DEBUGZ
	                {
	                    int	dl ;
			    if (sd.sp == NULL) sd.sp = "*NULL*" ;
	                    dl = strlinelen(sd.sp,sd.sl,40) ;
	                    strdcpy1(dbuf,dlen,sd.sp) ;
	                    dbuf[dl] = '\0' ;
	                    zprintf(NDF,"format: s sp{%p}\n",sd.sp) ;
	                    zprintf(NDF,"format: s sl=%d sp{%p}=>%s<\n",
	                        sd.sl,sd.sp,dbuf) ;
	                }
#endif /* CF_DEBUGZ */

	                rs = subinfo_fmtstr(sip,fsp,&sd) ;
	                fcode = 0 ;
	            }
	            break ;

/* handle "o"ctal numbers */
	        case 'o':
	            {
	                ULONG	unum ;
	                int	lenmod = fsp->lenmod ;
	                int	nd = 0 ;
	                int	i ;
	                cchar	*digtable = digtable_lo ;

	                switch (lenmod) {
	                case lenmod_longlong:
	                    unum = (ULONG) va_arg(ap,ULONG) ;
	                    nd = (((8*sizeof(ULONG))+2)/3) ;
	                    break ;
	                case lenmod_long:
	                    unum = (ULONG) va_arg(ap,ulong) ;
	                    nd = (((8*sizeof(ulong))+2)/3) ;
	                    break ;
	                default:
	                    unum = (ULONG) va_arg(ap,uint) ;
	                    nd = (((8*sizeof(uint))+2)/3) ;
	                    if (lenmod == lenmod_half) {
	                        nd = (((8*sizeof(ushort))+2)/3) ;
	                    }
	                    break ;
	                } /* end switch */

/* form the digits that we will (maximally) need */

	                bp = (tbuf + tlen) ;
	                *bp = '\0' ;
	                for (i = (nd-1) ; (unum > 0) && (i >= 0) ; i -= 1) {
	                    *--bp = digtable[unum & 7] ;
	                    unum >>= 3 ;
	                } /* end for (making the digits) */
	                if (i == 0) *--bp = '0' ;

#if	CF_DEBUGZ
			if (bp == NULL) bp = "*NULL*" ;
	                zprintf(NDF,"format: nd=%u b=%s\n",nd,bp) ;
#endif

	                bl = ((tbuf+tlen) - bp) ;

	            } /* end block */
	            break ;

/* handle he"x"adecimal numbers */
	        case 'p':
	        case 'P':
	        case 'x':
	        case 'X':
	            {
	                ULONG	unum ;
	                int	lenmod = fsp->lenmod ;
	                int	ndigits = 0 ;
	                int	i ;
	                int	f_lc = ((fcode == 'p') || (fcode == 'x')) ;
	                int	f_special = CF_SPECIALHEX ;
	                cchar	*digtable ;

	                if ((fcode == 'p') || (fcode == 'P')) {
	                    ulong	ul ;
	                    ul = (ulong) va_arg(ap,void *) ;
	                    unum = (ULONG) ul ;
	                    ndigits = (2 * sizeof(void *)) ;
	                } else {
	                    switch (lenmod) {
	                    case lenmod_longlong:
	                        unum = (ULONG) va_arg(ap,ULONG) ;
	                        ndigits = (2 * sizeof(ULONG)) ;
	                        break ;
	                    case lenmod_long:
	                        unum = (ULONG) va_arg(ap,ulong) ;
	                        ndigits = (2 * sizeof(ulong)) ;
	                        break ;
	                    default:
	                        unum = (ULONG) va_arg(ap,uint) ;
	                        ndigits = (2 * sizeof(uint)) ;
	                        if (lenmod == lenmod_half) {
	                            ndigits = (2 * sizeof(ushort)) ;
	                        }
	                        break ;
	                    } /* end switch */
	                } /* end if */

	                digtable = (f_lc) ? digtable_lo : digtable_hi ;

/* form the digits that we will (maximally) need */

	                bp = (tbuf + tlen) ;
	                *bp = '\0' ;
	                for (i = (ndigits-1) ; i >= 0 ; i -= 1) {

	                    *--bp = digtable[unum & 0x0F] ;
	                    unum >>= 4 ;
	                    if ((! f_special) && (unum == 0)) break ;

	                } /* end for (making the digits) */

#if	CF_DEBUGZ
			if (bp == NULL) bp = "*NULL*" ;
	                zprintf(NDF,"format: ndigits=%u b=%s\n",ndigits,bp) ;
#endif

	                bl = (bp-(tbuf+tlen)) ;

	            } /* end block */
	            break ;

/* handle decimal */
	        case 'u':
	        case 'd':
	        case 'i':
	            {
	                LONG	num = 0 ;
	                ULONG	unum = 0 ;
	                int	f_signed = ((fcode == 'i') || (fcode == 'd')) ;

	                switch (fsp->lenmod) {
	                case lenmod_longlong:
	                    if (f_signed) {
	                        num = (LONG) va_arg(ap,LONG) ;
	                    } else {
	                        unum = (ULONG) va_arg(ap,ULONG) ;
	                    }
	                    break ;
	                case lenmod_long:
	                    if (f_signed) {
	                        num = (LONG) va_arg(ap,long) ;
	                    } else {
	                        unum = (ULONG) va_arg(ap,ulong) ;
	                    }
	                    break ;
	                default:
	                    if (f_signed) {
	                        num = (LONG) va_arg(ap,int) ;
	                    } else {
	                        unum = (ULONG) va_arg(ap,uint) ;
	                    }
	                    if (fsp->lenmod == lenmod_half) {
	                        unum &= USHORT_MAX ;
	                        num &= USHORT_MAX ;
	                    }
	                    break ;
	                } /* end switch */

/* if signed, is the number negative? */

	                if (f_signed) {
	                    unum = (ULONG) num ;
	                    if (num < 0) unum = (- unum) ;
	                }

	                tbuf[tlen] = '\0' ;
	                bp = ulltostr(unum,(tbuf+tlen)) ;

	                if (f_signed && (num < 0)) {
	                    *--bp = '-' ;
	                    fsp->f.minus = TRUE ;
	                }

	                bl = ((tbuf+tlen) - bp) ;

	            } /* end block */
	            break ;

/* handle binary numbers */
	        case SWUCHAR('ß'):
	            {
	                ULONG	unum ;
	                int		ndigits = 0 ;
	                int		i ;
	                int		ch ;
#if	CF_BINARYMIN
#else /* CF_BINARYMIN */
	                int		f_special = CF_SPECIALHEX ;
#endif /* CF_BINARYMIN */
	                int		f_got = FALSE ;
	                const char	*digtable = digtable_lo ;

	                switch (fsp->lenmod) {
	                case lenmod_longlong:
	                    unum = (ULONG) va_arg(ap,LONG) ;
	                    ndigits = (8 * sizeof(LONG)) ;
	                    break ;
	                case lenmod_long:
	                    unum = (ULONG) va_arg(ap,long) ;
	                    ndigits = (8 * sizeof(long)) ;
	                    break ;
	                default:
	                    unum = (ULONG) va_arg(ap,int) ;
	                    ndigits = (8 * sizeof(int)) ;
	                    if (fsp->lenmod == lenmod_half) {
	                        unum &= USHORT_MAX ;
	                        ndigits = (8 * sizeof(short)) ;
	                    }
	                    break ;
	                } /* end switch */

#if	CF_DEBUGZ
	                zprintf(NDF,"format: binary ndigits=%u\n",ndigits) ;
#endif

#if	CF_BINARYMIN
	                {
	                    bp = tbuf ;

	                    if ((sizeof(LONG) > 4) && (ndigits > 32)) {
	                        for (i = 63 ; i >= 32 ; i -= 1) {
	                            ch = digtable[(unum>>i) & 1] ;
	                            if (f_got || (ch != '0')) {
	                                f_got = TRUE ;
	                                *bp++ = ch ;
	                            }
	                        }
	                    } /* end if */

	                    if (ndigits > 16) {
	                        for (i = 31 ; i >= 16 ; i -= 1) {
	                            ch = digtable[(unum>>i) & 1] ;
	                            if (f_got || (ch != '0')) {
	                                f_got = TRUE ;
	                                *bp++ = ch ;
	                            }
	                        }
	                    } /* end if */

	                    for (i = 15 ; i >= 0 ; i -= 1) {
	                        ch = digtable[(unum>>i) & 1] ;
	                        if (f_got || (ch != '0')) {
	                            f_got = TRUE ;
	                            *bp++ = ch ;
	                        }
	                    } /* end for */

	                    if (! f_got) {
	                        *bp++ = '0' ;
	                    }

	                    *bp = '\0' ;

#if	CF_DEBUGZ
	                    zprintf(NDF,"format: tbuf=>%s<\n",tbuf) ;
#endif

	                    bl = (bp - tbuf) ;
	                    bp = tbuf ;
	                }
#else /* CF_BINARYMIN */
	                {

/* form the digits that we will (maximally) need */

	                    bp = (tbuf + tlen) ;
	                    *bp = '\0' ;
	                    for (i = (ndigits-1) ; i >= 0 ; i -= 1) {
	                        *--bp = digtable[unum & 1] ;
	                        unum >>= 1 ;
	                        if ((! f_special) && (unum == 0)) break ;
	                    } /* end for (making the digits) */

#if	CF_DEBUGZ
			    if (bp == NULL) bp = "*NULL*" ;
	                    zprintf(NDF,"format: ndigits=%u b=%s\n",
				ndigits,bp) ;
#endif

	                    bl = (bp-(tbuf+tlen)) ;
	                }
#endif /* CF_BINARYMIN */

	            } /* end block (binary) */
	            break ;

/* handle floating point */
	        case 'e':
	        case 'f':
	        case 'g':
	            {
	                double	dv ;

#if	CF_DEBUGZ
	                zprintf(NDF,"format: float fcode=%c\n",fcode) ;
#endif

	                dv = (double) va_arg(ap,double) ;

#if	CF_FLOAT && LOCAL_SOLARIS
	                {
			    const int	w = fsp->width ;
			    const int	p = fsp->prec ;
	                    int		fill = -1 ;
	                    if (fsp->f.zerofill) fill = 0 ;
	                    rs = subinfo_float(sip,fcode,dv,w,p,fill,tbuf) ;
	                }
#else
	                {
	                    const char	*cp ;
	                    cp = "* no floating support *" ;
	                    rs = subinfo_strw(sip,cp,-1) ;
	                }
#endif /* CF_FLOAT */

	                fcode = 0 ;	/* no other output */
	            } /* end block */
	            break ;

#ifdef	COMMENT /* terminal codes are no longer supported! */

/* handle terminal cursor positioning */
	        case 'A':
	        case 'B':
	        case 'C':
	        case 'D':
	        case 'J':
	        case 'K':
	        case 'L':
	        case 'M':
	        case '@':
	        case '~':
	            {
	                int	width = fsp->width ;

	                if (width <= 1) {
	                    rs = lsprintf(buf,"\033[%c",fcode) ;
	                    bl = rs ;
	                } else {
	                    rs = lsprintf(buf,"\033[%u%c",width,fcode) ;
	                    bl = rs ;
	                }

	                if (rs >= 0) {
	                    if ((rs = subinfo_reserve(sip,bl)) >= 0) {
	                        rs = subinfo_strw(sip,bp,bl) ;
	                    }
	                }

	                fcode = 0 ;	/* no other output */
	            } /* end block */
	            break ;

/* absolute terminal cursor positioning */
	        case 'H':
	            {
	                int		width = fsp->width ;

	                if (width <= 1) {
	                    rs = lsprintf(buf,"\033[H") ;
	                    bl = rs ;
	                } else if (prec <= 1) {
	                    rs = lsprintf(buf,"\033[%uH",width) ;
	                    bl = rs ;
	                } else {
	                    rs = lsprintf(buf,"\033[%u;%uH",width,prec) ;
	                    bl = rs ;
	                }

	                if (rs >= 0) {
	                    rs = subinfo_reserve(sip,bl) ;
	                    if (rs >= 0)
	                        rs = subinfo_strw(sip,bp,bl) ;
	                }

	                fcode = 0 ;	/* no other output */
	            } /* end block */
	            break ;

#endif /* COMMENT */ /* terminal codes are no longer supported! */

#ifdef	COMMENT /* conflicts with another use of 'm' key letter! */

/* character rendition */
	        case 'm':
	            {
	                if (width <= 0) {
	                    lsprintf(buf,"\033[%c",fcode) ;
	                } else {
	                    lsprintf(buf,"\033[%d%c",width,fcode) ;
	                }
	                rs = storestring(&ld,buf) ;
	                fcode = 0 ;	/* no other output */
	            }
	            break ;

#endif /* COMMENT */

/* not a format-code character error out -- always */
	        default:
	            rs = SR_NOMSG ;
	            break ;

	        } /* end switch (handling this format code) */

/* determine if there are characters to output based on 'fcode' variable */

#if	CF_DEBUGZ
	        zprintf(NDF,"format: switch-out rs=%d fcode=%c (\\x%04X)\n",
	            rs,(isprintlatin(fcode) ? fcode : '°'),fcode) ;
#endif

	        if ((rs >= 0) && (fcode != '\0') && (bp != NULL)) {
	            rs = subinfo_emit(sip,fsp,bp,bl) ;

#if	CF_DEBUGZ
	            zprintf(NDF,"format: subinfo_emit() rs=%d\n",rs) ;
#endif

	        }

	    } /* end while (infinite loop) */

#if	CF_DEBUGZ
	    zprintf(NDF,"format: while-out rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && (*fmt != '\0')) {
	        rs = subinfo_cleanstrw(sip,fmt,-1) ;
	    }

	    fmtlen = subinfo_finish(sip) ;
	    if (rs >= 0) rs = fmtlen ;
	} /* end if (subinfo) */

#if	CF_DEBUGZ
	zprintf(NDF,"format: subinfo_finish() rs=%d\n",rs) ;
#endif

/* we are out of here! */

#if	CF_DEBUGZ
	{
	    int dl = strlinelen(ubuf,fmtlen,40) ;
	    strdcpy1(dbuf,dlen,ubuf) ;
	    dbuf[dl] = '\0' ;
	    zprintf(NDF,"format: ubuf=>%s<\n",dbuf) ;
	    zprintf(NDF,"format: ret rs=%d fmtlen=%u\n",rs,fmtlen) ;
	}
#endif /* CF_DEBUGZ */

	return (rs >= 0) ? fmtlen : rs ;
}
/* end subroutine (format) */


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


#if	CF_RESERVE

static int subinfo_reserve(SUBINFO *sip,int n)
{
	int		rs = SR_OK ;

	if (! sip->f.ov) {
	    int	rlen = (sip->ulen - sip->len) ;
	    if (n > rlen) {
	        sip->f.ov = TRUE ;
	        rs = SR_OVERFLOW ;
	    }
	} else
	    rs = SR_OVERFLOW ;

	return rs ;
}
/* end subroutine (subinfo_reserve) */

#endif /* CF_RESERVE */


static int subinfo_strw(SUBINFO *sip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		ml = 0 ;

#if	CF_DEBUGZ
	if (sp == NULL) sp = "*NULL*" ;
	zprintf(NDF,"format/subinfo_strw: ent sl=%d s=>%*s<\n",sl,sl,sp) ;
#endif

	if (! sip->f.ov) {
	    int		rlen ;
	    if (sl < 0) sl = strlen(sp) ;
	    rlen = (sip->ulen - sip->len) ;
	    if (sl > rlen) sip->f.ov = TRUE ;
	    ml = MIN(sl,rlen) ;
	    if (ml > 0) {
	        char	*bp = (sip->ubuf + sip->len) ;
#if	CF_DEBUGZ
	zprintf(NDF,"format/subinfo_strw: wri ml=%d s=>%*s<\n",ml,ml,sp) ;
#endif
	        memcpy(bp,sp,ml) ;
	        sip->len += ml ;
	    }
	    if (sip->f.ov) rs = SR_OVERFLOW ;
	} else {
	    rs = SR_OVERFLOW ;
	}

#if	CF_DEBUGZ
	zprintf(NDF,"format/subinfo_strw: ret rs=%d ml=%u\n",rs,ml) ;
#endif

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (subinfo_strw) */


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


static int subinfo_blanks(SUBINFO *sip,int n)
{
	int		rs = SR_OK ;
	int		nr = n ;

	while ((rs >= 0) && (nr > 0)) {
	    int m = MIN(BLANKSIZE,nr) ;
	    rs = subinfo_strw(sip,blanks,m) ;
	    nr -= m ;
	} /* end while */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (subinfo_blanks) */


static int subinfo_cleanstrw(SUBINFO *sip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	char		*abuf = NULL ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_CLEANSTR
	if (sip->f.mclean) {
	    int		hl = sl ;
	    int		f_eol = FALSE ;
	    if ((sl > 0) && (sp[sl-1] == '\n')) {
	        hl = (sl-1) ;
	        f_eol = TRUE ;
	    }
	    if (hasourbad(sp,hl)) {
	        int	size = (sl+1) ;
	        if ((rs = uc_malloc(size,&abuf)) >= 0) {
	            int		i, ch ;
	            for (i = 0 ; (i < hl) && *sp ; i += 1) {
	                ch = MKCHAR(sp[i]) ;
	                if (isourbad(ch)) ch = CH_BADSUB ;
	                abuf[i] = (char) ch ;
	            }
	            if (f_eol) abuf[i++] = '\n' ;
	            sl = i ;
	            sp = abuf ;
	        }
	    } /* end if (hasourbad) */
	} /* end if (option-clean) */
#endif /* CF_CLEANSTR */

	if (rs >= 0) {
	    rs = subinfo_strw(sip,sp,sl) ;
	    len = rs ;
	}

	if (abuf != NULL) uc_free(abuf) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_cleanstrw) */


static int subinfo_fmtstr(SUBINFO *sip,FMTSPEC *fsp,STRDATA *sdp)
{
	int		rs = SR_OK ;
	int		width = fsp->width ;
	int		prec = fsp->width ;
	int		sl = sdp->sl ;
	int		f_wint = sdp->f_wint ;
	int		f_wchar = sdp->f_wchar ;
	int		f_memalloc = FALSE ;
	int		fcode = 0 ;
	cchar		*sp = sdp->sp ;

#if	CF_DEBUGZ
	zprintf(NDF,"format/_fmtstr: ent\n") ;
#endif

/* possible necessary (at this time) conversion to regular characters */

	if (f_wint || f_wchar) {
	    const wint_t	*lsp = sdp->lsp ;
	    const wchar_t	*wsp = sdp->wsp ;
	    int			i = 0 ;
	    int			f_notnull = FALSE ;
	    if (f_wint) {
	        f_notnull = (lsp != NULL) ;
	        if (f_notnull) {
	            while (sl && (lsp[i] != 0)) {
	                i += 1 ;
	                sl -= 1 ;
	            }
	        }
	    } else {
	        f_notnull = (wsp != NULL) ;
	        if (f_notnull) {
	            while (sl && (wsp[i] != 0)) {
	                i += 1 ;
	                sl -= 1 ;
	            }
	        }
	    }
	    if (f_notnull) {
	        int 	size = (i + 1) * sizeof(char) ;
	        char	*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            int		j ;
	            int		ch ;
	            f_memalloc = TRUE ;
	            sp = p ;
	            sl = i ;
	            if (f_wint) {
	                for (j = 0 ; j < i ; j += 1) {
	                    if ((ch = (int) lsp[j]) <= UCHAR_MAX) {
	                        p[j] = (char) ch ;
	                    } else {
	                        p[j] = '¿' ;
	                    }
	                } /* end for */
	            } else {
	                for (j = 0 ; j < i ; j += 1) {
	                    if ((ch = (int) wsp[j]) <= UCHAR_MAX) {
	                        p[j] = (char) ch ;
	                    } else {
	                        p[j] = '¿' ;
	                    }
	                } /* end for */
	            } /* end if */
	            p[j] = 0 ;
	        } /* end if (memory-allocation) */
	    } /* end if (not-null) */
	} /* end if ('wint' or 'wchar') */

	if (rs >= 0) {

/* continue with normal character processing */

	    if ((sp == NULL) && (sl != 0)) {
	        sp = nullpointer ;
	        sl = -1 ;
	        width = -1 ;
	        prec = -1 ;
	    }

/* currently this is not needed if we did the string conversion above */

	    if ((sl != 0) && (! (f_wint || f_wchar))) {
	        sl = strnlen(sp,sl) ;
	    }

/* modify the string length based on the precision (truncate on left) */

	    if ((prec >= 0) && (sl > prec)) {
	        sp += (sl-prec) ;
	        sl = prec ;
	    }

	    if ((width > 0) && (sl > width)) width = sl ; /* the standard! */

	} /* end if (ok) */

/* continue normally */

	if ((rs >= 0) && (! fsp->f.left)) {
	    if ((width > 0) && (width > sl)) {
	        rs = subinfo_blanks(sip,(width - sl)) ;
	    }
	}

	if (rs >= 0) {
	    rs = subinfo_cleanstrw(sip,sp,sl) ;
	}

	if ((rs >= 0) && fsp->f.left) {
	    if ((width > 0) && (width > sl)) {
	        rs = subinfo_blanks(sip,(width - sl)) ;
	    }
	}

	if (f_memalloc && (sp != NULL)) uc_free(sp) ;

#if	CF_DEBUGZ
	zprintf(NDF,"format/_fmtstr: ret rs=%d fcode=\\x%04x\n",rs,fcode) ;
#endif

	return (rs >= 0) ? fcode : rs ;
}
/* end subroutine (subinfo_fmtstr) */


static int subinfo_emit(SUBINFO *sip,FMTSPEC *fsp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		fcode = fsp->fcode ;
	int		width = fsp->width ;
	int		prec = fsp->prec ;
	int		npad = 0 ;
	int		f_zerofill = fsp->f.zerofill ;
	int		f_plus = fsp->f.plus ;
	int		f_minus = fsp->f.minus ;
	int		f_plusminus = FALSE ;
	int		f_truncleft = FALSE ;
	int		f_isdigital = FALSE ;
	int		f_specialhex = FALSE ;

	if (sp == NULL) return SR_FAULT ;

#ifdef	COMMENT
	sl = strnlen(sp,sl) ;
#else
	if (sl < 0) sl = strlen(sp) ;
#endif

#if	CF_DEBUGZ 
	{
	    int 	dl = strlinelen(sp,sl,40) ;
	    char	dbuf[DEBUGBUFLEN + 1] ;
	    strdcpy1w(dbuf,DEBUGBUFLEN,sp,dl) ;
	    zprintf(NDF,"format/subinfo_emit: fcode=%c (\\x%04X)\n",
	        fcode,fcode) ;
	    zprintf(NDF,"format/subinfo_emit: sl=%d sp=>%s<\n",sl,dbuf) ;
	    zprintf(NDF,"format/subinfo_emit: width=%d prec=%d\n",
	        width,prec) ;
	    zprintf(NDF,"format/subinfo_emit: f_zerofill=%u\n",f_zerofill) ;
	}
#endif /* CF_DEBUGZ */

/* evaluate and setup */

	switch (fcode) {
	case 'S':
	case 's':
	case 't':
	case 'i':
	case 'd':
	case 'u':
	case SWUCHAR('ß'):
	case 'o':
	case 'X':
	case 'x':
	case 'P':
	case 'p':
	    f_truncleft = TRUE ;
	} /* end switch */

	switch (fcode) {
	case 'i':
	case 'd':
	case 'u':
	case SWUCHAR('ß'):
	case 'o':
	case 'e':
	case 'g':
	case 'f':
	case 'E':
	case 'G':
	    f_isdigital = TRUE ;
	} /* end switch */

	switch (fcode) {
	case SWUCHAR('ß'):
	case 'X':
	case 'x':
	case 'P':
	case 'p':
	    f_specialhex = TRUE ;
	    if (width < 0) {
	        width = sl ;
	        f_zerofill = TRUE ;
	    }
	} /* end switch */

	if (fcode > 0) {

	    if ((sl > 0) && f_isdigital) {
	        int f_p = (*sp == '+') ;
	        int f_m = (*sp == '-') ;
	        if (f_p || f_m) {
	            f_plus = f_p ;
	            f_minus = f_m ;
	            sp += 1 ;
	            sl -= 1 ;
	        }
	    } /* end if */

	    if (prec >= 0) {
	        if (sl > prec) {
	            if (f_truncleft) {
	                sp += (sl-prec) ; /* truncate on left */
	                sl -= (sl-prec) ;
	            } else {
	                sl = prec ; /* truncate on right */
	            }
	        }
	    } /* end if */

/* calculate the minimum field width */

	    {
	        int	ml = 0 ;
	        if (! f_specialhex) ml = sl ;
	        if ((prec >= 0) && (prec > ml)) ml = prec ;
	        if (f_plus | f_minus) ml += 1 ;
	        if (ml > width) width = ml ;
	    }

/* calculate any padding (blanks or zero-fills) */

	    f_plusminus = (f_plus || f_minus) ;
	    {
	        int ml = sl ;
	        if ((prec >= 0) && (prec > sl)) ml = prec ;
	        if (f_plusminus) ml += 1 ;
	        if (width > ml) npad = (width - ml) ;
	    } /* end block */

#if	CF_DEBUGZ 
	    zprintf(NDF,"format/subinfo_emit: npad=%d\n",npad) ;
	    zprintf(NDF,"format/subinfo_emit: f_zerofill=%u\n",f_zerofill) ;
#endif

/* print out any leading padding (field width) */

	    if ((rs >= 0) && (! fsp->f.left) && (! f_zerofill)) {
	        if (npad > 0) {
	            rs = subinfo_blanks(sip,npad) ;
	        }
	    } /* end if */

/* we may want to print a leading '-' before anything */

	    if ((rs >= 0) && f_plusminus) {
	        int	ch = (f_minus) ? '-' : '+' ;
	        rs = subinfo_char(sip,ch) ;
	        width -= 1 ;
	    } /* end if */

/* handle any alternates */

	    if ((rs >= 0) && fsp->f.alternate) {
	        switch (fcode) {
	        case 'x':
	        case 'X':
	            rs = subinfo_strw(sip,"0x",2) ;
	            break ;
	        case 'o':
	            if (sp[0] != '0') {
	                rs = subinfo_char(sip,'0') ;
	            }
	            break ;
	        } /* end switch */
	    } /* end if */

/* any zero-fill due to field width */

	    if ((rs >= 0) && (! fsp->f.left) && f_zerofill && (npad > 0)) {
	        int ch = (f_isdigital ? '0' : ' ') ;
	        int	i ;
	        for (i = 0 ; (rs >= 0) && (i < npad) ; i += 1) {
	            rs = subinfo_char(sip,ch) ;
	        }
	    } /* end if */

/* send out any filling due to precision */

	    if ((rs >= 0) && (prec >= 0) && (prec > sl)) {
	        int ch = (f_isdigital ? '0' : ' ') ;
	        int	i ;
	        for (i = 0 ; (rs >= 0) && (i < (prec - sl)) ; i += 1) {
	            rs = subinfo_char(sip,ch) ;
	        }
	    } /* end if */

/* send out the string itself */

	    if ((rs >= 0) && (sl > 0)) {
	        if (f_specialhex) {
	            if ((width >= 0) && (sl > width)) {
	                int	skip = (sl - width) ;
	                sp += skip ;
	                sl -= skip ;
	            }
	        }
	        rs = subinfo_cleanstrw(sip,sp,sl) ;
	    } /* end if */

/* send out trailing pad characters */

	    if ((rs >= 0) && fsp->f.left && (npad > 0)) {
	        rs = subinfo_blanks(sip,npad) ;
	    } /* end if */

	} /* end if (fcode) */

	return rs ;
}
/* end subroutine (subinfo_emit) */


/* convert floating point numbers */

#if	CF_FLOAT && LOCAL_SOLARIS

#define	DOFLOAT_STAGELEN	(310+MAXPREC+2)
#define	DOFLOAT_DEFPREC		MIN(4,MAXPREC)


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

#if	CF_DEBUGZ
	zprintf(NDF,"format/subinfo_float: fcode=%c width=%d prec=%d\n",
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

#if	CF_DEBUGZ
	    zprintf(NDF,"format/subinfo_float: xconvert() b=>%s<\n",buf) ;
#endif

	    remlen = width ;
	    stagelen = prec + dpp ;
	    i = DOFLOAT_STAGELEN ;
	    j = stagelen ;

/* output any characters in the floating buffer after the decimal point */

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

#if	CF_DEBUGZ
	    zprintf(NDF,"format/subinfo_float: stage=>%s<\n",(stage+i)) ;
#endif

	    while ((rs >= 0) && (i < DOFLOAT_STAGELEN)) {
	        rs = subinfo_char(sip,stage[i++]) ;
	    }

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (subinfo_float) */

#endif /* CF_FLOAT */


#if	CF_LSPRINTF && CF_LPRINT

static int lsprintf(char *buf,cchar *fmt,...)
{
	int		rs ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = format(buf,MAXLEN,0,fmt,ap) ;
	    va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (lsprintf) */

#endif /* CF_LSPRINTF */


#if	CF_LVSPRINTF && CF_LPRINT

static int lvsprintf(char *buf,cchar *fmt,va_list ap)
{
	int		rs = format(buf,MAXLEN,0,fmt,ap) ;
	return rs ;
}
/* end subroutine (lvsprintf) */

#endif /* CF_LVSPRINTF */


#if	CF_LSNPRINTF && CF_LPRINT

/* standard routine to format a string to a memory buffer */
static int snprintf(char buf,int buflen,cchar *fmt,...)
{
	int		rs ;

	if (buf == NULL) return SR_FAULT ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = format(buf,buflen,0,fmt,ap) ;
	    va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (snprintf) */

#endif /* CF_LSNPRINTF */


#if	CF_LVSNPRINTF && CF_LPRINT

static int vsnprintf(char *rbuf,int rlen,cchar *fmt,va_list ap)
{
	int		rs = format(rbuf,rlen,0,fmt,ap) ;
	return rs ;
}
/* end subroutine (vsnprintf) */

#endif /* CF_LVSNPRINTF */


#if	CF_BINCHAR

static int binchar(ULONG num,int i)
{
	int		ch = (rshiftul(num,i) & 1) + '0' ;
	return ch ;
}
/* end subroutine (binchar) */


static ULONG rshiftul(ULONG v,int n)
{
	return (v >> n) ;
}
/* end subroutine (rshiftul) */

#endif /* CF_BINCHAR */


#if	CF_CLEANSTR
static int hasourbad(cchar *sp,int sl)
{
	int		f = FALSE ;
	while (sl && *sp) {
	    const int	ch = MKCHAR(*sp) ;
	    f = isourbad(ch) ;
	    if (f) break ;
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	return f ;
}
/* end subroutine (hasourbad) */
#endif /* CF_CLEANSTR */


int isourbad(int ch)
{
	int		f = FALSE ;
	f = f || isprintlatin(ch) ;
	f = f || (ch == '\r') || (ch == '\n') ;
	f = f || (ch == CH_BS) ;
	f = f || (ch == CH_BELL) ;
	f = f || (ch == CH_VT) || (ch == CH_FF) ;
	f = f || (ch == CH_SI) || (ch == CH_SO) ;
	f = f || (ch == CH_SS2) || (ch == CH_SS3) ;
	return (! f) ;
}
/* end subroutine (isourbad) */


