/* envs_procxe */

/* process an environment file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* special */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) an environment file and put all of
        the environment variables into an ENVS object (supplied). New
        environment variables just get added to the list. Old environment
        variables already on the list are deleted with a new definition is
        encountered.

	Synopsis:

	int envs_procxe(nlp,clp,envv,dlp,fname)
	ENVS		*nlp ;
	EXPCOOK		*clp ;
	cchar		*envv[] ;
	vecstr		*dlp ;
	cchar		fname[] ;

	Arguments:

	nlp		new-list-pointer, new (forming) environment list
	clp		cookie-list-pointer
	envv		process environment
	dlp		"defines" list pointer
	fname		file to process

	Returns:

	>=0		count of environment variables
	<0		bad


	Notes:

	- Assignments:
	=		any previous value is overwritten
	= Additions:
	+=		added with no separation
	­=		separated by ' '
	:=		separated by ':'
	¶=		separated by ':', must be a directory 
	µ=		separated by ' ', must be a directory


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<expcook.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<ascii.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"envs.h"


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	DEFNAMELEN
#define	DEFNAMELEN	120
#endif

#ifndef	ENVNAMELEN
#define	ENVNAMELEN	120
#endif

#define	NDEBFN		"/tmp/envset.deb"

#define	SUBINFO		struct xsubinfo
#define	ASSTYPES	struct xassigntypes


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfthing(cchar *,int,cchar *,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	nextfieldterm(cchar *,int,cchar *,cchar **) ;
extern int	vstrkeycmp(cchar **,cchar **) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif
#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct xsubinfo {
	ENVS		*nlp ;
	EXPCOOK		*clp ;
	cchar		**envv ;
	vecstr		*dlp ;
} ;

struct xassigntypes {
	uint		add:1 ;		/* add (append) */
	uint		sep:1 ;		/* separator */
	uint		dir:1 ;		/* check if a directory */
	uint		path:1 ;	/* treat as a "path" */
	uint		ass:1 ;
	uint		uniqstr:1 ;	/* unique string */
} ;


/* forward references */

static int	procexpandline(SUBINFO *,cchar *,int) ;
static int	procline(SUBINFO *,cchar *,int) ;
static int	procliner(SUBINFO *,cchar *,int,ASSTYPES *,int,cchar *,int) ;
static int	procdeps(SUBINFO *,cchar *,int) ;
static int	procvalues(SUBINFO *,BUFFER *,cchar *,cchar *,int) ;
static int	procvalue(SUBINFO *,BUFFER *,cchar *,cchar *,int) ;
static int	procsubdef(SUBINFO *,BUFFER *,cchar *,int) ;


/* local variables */

