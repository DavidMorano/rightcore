/* main (LOGDIR) */

/* program to return a user's home login directory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history :

	= 1989-03-01, David A­D­ Morano

        This subroutine was originally written. This whole program, LOGDIR, is
        needed for use on the Sun CAD machines because Sun doesn't support
        LOGDIR or LOGNAME at this time. There was a previous program but it is
        lost and not as good as this one anyway. This one handles NIS+ also.
        (The previous one didn't.)


	= 1998-06-01, David A­D­ Morano

	I enhanced the program a little to print out some other user
	information besides the user's name and login home directory.


	= 1999-03-01, David A­D­ Morano

	I enhanced the program to also print out effective UID and
	effective GID.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Sysnopsis:

	$ logdir [username]
	$ logname [username] [keyword(s) [...]]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<realname.h>
#include	<mallocstuff.h>
#include	<pwfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	PWENTRY_BUFLEN
#define	PWENTRY_BUFLEN	2048
#endif

#ifndef	BUFLEN
#define	BUFLEN		LINEBUFLEN
#endif


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	matstr(cchar **,int,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	lastlogin(char *,uid_t,time_t *,char *,char *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	udomain(const char *,char *,int,const char *) ;
extern int	isdigitlatin(int) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* forward references */


/* local global variables */


/* local structures */

static cchar	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_of,
	argopt_overlast
} ;

static cchar	*qopts[] = {
	"logname",
	"username",
	"uid",
	"gid",
	"groupname",
	"gecos",
	"gecosname",
	"mailname",
	"homedir",
	"shell",
	"name",
	"fullname",
	"organization",
	"nodename",
	"hostname",
	"domainname",
	"nisdomain",
	"password",
	"passwd",
	"euid",
	"egid",
	"realname",
	"account",
	"bin",
	"office",
	"wphone",
	"hphone",
	"printer",
	"lstchg",
	"lastlog",
	"udomain",
	"loghost",
	NULL
} ;

enum qopts {
	qopt_logname,
	qopt_username,
	qopt_uid,
	qopt_gid,
	qopt_groupname,
	qopt_gecos,
	qopt_gecosname,
	qopt_mailname,
	qopt_homedir,
	qopt_shell,
	qopt_name,
	qopt_fullname,
	qopt_organization,
	qopt_nodename,
	qopt_hostname,
	qopt_domain,
	qopt_nisdomain,
	qopt_password,
	qopt_passwd,
	qopt_euid,
	qopt_egid,
	qopt_realname,
	qopt_account,
	qopt_bin,
	qopt_office,
	qopt_wphone,
	qopt_hphone,
	qopt_printer,
	qopt_lstchg,
	qopt_lastlog,
	qopt_udomain,
	qopt_loghost,
	qopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct proginfo	pi, *pip = &pi ;
	struct ustat	sb ;
	struct passwd	*pep ;
	struct group	*gep = NULL ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		nisfile, *nfp = &nisfile ;
	PWENTRY		entry ;
	time_t		daytime = time(NULL) ;
	const uid_t	uid_cur = getuid() ;

	int	argr, argl, aol, akl, avl ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	rs, rs1 ;
	int	len ;
	int	sl, ci ;
	int	uid ;
	int	ex = EX_INFO ;
	int	fd_debug ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	entrybuf[PWENTRY_BUFLEN + 1] ;
	const char	*ofname = NULL ;
	const char	*pwfname = NULL ;
	const char	*un = NULL ;
	const char	*cp, *cp2 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	sfbasename(argv[0],-1,&cp) ;
	pip->progname = cp ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	            if (isdigitlatin(ach)) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&argvalue) < 0))
	                    goto badarg ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

