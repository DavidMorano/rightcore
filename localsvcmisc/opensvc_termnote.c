/* opensvc_termnote */

/* LOCAL facility open-service (termnote) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano

        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a facility-open-service module.

	Synopsis:

	int opensvc_termnote(pr,prn,of,om,argv,envv,to)
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
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<termnote.h>
#include	<localmisc.h>

#include	"opensvc_termnote.h"
#include	"defs.h"


/* local defines */

#define	DEFNTERMS	3		/* default number of terminals */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	opentermnote(const char *,const char **,int,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;

extern const char	*getourenv(const char **,const char *) ;


/* local structures */

struct subinfo_flags {
	uint		all:1 ;
	uint		biff:1 ;
	uint		bell:1 ;
	uint		max:1 ;
	uint		akopts:1 ;
} ;

struct subinfo {
	SUBINFO_FL	have, f, changed, final ;
	KEYOPT		akopts ;
	const char	**envv ;
	int		max ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char **) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_opts(SUBINFO *) ;


/* local variables */

static const char *argopts[] = {
	"all",
	"biff",
	"bell",
	"af",
	NULL
} ;

enum argopts {
	argopt_all,
	argopt_biff,
	argopt_bell,
	argopt_af,
	argopt_overlast
} ;

static const char *akonames[] = {
	"all",
	"biff",
	"bell",
	"max",
	NULL
} ;

enum akonames {
	akoname_all,
	akoname_biff,
	akoname_bell,
	akoname_max,
	akoname_overlast
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


int opensvc_termnote(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO	si, *sip = &si ;
	BITS		pargs ;
	VECSTR		names ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		opts ;
	int		v ;
	int		n ;
	int		pan = 0 ;
	int		fd = -1 ;
	int		f_optplus, f_optminus, f_optequal ;
	int		f ;
	const char	**recips = NULL ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*afname = NULL ;
	const char	*cp ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

	rs = subinfo_start(sip,envv) ;
	if (rs < 0) goto ret0 ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	ai = 0 ;
	ai_max = 0 ;
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

	        if (isdigit(argp[1])) {

	            argval = NULL ;

	        } else if (argp[1] == '-') {

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

	                case argopt_all:
	                    sip->f.all = TRUE ;
	                    break ;

	                case argopt_biff:
	                    sip->f.biff = TRUE ;
	                    break ;

	                case argopt_bell:
	                    sip->f.bell = TRUE ;
	                    break ;

/* argument-list file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

	                    case 'a':
	                        sip->f.all = TRUE ;
	                        break ;

	                    case 'b':
	                        sip->f.biff = TRUE ;
	                        break ;

	                    case 'r':
	                        sip->f.bell = TRUE ;
	                        break ;

/* maximum terminals per user */
	                    case 'm':
	                        sip->have.max = TRUE ;
	                        sip->final.max = TRUE ;
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdeci(argp,argl,&v) ;
	                            sip->max = v ;
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&sip->akopts,argp,argl) ;
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

	if (rs < 0) goto badarg ;

/* initialize for action */

	if ((! sip->final.max) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&v) ;
	    sip->max = v ;
	}

	if (rs >= 0)
	    rs = subinfo_opts(sip) ;

	if (rs < 0) goto badarg ;

	if (sip->max < 0) {
	    sip->max = DEFNTERMS ;
	} else if (sip->max == 0) {
	    sip->max = INT_MAX ;
	}

	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&names,0,opts) ;
	if (rs < 0) goto badnamestart ;

/* process an argument list (if we have one) */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        pan += 1 ;
	        if (cp[0] != '\0') {
	            rs = vecstr_adduniq(&names,cp,-1) ;
	        }
	    }

	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) rs = SR_NOENT ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		FIELD		fsb ;
		const int	llen = LINEBUFLEN ;
	        int		len ;
		int		fl ;
	        char		lbuf[LINEBUFLEN + 1] ;
		const char	*fp ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((rs = field_start(&fsb,lbuf,len)) >= 0) {

	                while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	                    if (fl == 0) continue ;

	                    pan += 1 ;
	        	    rs = vecstr_adduniq(&names,fp,fl) ;

	                    if (rs < 0) break ;
	                    if (fsb.term == '#') break ;
	                } /* end while */

	                field_finish(&fsb) ;
	            } /* end if (field) */

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } /* end if (file open) */

	} /* end if (processing file argument file list) */

	n = 0 ;
	if (rs >= 0) {
	    rs = vecstr_getvec(&names,&recips) ;
	    n = rs ;
	}

	if (rs >= 0) {
	    if (n > 0) {

	        opts = 0 ;
	        if (sip->f.all) opts |= TERMNOTE_OALL ;
	        if (sip->f.biff) opts |= TERMNOTE_OBIFF ;
	        if (sip->f.bell) opts |= TERMNOTE_OBELL ;

	        rs = opentermnote(pr,recips,sip->max,opts) ;
	        fd = rs ;

	    } else
	        rs = SR_INVALID ;
	} /* end if (proceeded w/ opentermnote) */

/* finish up */

	rs1 = vecstr_finish(&names) ;
	if (rs >= 0) rs = rs1 ;

/* get out */
badnamestart:
badarg:
	bits_finish(&pargs) ;

badpargs:
	rs1 = subinfo_finish(sip) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs < 0) && (fd >= 0)) {
	    u_close(fd) ;
	    fd = -1 ;
	}

ret0:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_termnote) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,const char **envv)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->envv = envv ;
	sip->max = -1 ;
	rs = keyopt_start(&sip->akopts) ;
	sip->f.akopts = (rs >= 0) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f.akopts) {
	    sip->f.akopts = FALSE ;
	    rs1 = keyopt_finish(&sip->akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_opts(SUBINFO *sip)
{
	KEYOPT		*kop = &sip->akopts ;
	KEYOPT_CUR	kcur ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getourenv(sip->envv,VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		oi ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

	    while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	        if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            switch (oi) {

	            case akoname_all:
	                if (! sip->final.all) {
	                    sip->have.all = TRUE ;
	                    sip->final.all = TRUE ;
	                    sip->f.all = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        sip->f.all = (rs > 0) ;
	                    }
	                }
	                break ;

	            case akoname_biff:
	                if (! sip->final.biff) {
	                    sip->have.biff = TRUE ;
	                    sip->final.biff = TRUE ;
	                    sip->f.biff = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        sip->f.biff = (rs > 0) ;
	                    }
	                }
	                break ;

	            case akoname_bell:
	                if (! sip->final.bell) {
	                    sip->have.bell = TRUE ;
	                    sip->final.bell = TRUE ;
	                    sip->f.bell = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        sip->f.bell = (rs > 0) ;
	                    }
	                }
	                break ;

	            case akoname_max:
	                if (! sip->final.bell) {
	                    sip->have.bell = TRUE ;
	                    sip->final.bell = TRUE ;
	                    sip->f.bell = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        sip->f.bell = (rs > 0) ;
	                    }
	                }
	                break ;

	            } /* end switch */

	            c += 1 ;
	        } else
		    rs = SR_INVALID ;

	   	if (rs < 0) break ;
	    } /* end while (looping through key options) */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_opts) */