static const uchar	vterms[32] = {
	0x00, 0x02, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x0C,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const uchar	dterms[32] = {
	0x00, 0x02, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

#ifdef	COMMENT
static cchar	ssb[] = {
	CH_LBRACE, CH_RBRACE, 0
} ;
#endif /* COMMENT */

static cchar	ssp[] = {
	CH_LPAREN, CH_RPAREN, 0
} ;

static cchar	*strassign = "+:;¶µ­Ð=-" ;


/* exported subroutines */


int envs_procxe(ENVS *nlp,EXPCOOK *clp,cchar **envv,vecstr *dlp,cchar *fname)
{
	SUBINFO		li, *sip = &li ;
	bfile		xefile, *xfp = &xefile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (nlp == NULL) return SR_FAULT ;
	if (clp == NULL) return SR_FAULT ;
	if (envv == NULL) return SR_FAULT ;
	if (dlp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	sip->nlp = nlp ;
	sip->clp = clp ;
	sip->envv = envv ;
	sip->dlp = dlp ;

	if ((rs = bopen(xfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		cl ;
	    int		len ;
	    cchar	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadlines(xfp,lbuf,llen,NULL)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

	        if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
		    if (cp[0] != '#') {
	                c += 1 ;
	                rs = procexpandline(sip,cp,cl) ;
		    }
		}

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = bclose(xfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-open) */

#if	CF_DEBUGS
	debugprintf("envs_procxe: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (envs_procxe) */


/* local subroutines */


static int procexpandline(SUBINFO *sip,cchar *sp,int sl)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (sl < 0) sl = strlen(sp) ;

	if (strnchr(sp,sl,'%') != NULL) {
	    BUFFER	b ;
	    EXPCOOK	*clp = sip->clp ;
	    const int	bsize = (sl+20) ;
	    if ((rs = buffer_start(&b,bsize)) >= 0) {

	        if ((rs = expcook_expbuf(clp,0,&b,sp,sl)) >= 0) {
	            const int	bl = rs ;
	            cchar	*bp ;
	            if ((rs = buffer_get(&b,&bp)) >= 0) {
	                rs = procline(sip,bp,bl) ;
	                len = rs ;
	            } /* end if (buffer-get) */
	        } /* end if (expand-buf) */

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} else {
	    rs = procline(sip,sp,sl) ;
	    len = rs ;
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procexpandline) */


static int procline(SUBINFO *sip,cchar *sp,int sl)
{
	ASSTYPES	at ;
	int		rs = 1 ; /* something positive (see code below) */
	int		el ;
	int		enl = 0 ;
	int		len = 0 ;
	cchar		*tp, *ep ;
	cchar		*enp = NULL ;
	char		envname[ENVNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("envs_procxe/procline: ent s=>%t<\n",sp,sl) ;
#endif

/****
	LOCAL ?		PATH ¶= 		$(LOCAL)/bin
	LOCAL ?		PATH ¶= 		$(LOCAL)/bin
	PATH 		     ¶= 		$(LOCAL)/bin
	PATH  					$(LOCAL)/bin
	****/

	memset(&at,0,sizeof(ASSTYPES)) ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

/* extract any dependencies (if we have any) */

	if (((tp = strnpbrk(sp,sl,"?+:;¶µ­=#\t ")) != NULL) && (*tp == '?')) {

	    if ((rs = procdeps(sip,sp,(tp - sp))) > 0) {
	        sl -= ((tp+1) - sp) ;
	        sp = (tp+1) ;
	        while (sl && CHAR_ISWHITE(*sp)) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	    } /* end if (procdeps) */

	} /* end if (getting dependencies) */

#if	CF_DEBUGN
	nprintf(NDEBFN,"envs_procxe/procline: tch=(%02x) tch=>%c<\n",
	    *tp,((isprintlatin(*tp)) ? *tp : 0xbf)) ;
	nprintf(NDEBFN,"envs_procxe/procline: rem=>%t<\n",sp,sl) ;
#endif

#if	CF_DEBUGS
	debugprintf("envs_procxe/procline: rem=>%t<\n",sp,sl) ;
	debugprintf("envs_procxe/procline: cch=(%02x) cch=>%c<\n",
	    *sp,((isprintlatin(*sp)) ? *sp : 0xbf)) ;
	debugprintf("envs_procxe/procline: tch=(%02x) tch=>%c<\n",
	    *tp,((isprintlatin(*tp)) ? *tp : 0xbf)) ;
#endif

	if (rs > 0) { /* greater-than */
	    if ((el = nextfieldterm(sp,sl,strassign,&ep)) > 0) {
	        int	ach ;
	        int	sch = 0 ;
	        int	f_exit = FALSE ;
	        int	f_go = TRUE ;
	        sl -= ((ep+el)-sp) ;
	        sp = (ep+el) ;
	        while (sl && CHAR_ISWHITE(*sp)) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	        while ((tp = strnpbrk(sp,1,strassign)) != NULL) {
	            ach = MKCHAR(tp[0]) ;
	            switch (ach) {
	            case '+':
	                at.add = TRUE ;
	                break ;
	            case ':':
	            case ';':
	                at.add = TRUE ;
	                at.sep = TRUE ;
	                sch = ach ;
	                break ;
	            case '-':
	            case '=':
	                at.ass = TRUE ;
	                f_exit = TRUE ;
	                break ;
	            case MKCHAR('Ð'):
	                at.dir = TRUE ;
	                break ;
	            case MKCHAR('¶'):
	                at.add = TRUE ;
	                at.sep = TRUE ;
	                at.dir = TRUE ;
	                at.uniqstr = FALSE ;
	                at.path = TRUE ;
	                if (sch == 0) sch = ':' ;
	                break ;
	            case MKCHAR('µ'):
	                at.add = TRUE ;
	                at.sep = TRUE ;
	                at.dir = TRUE ;
	                at.uniqstr = TRUE ;
	                if (sch == 0) sch = ' ' ;
	                break ;
	            case MKCHAR('­'):
	                at.add = TRUE ;
	                at.sep = TRUE ;
	                if (sch == 0) sch = ' ' ;
	                break ;
	            default:
	                f_exit = TRUE ;
	                break ;
	            } /* end switch */
	            sp += 1 ;
	            sl -= 1 ;
#if	CF_DEBUGS
	            debugprintf("envs_procxe/procline: cch=(%02x) cch=>%c<\n",
	                *sp,((isprintlatin(*sp)) ? *sp : 0xbf)) ;
#endif
	            if (f_exit) break ;
	        } /* end while */
	        while (sl && CHAR_ISWHITE(*sp)) {
	            sp += 1 ;
	            sl -= 1 ;
	        }

#if	CF_DEBUGN
	        nprintf(NDEBFN,"envs_procxe/procline: rem=>%t<\n",sp,sl) ;
	        nprintf(NDEBFN,"envs_procxe/procline: el=%d e=%t\n",
	            el,ep,el) ;
	        nprintf(NDEBFN,"envs_procxe/procline: f_add=%u\n",at.add) ;
	        nprintf(NDEBFN,"envs_procxe/procline: f_sep=%u\n",at.sep) ;
	        nprintf(NDEBFN,"envs_procxe/procline: f_dir=%u\n",at.dir) ;
	        nprintf(NDEBFN,"envs_procxe/procline: f_ass=%u\n",at.ass) ;
	        nprintf(NDEBFN,"envs_procxe/procline: sch=>%c<\n",sch) ;
	        nprintf(NDEBFN,"envs_procxe/procline: tch=>%c<\n",
	            ((tp != NULL) ?tp[0]:0xbf)) ;
#endif /* CF_DEBUGN */

#if	CF_DEBUGS
	        debugprintf("envs_procxe/procline: rem=>%t<\n",sp,sl) ;
	        debugprintf("envs_procxe/procline: el=%d e=%t\n",
	            el,ep,el) ;
	        debugprintf("envs_procxe/procline: f_add=%u\n",at.add) ;
	        debugprintf("envs_procxe/procline: f_sep=%u\n",at.sep) ;
	        debugprintf("envs_procxe/procline: f_dir=%u\n",at.dir) ;
	        debugprintf("envs_procxe/procline: f_ass=%u\n",at.ass) ;
	        debugprintf("envs_procxe/procline: sch=>%c<\n",sch) ;
	        debugprintf("envs_procxe/procline: tch=>%c<\n",
	            ((tp != NULL) ?tp[0]:0xbf)) ;
#endif /* CF_DEBUGS */

	        {
	            int		cl ;
	            cchar	*cp ;
	            if ((cl = sfshrink(ep,el,&cp)) > 0) {
	                enp = envname ;
	                enl = cl ;
	                if (snwcpy(envname,ENVNAMELEN,cp,cl) < 0) {
	                    f_go = FALSE ;
			}
	            }
	        }

#if	CF_DEBUGN
	        nprintf(NDEBFN,"envs_procxe/procline: en=>%s<\n",enp) ;
#endif

/* do we already have this definition? */

#if	CF_DEBUGS
	        debugprintf("envs_procxe/procline: 2 env=%s\n",envname) ;
#endif

/* loop processing the values */

#if	CF_DEBUGS
	        debugprintf("envs_procxe/procline: f_go=%u values=>%t<\n",
			f_go,sp,sl) ;
#endif

	        if (f_go) {
	            rs = procliner(sip,enp,enl,&at,sch,sp,sl) ;
		}

	    } /* end if (nextfieldterm) */
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("envs_procxe/procline: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procline) */


static int procliner(sip,enp,enl,atp,sch,sp,sl)
SUBINFO		*sip ;
cchar		enp[] ;
int		enl ;
cchar		*sp ;
ASSTYPES	*atp ;
int		sch ;
int		sl ;
{
	BUFFER		b ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("envs_procxe/procliner: ent sch=>%c< sl=%d\n",sch,sl) ;
	debugprintf("envs_procxe/procliner: s=>%t<\n",sp,
	    strlinelen(sp,sl,40)) ;
#endif

	if ((sl >= 0) && ((rs = buffer_start(&b,sl)) >= 0)) {

	    if ((rs = procvalues(sip,&b,ssp,sp,sl)) >= 0) {
	        int	f_store = TRUE ;
	        cchar	*cp ;
	        if ((rs = buffer_get(&b,&cp)) >= 0) {
	            int	cl = rs ;

#if	CF_DEBUGS
	        debugprintf("envs_procxe/procliner: buf=>%t<\n",cp,cl) ;
	        debugprintf("envs_procxe/procliner: f_dir=%u\n",atp->dir) ;
#endif

	        if ((rs >= 0) && f_store && atp->uniqstr) {
	            ENVS	*nlp = sip->nlp ;
	            if ((rs = envs_substr(nlp,enp,enl,sp,sl)) > 0) {
			f_store = FALSE ;
	            } else if (rs == SR_NOTFOUND) {
	                rs = SR_OK ;
	            }
		} /* end if (unique string) */

	        if ((rs >= 0) && f_store && atp->dir) {
	            if (cl > 0) {
	                const int	dlen = MAXPATHLEN ;
	                char		dbuf[MAXPATHLEN+1] ;
#if	CF_DEBUGS
	                debugprintf("envs_procxe/procliner: dir? c»>%t<\n",
	                    cp,strlinelen(cp,cl,40)) ;
#endif
	                if ((rs = snwcpy(dbuf,dlen,cp,cl)) >= 0) {
	                    struct ustat	sb ;
	                    if (u_stat(dbuf,&sb) >= 0) {
	                        if (! S_ISDIR(sb.st_mode)) {
	                            f_store = FALSE ;
				}
	                    } else
	                        f_store = FALSE ;
	                } /* end if (snwcpy) */
	            } else
	                f_store = FALSE ;
	        } /* end if (directory check) */

#if	CF_DEBUGS
	        debugprintf("envs_procxe/procliner: mid rs=%d f_store=%u\n",
			rs,f_store) ;
#endif

	        if ((rs >= 0) && f_store) {
	            ENVS	*nlp = sip->nlp ;

#if	CF_DEBUGN
	            debugprintf("envs_procxe/procliner: sep=%u\n",atp->sep) ;
	            nprintf(NDEBFN,"envs_procxe/procliner: add=>%t<\n",
			cp,cl) ;
#endif

	            if (atp->sep) {
#if	CF_DEBUGS
	            debugprintf("envs_procxe/procliner: sep\n") ;
#endif
	                if ((rs = envs_present(nlp,enp,enl)) > 0) {
	                    char	sbuf[2] ;
	                    sbuf[0] = sch ;
	                    sbuf[1] = '\0' ;
	                    rs = envs_store(nlp,enp,TRUE,sbuf,1) ;
	                } else if (rs == SR_NOTFOUND) {
	                    rs = SR_OK ;
	                }
	            } /* end if (sep) */

#if	CF_DEBUGS
	            debugprintf("envs_procxe/procliner: mid2 rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {
	                int	f_append = atp->add ;
#if	CF_DEBUGS
			if (cp != NULL) {
	                debugprintf("envs_procxe/procliner: cl=%u add=>%t<\n",
			cl,cp,strlinelen(cp,cl,40)) ;
			} else {
	                debugprintf("envs_procxe/procliner: c=NULL\n") ;
			}
	                debugprintf("envs_procxe/procliner: f_append=%u\n",
			f_append) ;
#endif
	                rs = envs_store(nlp,enp,f_append,cp,cl) ;
	            } /* end if */

	        } /* end if (store) */

	        } /* end if (buffer_get) */
	    } /* end if (procvalues) */

	    len = buffer_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (buffer initialization) */

#if	CF_DEBUGS
	debugprintf("envs_procxe/procliner: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procliner) */


static int procdeps(SUBINFO *sip,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		rs1 = 0 ;
	int		f = TRUE ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    char	defbuf[DEFNAMELEN + 1] ;

	    while ((fl = field_get(&fsb,dterms,&fp)) >= 0) {
	        if (fl > 0) {

#if	CF_DEBUGS
	            debugprintf("envs_procxe/procdeps: dep=%t\n",fp,fl) ;
#endif

	            if ((rs1 = snwcpy(defbuf,DEFNAMELEN,fp,fl)) >= 0) {
	                rs1 = vecstr_search(sip->dlp,defbuf,vstrkeycmp,NULL) ;
		    }

#if	CF_DEBUGS
	            debugprintf("envs_procxe/procdeps: "
			"vecstr_search() rs1=%d\n",rs1) ;
#endif

		}
	        if (rs1 < 0) break ;
	    } /* end while */

	    f = (rs1 >= 0) ;
	    rs1 = field_finish(&fsb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (field) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procdeps) */


/* process definition values */
static int procvalues(SUBINFO *sip,BUFFER *bp,cchar *ss,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		len = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    const int	flen = sl ;
	    char	*fbuf ;
	    if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	        int	fl ;
	        int	c = 0 ;
	        while ((fl = field_sharg(&fsb,vterms,fbuf,flen)) >= 0) {
	            if (fl > 0) {

#if	CF_DEBUGS
	                debugprintf("envs_procxe/procvalues: val=%t\n",
	                    fbuf,fl) ;
#endif

	                if (c++ > 0) {
	                    rs = buffer_char(bp,' ') ;
	                    len += rs ;
	                }

	                if (rs >= 0) {
	                    rs = procvalue(sip,bp,ss,fbuf,fl) ;
	                    len += rs ;
	                }

	            } /* end if (non-zero) */
	            if (fsb.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while (looping over values) */
	        uc_free(fbuf)  ;
	    } /* end if (m-a) */
	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procvalues) */


static int procvalue(SUBINFO *sip,BUFFER *bp,cchar *ss,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		kl, cl ;
	int		len = 0 ;
	cchar		*kp, *cp ;

#if	CF_DEBUGS
	{
	    debugprintf("envs_procxe/procvalue: s=>%t<\n",sp,sl) ;
	    kl = sfthing(sp,sl,ss,&kp) ;
	    debugprintf("envs_procxe/procvalue: sfthing() kl=%d\n",kl) ;
	}
#endif

	while ((kl = sfthing(sp,sl,ss,&kp)) >= 0) {

	    cp = sp ;
	    cl = ((kp - 2) - sp) ;

#if	CF_DEBUGS
	    debugprintf("envs_procxe/procvalue: leader=>%t<\n",cp,cl) ;
#endif

	    if (cl > 0) {
	        rs = buffer_strw(bp,cp,cl) ;
	        len += rs ;
	    }

	    if ((rs >= 0) && (kl > 0)) {
	        rs = procsubdef(sip,bp,kp,kl) ;
	        len += rs ;
	    }

	    sl -= ((kp + kl + 1) - sp) ;
	    sp = (kp + kl + 1) ;

#if	CF_DEBUGS
	    debugprintf("envs_procxe/procvalue: remainder=>%t<\n",
	        sp,((sp[sl - 1] == '\n') ? (sl - 1) : sl)) ;
#endif

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = buffer_strw(bp,sp,sl) ;
	    len += rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procvalue) */


static int procsubdef(SUBINFO *sip,BUFFER *bp,cchar *kp,int kl)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (kl != 0) {
	    int		al = 0 ;
	    int		cl ;
	    cchar	*ap = NULL ;
	    cchar	*tp, *cp ;
	    char	defbuf[DEFNAMELEN + 1] ;

	    if ((tp = strnchr(kp,kl,'=')) != NULL) {
	        ap = (tp + 1) ;
	        al = (kp + kl) - (tp + 1) ;
	        kl = (tp - kp) ;
	    }

	    if ((rs1 = snwcpy(defbuf,DEFNAMELEN,kp,kl)) >= 0) {
	        int	(*vs)(cchar **,cchar **) = vstrkeycmp ;

/* lookup the define key-name that we have */

	        if ((rs1 = vecstr_search(sip->dlp,defbuf,vs,&cp)) >= 0) {
	            cl = strlen(cp) ;
	            if ((tp = strchr(cp,'=')) != NULL) {
	                cl -= ((tp + 1) - cp) ;
	                cp = (tp + 1) ;
	            }
	        }

#if	CF_DEBUGS
	        if (rs1 >= 0)
	            debugprintf("envs_procxe/procsubdef: value=>%s<\n",cp) ;
#endif

/* perform any appropriate substitution */

	        if (rs1 >= 0) {
	            rs = buffer_strw(bp,cp,cl) ;
	            len += rs ;
	        } else if (al > 0) {
	            rs = buffer_strw(bp,ap,al) ;
	            len += rs ;
	        }

	    } /* end if (snwcpy) */

	} /* end if (non-zero) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procsubdef) */


