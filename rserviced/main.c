/* main */

/* Remote Service Listener (RSL) */
/* last modified %G% version %I% */


#define	CF_PRINT	1
#define	CF_FIELD	0


/* revision history:

	= 1991-09-10, David A­D­ Morano


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This is the Remote Service Listener daemon program.
	This program listens to a UNIX file system domain
	directory name for incoming service jobs in the form of
	files.  When a job file is detected, this daemon looks
	up the requested service name in the service name database
	file and forks off the requested service daemon to handle the
	service request.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<field.h>
#include	<srvtab.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MIN(a,b)	(((a) < (b)) ? (a) : (b))


/* function option flags */

#define	FO_LOCK		0x01
#define	FO_READ		0x02
#define	FO_WRITE	0x04


/* external subroutines */

extern int	getpwd(char *,int) ;

extern int	quoted(), expand() ;


/* forwards */


/* other externals */


/* local structures */

struct namelist {
	char	*namep ;
	char	min, max ;
} ;


/* read-only statics */

static unsigned char 	fterms[32] = {
	0x7F, 0xFE, 0xC0, 0xFE,
	0x8B, 0x00, 0x00, 0x24, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;


#define	KEY_CON		0	/* control directory */
#define	KEY_LOG		1	/* log file */
#define	KEY_SRV		2	/* functional command */
#define	KEY_EXP		3	/* environment variable for export */
#define	KEY_WAIT	4	/* lock file wait time out */
#define	KEY_OVERLAST	5	/* number of key types */


struct const namelist		cfk[] = { 	/* configuration file key */
	"control", 1, 7,
	"logfile", 2, 3,
	"service", 1, 3,
	"export", 1, 5,
	"wait",1, 4,
	"", 0, 0,
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	ss ;

	struct field	fsb ;

	struct system	system[NSYSTEM] ;

	struct direct	direntry ;

	struct expand	se ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		configfile, *cfp = &configfile ;
	bfile		logfile, *lfp = &logfile ;
	bfile		tmpfile, *tfp = &tmpfile ;

	long	clock ;

	int	argl, aol ;
	int	i, j, k ;
	int	c ;
	int	len, rs ;
	int	funlen ;
	int	pid, rs_child ;
	int	wtime ;
	int	line ;
	int	npa = 0 ;
	int	nsys = 0 ;
	int	nlenv = 0 ;
	int	pa ;
	int	si ;
	int	funopt ;
	int	fdpipe[2] ;

	int	f_debug = FALSE ;
	int	f_v = FALSE ;
	int	f_release = FALSE ;
	int	f_builtin ;
	int	f_log ;
	int	f_filt = FALSE ;
	int	f_map = FALSE ;
	int	f_dict = FALSE ;

	char	*progname, *argp, *aop ;
	char	*cp ;

	char	*lenviron[NLENV] ;
	char	linebuf[LINELEN] ;
	char	funbuf[LINELEN] ;
	char	buf[BUFLEN] ;
	char	*bp ;
	char	*paname[NUMSCRIPTS] ;	/* script file name [pointer] array */

	char	pwd[PATHLEN] ;
	char	envbuf[ENVLEN], *ebp = envbuf ;
	char	namebuf[REALNAMELEN], *nbp = namebuf ;

	char	tmpfname[PATHLEN] ;
	char	*funname, *sysname = ((char *) 0) ;
	char	*configfname = DEFCONFIGFILE ;
	char	*logfname = DEFLOGFILE ;
	char	*filtfname = DEFFILTFILE ;
	char	*mapfname = DEFMAPFILE ;
	char	*dictfname = DEFDICTFILE ;
	char	*lockfname ;

	char    *releasep = DEFRELEASE ;
	char    *libdirp = DEFLIBDIR ;
	char    *condirp = DEFCONDIR ;
	char    *workdirp = NULL ;

	char	*username ;
	char	*timestr ;


	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	if (bopen(ofp,BOUT,"wct",0666) < 0) return BAD ;

/* perform some initial stuff */

	username = getenv("LOGNAME") ;

	if (username == ((char *) 0)) username = "**unknown**" ;

	cp = getenv("A_SATCONFIG") ;

	if (cp != ((char *) 0)) configfname = cp ;

	cp = getenv("A_SYSTEM") ;

	if (cp != ((char *) 0)) sysname = cp ;

	cp = getenv("A_RELEASE") ;

	if (cp != ((char *) 0)) {

	    releasep = cp ;
	    f_release = 2 ;
	}

	progname = argv[0] ;
	len = strlen(progname) ;

/* set the default function to the name after "sat" in our program name */

	if ((len > 3) && (substring(progname,len,"sat") == 0)) {

	    funname = progname + 3 ;

	}

/* set working directory */

	if (getpwd(pwd,PATHLEN) < 0) workdirp = DEFWORKDIR ;

/* start parsing the arguments */

	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;

	            while (--aol) {

	                akp += 1 ;
	                switch ((int) *aop) {

	                case 'V':
	                    f_v = TRUE ;
	                    bprintf(ofp,"%s version: %s\n",progname,VERSION) ;

	                    break ;

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'c':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) configfname = argp ;

	                    break ;

	                case 'f':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) funname = argp ;

	                    break ;

	                case 'r':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) releasep = argp ;

	                    f_release = 3 ;
	                    break ;

	                case 's':
			case 'm':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) sysname = argp ;

	                    break ;

	                case 'w':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) workdirp = argp ;

	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

			case '?':
	                    goto usage ;

	                } /* end switch */

	            } /* end while */

	        } /* end if */

	    } else {

	        if (npa < NUMSCRIPTS) {

	            if (argl > 0) paname[npa++] = argp ;

	        } else {

	            bprintf(efp,
	                "%s: extra script arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */


/* miscellaneous */


/* check arguments */

	if (f_debug) {

	    bprintf(efp,"function (%s)\n",funname) ;

	}

/* perform initialization processing */


/* check if our function is a built-in one or not */

	f_builtin = FALSE ;
	for (i = 0 ; i < FUN_OVERLAST ; i += 1) {

	    if (strcmp(funtab[i],funname) == 0) {

	        f_builtin = TRUE ;
	        break ;
	    }
	}

	rs = readcon(configfname,&cs) ;

/* after all of this, do we even have a system name ? */

	if (sysname == NULL) {

	    bprintf(efp,"%s: no system was specified or configured\n",
	        progname) ;

	    goto badret ;
	}

/* open the log file */

	f_log = TRUE ;
	if ((rs = bopen(lfp,logfname,"wca",0664)) < 0) {

	    f_log = FALSE ;
	    bprintf(efp,"%s: could not open the log file - rs (%d)\n",
	        progname,rs) ;

	}

	if (f_log) {

	    clock = time(0L) ;

	    timestr = ctime(&clock) ;

	    timestr[19] = '\0' ;
	    bprintf(lfp,"%s %-11s %-16s\n\n",
	        timestr,funname,username,sysname) ;

	}

/* perform some sanity checking on the information that we have so far */

/* does the specified system name exist in the database ? */

	for (si = 0 ; si < nsys ; si += 1) {

	    if (strcmp(sysname,system[si].name) == 0) break ;

	}

	if (si >= nsys) {

	    bprintf(efp,"%s: could not find system \"%s\" in the database\n",
	        progname,sysname) ;

	    goto badretlog ;
	}

/* finish populating the system record which has been chosen */

	if (system[si].filtfile[0] == '\0')
	    strncpy(system[si].filtfile,filtfname,PATHLEN) ;

	if (system[si].mapfile[0] == '\0')
	    strncpy(system[si].mapfile,mapfname,PATHLEN) ;

	if (system[si].dictfile[0] == '\0')
	    strncpy(system[si].dictfile,dictfname,PATHLEN) ;

/* get the release if it is specified in the environment */

	cp = getenv("A_RELEASE") ;

	if ((cp == NULL) || (strcmp(cp,releasep) != 0)) {

	    bp = ebp ;
	    cp = "A_RELEASE" ;
	    strcpy(ebp,cp) ;

	    ebp += strlen(cp) ;

	    strcat(ebp,"=") ;

	    ebp += 1 ;
	    strcat(ebp,releasep) ;

	    ebp += strlen(releasep) ;

	    putenv(bp) ;

	}

/* get the library directory if it is specified in the environment */

	cp = getenv("A_LIBDIR") ;

	if (cp != NULL) libdirp = cp ;

	else {

	    if ((cp == NULL) || (strcmp(cp,libdirp) != 0)) {

	        bp = ebp ;
	        cp = "A_LIBDIR" ;
	        strcpy(ebp,cp) ;

	        ebp += strlen(cp) ;

	        strcat(ebp,"=") ;

	        ebp += 1 ;
	        strcat(ebp,libdirp) ;

	        ebp += strlen(libdirp) ;

	        putenv(bp) ;

	    }
	}

/* if we have a release, we can do more with mapping and dictionary files */

	if (f_release) {

	    if ((strlen(libdirp) + strlen(releasep) + 3) > PATHLEN) {

	        bprintf(efp,
	            "%s: cannot load path of SAT mapping and diction files\n",
	            progname) ;

	    } else {

	        sprintf(system[si].mapfile,"%s/%s.m",
	            libdirp,releasep) ;

	        sprintf(system[si].dictfile,"%s/%s.d",
	            libdirp,releasep) ;

	    }
	}

/* create the lock file name */

	if (condirp == NULL)
	    len = sprintf(buf,"%s%s",LOCKPREFIX,sysname) ;

	else
	    len = sprintf(buf,"%s/%s%s",condirp,LOCKPREFIX,sysname) ;

	if ((len + 1) > (REALNAMELEN - (nbp - namebuf))) goto badnamestore ;

	lockfname = nbp ;
	strncpy(nbp,buf,len) ;

	nbp += len ;
	*nbp++ = '\0' ;

/* create a temporary pipe file */

	pid = getpid() ;

	if ((strlen(workdirp) + 16) > PATHLEN) {

	    bprintf(efp,
	        "%s: the temporary file prefix is too big\n",
	        progname) ;

	    goto badret ;
	}

	j = pid ;
	for (i = 0 ; i < TMPTRIES ; i += 1) {

	    if (workdirp == NULL)
	        sprintf(tmpfname,"%s%08X",TMPPREFIX,j + i) ;

	    else
	        sprintf(tmpfname,"%s/%s%08X",workdirp,TMPPREFIX,j + i) ;

	    if (mknod(tmpfname,0010664,0) >= 0) break ;

	}

	if (i >= TMPTRIES) {

	    bprintf(efp,
	        "%s: could not create the temporary pipe file - errno (%d)\n",
	        progname,errno) ;

	    goto badret ;
	}

	if (f_debug) bprintf(efp,"created the temp pipe file\n") ;

/* open the temporary file */

	if ((rs = bopen(tfp,tmpfname,"rw",0664)) < 0) {

	    bprintf(efp,
	        "%s: could not open the pipe file (%d)\n",
	        progname,rs) ;

	    goto badret ;
	}

	if (f_debug) bprintf(efp,"opened the temp pipe file\n") ;

/* fill in the substitution expansion structure */

	se.s = sysname ;
	se.f = filtfname ;
	se.m = mapfname ;
	se.d = dictfname ;
	se.l = (libdirp == NULL) ? "." : libdirp ;
	se.c = (condirp == NULL) ? "." : condirp ;
	se.w = (workdirp == NULL) ? "." : workdirp ;
	se.r = releasep ;

/* loop through the arguments */

	for (pa = 0 ; pa < npa ; pa += 1) {

	    se.a = paname[pa] ;

/* if it was a built-in function, handle it now */

	    if (f_builtin) {

	        for (i = 0 ; i < FUN_OVERLAST ; i += 1) {

	            if (strcmp(funtab[i],funname) == 0) break ;

	        }

	        switch (i) {

	        case FUN_DBQ:
	        case FUN_DB:
	            bprintf(ofp,"A_SATCONFIG=%s\n",configfname) ;

	            bprintf(ofp,"A_LIBDIR=%s\n",libdirp) ;

	            bprintf(ofp,"A_SYSTEM=%s\n",sysname) ;

	            bprintf(ofp,"A_RELEASE=%s\n",releasep) ;

	            bprintf(ofp,"GMACH=%s\n",sysname) ;

	            cp = system[si].gtdev ;
	            if ((cp != NULL) && (strcmp(cp,"-") != 0)) {

	                bprintf(ofp, "GTDEV=%s\n",cp) ;

	            }

	            cp = system[si].gtdev2 ;
	            if ((cp != NULL) && (strcmp(cp,"-") != 0)) {

	                bprintf(ofp, "GTDEV2=%s\n",cp) ;

	            }

	            bprintf(ofp,"A_SATFILTER=%s\n",system[si].filtfile) ;

	            bprintf(ofp,"A_SATMAP=%s\n",system[si].mapfile) ;

	            bprintf(ofp,"A_SATDICT=%s\n",system[si].dictfile) ;

	            for (i = 0 ; i < nlenv ; i += 1) {

	                bprintf(ofp,"%s\n",lenviron[i]) ;

	            }
	            break ;

		case FUN_UNLOCK:
			unlink(lockfname) ;

			break ;

	        default:
	            bprintf(efp,"%s: error in built-in function\n",
	                progname) ;

	        }

	    } else {

	        if (f_debug) bprintf(efp,
	            "%s: you have chosen a custom function\n",progname) ;

	        if ((len = expand(buf,BUFLEN,funbuf,funlen,&se)) < 0) {

	            bflush(efp) ;

	            bprintf(efp,
	                "%s: error in exapnding the function command",
	                progname) ;

	            bprintf(efp," string\n") ;

	            goto badret ;
	        }

	        buf[len] = '\0' ;
	        if ((cp = getenv("SHELL")) == NULL) cp = "/bin/sh" ;

	        if (f_debug || f_v) {

	            bprintf(ofp,
	                "%s: about to execute command string :\n",progname) ;

	            bprintf(ofp,"%W\n",buf,len) ;

	        }

/* check for readability */

	        if (funopt & FO_READ) {

	            if (f_debug)
	                bprintf(efp,"checking argument for readability\n") ;

	        }

/* check for writability */

	        if (funopt & FO_READ) {

	            if (f_debug)
	                bprintf(efp,"checking argument for writability\n") ;

	        }

/* capture the lock if specified */

	        if (funopt & FO_LOCK) {

	            if (f_debug) bprintf(efp,
	                "lock file name \"%s\"\n",lockfname) ;

	            k = wtime ;
	            while (k > 0) {

	                rs = open(lockfname,O_RDWR | O_CREAT | O_EXCL,0664) ;

	                if ((rs < 0) && f_debug) bprintf(efp,
	                    "lock file open errno (%d)\n",errno) ;

	                if (rs >= 0) break ;

	                sleep(1) ;

	                k -= 1 ;
	            }

	            if (k == 0) {

	                bprintf(efp,
	                    "%s: could not capture the system lock\n",
	                    progname) ;

	                goto badret ;
	            }

	        }

/* go for it */

	        if ((pid = fork()) == 0) {

	            execlp(cp,"SATSHELL",tmpfname,0) ;

	            exit(BAD) ;
	        }

/* write the expanded command string to a temporary file */

	        rs = bprintf(tfp,"%W\nexit\n",buf,len) ;

	        if (rs >= 0) rs = bflush(tfp) ;

	        if (rs < 0) {

	            bprintf(efp,
	                "%s: bad write to file (%d)\n",
	                progname,rs) ;

	            goto badret ;
	        }

	        if (f_debug) bprintf(efp,
	            "waiting for child to exit\n") ;

	        while ((rs = wait(&rs_child)) != pid) ;

/* release the lock if it was captured */

	        if (funopt & FO_LOCK) unlink(lockfname) ;

	    }

	} /* end for */

/* perform some cleanup stuff */

/* remove the temporary file */

	bclose(tfp) ;

	unlink(tmpfname) ;

/* end of this run */

#if	CF_PRINT
	if (f_debug) bprintf(efp,"%s: program finishing\n",progname) ;
#endif

	if (f_log) bclose(lfp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	bclose(ofp) ;

	bclose(efp) ;

	return BAD ;

badretlog:
	bclose(lfp) ;

	goto badret ;

usage:
	bprintf(efp,
	    "usage: %s [-s sys] [-r rel] [-v] [-f fun] [-D] ",
	    progname) ;

	bprintf(efp,"[-c config]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

badread:
	bprintf(efp,"%s: bad read on input file (%d)\n",
	    progname,len) ;

	goto badret ;

baddir:
	bprintf(efp,"%s: bad working directory specified - errno %d\n",
	    progname,rs) ;

	goto badret ;

badconfig:
	bclose(cfp) ;

	bprintf(efp,"%s: error in configuration file at line %d\n",
	    progname,line) ;

	goto badret ;

badnamestore:
	bprintf(efp,"%s: not enough space for name storage\n",
	    progname) ;

	goto badret ;

badarg:
	bprintf(efp,"bad argument(s) given\n") ;

	goto badret ;
}


