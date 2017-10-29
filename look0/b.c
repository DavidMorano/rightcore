/* main (look) */

/* this is the LOOK program (for looking up words in a dictionary) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_CHAR		1		/* use 'char(3dam)' */


/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * David Hitz of Auspex Systems, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Copyright © 1998,2011 David A­D­ Morano.  All rights reserved. */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n" ;
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)look.c	8.1 (Berkeley) 6/14/93" ;
#endif /* not lint */


/*
 * look -- find lines in a sorted list.
 *
 * The man page said that TABs and SPACEs participate in -d comparisons.
 * In fact, they were ignored.  This implements historic practice, not
 * the manual page.
 */


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<vecobj.h>
#include	<char.h>
#include	<naturalwords.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_look.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#undef	MAXSTRLEN
#define	MAXSTRLEN	256		/* GNU man-page say this */

#ifndef	WORDF
#define	WORDF		"share/dict/words"
#endif

/*
 * FOLD and DICT convert characters to a normal form for comparison,
 * according to the user specified flags.
 *
 * DICT expects integers because it uses a non-character value to
 * indicate a character which should not participate in comparisons.
 */
#define NO_COMPARE	(-2)

#if	CF_CHAR
#define FOLD(c)		CHAR_TOFC(c)
#define DICT(c)		(isdict(c) ? (c) & 0xFF /* int */ : NO_COMPARE)
#else /* CF_CHAR */
#define FOLD(c) (isupper(c) ? tolower(c) : (char) (c))
#define DICT(c) (isalnum(c) ? (c) & 0xFF /* int */ : NO_COMPARE)
#endif /* CF_CHAR */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	strnndictcmp(const char *,int,const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdict(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		sort ;
	uint		w ;
	uint		f ;
	uint		d ;
	uint		q ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, final, changed ;
	struct proginfo	*pip ;
} ;

struct word {
	const char	*wp ;
	int		wl ;
} ;


/* forward references */

int		p_look(int,const char **,const char **,void *) ;

static int	usage(struct proginfo *) ;

static int	procsort(struct proginfo *,void *,const char *) ;
static int	procsortread(struct proginfo *,vecobj *,const char *,int) ;
static int	procsortout(struct proginfo *,void *,vecobj *) ;

static int	process(struct proginfo *,void *,const char *,
			const char *) ;
static int	look(struct proginfo *,void *,
			const char *,const char *,const char *) ;
static int	mksword(struct proginfo *,char *,int,const char *) ;

static int      compare(struct proginfo *,
			const char *,const char *,const char *,int *) ;

static int	print_from(struct proginfo *,void *,const char *,
			const char *,const char *) ;

static int	wordcmp(const void *,const void *) ;

#if	CF_DEBUGS || CF_DEBUG
static int	strtlen(const char *) ;
#endif

const char	*getourenv(const char **,const char *) ;

static const char    *binary_search(struct proginfo *,const char *,
			const char *,const char *) ;

