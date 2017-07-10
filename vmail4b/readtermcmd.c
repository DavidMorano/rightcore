/* uterm_readcmd */
/* lang=C89 */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes characters in order to determine what type of
        control sequence we have. The first character of the sequence is
        provided when we are called. Subsequent characters are read by us.

	Synopsis:

	int uterm_readcmd(utp,ckp,to,ich)
	UTERM		*utp ;
	TERMCMD		*ckp ;
	int		to ;
	int		ich ;

	Arguments:

	utp		UTERM object-pointer 
	ckp		TERMCMD object pointer (for results)
	to		time-out
	ich		initial (first) character if any (zero if none)

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<cfdec.h>
#include	<uterm.h>
#include	<termcmd.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold an 'int128_t' */
#endif

#define	SUB		struct sub

#define	CMD		TERMCMD

#ifndef	UC
#define	UC(ch)		((uchar)(ch))
#endif

#define	TR_OPTS		(FM_NOFILTER | FM_NOECHO | FM_RAWIN | FM_TIMED)


/* external subroutines */

extern int	ndigits(int,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* local structures */

struct sub {
	CMD		*ckp ;
	UTERM		*utp ;
	int		to ;		/* time-out */
	int		pi ;		/* number-prameters envountered */
	int		ii ;		/* intermediate-str index (length) */
	int		di ;		/* device-str index (length) */
	int		ich ;		/* initial character */
	int		rlen ;		/* read-length */
	int		maxdig ;	/* maximum digits in TERMCMD_MAXPVAL */
	int		f_error:1 ;
} ;


/* forward subroutines */

static int sub_start(SUB *,CMD *,UTERM *,int,int) ;
static int sub_readch(SUB *) ;
static int sub_proc_esc(SUB *) ;
static int sub_proc_csi(SUB *) ;
static int sub_proc_dcs(SUB *) ;
static int sub_proc_pf(SUB *) ;
static int sub_proc_reg(SUB *,int) ;
static int sub_proc_escmore(SUB *,int) ;
static int sub_loadparam(SUB *,const char *,int) ;
static int sub_finish(SUB *) ;

static int isinter(int) ;
static int isfinalesc(int) ;
static int isfinalcsi(int) ;
static int isparam(int) ;
static int iscancel(int) ;


/* local variables */

#ifdef	COMMENT
termcmdtype_reg,		/*     */
termcmdtype_esc,		/*     */
termcmdtype_csi,		/* '[' */
termcmdtype_dcs,		/* 'P' */
termcmdtype_pf,			/* 'O' */
termcmdtype_overlast
#endif /* COMMENT */

static const uchar	cans[] = "\030\032\033" ;

#ifdef	COMMENT
static const int	alts[] = {
	    CH_DEL,		/* reg */
	    CH_DEL,		/* ESC */
	    '[',		/* CSI */
	    'P',		/* DCS */
	    'O',		/* PF (also SS3) */
	    0
} ;
#endif /* COMMENT */


/* exported subroutines */


int uterm_readcmd(UTERM *utp,TERMCMD *ckp,int to,int ich)
{
	SUB		si ;
	int		rs ;
	int		rs1 ;

	if (utp == NULL) return SR_FAULT ;
	if (ckp == NULL) return SR_FAULT ;

	if ((rs = termcmd_clear(ckp)) >= 0) {
	    if ((rs = sub_start(&si,ckp,utp,to,ich)) >= 0) {
	        rs = 0 ;
	        while (rs == 0) {
	            if ((rs = sub_readch(&si)) >= 0) {
	                int	ch = rs ;
	                switch (ch) {
	                case CH_ESC:
	                    rs = sub_proc_esc(&si) ;
	                    break ;
	                case CH_CSI:
	                    rs = sub_proc_csi(&si) ;
	                    break ;
	                case CH_DCS:
	                    rs = sub_proc_dcs(&si) ;
	                    break ;
	                case CH_SS3:
	                    rs = sub_proc_pf(&si) ;
	                    break ;
	                default:
	                    rs = sub_proc_reg(&si,ch) ;
	                    break ;
	                } /* end switch */
	            } /* end if (read first character) */
	        } /* end while (repeating due to CANCEL) */
	        rs1 = sub_finish(&si) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sub) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (uterm_readcmd) */


/* local subroutines */


static int sub_start(SUB *sip,CMD *ckp,UTERM *utp,int to,int ich)
{
	int		rs = SR_OK ;
	memset(sip,0,sizeof(SUB)) ;
	sip->maxdig = ndigits(TERMCMD_MAXPVAL,10) ;
	sip->ckp = ckp ;
	sip->utp = utp ;
	sip->to = to ;
	sip->ich = ich ;
	{
	    int	i ;
	    for (i = 0 ; i < TERMCMD_NP ; i += 1) ckp->p[i] = -1 ;
	}
	return rs ;
}
/* end subroutine (sub_start) */


static int sub_finish(SUB *sip)
{
	if (sip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (sub_finish) */


static int sub_readch(SUB *sip)
{
	int		rs = SR_OK ;
	int		rch = 0 ;

	if (sip->ich != 0) {
	    rch = sip->ich ;
	    sip->ich = 0 ;
	} else {
	    UTERM	*utp = sip->utp ;
	    const int	ropts = TR_OPTS ;
	    const int	to = sip->to ;
	    char	rbuf[2] ;
	    rs = uterm_reade(utp,rbuf,1,to,ropts,NULL,NULL) ;
	    rch = (rbuf[0] & 0xff) ;
	    if (rs > 0) sip->rlen += 1 ;
	}

	return (rs >= 0) ? rch : rs ;
}
/* end subroutine (sub_readch) */


static int sub_proc_esc(SUB *sip)
{
	CMD		*ckp = sip->ckp ;
	int		rs ;

	ckp->name = termcmdtype_esc ;

	if ((rs = sub_readch(sip)) >= 0) {
	    int	ch = rs ;
	    switch (ch) {
	    case '[':
	        rs = sub_proc_csi(sip) ;
	        break ;
	    case 'P':
	        rs = sub_proc_dcs(sip) ;
	        break ;
	    case 'O':
	        rs = sub_proc_pf(sip) ;
	        break ;
	    default:
	        if (iscancel(ch)) {
	            termcmd_clear(ckp) ;
	            if (ch == CH_ESC) sip->ich = ch ;
	            rs = 0 ;	/* signal CANCEL */
	        } else {
	            rs = sub_proc_escmore(sip,ch) ;
	        } /* end if */
	        break ;
	    } /* end switch */
	} /* end if (sub_readch) */

	return rs ;
}
/* end subroutine (sub_proc_esc) */


static int sub_proc_escmore(SUB *sip,int ch)
{
	CMD		*ckp = sip->ckp ;
	const int	ilen = TERMCMD_ISIZE ;
	int		rs = SR_OK ;

	ckp->type = termcmdtype_esc ;
	ckp->istr[0] = '\0' ;
	ckp->dstr[0] = '\0' ;

	while (isinter(ch)) {
	    if (sip->ii < ilen) {
	        ckp->istr[sip->ii++] = ch ;
	    } else {
	        ckp->f.iover = TRUE ;
	    }
	    rs = sub_readch(sip) ;
	    if (rs < 0) break ;
	    ch = rs ;
	} /* end while */
	ckp->istr[sip->ii] = 0 ;

	if (rs >= 0) {
	    if (isfinalesc(ch)) {
	        ckp->name = ch ;
	        rs = 1 ;		/* signal DONE */
	    } else if (iscancel(ch)) {
	        termcmd_clear(ckp) ;
	        ckp->name = 0 ;		/* error */
	        if (ch == CH_ESC) {
	            sip->ich = ch ;
	            rs = 0 ;		/* signal CANCEL w/ continue */
	        } else
	            rs = 1 ;		/* signal DONE w/ error */
	    } else {
	        ckp->name = 0 ;		/* error */
	        rs = 1 ;		/* signal DONE w/ error */
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (sub_proc_escmore) */


static int sub_proc_csi(SUB *sip)
{
	CMD		*ckp = sip->ckp ;
	int		rs ;

	ckp->type = termcmdtype_csi ;
	ckp->istr[0] = '\0' ;
	ckp->dstr[0] = '\0' ;

	if ((rs = sub_readch(sip)) >= 0) {
	    const int	ilen = TERMCMD_ISIZE ;
	    const int	dlen = DIGBUFLEN ;
	    int		dl = 0 ;
	    int		ch = rs ;
	    int		f_expecting = FALSE ;
	    int		f_dover = FALSE ;
	    int		f_leadingzero = TRUE ;
	    char	dbuf[DIGBUFLEN+1] = { 0 } ;

	    if (ch == '?') {
	        ckp->f.private = TRUE ;
	        rs = sub_readch(sip) ;
	        ch = rs ;
	    }

	    while ((rs >= 0) && isparam(ch)) {
	        if (isdigitlatin(ch)) {
	            if ((ch != '0') || (! f_leadingzero)) {
	                if (ch != '0') f_leadingzero = FALSE ;
#if	CF_DEBUGS
	                debugprintf("uterm_readcmd/sub_csi: dl=%u dig=%c\n",
	                    dl,ch) ;
#endif
	                if (dl < dlen) {
	                    dbuf[dl++] = ch ;
	                } else {
	                    f_dover = TRUE ;
	                }
	            }
	        } else if (ch == ';') {
	            f_expecting = TRUE ;
	            if (f_dover) {
	                sip->f_error = TRUE ;
	            } else {
	                rs = sub_loadparam(sip,dbuf,dl) ;
	                dbuf[0] = '\0' ;
	                dl = 0 ;
	            }
	        }
	        rs = sub_readch(sip) ;
	        ch = rs ;
	        if (rs < 0) break ;
	    } /* end while (loading parameters) */

	    if ((rs >= 0) && ((dl > 0) || f_expecting)) {
	        rs = sub_loadparam(sip,dbuf,dl) ;
	        dbuf[0] = '\0' ;
	        dl = 0 ;
	    } /* end if (loading the last parameter) */

	    while ((rs >= 0) && isinter(ch)) {
	        if (sip->ii < ilen) {
	            ckp->istr[sip->ii++] = ch ;
	        } else {
	            ckp->f.iover = TRUE ;
	        }
	        rs = sub_readch(sip) ;
	        if (rs < 0) break ;
	        ch = rs ;
	    } /* end while */

	    if (rs >= 0) {
	        if (isfinalcsi(ch)) {
	            ckp->name = ch ;
	            rs = 1 ;		/* signal DONE */
	        } else if (iscancel(ch)) {
	            termcmd_clear(ckp) ;
	            ckp->name = 0 ;		/* error */
	            if (ch == CH_ESC) {
	                sip->ich = ch ;
	                rs = 0 ;		/* signal CANCEL w/ continue */
	            } else {
	                rs = 1 ;		/* signal DONE w/ error */
	            }
	        } else {
	            ckp->name = 0 ;		/* error */
	            rs = 1 ;		/* signal DONE w/ error */
	        }
	    } /* end if */

	    if ((rs >= 0) && (sip->pi > 0)) {
	        if (sip->pi < TERMCMD_NP) {
	            ckp->p[sip->pi] = -1 ;
	        }
	    }

	} /* end if (sub_readch) */

	return rs ;
}
/* end subroutine (sub_proc_csi) */


static int sub_proc_dcs(SUB *sip)
{
	CMD		*ckp = sip->ckp ;
	int		rs ;

	ckp->type = termcmdtype_dcs ;
	ckp->istr[0] = '\0' ;
	ckp->dstr[0] = '\0' ;

	if ((rs = sub_readch(sip)) >= 0) {
	    const int	ilen = TERMCMD_ISIZE ;
	    const int	dlen = DIGBUFLEN ;
	    int		dl = 0 ;
	    int		ch = rs ;
	    int		f_expecting = FALSE ;
	    int		f_dover = FALSE ;
	    int		f_leadingzero = TRUE ;
	    char	dbuf[DIGBUFLEN+1] = { 0 } ;

	    if (ch == '?') {
	        ckp->f.private = TRUE ;
	        rs = sub_readch(sip) ;
	        ch = rs ;
	    }

	    while ((rs >= 0) && isparam(ch)) {
	        if (isdigitlatin(ch)) {
	            if ((ch != '0') || (! f_leadingzero)) {
	                if (ch != '0') f_leadingzero = FALSE ;
	                if (dl < dlen) {
	                    dbuf[dl++] = ch ;
	                } else
	                    f_dover = TRUE ;
	            }
	        } else if (ch == ';') {
	            f_expecting = TRUE ;
	            if (f_dover) {
	                sip->f_error = TRUE ;
	            } else {
	                rs = sub_loadparam(sip,dbuf,dl) ;
	                dbuf[0] = '\0' ;
	                dl = 0 ;
	            }
	        }
	        rs = sub_readch(sip) ;
	        ch = rs ;
	        if (rs < 0) break ;
	    } /* end while (loading parameters) */

	    if ((rs >= 0) && ((dl > 0) || f_expecting)) {
	        rs = sub_loadparam(sip,dbuf,dl) ;
	        dbuf[0] = '\0' ;
	        dl = 0 ;
	    } /* end if (loading the last parameter) */

	    while ((rs >= 0) && isinter(ch)) {
	        if (sip->ii < ilen) {
	            ckp->istr[sip->ii++] = ch ;
	        } else {
	            ckp->f.iover = TRUE ;
	        }
	        rs = sub_readch(sip) ;
	        if (rs < 0) break ;
	        ch = rs ;
	    } /* end while */

	    if (rs >= 0) {
	        if (isfinalcsi(ch)) {
	            ckp->name = ch ;	/* no error */
	            rs = 1 ;		/* signal OK */
	        } else if (iscancel(ch)) {
	            termcmd_clear(ckp) ;
	            ckp->name = 0 ;		/* error (CANCEL) */
	            if (ch == CH_ESC) {
	                sip->ich = ch ;
	                rs = 0 ;		/* signal CANCEL w/ continue */
	            } else {
	                rs = 1 ;		/* signal w/ error */
	            }
	        } else {
	            ckp->name = 0 ;		/* error */
	            rs = 1 ;		/* signal w/ error */
	        }
	    } /* end if */

	    if ((rs >= 0) && (ckp->name != 0)) {
	        int	f_seenesc = FALSE ;
	        const int	dlen = TERMCMD_DSIZE ;
	        while (rs >= 0) {
	            rs = sub_readch(sip) ;
	            if (rs < 0) break ;
	            ch = rs ;
	            if (f_seenesc) {
	                if (ch != CH_BSLASH) {
	                    sip->ich = ch ;
	                    rs = 0 ;		/* signal CANCEL */
	                }
	                break ;
	            }
	            if (ch == CH_ST) break ;
	            if (ch != CH_ESC) {
	                if (iscancel(ch)) {
	                    rs = 0 ;		/* signcal CANCEL */
	                    break ;
	                }
	                if (sip->di < dlen) {
	                    ckp->dstr[sip->di++] = ch ;
	                } else {
	                    ckp->f.dover = TRUE ;
	                }
	            } else {
	                f_seenesc = TRUE ;
	            }
	        } /* end while */
	    } /* end if */

	    if ((rs >= 0) && (sip->pi > 0)) {
	        if (sip->pi < TERMCMD_NP) {
	            ckp->p[sip->pi] = -1 ;
	        }
	    }

	} /* end if (sub_readch) */

	return rs ;
}
/* end subroutine (sub_proc_dcs) */


static int sub_proc_pf(SUB *sip)
{
	CMD		*ckp = sip->ckp ;
	int		rs ;

	ckp->type = termcmdtype_pf ;
	ckp->istr[0] = '\0' ;
	ckp->dstr[0] = '\0' ;

	if ((rs = sub_readch(sip)) >= 0) {
	    const int	ch = rs ;
	    if (isfinalcsi(ch)) {
	        ckp->name = ch ;
	        rs = 1 ;		/* signal DONE */
	    } else if (iscancel(ch)) {
	        termcmd_clear(ckp) ;
	        ckp->name = 0 ;		/* error */
	        if (ch == CH_ESC) {
	            sip->ich = ch ;
	            rs = 0 ;		/* signal CANCEL w/ continue */
	        } else {
	            rs = 1 ;		/* signal DONE w/ error */
	        }
	    } else {
	        ckp->name = 0 ;		/* error */
	        rs = 1 ;		/* signal DONE w/ error */
	    }
	} /* end if (sub_readch) */

	return rs ;
}
/* end subroutine (sub_proc_pf) */


static int sub_proc_reg(SUB *sip,int ich)
{
	CMD		*ckp = sip->ckp ;
	int		rs = 1 ;		/* signal DONE */

	ckp->type = termcmdtype_reg ;
	ckp->name = ich ;

	return rs ;
}
/* end subroutine (sub_proc_reg) */


static int sub_loadparam(SUB *sip,const char *dbuf,int dl)
{
	CMD		*ckp = sip->ckp ;
	const int	nparams = TERMCMD_NP ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		v = 0 ;

	if (dl > 0) {
	    if (dl > sip->maxdig) {
	        v = TERMCMD_MAXPVAL ;
	    } else {
	        rs1 = cfdeci(dbuf,dl,&v) ;
	    }
	}
	if (rs1 >= 0) {
	    if (v > TERMCMD_MAXPVAL) v = TERMCMD_MAXPVAL ;
	    if (sip->pi >= nparams) {
	        int	i ;
	        for (i = 1 ; i < nparams ; i += 1) {
	            ckp->p[i-1] = ckp->p[i] ;
	        }
	        ckp->p[i] = -1 ;
	        sip->pi -= 1 ;
	    }
	    ckp->p[sip->pi++] = v ;
	} else {
	    sip->f_error = TRUE ;
	}

	return rs ;
}
/* end subroutine (sub_loadparam) */


static int isinter(int ch)
{
	return ((ch >= 0x20) && (ch <= 0x2F)) ;
}
/* end subroutines (isinter) */


static int isfinalesc(int ch)
{
	return ((ch >= 0x30) && (ch <= 0x7E)) ;
}
/* end subroutines (isfinalesc) */


static int isfinalcsi(int ch)
{
	return ((ch >= 0x40) && (ch <= 0x7E)) ;
}
/* end subroutines (isfinalcsi) */


static int isparam(int ch)
{
	return ((ch >= 0x30) && (ch <= 0x3F)) ;
}
/* end subroutines (isparam) */


static int iscancel(int ch)
{
	int		i ;
	int		f = FALSE ;
	for (i = 0 ; cans[i] ; i += 1) {
	    f = (ch == cans[i]) ;
	    if (f) break ;
	}
	return f ;
}
/* end subroutines (iscancel) */