/* output file name */
	                    case argopt_of:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                ofname = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                ofname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
				const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* alternate passwd file */
	                        case 'p':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pwfname = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

				if (rs < 0) break ;
	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                ex = EX_USAGE ;
	                bprintf(efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: special debugging turned on\n") ;
#endif


/* open the output file */

	if (ofname != NULL) {
	    rs = bopen(ofp,ofname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: done w/ opening output \n") ;
#endif


/* get the first positional argument as the username to key on */

	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i))
	        break ;

	} /* end for */

	if (i <= maxai)
	    BACLR(argpresent,i) ;

	if ((i > maxai) || (strcmp(argv[i],"-") == 0)) {
	    char	ubuf[USERNAMELEN+1] ;
	    if (getusername(ubuf,USERNAMELEN,-1) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: username() username=%s\n",buf) ;
#endif

	        f_self = TRUE ;
	        un = mallocstr(ubuf) ;

	    }

	} else
	    un = argv[i] ;

	f_entok = FALSE ;
	if (pwfname == NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: about to get PW entry, un=%s\n",un) ;
#endif

	    if (un != NULL) {

	        rs = getpwentry_name(un,&entry,entrybuf,PWENTRY_BUFLEN) ;

	        if ((rs < 0) && isdigit(un[0])) {

	            if ((rs = cfdeci(un,-1,&uid)) >= 0)
	                rs = getpwentry_uid(uid,&entry,
	                    entrybuf,PWENTRY_BUFLEN) ;

	        }

	    } else
	        rs = getpwentry_uid(uid_cur,&entry,entrybuf,PWENTRY_BUFLEN) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: getpwentry() rs=%d\n",rs) ;
#endif

	} else {
	    PWFILE	pf ;

	    if ((rs = pwfile_open(&pf,pwfname)) >= 0) {

	        rs = SR_NOENT ;
	        if (un != NULL)
	            rs = pwfile_fetchuser(&pf,un,NULL,
	                &entry,entrybuf,PWENTRY_BUFLEN) ;

	        pwfile_close(&pf) ;
	    } /* end if (opened PWFILE DB) */

	} /* end if (system or file) */

	if (rs >= 0)
	    f_entok = TRUE ;


/* perform the default actions */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: default actions\n") ;
#endif

	if (strcmp(pip->progname,"logdir") == 0) {

	    if (rs >= 0) {

	        if (npa <= 1)
	            bprintf(ofp,"%s\n",entry.dir) ;

	    } else if (! pip->f.quiet)
	        bprintf(efp,"%s: user \"%s\" not found\n",
	            pip->progname,un) ;

	} else {

	    if (rs >= 0) {

	        if (npa <= 1)
	            bprintf(ofp,"%s\n",un) ;

	    } else if (! pip->f.quiet)
	        bprintf(efp,"%s: user \"%s\" not found\n",
	            pip->progname,un) ;

	} /* end if ('logdir' or 'logname') */


	ex = EX_NOUSER ;
	if (rs < 0)
	    goto baduser ;


/* initialize some stuff */

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	ex = EX_OK ;