static const char    *linear_search(struct proginfo *,const char *,
			const char *,const char *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*prnames[] = {
	"NCMP",
	"LOCAL",
	"GNU",
	"EXTRA",
	NULL
} ;

static const char	*wordfiles[] = {
	"/usr/add-on/ncmp/share/dict/words",
	"/usr/add-on/local/share/dict/words",
	"/usr/add-on/gnu/share/dict/words",
	"/usr/share/lib/dict/words",
	"/usr/share/dict/words",
	"/usr/dict/words",
	NULL
} ;


/* exported subroutines */


int b_look(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = p_look(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_look) */


int p_look(argc,argv,envv,contextp)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	BITS		pargs ;
	SHIO		errfile ;
	SHIO		ofile, *ofp = &ofile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*file ;
	const char	*cp ;

	char	string[MAXSTRLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	pip->lip = lip ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;

	file = NULL ;

	string[0] = '\0' ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

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

	        if (isdigit(argp[1])) {

	            argval = (argp + 1) ;

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

/* keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-file */
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

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
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
	                    int	kc = (*akp & 0xff) ;

	                    switch (kc) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* dictionary order */
	                    case 'd':
	                        lip->final.d = TRUE ;
	                        lip->f.d = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.d = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* fold-case */
	                    case 'f':
	                        lip->final.f = TRUE ;
	                        lip->f.f = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.f = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'q':
	                        lip->f.q = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.q = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* sort mode */
	                    case 's':
	                        lip->f.sort = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.sort = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* whole word */
	                    case 'w':
	                        lip->f.w = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.w = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
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

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(pip->efp,SHIO_CSETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* more initialization */

	if (efname == NULL) afname = getourenv(envv,VARAFNAME) ;

/* process */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    switch (pan) {

	    case 0:
	        if (! lip->final.d) lip->f.d = TRUE ;
	        if (! lip->final.f) lip->f.f = TRUE ;
	        strwcpy(string,cp,MAXSTRLEN) ;
	        break ;

	    case 1:
	        file = cp ;
	        break ;

	    } /* end switch */

	    pan += 1 ;

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: prefix=%s\n",string) ;
	    debugprintf("main: file=%s\n",file) ;
	}
#endif

/* find a file if we don't have one already */

	if (file == NULL) {

	    rs = SR_NOENT ;
	    if ((cp = getourenv(envv,VARWORDS)) != NULL) {
	        file = cp ;
	        rs = perm(cp,-1,-1,NULL,R_OK) ;
	    }

	    if (rs < 0) {

	        for (i = 0 ; prnames[i] != NULL ; i += 1) {

	            cp = getourenv(envv,prnames[i]) ;

	            if (cp == NULL) continue ;

	            file = tmpfname ;
	            mkpath2(tmpfname,cp,WORDF) ;

	            rs = perm(tmpfname,-1,-1,NULL,R_OK) ;
	            if (rs >= 0) break ;

	        } /* end for */

	    } /* end if (moderately tough measures) */

	    if (rs < 0) {

	        for (i = 0 ; wordfiles[i] != NULL ; i += 1) {

	            rs = perm(wordfiles[i],-1,-1,NULL,R_OK) ;

	            file = (char *) wordfiles[i] ;
	            if (rs >= 0) break ;

	        } /* end for */

	    } /* end if (tough measures) */

	    if (rs < 0)
	        file = WORDSFNAME ;

	} /* end if (finding file) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: dict=%s\n",pip->progname,file) ;
	    shio_printf(pip->efp, "%s: fold=%u dictorder=%u\n",
	        pip->progname,lip->f.f,lip->f.d) ;
	}

/* continue */

#if	CF_DEBUGS
	debugprintf("main: file=%s f_dict=%u f_fold=%u\n",
	    file,lip->f.f,lip->f.f) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: dict=%s\n", file) ;
	    debugprintf("main: f_sort=%u f_dict=%u f_fold=%u\n",
	        lip->f.sort,lip->f.f,lip->f.f) ;
	}
#endif

/* print out a sorted list */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {

	    if (lip->f.sort) {
	        rs = procsort(pip,ofp,file) ;
	    } else {
	        rs = process(pip,ofp,file,string) ;
	    }

	    shio_close(ofp) ;
	} /* end if (output-open) */

done:
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} else if ((rs == 0) && (ex == EX_OK)) {
	    ex = 1 ;
	}

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

ret1:
	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: bad or invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-df] [-w] <string> [<file>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-of <ofile>] [-s]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procsort(pip,ofp,fname)
struct proginfo	*pip ;
void		*ofp ;
const char	fname[] ;
{
	VECOBJ		wlist ;
	const int	n = 250000 ;
	int		rs ;
	int		size ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procsort: ent fname=%s\n",fname) ;
#endif

	if (fname[0] == '-') fname = "/dev/stdin" ;

	size = sizeof(struct word) ;
	if ((rs = vecobj_start(&wlist,size,n,0)) >= 0) {
	    const mode_t	operms = 0666 ;
	    const int		oflags = O_RDONLY ;
	    if ((rs = uc_open(fname,oflags,operms)) >= 0) {
	        struct ustat	sb ;
	        int	fd = rs ;
	        if (((rs = u_fstat(fd,&sb)) >= 0) && S_ISREG(sb.st_mode)) {
	            const size_t	fs = (size_t) (sb.st_size & LONG_MAX) ;
	            size_t 		ps = getpagesize() ;
	            if (fs > 0) {
	                size_t		ms = MAX(fs,ps) ;
	                const int	mp = PROT_READ ;
	                const int	mf = MAP_SHARED ;
	                void		*md ;
	                if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                    int		madv = MADV_SEQUENTIAL ;
	                    int		mdl = ms ;
	                    const char	*mdp = md ;
	                    const caddr_t	ma = (const caddr_t) md ;

	                    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	                        rs = procsortread(pip,&wlist,mdp,mdl) ;
	                        if (rs >= 0) {
	                            madv = MADV_RANDOM ;
	                            rs = uc_madvise(ma,ms,madv) ;
	                        }
	                    } /* end if (memory-advice) */

	                    if (rs >= 0)
	                        rs = vecobj_sort(&wlist,wordcmp) ;

	                    if (rs >= 0) {
	                        rs = procsortout(pip,ofp,&wlist) ;
	                        wlen += rs ;
	                    }

	                    u_munmap(md,ms) ;
	                } /* end if (map file) */
	            } /* end if (non-zero-length) */
	        } else {
	            if (rs >= 0) rs = SR_NOTSUP ;
	        }
	        u_close(fd) ;
	    } /* end if (file-open) */
	    vecobj_finish(&wlist) ;
	} /* end if (word-list) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procsort: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsort) */


