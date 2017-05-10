/* PCNFSD */

/* daemon program to help out PCs */


#define DEBUGS	0
#define DEBUG	1


#ifdef sccs
static char     sccsid[] = "@(#)pcnfsd.c	1.4" ;
#endif

/* * Copyright (c) 1986 by Sun Microsystems, Inc. */

/*
 * pcnfsd.c 
 *
 * 
The PCNFS daemon
is intended to remedy the lack of certain critical generic network
 * services by providing an simple, customizable set of RPC-based
 * mechanisms. For this reason, Sun Microsystems Inc. is distributing it
 * in source form as part of the PC-NFS release. 
 *
 * Background: The first NFS networks were composed of systems running
 * derivatives of the 4.2BSD release of Unix (Sun's, VAXes, Goulds and
 * Pyramids). The immediate utility of the resulting networks was derived
 * not only from NFS but also from the availability of a number of TCP/IP
 * based network services derived from 4.2BSD. Furthermore the thorny
 * question of network-wide user authentication, while remaining a
 * security hole, was solved at least in terms of a convenient usage model
 * by the Yellow Pages distributed data base facility, which allows
 * multiple Unix systems to refer to common password and group files. 
 *
 * The PC-NFS Dilemma: When Sun Microsystems Inc. ported NFS to PC's, two
 * things became apparent. First, the memory constraints of the typical PC
 * meant that it would be impossible to incorporate the pervasive TCP/IP
 * based service suite in a resident fashion. Indeed it was not at all
 * clear that the 4.2BSD services would prove sufficient: with the advent
 * of Unix System V and (experimental) VAX-VMS NFS implementations, we had
 * to consider the existence of networks with no BSD-derived Unix hosts.
 * The two key types of functionality we needed to provide were remote
 * login and print spooling. The second critical issue  was that of user
 * authentication. Traditional time-sharing systems such as Unix and VMS
 * have well- established user authentication mechanisms based upon user
 * id's and passwords: by defining appropriate mappings, these could
 * suffice for network-wide authentication provided that appropriate
 * administrative procedures were enforced. The PC, however, is typically
 * a single-user system, and the standard DOS operating environment
 * provides no user authentication mechanisms. While this is acceptable
 * within a single PC, it causes problems when attempting to connect to a
 * heterogeneous network of systems in which access control, file space
 * allocation, and print job accounting and routing may all be based upon
 * a user's identity. The initial (and default) approach is to use the
 * pseudo-identity 'nobody' defined as part of NFS to handle problems such
 * as this. However, taking ease of use into consideration, it became
 * necessary to provide a mechanism for establishing a user's identity. 
 *
 * Initially we felt that we needed to implement two types of functionality:
 * user authentication and print spooling. (Remote login is addressed by
 * the Telnet module.) Since no network services were defined within the
 * NFS architecture to support these, it was decided to implement them in
 * a fairly portable fashion using Sun's Remote Procedure Call protocol.
 * Since these mechanisms will need to be re-implemented on a variety of
 * software environments, we have tried to define a very general model. 
 *
 * Authentication: NFS adopts the Unix model of using a pair of integers
 * (uid, gid) to define a user's identity. This happens to map tolerably
 * well onto the VMS system. 'pcnfsd' implements a Remote Procedure which
 * is required to map a username and password into a (uid, gid) pair.
 * Since we cannot predict what mapping is to be performed, and since we
 * do not wish to pass clear-text passwords over the net, both the
 * username and the password are mildly scrambled using a simple XOR
 * operation.  The intent is not to be secure (the present NFS architecture
 * is inherently insecure) but to defeat "browsers". 
 *
 * The authentication RPC will be invoked when the user enters the PC-NFS
 * command: 
 *
 * NET NAME user [password|*] 
 *
 *
 * Printing: The availability of NFS file operations simplifies the print
 * spooling mechanisms. There are two services which 'pcnfsd' has to
 * provide:
 *   pr_init:	given the name of the client system, return the
 * name of a directory which is exported via NFS and in which the client
 * may create spool files.
 *  pr_start: given a file name, a user name, the printer name, the client
 * system name and an option string, initiate printing of the file
 * on the named printer. The file name is relative to the directory
 * returned by pr_init. pr_start is to be "idempotent": a request to print
 * a file which is already being printed has no effect. 
 *
 * Intent: The first versions of these procedures are implementations for Sun
 * 2.0/3.0 software, which will also run on VAX 4.2BSD systems. The intent
 * is to build up a set of implementations for different architectures
 * (Unix System V, VMS, etc.). Users are encouraged to submit their own
 * variations for redistribution. If you need a particular variation which
 * you don't see here, either code it yourself (and, hopefully, send it to
 * us at Sun) or contact your Customer Support representative. 
 */




