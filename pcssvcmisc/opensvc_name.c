/* opensvc_name */

/* PCS facility service (name) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_name(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<sysusernames.h>
#include	<getxusername.h>
#include	<pcsns.h>
#include	<filebuf.h>
#include	<nulstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_name.h"
#include	"defs.h"


/* local defines */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


#define	NDF	"opensvcname.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,int,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	openshmtmp(char *,int,mode_t) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		full:1 ;
} ;

struct subinfo {
	cchar		*pr ;
	cchar		**envv ;
	SUBINFO_FL	f ;
	PCSNS		ns ;
	int		w ;
} ;


/* forward references */

static int subinfo_start(SUBINFO *,cchar *,cchar **,int f) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_af(SUBINFO *,FILEBUF *,cchar *) ;
static int subinfo_def(SUBINFO *,FILEBUF *) ;
static int subinfo_all(SUBINFO *,FILEBUF *) ;
static int subinfo_args(SUBINFO *,FILEBUF *,ARGINFO *,BITS *,cchar *) ;
static int subinfo_users(SUBINFO *,FILEBUF *,cchar *,int) ;
static int subinfo_user(SUBINFO *,FILEBUF *,cchar *,int) ;

static int	getuser(cchar **,char *,int) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"sn",
	"af",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_af,
	argopt_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


/* ARGSUSED */
int opensvc_name(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f_full = FALSE ;
	int		f_all = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*afname = NULL ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_name: ent\n") ;
#endif

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	f_akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            prn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                prn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-list name */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* call users */
	                    case 'a':
	                        f_all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                f_all = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* type of name (regular or full) */
	                    case 'f':
	                        f_full = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                f_full = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_logwelcome: ai_pos=%u ai_max=%u\n",
	    ai_pos,ai_max) ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = openshmtmp(NULL,0,0664)) >= 0) {
	        SUBINFO	si ;
	        fd = rs ;
	        if ((rs = subinfo_start(&si,pr,envv,f_full)) >= 0) {
		    FILEBUF	b ;
		    if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {
		        if (f_all) {
			    rs = subinfo_all(&si,&b) ;
		        } else {
			    cchar	*afn = afname ;
			    rs = subinfo_args(&si,&b,&ainfo,&pargs,afn) ;
		        }
		        rs1 = filebuf_finish(&b) ;
		        if (rs >= 0) rs = rs1 ;
		    } /* end if (filebuf) */
	            rs1 = subinfo_finish(&si) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (subinfo) */
	        if (rs >= 0) {
		    rs = u_rewind(fd) ;
	        } else
	            u_close(fd) ;
	    } /* end if (tmp-file) */
	} /* end if (ok) */

	if (f_akopts) {
	    f_akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_name) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *pr,cchar **ev,int f)
{
	int		rs ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->envv = ev ;
	sip->w = (f) ? pcsnsreq_fullname : pcsnsreq_pcsname ;
	sip->f.full = f ;
	rs = pcsns_open(&sip->ns,pr) ;
	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = pcsns_close(&sip->ns) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_all(SUBINFO *sip,FILEBUF *ofp)
{
	SYSUSERNAMES	u ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = sysusernames_open(&u,NULL)) >= 0) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    while ((rs = sysusernames_readent(&u,ubuf,ulen)) > 0) {
		rs = subinfo_user(sip,ofp,ubuf,rs) ;
		wlen += rs ;
		if (rs < 0) break ;
	    } /* end while (reading entreies) */
	    rs1 = sysusernames_close(&u) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sysusernames) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_all) */


/* ARGSUSED */
static int subinfo_args(SUBINFO *sip,FILEBUF *ofp,ARGINFO *aip,BITS *bop,
		cchar *afn)
{
	const int	argc = aip->argc ;
	int		rs = SR_OK ;
	int		ai ;
	int		wlen = 0 ;
	int		f ;
	cchar		**argv = aip->argv ;
	cchar		*cp ;
	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
		cp = argv[ai] ;
	        if (cp[0] != '\0') {
	    	    rs = subinfo_user(sip,ofp,cp,-1) ;
	    	    wlen += rs ;
		}
	    }
	} /* end for (handling positional arguments) */
	if (rs >= 0) {
	    rs = subinfo_af(sip,ofp,afn) ;
	    wlen += rs ;
	}
	if ((rs >= 0) && (wlen == 0)) {
	    rs = subinfo_def(sip,ofp) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_args) */


static int subinfo_af(SUBINFO *sip,FILEBUF *ofp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDONLY ;
	    if ((rs = uc_open(afn,of,om)) >= 0) {
		FILEBUF		afile, *afp = &afile ;
		const int	to = -1 ;
		const int	afd = rs ;
		if ((rs = filebuf_start(afp,afd,0L,0,0)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
		    int		cl ;
		    cchar	*cp ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = filebuf_readline(afp,lbuf,llen,to)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
				lbuf[((cp-lbuf)+cl)] = '\0' ;
	                        rs = subinfo_users(sip,ofp,cp,cl) ;
				wlen += rs ;
	                    }
	                }

			if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = filebuf_finish(afp) ;
	    	    if (rs >= 0) rs = rs1 ;
	        }  /* end if (filebuf) */
		u_close(afd) ;
	    } /* end if (file) */
	} /* end if (processing argument-list file) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_af) */


static int subinfo_def(SUBINFO *sip,FILEBUF *ofp)
{
	const int	ulen = USERNAMELEN ;
	int		rs ;
	int		wlen = 0 ;
	cchar		**envv = sip->envv ;
	char		ubuf[USERNAMELEN+1] ;
	if ((rs = getuser(envv,ubuf,ulen)) > 0) {
	    rs = subinfo_user(sip,ofp,ubuf,rs) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_def) */


static int subinfo_users(SUBINFO *sip,FILEBUF *ofp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = subinfo_user(sip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_users) */


static int subinfo_user(SUBINFO *sip,FILEBUF *ofp,cchar *sp,int sl)
{
	NULSTR		n ;
	const int	w = sip->w ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*un ;
	if ((rs = nulstr_start(&n,sp,sl,&un)) >= 0) {
	    const int	rlen = REALNAMELEN ;
	    char	rbuf[REALNAMELEN+1] ;
	    if ((rs = pcsns_get(&sip->ns,rbuf,rlen,un,w)) >= 0) {
	        rs = filebuf_print(ofp,rbuf,rs) ;
	        wlen += rs ;
	    }
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_user) */


static int getuser(cchar **envv,char *ubuf,int ulen)
{
	int		rs = SR_OK ;
	cchar		*un = getourenv(envv,VARMOTDUSER) ;
	ubuf[0] = '\0' ;
	if ((un == NULL) || (un[0] == '\0')) {
	    cchar	*uidp = getourenv(envv,VARMOTDUID) ;
	    if ((uidp != NULL) && (uidp[0] != '\0')) {
		int		v ;
		if ((rs = cfdeci(uidp,-1,&v)) >= 0) {
		    const uid_t	uid = v ;
		    rs = getusername(ubuf,ulen,uid) ;
		}
	    } else {
		rs = getusername(ubuf,ulen,-1) ;
	    }
	} else {
	    rs = sncpy1(ubuf,ulen,un) ;
	}
	return rs ;
}
/* end subroutine (getuser) */