/* read all of the words in */
static int procsortread(pip,wlp,mdp,mdl)
struct proginfo	*pip ;
vecobj		*wlp ;
const char	*mdp ;
int		mdl ;
{
	struct word	w ;
	int		rs = SR_OK ;
	int		len ;
	int		ll ;
	int		wl ;
	const char	*tp ;
	const char	*lp ;
	const char	*wp ;

	while ((tp = strnchr(mdp,mdl,'\n')) != NULL) {
	    len = ((tp + 1) - mdp) ;

	    lp = mdp ;
	    ll = (len - 1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procsort: w=>%t<\n",lp,ll) ;
#endif

	    if ((tp = strnchr(lp,ll,'#')) != NULL)
	        ll = (tp - lp) ;

	    if ((wl = sfshrink(lp,ll,&wp)) > 0) {
	        w.wp = wp ;
	        w.wl = wl ;
	        rs = vecobj_add(wlp,&w) ;
	    }

	    mdp += len ;
	    mdl -= len ;

	    if (rs < 0) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (procsortread) */


static int procsortout(pip,ofp,wlp)
struct proginfo	*pip ;
void		*ofp ;
vecobj		*wlp ;
{
	struct word	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;

	for (i = 0 ; vecobj_get(wlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs = shio_printline(ofp,ep->wp,ep->wl) ;
	    wlen += rs ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsortout) */


static int process(pip,ofp,dfname,string)
struct proginfo	*pip ;
void		*ofp ;
const char	dfname[] ;
const char	string[] ;
{
	int		rs ;

	if ((rs = uc_open(dfname,O_RDONLY,0666)) >= 0) {
	    int	fd = rs ;
	    if ((rs = uc_fsize(fd)) >= 0) {
	        size_t		ms = rs ;
	        const int	mp = PROT_READ ;
	        const int	mf = MAP_SHARED ;
	        const char	*md ;
	        if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	            const char	*front = md ;
	            const char	*back = (md+ms) ;

	            rs = look(pip,ofp,string,front,back) ;

	            u_munmap(md,ms) ;
	        } /* end if (mapfile) */
	    } /* end if (fsize) */
	    u_close(fd) ;
	} /* end if (file-open) */

	return rs ;
}
/* end subroutine (process) */


static int look(pip,ofp,string,front,back)
struct proginfo	*pip ;
void		*ofp ;
const char	*string, *front, *back ;
{
	struct locinfo	*lip = pip->lip ;
	const int	slen = NATURALWORDLEN ;
	int		rs ;
	int		c = 0 ;
	char		sword[NATURALWORDLEN+1] ;

	if ((rs = mksword(pip,sword,slen,string)) > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("main/look: rewritten string=>%s<\n",sword) ;
	        debugprintf("main/look: binary_search()\n") ;
	    }
#endif

	    front = binary_search(pip,sword,front,back) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/look: linear_search\n") ;
#endif

	    front = linear_search(pip,sword,front,back) ;

	    if (front) {
	        rs = print_from(pip,ofp,sword,front,back) ;
	        c = rs ;
	    }

	} /* end if (mksword) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/look: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (look) */


static int mksword(pip,rbuf,rlen,s)
struct proginfo	*pip ;
char		rbuf[] ;
int		rlen ;
const char	*s ;
{
	struct locinfo	*lip = pip->lip ;
	int		rs = SR_OK ;
	int		i ;
	int		ch, dch, fch ;
	const char	*readp ;
	char		*writep ;

	readp = s ;
	writep = rbuf ;
	for (i = 0 ; (i < rlen) && (s[i] != '\0') ; i += 1) {
	    ch = MKCHAR(*readp++) ;
	    if (ch == 0) break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/look: och=%c\n",ch) ;
#endif

	    dch = (lip->f.d) ? DICT(ch) : ch ;



#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/look: ch=%c dch=%c(%02x)\n",ch,dch,dch) ;
#endif

	    if (dch != NO_COMPARE) {
		int	fch = (lip->f.f) ? FOLD(dch) : fch ;
	        *(writep++) = fch ;
	    }

	} /* end for */
	*writep = '\0' ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mksword) */


/*
 * Binary search for "string" in memory between "front" and "back".
 *
 * This routine is expected to return a pointer to the start of a line at
 * *or before* the first word matching "string".  Relaxing the constraint
 * this way simplifies the algorithm.
 *
 * Invariants:
 * 	'front' points to the beginning of a line at or before the first
 *	matching string.
 *
 * 	'back' points to the beginning of a line at or after the first
 *	matching line.
 *
 * Base of the Invariants.
 * 	front = NULL;
 *	back = EOF;
 *
 * Advancing the Invariants:
 *
 * 	p = first newline after halfway point from front to back.
 *
 * 	If the string at "p" is not greater than the string to match,
 *	p is the new front.  Otherwise it is the new back.
 *
 * Termination:
 *
 * 	The definition of the routine allows it [to] return at any point,
 *	since front is always at or before the line to print.
 *
 * 	In fact, it returns when the chosen "p" equals "back".  This
 *	implies that there exists a string [that] is [at] least half as long as
 *	(back - front), which in turn implies that a linear search will
 *	be no more expensive than the cost of simply printing a string or two.
 *
 * 	Trying to continue with binary search at this point would be
 *	more trouble than it's worth.
 */

#define	SKIP_PAST_NEWLINE(p, back) \
	while (((p) < (back)) && (*(p)++ != '\n')) ;

static const char *binary_search(pip,string,front,back)
struct proginfo	*pip ;
const char	*string, *front, *back ;
{
	struct locinfo	*lip = pip->lip ;
	int		rc = 0 ;
	const char	*p ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/binary_search: string=%s\n",string) ;
	    debugprintf("main/binary_search: front(%p) back(%p)\n",
	        front,back) ;
	}
#endif

	p = front + ((back - front) / 2) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/binary_search: p(%p)\n",p) ;
	}
#endif

	SKIP_PAST_NEWLINE(p, back) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/binary_search: after skip\n") ;
	    debugprintf("main/binary_search: len(p)=%u\n",strtlen(p)) ;
	    debugprintf("main/binary_search: starting p=%s\n",p) ;
	}
#endif

/* If the file changes underneath us, make sure we don't infinitely loop. */

	while ((p < back) && (back > front)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main/binary_search: p=%t\n",p,strtlen(p)) ;
	    }
#endif

	    if ((rc = compare(pip,string, p, back,NULL)) > 0) {
	        front = p ;
	    } else
	        back = p ;

	    p = front + ((back - front) / 2) ;
	    SKIP_PAST_NEWLINE(p, back) ;

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/binary_search: ret front(%p)\n",front) ;
#endif

	return (front) ;
}
/* end subroutine (binary_search) */


/*
 * Find the first line that starts with string, linearly searching from front
 * to back.
 *
 * Return NULL for no such line.
 *
 * This routine assumes:
 * 	+ front points at the first character in a line
 *	+ front is before or at the first line to be printed
 */

static const char *linear_search(pip,string,front,back)
struct proginfo	*pip ;
const char	*string, *front, *back ;
{
	struct locinfo	*lip = pip->lip ;
	int		rc = 0 ;

	while (front < back) {

	    rc = compare(pip,string, front, back,NULL) ;
	    if (rc <= 0) break ;

	    SKIP_PAST_NEWLINE(front, back) ;

	} /* end while */

	return ((rc == 0) ? front : NULL) ;
}
/* end subroutine (linear_search) */


/* print as many lines as match string, starting at front */
static int print_from(pip,ofp,string,front,back)
struct proginfo	*pip ;
void		*ofp ;
const char	*string, *front, *back ;
{
	struct locinfo	*lip = pip->lip ;
	int		rs = SR_OK ;
	int		m ;
	int		c = 0 ;
	int		f_mat = TRUE ;
	const char	*tp ;

#if	CF_DEBUGS
	debugprintf("main/print_from: s=>%s<\n",string) ;
#endif

	while ((rs >= 0) && (front < back) && 
	    (compare(pip,string,front,back,NULL) == 0)) {

	    if ((tp = strchr(front,'\n')) != NULL) {

	        if (lip->f.w) {

	            f_mat = (compare(pip,string,front,tp,&m) == 0) ;
	            f_mat = f_mat && (string[m] == '\0') && (front[m] == '\n') ;
	            if (f_mat) {
	                c += 1 ;
	                if (! lip->f.q) {
	                    rs = shio_printline(ofp,front,(tp - front)) ;
	                }
	            }

	        } else {

	            c += 1 ;
	            if (! lip->f.q) {
	                rs = shio_printline(ofp,front,(tp - front)) ;
	            }

	        } /* end if (whole or partial) */

	        front = (tp + 1) ;
	    } else
	        front = back ;

	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (print_from) */


/*
 * Return LESS, GREATER, or EQUAL depending on how the string1 compares with
 * string2 (s1 ??? s2).
 *
 * 	o Matches up to len(s1) are EQUAL.
 *	o Matches up to len(s2) are GREATER.
 *
 * Compare understands about the -f and -d flags, and treats comparisons
 * appropriately.
 *
 * The string "s1" is NUL terminated.  The string s2 is '\n' terminated (or
 * "back" terminated).
 */

static int compare(pip,s1,s2,back,np)
struct proginfo	*pip ;
const char	*s1, *s2, *back ;
int		*np ;
{
	struct locinfo	*lip = pip->lip ;
	int		ch1, ch2 ;
	int		fch1, fch2 ;
	int		i, j ;
	int		rc = 0 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	{
	    int	s2len = MIN(strtlen(s2),(back - s2)) ;
#ifdef	COMMENT
	    debugprintf("look/compare: s1(%p) s2(%p)\n",s1,s2) ;
	    debugprinthex("look/compare: s1=",COLUMNS,s1,-1) ;
	    debugprinthex("look/compare: s2=",COLUMNS,s2,s2len) ;
#endif /* COMMENT */
	    debugprintf("look/compare: s1=%s\n",s1) ;
	    debugprintf("look/compare: s2=%t\n",s2,s2len) ;
	}
#endif /* CF_DEBUGS */

	i = 0 ;
	j = 0 ;
	while (s1[i] && ((s2+j) < back) && (s2[j] != '\n')) {

	    ch1 = (s1[i] & UCHAR_MAX) ;
	    ch2 = (s2[j] & UCHAR_MAX) ;

#ifdef	OPTIONAL
	    if (lip->f.d && (! isdict(ch1))) {
	        i += 1 ;		/* ignore character in comparison */
	        continue ;
	    }
#endif /* OPTIONAL */

	    if (lip->f.d && (! isdict(ch2))) {
	        j += 1 ;		/* ignore character in comparison */
	        continue ;
	    }

	    if (lip->f.f) {
	        fch1 = FOLD(ch1) ;
	        fch2 = FOLD(ch2) ;
	        rc = (fch1 - fch2) ;

#if	CF_DEBUGS
	        debugprintf("look/compare: fch1=%c\n",fch1) ;
	        debugprintf("look/compare: fch2=%c\n",fch2) ;
	        debugprintf("look/compare: rc=%d\n",rc) ;
#endif

	    } else
	        rc = (ch1 - ch2) ;

#if	CF_DEBUGS
	    debugprintf("look/compare: rc=%d\n",rc) ;
#endif

	    if (rc != 0) {
	        f = TRUE ;
	        break ;
	    }

	    i += 1 ;
	    j += 1 ;

	} /* end while */

#ifdef	COMMENT
	if ((! f) && (rc == 0)) {
	    f = ((s1[i] == '\0') && (s2[j] == '\n')) ;
	    if (! f)
	        rc = ((s1[i]) ? 1 : 0) ;
	}
#else /* COMMENT */
	if ((! f) && (rc == 0))
	    rc = ((s1[i]) ? 1 : 0) ;
#endif /* COMMENT */

	if (np != NULL) *np = j ;

#if	CF_DEBUGS
	debugprintf("look/compare: ret i=%u rc=%d\n",i,rc) ;
#endif

	return rc ;
}
/* end subroutine (compare) */


static int wordcmp(v1p,v2p)
const void	*v1p, *v2p ;
{
	struct word	*e1p, **e1pp = (struct word **) v1p ;
	struct word	*e2p, **e2pp = (struct word **) v2p ;
	int		rc = 0 ;

	if (*e1pp == NULL) {
	    rc = 1 ;
	} else {
	    if (*e2pp == NULL) {
	        rc = -1 ;
	    } else {
	        e1p = *e1pp ;
	        e2p = *e2pp ;
	        rc = strnndictcmp(e1p->wp,e1p->wl,e2p->wp,e2p->wl) ;
	    }
	}

	return rc ;
}
/* end subroutine (wordcmp) */


#if	CF_DEBUGS || CF_DEBUG
static int strtlen(s)
const char	s[] ;
{
	int		i ;
	for (i = 0 ; *s && (*s != '\n') ; i += 1) {
	    s += 1 ;
	}
	return i ;
}
/* end subroutine (strtlen) */
#endif /* CF_DEBUGS */