#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<sys/file.h>
#include	<sys/stat.h>
#include	<rpc/rpc.h>
#include	<unistd.h>
#include	<pwd.h>
#include	<shadow.h>
#include	<signal.h>
#include	<string.h>
#include	<crypt.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"cmdopts.h"
#include	"defs.h"




/* * *************** RPC parameters ******************** */

#define	PCNFSDPROG	((long) 150001)
#define	PCNFSDVERS	((long) 1)

/* subroutines */

#define	PCNFSD_AUTH	((long) 1)
#define	PCNFSD_PRINIT	((long) 2)
#define	PCNFSD_PRSTART	((long) 3)


/* * ************* Other #define's ********************** */

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define	zchar		0x5b



/* structures */

/* * *********** XDR structures, etc. ******************** */

enum arstat {
	AUTH_RES_OK, AUTH_RES_FAKE, AUTH_RES_FAIL

} ;

enum pirstat {
	PI_RES_OK, PI_RES_NO_SUCH_PRINTER, PI_RES_FAIL

} ;

enum psrstat {
	PS_RES_OK, PS_RES_ALREADY, PS_RES_NULL, PS_RES_NO_FILE,
	    PS_RES_FAIL

} ;

struct auth_args {
	char           *aa_ident ;
	char           *aa_password ;
} ;

struct auth_results {
	enum arstat     ar_stat ;
	long            ar_uid ;
	long            ar_gid ;
} ;

struct pr_init_args {
	char           *pia_client ;
	char           *pia_printername ;
} ;

struct pr_init_results {
	enum pirstat    pir_stat ;
	char           *pir_spooldir ;
} ;

struct pr_start_args {
	char           *psa_client ;
	char           *psa_printername ;
	char           *psa_username ;
	char           *psa_filename;	/* within the spooldir */
	char           *psa_options ;
} ;

struct pr_start_results {
	enum psrstat    psr_stat ;
} ;


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;

extern char	*strdirname(char *) ;
extern char	*strbasename(char *) ;


/* forward references */

bool_t		xdr_auth_args() ;
bool_t		xdr_auth_results() ;
bool_t		xdr_pr_init_args() ;
bool_t		xdr_pr_init_results() ;
bool_t		xdr_pr_start_args() ;
bool_t		xdr_pr_start_results() ;

int		logfile_printf() ;

char		*sub_authproc() ;
char		*sub_prinit() ;
char		*sub_prstart() ;

void		free_child() ;
void		run_ps630() ;
void		scramble() ;


/* global data */

struct global	g ;

struct ustat     statbuf ;

char		spfname[] = "/etc/shadow" ;
char            pathname[MAXPATHLEN + 1] ;
char            new_pathname[MAXPATHLEN + 1] ;



/* * ********************** main ********************* */




int main(argc, argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	logfile, *lfp = &logfile ;

	struct utsname		uts ;

	struct ustat		sb ;

	int	f1, f2, f3 ;
	int	argr, argl, aol, avl, npa, maxai, kwi ;
	int	ngi, i ;
	int	rs ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	arg_word[4] ;
	char	*errfname = NULL ;
	char	buf[MAXPATHLEN + 1] ;
	char	*cp ;


	if ((argv[0] != NULL) && (argv[0][0] != '\0'))
	    g.progname = strbasename(argv[0]) ;

	else
	    g.progname = DEF_PROGNAME ;

/* early initialization */

	g.efp = efp ;
	g.lfp = lfp ;
	g.logfname = DEF_LOGFILE ;
	g.spooldir = DEF_SPOOLDIR ;
	g.debuglevel = 0 ;

#if	SYSV
	g.f.sysv_ct = TRUE ;
#else
	g.f.sysv_ct = FALSE ;
#endif

/* OK, decide what to do with standard error */

	if (bopen(g.efp,(char *) 2,"dwca",0666) < 0) {

	    close(2) ;

	    errfname = "/var/tmp/pcnfsd" ;
	    if (bopen(g.efp,errfname,"wca",0666) < 0) {

	        errfname = "/dev/console" ;
	        bopen(g.efp,errfname,"wca",0666) ;

	    }
	}

/* process program arguments */

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

#if	DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
#endif

	                aol = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                f_optequal = TRUE ;

	            } else
	                avl = 0 ;