/* loop through the arguments processing them */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: checking for positional arguments\n") ;
#endif

	pan = 1 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: positional argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

	        if ((ci = matostr(qopts,2,argv[i],-1)) >= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("main: config keyword=%s i=%d\n",
	                    argv[i],ci) ;
#endif

	            switch (ci) {

	            case qopt_logname:
	            case qopt_username:
	                cp = entry.username ;
	                bprintf(ofp,"%s\n",((cp != NULL) ? cp : "")) ;

	                break ;

	            case qopt_uid:
	                if (f_entok)
	                    bprintf(ofp,"%d\n",entry.uid) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_gid:
	                if (f_entok)
	                    bprintf(ofp,"%d\n",entry.gid) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_groupname:
	                if (f_self) {
	                    gid_t	gid_cur = getgid() ;

	                    gep = getgrgid(gid_cur) ;

	                } else if (f_entok)
	                    gep = getgrgid(entry.gid) ;

	                if (gep != NULL)
	                    bprintf(ofp,"%s\n",gep->gr_name) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_gecos:
	                if (f_entok)
	                    bprintf(ofp,"%s\n", entry.gecos) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_gecosname:
	                if (f_entok) {
	                    rs1 = mkgecosname(buf,BUFLEN,entry.gecos) ;
	                    bprintline(ofp,buf,rs1) ;
	                } else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_homedir:
	                if (f_entok)
	                    bprintf(ofp,"%s\n",entry.dir) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_shell:
	                if (f_entok)
	                    bprintf(ofp,"%s\n",entry.shell) ;

	                else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_name:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("NAME") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.realname ;

	                if (cp != NULL) {
	                    realname	rn ;
			    if ((rs = realname_start(&rn,cp,-1)) >= 0) {

	                    sl = realname_name(&rn,buf,BUFLEN) ;

	                    bprintf(ofp,"%s\n",buf) ;

	                    realname_finish(&rn) ;
			    } /* end if (realname) */
	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_mailname:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("NAME") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.realname ;

	                if (cp != NULL) {
	                    realname	rn ;
			    if ((rs = realname_start(&rn,cp,-1)) >= 0) {

	                    sl = realname_mailname(&rn,buf,BUFLEN) ;

	                    bprintf(ofp,"%s\n",buf) ;

	                    realname_finish(&rn) ;
			    } /* end if (realname) */
	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_fullname:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("FULLNAME") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.realname ;

	                if (cp != NULL) {
	                    realname	rn ;
			    if ((rs = realname_start(&rn,cp,-1)) >= 0) {

	                    sl = realname_fullname(&rn,buf,BUFLEN) ;

	                    bprintf(ofp,"%s\n",buf) ;

	                    realname_finish(&rn) ;
			    } /* end if (realname) */
	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_organization:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("ORGANIZATION") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.organization ;

	                bprintf(ofp,"%s\n",((cp != NULL) ? cp : "")) ;

	                break ;

	            case qopt_nodename:
	                if (nodename[0] == '\0')
	                    getnodedomain(nodename,domainname) ;

	                bprintf(ofp,"%s\n", nodename) ;

	                break ;

	            case qopt_domain:
	            case qopt_udomain:
	                if (f_entok) {
	                    char	udomainname[MAXHOSTNAMELEN + 1] ;

	                    cp = entry.username ;
	                    rs = udomain(NULL,udomainname,-1,cp) ;

	                    if (rs <= 0) {

	                        if (nodename[0] == '\0')
	                            getnodedomain(nodename,domainname) ;

	                        bprintf(ofp,"%s",domainname) ;

	                    } else
	                        bwrite(ofp,udomainname,rs) ;

	                    bputc(ofp,'\n') ;

	                } else {

	                    if (nodename[0] == '\0')
	                        getnodedomain(nodename,domainname) ;

	                    bprintf(ofp,"%s\n",domainname) ;

	                }

	                break ;

	            case qopt_hostname:
	                if (nodename[0] == '\0')
	                    getnodedomain(nodename,domainname) ;

	                cp = "" ;
	                if (nodename[0] != '\0') {

	                    cp = nodename ;
	                    if (domainname[0] != '\0') {

	                        cp = buf ;
	                        bufprintf(buf,BUFLEN,
	                            "%s.%s",
	                            nodename,domainname) ;

	                    }

	                } /* end if (nodename) */

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_nisdomain:
	                cp = "" ;
	                sl = 1 ;
	                if (bopen(nfp,NISDOMAINNAME,"r",0666) >= 0) {

	                    if ((len = breadline(nfp,buf,BUFLEN)) > 0)
	                        sl = sfshrink(buf,len,&cp) ;

	                    bclose(nfp) ;
	                } /* end if (reading NIS file) */

/* remove all trailing dots from the NIS domain name */

	                while ((sl > 0) && (cp[sl - 1] == '.'))
	                    sl -= 1 ;

	                bprintline(ofp,cp,sl) ;

	                break ;

	            case qopt_password:
	            case qopt_passwd:
	                if (f_entok && (entry.password != NULL)) {
	                    bprintf(ofp,"%s\n",entry.password) ;
	                } else
	                    bprintf(ofp,"\n") ;

	                break ;

	            case qopt_euid:
	                bprintf(ofp,"%d\n",geteuid()) ;

	                break ;

	            case qopt_egid:
	                bprintf(ofp,"%d\n",getegid()) ;

	                break ;

	            case qopt_realname:
	                cp = "" ;
	                if (f_entok && (entry.realname != NULL))
	                    cp = entry.realname ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_account:
	                cp = "" ;
	                if (f_entok && (entry.account != NULL))
	                    cp = entry.account ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_bin:
	                cp = "" ;
	                if (f_entok && (entry.bin != NULL))
	                    cp = entry.bin ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_office:
	                cp = "" ;
	                if (f_entok && (entry.office != NULL))
	                    cp = entry.office ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_wphone:
	                cp = "" ;
	                if (f_entok && (entry.wphone != NULL))
	                    cp = entry.wphone ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_hphone:
	                cp = "" ;
	                if (f_entok && (entry.hphone != NULL))
	                    cp = entry.hphone ;

	                bprintf(ofp,"%s\n",cp) ;

	                break ;

	            case qopt_printer:
	                cp = NULL ;
	                if (f_self)
	                    cp = getenv("PRINTER") ;

	                if ((cp == NULL) && f_entok)
	                    cp = entry.printer ;

	                bprintf(ofp,"%s\n",((cp != NULL) ? cp : "")) ;

	                break ;

	            case qopt_lstchg:
	                if (f_entok)
	                    bprintf(ofp,"%ld\n",entry.lstchg) ;

	                else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_lastlog:
	                if (f_entok) {

	                    time_t	t ;

	                    char	hostname[16 + 1], line[8 + 1] ;
	                    char	timebuf[40] ;


	                    rs = lastlogin(NULL,entry.uid,
	                        &t,hostname,line) ;

	                    if (rs >= 0) {

	                        bprintf(ofp,"%s %-8s %16s",
	                            timestr_log(t,timebuf),
	                            line,hostname) ;

	                        if (pip->verboselevel >= 1) {

	                            bprintf(ofp," (%17s)",
	                                timestr_elapsed((daytime - t),
	                                timebuf)) ;

	                        }

	                        bputc(ofp,'\n') ;

	                    } else
	                        bputc(ofp,'\n') ;

	                } else
	                    bputc(ofp,'\n') ;

	                break ;

	            case qopt_loghost:
	                {
	                    char	loghostbuf[MAXHOSTNAMELEN + 1] ;


	                    rs = getloghost(loghostbuf,MAXHOSTNAMELEN) ;

	                    if (rs > 0)
	                        bprintf(ofp,"%s\n",loghostbuf) ;

	                    else
	                        bputc(ofp,'\n') ;

	                } /* end block */

	                break ;

	            default:
	                bprintf(efp,
	                    "%s: extra positional arguments ignored\n",
	                    pip->progname) ;

	            } /* end switch */

	        } else {

	            ex = EX_DATAERR ;
	            bprintf(ofp,"*\n") ;

	        } /* end if */

	    } /* end if (got a positional argument) */

	} /* end for (handling positional arguments) */


/* we are done */
done:
ret2:

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: program finishing\n",
	        pip->progname) ;
#endif

	bclose(ofp) ;

/* early return thing */
retearly:
ret1:
	bclose(efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the information type thing */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [username|- [keyword(s) ...]]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	bprintf(efp,"%s:    possible configuration keywords are :\n",
	    pip->progname) ;

	for (i = 0 ; qopts[i] != NULL ; i += 1) {

	    if ((i % USAGECOLS) == 0)
	        bprintf(efp,"%s: \t",
	            pip->progname) ;

	    bprintf(efp,"%-16s",qopts[i]) ;

	    if ((i % USAGECOLS) == 3)
	        bprintf(efp,"\n") ;

	} /* end for */

	if ((i % USAGECOLS) != 0)
	    bprintf(efp,"\n") ;

	goto retearly ;

/* the bad things */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto retearly ;

/* not found */
baduser:
	ex = EX_NOUSER ;
	if (! pip->f.quiet)
	    bprintf(efp,"%s: could not get information for \"%s\" (rs %d)\n",
	        pip->progname,un,rs) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open the output file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(efp) ;

	goto retearly ;
}
/* end subroutine (main) */