/* do we have a keyword match or should we assume only key letters ? */

	            if ((kwi = matstr(options,aop,aol)) >= 0) {

	                switch (kwi) {

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *aop) {

	                    case 'D':
	                        g.f.debug = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &g.debuglevel) != OK)
					goto badargvalue ;

	                        }

	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* log file */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.logfname = 
					strbasename(avp) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
				g.logfname = strbasename(argp) ;

	                        }

	                        break ;

/* spool area */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            g.spooldir = NULL ;
	                            if (avl) g.spooldir = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.spooldir = argp ;

	                        }

	                        break ;

	                    default:
	                        bprintf(g.efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                    akp += 1 ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (i < MAXPARGS) {

	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXPARGS) {

	            BASET(arg_word,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


/* check arguments */

#if	DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (g.f.debug) {

	    bprintf(g.efp,
	        "%s: debbugging turned on to level %d\n",
	        g.progname,g.debuglevel) ;

	    bcontrol(g.efp,BC_LINEBUF,0) ;

	    bflush(g.efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s [%s]\n",
	        g.progname,VERSION,(g.f.sysv_ct) ? "SYSV" : "BSD") ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

/* check that the log file exists */

	g.f.logging = TRUE ;
	if ((rs = bopen(g.lfp,g.logfname,"wca",0666)) < 0) {

/* try to get rid of the old file and create a new one */

	    g.f.logging = FALSE ;
	    unlink(g.logfname) ;

	    if ((rs = bopen(g.lfp,g.logfname,"wca",0666)) >= 0) {

	        g.f.logging = TRUE ;
	        bcontrol(g.lfp,BC_CHMOD,0666) ;

	    } else {

/* try to create the directory the log file is in if it doesn't exist */

	        strcpy(buf,g.logfname) ;

	        cp = strdirname(buf) ;

	        if (access(cp,W_OK) < 0) {

	            if (mkdir(cp,0777) >= 0) {

	                chmod(cp,0775) ;

	                if (bopen(g.lfp,g.logfname,"wca",0666) >= 0)
	                    g.f.logging = TRUE ;

	            }

	        }

	    }

	} /* end if (opening the log file) */



/* check access of the spool directory */

	if ((stat(g.spooldir, &statbuf) != 0) || 
	    (! (statbuf.st_mode & S_IFDIR))) goto badspooldir ;

	if (access(g.spooldir,W_OK) != 0) goto badspooldir ;


/* get who we are */

	uname(&uts) ;

	g.nodename = uts.nodename ;
	g.domain = NULL ;
	if ((cp = strchr(uts.nodename,'.')) != NULL) {

	    *cp++ = '\0' ;
	    g.domain = cp ;
	}

/* make a log entry if we can */

	cp = "" ;
	if (g.domain != NULL) cp = g.domain ;

	logfile_printf("program started host=%s%s%s\n",g.nodename,
	    (g.domain == NULL) ? "" : ".",cp) ;

/* does this machine use shadow passwords ? */

	g.f.shadow = FALSE ;
	g.f.useshadow = FALSE ;
	if (stat(spfname,&sb) >= 0) {

	    if (g.debuglevel > 1) bprintf(g.efp,
	        "%s: a shadow password file exists\n",
	        g.progname) ;

	    g.f.shadow = TRUE ;
	    if (access(spfname,R_OK) >= 0)
	        g.f.useshadow = TRUE ;

	} /* end if */


/* OK, start in */

	bflush(g.efp) ;

	rs = OK ;
	if (fork() == 0) {

#ifdef	COMMENT
/*  Comment out for now */

	    if ((f1 = open("/dev/null", O_RDONLY)) == -1) {

	        bprintf(g.efp, 
	            "pcnfsd: couldn't open /dev/null\r\n") ;

	        exit(1) ;
	    }

	    if ((f2 = open("/dev/console", O_WRONLY)) == -1) {

	        bprintf(g.efp, 
	            "pcnfsd: couldn't open /dev/console\r\n") ;

	        exit(1) ;
	    }

	    if ((f3 = open("/dev/console", O_WRONLY)) == -1) {

	        bprintf(g.efp, 
	            "pcnfsd: couldn't open /dev/console\r\n") ;

	        exit(1) ;
	    }

	    dup2(f1, 0) ;

	    dup2(f2, 1) ;

	    dup2(f3, 2) ;

/* end of commented out stuff */

#else
	    close(0) ;

	    close(1) ;
#endif

/* register ourself with the RPC BIND daemon */

	bprintf(g.efp,"%s: registering as version %d\n",
		g.progname,PCNFSDVERS) ;

	    rpc_reg(PCNFSDPROG, PCNFSDVERS, PCNFSD_AUTH, sub_authproc,
	        xdr_auth_args, xdr_auth_results,NULL) ;

	    rpc_reg(PCNFSDPROG, PCNFSDVERS, PCNFSD_PRINIT, sub_prinit,
	        xdr_pr_init_args, xdr_pr_init_results,NULL) ;

	    rpc_reg(PCNFSDPROG, PCNFSDVERS, PCNFSD_PRSTART, sub_prstart,
	        xdr_pr_start_args, xdr_pr_start_results,NULL) ;

/* run it through the loops */

	    svc_run() ;

	    bprintf(g.efp, "%s: error - 'svc_run' returned\n",
	        g.progname) ;

	    rs = BAD ;

	} /* end if (fork) */

done:
	if (g.f.logging) bclose(g.lfp) ;

earlyret:
	bclose(g.efp) ;

	return rs ;

usage:
	bprintf(g.efp,"%s: USAGE> %s [-VDv] [-l[=log]] [-s spooldir]\n",
	    g.progname,g.progname) ;

	rs = BAD ;
	goto done ;

badspooldir:
	bprintf(g.efp,
	    "%s: bad spool directory \"%s\"\n", 
	    g.progname,g.spooldir) ;

	rs = BAD ;
	goto done ;

badargextra:
	bprintf(efp,"%s: there is no value associated with this option\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badret:
	rs = BAD ;
	goto done ;

}
/* end subroutine (main) */


/* * ******************* RPC procedures ************** */

char *sub_authproc(a)
struct auth_args	*a ;
{
	static struct auth_results r ;

	struct passwd	*p ;

	struct spwd	*spp ;

	struct ustat	sb ;

	int             c1, c2 ;
	int		f_psswdv = FALSE ;
	int		f_passed = FALSE ;
	int		f_cmp ;

	char            username[32 + 1] ;
	char            password[64 + 1] ;
	char		*pw_given, *pw_stored ;


	r.ar_stat = AUTH_RES_FAIL ;	/* assume failure */
	scramble(a->aa_ident, username) ;

	scramble(a->aa_password, password) ;

#if DEBUG
	if (g.debuglevel > 1)
	    bprintf(g.efp, "%s: AUTHPROC username=%s\r\n", 
	        g.progname,username) ;
#endif DEBUG

/* find this user by name */

	p = getpwnam(username) ;

	if (p == NULL) {

	    logfile_printf("authproc: user not found, u=%s\n",
	        username) ;

	    return ((char *) &r) ;
	}

	c1 = strlen(password) ;

/* does this machine use shadow passwords ? */

	if (g.f.useshadow) {

	    if (g.debuglevel > 1) bprintf(g.efp,
	        "%s: using shadow passwords\n",
	        g.progname) ;

	        if ((spp = getspnam(username)) == NULL) {

	            logfile_printf("authproc: user not found in shadow file, u=%s\n",
	                username) ;

	            if (g.debuglevel > 0) bprintf(g.efp,
	                "%s: AUTHPROC user not found in shadow file, u=%s\n",
	                g.progname,username) ;

	            return ((char *) &r) ;
	        }

	    pw_stored = spp->sp_pwdp ;
	    c2 = strlen(pw_stored) ;

	    pw_given = crypt(password, pw_stored) ;

	    f_cmp = (strcmp(pw_stored, pw_given) == 0) ;

/* if both the supplied and the stored passwords are zero length, allow it */

	    if (((c1 == 0) && (c2 == 0)) || f_cmp) f_passed = TRUE ;

	} /* end if (using shadow file) */

	if (! f_passed) {

	    pw_stored = p->pw_passwd ;
	    c2 = strlen(pw_stored) ;

	    pw_given = crypt(password, pw_stored) ;

	    f_cmp = (strcmp(pw_stored, pw_given) == 0) ;

/* if both the supplied and the stored passwords are zero length, allow it */

	    if (((c1 == 0) && (c2 == 0)) || f_cmp) f_passed = TRUE ;

	} /* end if (using regular password file) */

	if (f_passed) {

	    r.ar_stat = AUTH_RES_OK ;
	    r.ar_uid = p->pw_uid ;
	    r.ar_gid = p->pw_gid ;
	    logfile_printf("authproc: user validated, u=%s (%d)\n",
	        username,p->pw_uid) ;

	} else {

	    logfile_printf("authproc: user password not valid, u=%s\n",
	        username) ;

#if	DEBUG
	    if (g.debuglevel > 1) {

	        bprintf(g.efp,"%s: AUTHPROC user password not valid, u=%s\n",
	            g.progname,username) ;

	        bprintf(g.efp,
	            "%s: AUTHPROC p=%s given=%s stored=%s\n",
	            g.progname,password,pw_given,pw_stored) ;

	    }
#endif

	}

	return ((char *) &r) ;
}
/* end subroutine (sub_authproc) */


char	*sub_prinit(pi_arg)
struct pr_init_args *pi_arg ;
{
	static struct pr_init_results pi_res ;

	int             dir_mode = 0777 ;


/* get pathname of current directory and return to client */

	sprintf(pathname,"%s/%s",g.spooldir,
	    pi_arg->pia_client) ;

/* ignore the return code */

	mkdir(pathname,0775) ;

	if ((stat(pathname, &statbuf) != 0) || 
	    (! (statbuf.st_mode & S_IFDIR))) {

	    bprintf(g.efp,
	        "%s: unable to create spool directory \"%s\"\n",
	        g.progname,pathname) ;

/* null to tell client bad vibes */

	    pathname[0] = '\0' ;
	    pi_res.pir_stat = PI_RES_FAIL ;

	} else {

	    pi_res.pir_stat = PI_RES_OK ;
	}

	pi_res.pir_spooldir = &pathname[0] ;
	chmod(pathname, dir_mode) ;

	logfile_printf("prinit: client=%s stat=%d\n",
	    pi_arg->pia_client,pi_res.pir_stat) ;

#if DEBUG
	if (g.debuglevel > 1)
	    bprintf(g.efp, "%s: PRINIT pathname=%s\n", 
	        g.progname,pathname) ;
#endif DEBUG

	return ((char *) &pi_res) ;
}
/* end subroutine (sub_prinit) */


char	*sub_prstart(ps_arg)
struct pr_start_args *ps_arg ;
{
	static struct pr_start_results ps_res ;

	struct passwd  *p ;

	long		myseed ;
	long		rnum ;

	int             pid ;
	int		z ;
	int		file_mode = 0600 ;

	char		snum[20] ;
	char		cmdbuf[(2 * MAXPATHLEN) + 1] ;


/* when child terminates it sends a signal which we must get */

#ifdef	SYSV
	signal(SIGCLD, free_child) ;
#else
	signal(SIGCHLD, free_child) ;
#endif	SYSV

	sprintf(pathname,"%s/%s/%s",
	    g.spooldir, ps_arg->psa_client,
	    ps_arg->psa_filename) ;

#if DEBUG
	if (g.debuglevel > 1) {

	    bprintf(g.efp, 
	        "%s: PRSTART pathname=%s\n", 
	        g.progname,pathname) ;

	    bprintf(g.efp, 
	        "%s: PRSTART username= %s\n", 
	        g.progname,ps_arg->psa_username) ;

	    bprintf(g.efp, 
	        "%s: PRSTART client= %s\n", 
	        g.progname,ps_arg->psa_client) ;

	    bprintf(g.efp, 
	        "%s: PRSTART printer= %s\n", 
	        g.progname,ps_arg->psa_printername) ;

	}
#endif DEBUG

	logfile_printf("prstart: entered, c=%s u=%s\n",
	    ps_arg->psa_client,ps_arg->psa_username) ;

	logfile_printf("prstart: d=%s f=%s\n",
	    ps_arg->psa_printername, ps_arg->psa_filename) ;

/* start in */

	if (stat(pathname, &statbuf) != 0) {

/*
* We can't stat the file. Let's try appending '.spl' and
* see if it's already in progress. 
*/

#if DEBUG
	    if (g.debuglevel > 1)
	        bprintf(g.efp, "%s: ...can't stat it\n",
	            g.progname) ;
#endif DEBUG

	    strcat(pathname, ".spl") ;

	    if (stat(pathname, &statbuf) != 0) {

/* it really doesn't exist */

#if DEBUG
	        if (g.debuglevel > 1) bprintf(g.efp, 
	            "%s: ...PRSTART returns PS_RES_NO_FILE\n",
	            g.progname) ;
#endif DEBUG

	        logfile_printf("prstart: no file, c=%s u=%s\n",
	            ps_arg->psa_client,ps_arg->psa_username) ;

	        ps_res.psr_stat = PS_RES_NO_FILE ;
	        return ((char *) &ps_res) ;

	    } /* end if (no file found) */

/* it is already on the way */

#if DEBUG
	    if (g.debuglevel > 1) bprintf(g.efp, 
	        "%s: ...PRSTART returns PS_RES_ALREADY\n",
	        g.progname) ;
#endif DEBUG

	    logfile_printf("prstart: already, c=%s u=%s\n",

	        ps_arg->psa_client,ps_arg->psa_username) ;

	    ps_res.psr_stat = PS_RES_ALREADY ;
	    return ((char *) &ps_res) ;

	} /* end if (couldn't get a 'stat') */

	if (statbuf.st_size == 0) {

/* it is an empty file - don't print it, just kill it */

	    unlink(pathname) ;

#if DEBUG
	    if (g.debuglevel > 1)
	        bprintf(g.efp, "%s: ...PRSTART returns PS_RES_NULL\n",
	            g.progname) ;
#endif DEBUG

	    logfile_printf("prstart: empty file, c=%s u=%s\n",
	        ps_arg->psa_client,ps_arg->psa_username) ;

	    ps_res.psr_stat = PS_RES_NULL ;
	    return ((char *) &ps_res) ;

	} /* end if (size was zero) */

/*
* The file is real, has some data, and is not already going out.
* We rename it by appending '.spl' and exec "lpr" to do the
* actual work. 
*/

	strcpy(new_pathname, pathname) ;

	strcat(new_pathname, ".spl") ;

#if DEBUG
	if (g.debuglevel > 1)
	    bprintf(g.efp,
	        "%s: ...renaming %s -> %s\n", 
	        g.progname,pathname, new_pathname) ;
#endif DEBUG

/* see if the new filename exists so as not to overwrite it */

	for (z = 0 ; z < 100 ; z += 1) {

	    if (stat(new_pathname, &statbuf) == 0) {

/* rebuild a new name */

	        strcpy(new_pathname, pathname);

	        myseed = 1 ;
	        srand48(myseed) ;

/* get a number */

	        sprintf(snum,"%ld",lrand48());

	        strncat(new_pathname, snum, 3) ;

	        strcat(new_pathname, ".spl");		/* new spool file */

#if	DEBUG
	        if (g.debuglevel > 1) bprintf(g.efp, 
	            "%s: ...created new spl file -> %s\n", 
	            g.progname,new_pathname) ;
#endif	DEBUG

	    } else
	        break ;

	} /* end for */

	if (rename(pathname, new_pathname) != 0) {

/*
* CAVEAT: Microsoft changed rename for Microsoft C V3.0.
* Check this if porting to Xenix. 
*/

/* should never happen */

	    bprintf(g.efp,
	        "pcnfsd: spool file rename (%s->%s) failed\n",
	        pathname, new_pathname) ;

	    logfile_printf("prinit: 'rename' failed\n") ;

	    ps_res.psr_stat = PS_RES_FAIL ;
	    return ((char *) &ps_res) ;
	}

	bflush(g.efp) ;

	if ((pid = fork()) == 0) {

#if DEBUG
	    if (g.debuglevel > 1) bprintf(g.efp, 
	        "%s: ...print options = >%s<\n", 
	        g.progname,ps_arg->psa_options) ;
#endif DEBUG

/* put stuff into the environment for the UUEXEC command */

	sprintf(cmdbuf,"UU_MACHINE=%s",
	    ps_arg->psa_client) ;

	putenv(cmdbuf) ;

	sprintf(cmdbuf,"UU_USER=%s",
	    ps_arg->psa_username) ;

	putenv(cmdbuf) ;

/* check for options ?? (what are these and where do they come from) */

	    if (ps_arg->psa_options[1] == 'd') {

/*
* This is a Diablo print stream. Apply the ps630
* filter with the appropriate arguments. 
*/

#if DEBUG
	        if (g.debuglevel > 1)
	            bprintf(g.efp, "%s: ...run_ps630 invoked\n",
	                g.progname) ;
#endif DEBUG
	        run_ps630(new_pathname, ps_arg->psa_options) ;

	    } /* end if ('d' option) */

/* prep to spawn child for unison needs */

	    p = getpwnam(ps_arg->psa_username) ;

/* spawn the process and set the UID and GID */

	    chown(new_pathname,p->pw_uid,-1) ;

	    setuid(p->pw_uid) ;

	    setgid(p->pw_gid) ;

	    chmod(new_pathname, file_mode) ;

/* try to change to the user's HOME directory */

	chdir(p->pw_dir) ;

	    sprintf(cmdbuf,"exec %s %s -l post -d %s %s",
		UUEXEC,
	        DEF_PRINTCMD,
	        ps_arg->psa_printername,
	        new_pathname) ;

	    system(cmdbuf) ;

	    logfile_printf("prinit: 'exec' failed\n") ;

	    bprintf(g.efp,"%s: exec prt failed",
	        g.progname) ;

	    bclose(g.efp) ;

	    exit(0);	/* end of child process */

	} /* end if (child) */

/* parent process continues here */

	if (pid == -1) {

#if DEBUG
	    if (g.debuglevel > 1) bprintf(g.efp, 
	        "%s: ...PRSTART returns PS_RES_FAIL\n",
	        g.progname) ;
#endif DEBUG

	    logfile_printf("prinit: fork failed") ;

	    ps_res.psr_stat = PS_RES_FAIL ;

	} else {

#if DEBUG
	    if (g.debuglevel > 1) bprintf(g.efp, 
	        "%s: ...forked child #%d\n", 
	        g.progname,pid) ;
#endif DEBUG


#if DEBUG
	    if (g.debuglevel > 1) bprintf(g.efp, 
	        "%s: ...PRSTART returns PS_RES_OK\n",
	        g.progname) ;
#endif DEBUG

	    logfile_printf("prinit: queued OK") ;

	    ps_res.psr_stat = PS_RES_OK ;

	} /* end if */

	logfile_printf("prinit: exiting, rs=%d\n",ps_res.psr_stat) ;

	return ((char *) &ps_res) ;
}
/* end subroutine (sub_prstart) */


char	*mapfont(f, i, b)
char            f ;
char            i ;
char            b ;
{
	static char     fontname[64] ;


	fontname[0] = 0;	/* clear it out */

	switch (f) {

	case 'c':
	    strcpy(fontname, "Courier") ;
	    break ;

	case 'h':
	    strcpy(fontname, "Helvetica") ;
	    break ;

	case 't':
	    strcpy(fontname, "Times") ;
	    break ;

	default:
	    strcpy(fontname, "Times-Roman") ;
	    goto exit ;
	}

	if (i != 'o' && b != 'b') {	/* no bold or oblique */

	    if (f == 't')	/* special case Times */
	        strcat(fontname, "-Roman") ;

	    goto exit ;
	}

	strcat(fontname, "-") ;

	if (b == 'b')
	    strcat(fontname, "Bold") ;

	if (i == 'o')		/* o-blique */
	    strcat(fontname, f == 't' ? "Italic" : "Oblique") ;

exit:
	return (&fontname[0]) ;
}
/* end subroutine (mapfont) */


/*
 * run_ps630 performs the Diablo 630 emulation filtering process. ps630 is
 * currently broken in the Sun release: it will not accept point size or
 * font changes. If your version is fixed, define the symbol
 * PS630_IS_FIXED and rebuild pcnfsd. 
 */


void run_ps630(file, options)
char           *file ;
char           *options ;
{
	int             i ;

	char            tmpfile[256] ;
	char            commbuf[256] ;


	strcpy(tmpfile, file) ;

	strcat(tmpfile, "X");	/* intermediate file name */

#ifdef PS630_IS_FIXED
	sprintf(commbuf, "ps630 -s %c%c -p %s -f ",
	    options[2], options[3], tmpfile) ;

	strcat(commbuf, mapfont(options[4], options[5], options[6])) ;
	strcat(commbuf, " -F ") ;
	strcat(commbuf, mapfont(options[7], options[8], options[9])) ;
	strcat(commbuf, "  ") ;
	strcat(commbuf, file) ;
#else PS630_IS_FIXED

/*
		 * The pitch and font features of ps630 appear to be broken at
		 * this time. If you think it's been fixed at your site, define
		 * the compile-time symbol `ps630_is_fixed'. 
		 */

	sprintf(commbuf, "/usr/local/bin/ps630 -p %s %s", tmpfile, file) ;

#endif PS630_IS_FIXED


	if (i = system(commbuf)) {

/*
* Under (un)certain conditions, ps630 may return -1
* even if it worked. Hence the commenting out of this
* error report. 
*/

	    bprintf(g.efp, "\r\npcnfsd: nrun_ps630 rc=%d\r\n", i) ;

/* exit(1); */

	}

	if (rename(tmpfile, file)) {

	    perror("run_ps630: rename") ;

	    exit(1) ;
	}
}
/* end subroutine (run_ps630) */


void free_child(a)
int	a ;
{
	int             pid ;
	int             pstatus ;


	pid = wait(&pstatus);	/* clear exit of child process */

#if DEBUG
	if ((g.debuglevel > 1) || pstatus)
	    bprintf(g.efp, 
	        "%s: FREE_CHILD process #%d exited with status ^x%04X\n",
	        g.progname,pid, pstatus) ;
#endif DEBUG

}
/* end subroutine (free_child) */


/* * ************** Support procedures *********************** */

void scramble(s1, s2)
char           *s1 ;
char           *s2 ;
{
	while (*s1) {

	    *s2++ = (*s1 ^ zchar) & 0x7f ;
	    s1++ ;
	}
	*s2 = 0 ;
}
/* end subroutine (scramble) */


/* * *************** XDR procedures ***************** */

bool_t xdr_auth_args(xdrs, aap)
XDR            *xdrs ;
struct auth_args *aap ;
{

	return (xdr_string(xdrs, &aap->aa_ident, 32) &&
	    xdr_string(xdrs, &aap->aa_password, 64)) ;
}

bool_t xdr_auth_results(xdrs, arp)
XDR            *xdrs ;
struct auth_results *arp ;
{

	return (xdr_enum(xdrs, (int *) &arp->ar_stat) &&
	    xdr_long(xdrs, &arp->ar_uid) &&
	    xdr_long(xdrs, &arp->ar_gid)) ;
}

bool_t xdr_pr_init_args(xdrs, aap)
XDR            *xdrs ;
struct pr_init_args *aap ;
{

	return (xdr_string(xdrs, &aap->pia_client, 64) &&
	    xdr_string(xdrs, &aap->pia_printername, 64)) ;
}

bool_t xdr_pr_init_results(xdrs, arp)
XDR            *xdrs ;
struct pr_init_results *arp ;
{

	return (xdr_enum(xdrs, (int *) &arp->pir_stat) &&
	    xdr_string(xdrs, &arp->pir_spooldir, 255)) ;
}

bool_t xdr_pr_start_args(xdrs, aap)
XDR            *xdrs ;
struct pr_start_args *aap ;
{


	return (xdr_string(xdrs, &aap->psa_client, 64) &&
	    xdr_string(xdrs, &aap->psa_printername, 64) &&
	    xdr_string(xdrs, &aap->psa_username, 64) &&
	    xdr_string(xdrs, &aap->psa_filename, 64) &&
	    xdr_string(xdrs, &aap->psa_options, 64)) ;
}

bool_t xdr_pr_start_results(xdrs, arp)
XDR            *xdrs ;
struct pr_start_results *arp ;
{

	return (xdr_enum(xdrs, (int *) &arp->psr_stat)) ;
}



