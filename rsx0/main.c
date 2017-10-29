/* main */

/* program to get time from a network time server host */
/* last modified %G% version %I% */


#define	CF_ALARM		1


/* revision history:

	= 1992-03-01, David A­D­ Morano


*/


/**********************************************************************

	This program will create and send a job to a remote service
	execution daemon.

	Synopsis:

	rsx [-D] [-V] server service [-- service_arguments] < input_file


***********************************************************************/




#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/socket.h>
#include	<sys/in.h>
#include	<signal.h>
#include	<netdb.h>
#include	<errno.h>

#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define		VERSION		"0"
#define		NPARG		10	/* number of positional arguments */

#define		CONNTIMEOUT	120
#define		READTIMEOUT	30
#define		NETTIMELEN	4
#define		BUFLEN		100



/* external subroutines */

extern int	stime() ;

extern int	cfdec() ;

extern struct tm	*localtime() ;


/* external variables */


/* forward references */

int		timeout();




int main(argc, argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;

	bfile		outfile, *ofp = &outfile ;

	struct tm	*local_tsp, *remote_tsp ;

	struct sockaddr_in	server ;
	struct sockaddr		from ;
	struct servent		*sp ;
	struct protoent		*pp ;
	struct hostent		*hp ;

	long		clock, newclock, lw ;

	int		argl, aol, pan ;
	int		pid, i, len, rs ;
	int		s ;
	int		f_debug = FALSE ;
	int		f_usage = FALSE ;
	int		f_settime = FALSE ;
	int		f_log = FALSE ;

	unsigned short	uid ;

	char		*progname, *argp, *aop ;
	char		buf[BUFLEN] ;
	char		*cp ;
	char		*hostname[NPARG] ;
	char		*logfname = NULL ;


	progname = argv[0] ;
	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	pan = 0 ;			/* number of positional so far */
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

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s version: %s\n",
	                        progname,VERSION) ;

	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

			switch (pan) {

		case 0:
	            if (argl > 0) hostname[pan++] = argp ;


		case 1:


		} /* end switch */

	        } else {

	            bprintf(efp,"%s: extra command line arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (f_usage) goto usage ;

	if (f_debug) {

	    bprintf(efp,"%s: about to check arguments\n",
	        progname) ;

	    bflush(efp) ;

	}

/* check arguments */

	if (pan < 1) {

	    bprintf(efp,"%s: no remote server was specified\n",
	        progname) ;

	    goto done ;
	}


/* miscellaneous */

	uid = geteuid() ;

	pid = getpid() ;


	if (f_debug) {

	    bprintf(efp,"%s: got valid arguments\n",
	        progname) ;

	    bflush(efp) ;

	}

/* look up some miscellaneous stuff in various databases */

	if ((sp = getservbyname("time", "tcp")) == ((struct servent *) 0)) {

	    bprintf(efp,
	        "%s: time service not in services database\n",
	        progname) ;

	    goto done ;

	}

	if ((pp = getprotobyname("tcp")) == ((struct protoent *) 0)) {

	    bprintf(efp,
	        "%s: TCP protocol not in protocols database\n",
	        progname) ;

	    goto done ;

	}

	if (f_debug) {

	    bprintf(efp,"%s: got miscellaneous database stuff\n",
	        progname) ;

	    bflush(efp) ;

	}


/* look up the hosts given us and try to connect to one of them */

#if	CF_ALARM
	signal(SIGALRM,timeout) ;
#endif

	for (i = 0 ; i < pan ; i += 1) {

	    if (f_debug) {

	        bprintf(efp,"%s: trying host '%s'\n",
	            progname,hostname[i]) ;

	    }

	    if ((hp = gethostbyname(hostname[i])) == ((struct hostent *) 0)) {

	        bprintf(efp,"%s: host '%s' not in hosts database\n",
	            progname,hostname[i]) ;

	        continue ;
	    }

	    if (f_debug) {

	        bprintf(efp,"%s: looked up the host\n",
	            progname) ;

	        bflush(efp) ;

	    }

/* create a socket */

	if ((s = socket(AF_INET, SOCK_STREAM, pp->p_proto)) == 0) {

	    bprintf(efp,"%s: socket problem (%d)",
	        progname,s) ;

	    goto done ;
	}

	if (f_debug) {

	    bprintf(efp,"%s: created a socket \n",
	        progname) ;

	    bflush(efp) ;

	}

	bzero((char *) &server, sizeof(server)) ;

	server.sin_port = sp->s_port;
	    bcopy(hp->h_addr, (char *) &server.sin_addr, hp->h_length) ;

	    server.sin_family = hp->h_addrtype ;

/* connect to the host */

#if	CF_ALARM
	    alarm(CONNTIMEOUT) ;
#endif

	    if ((rs = connect(s, &server, sizeof(server))) < 0) {

	        bprintf(efp,"%s: connect problem (errno %d)\n",
	            progname,errno);

		close(s) ;

	        continue ;
	    }

	    if (f_debug) {

	        bprintf(efp,"%s: connected to the time server host\n",
	            progname) ;

	        bflush(efp) ;

	    }

#if	CF_ALARM
	    alarm(READTIMEOUT) ;
#endif

	    if ((rs = read(s,buf,NETTIMELEN)) != NETTIMELEN) {

	        if (rs == EINTR) bprintf(efp,"%s: read interrupt\n",
	            progname) ;

	        else if (rs < 0) bprintf(efp,"%s: read error (%d)\n",
	            progname,rs);

	        else bprintf(efp,"%s: didn't read enough from socket\n",
	            progname) ;

		close(s) ;

	        continue ;
	    }

	    close(s) ;

	    break ;

	} /* end for */

	if (i >= pan) {

	    bprintf(efp,"%s: no more hosts to try - exting\n",
	        progname) ;

	    goto done ;
	}

	alarm(0) ;

	memcpy(&lw,buf,4) ;

	newclock = ntohl(lw) ;

	uid = geteuid() ;

	clock = time(0L) ;

	newclock -= EPOCHDIFF ;

	if ((uid == 0) && f_settime) {

	    if (newclock < MINDATE) {

	        bprintf(efp,
	            "%s: didn't get plausible time from time server '%s'\n",
	            progname,hostname[i]) ;

	        goto done ;
	    }

	    if (abs(newclock - clock) > LARGETIME) {

	        bprintf(efp,"%s: remote time is too far off from our own\n",
	            progname) ;

	    } else {

	        if ((newclock - clock) > 0) {

	            if ((newclock - clock) > SMALLTIME) {

	                newclock = clock + SMALLTIME ;

	            }

	            if ((rs = stime(&newclock)) < 0) {

	                bprintf(efp,
	                    "%s: could not set system time (errno %d)\n",
	                    progname,errno) ;

	            } else {

	                if (f_log) {

	                    if ((rs = bopen(lfp,logfname,"wca",0666)) < 0) {

	                        bprintf(efp,
	                            "%s: could not open the log file (%d)\n",
	                            progname,rs) ;

	                    } else {

	                        local_tsp = localtime(&clock) ;

	                        bprintf(lfp,"\nsystem time updated\n") ;

	                        bprintf(lfp,
	                            "local date\t%02d/%02d/%d %02d:%02d:%02d\n",
	                            local_tsp->tm_mon + 1,
	                            local_tsp->tm_mday,
	                            local_tsp->tm_year,
	                            local_tsp->tm_hour,
	                            local_tsp->tm_min,
	                            local_tsp->tm_sec) ;

	                        remote_tsp = localtime(&newclock) ;

	                        bprintf(lfp,
	                            "remote date\t") ;

	                        bprintf(lfp,
	                            "%02d/%02d/%d %02d:%02d:%02d\n",
	                            remote_tsp->tm_mon + 1,
	                            remote_tsp->tm_mday,
	                            remote_tsp->tm_year,
	                            remote_tsp->tm_hour,
	                            remote_tsp->tm_min,
	                            remote_tsp->tm_sec) ;

	                        bclose(lfp) ;

	                    }
	                }

	            }
	        }
	    }
	}

	if ((uid != 0) || (! f_settime) || f_debug) {

	    if ((rs = bopen(ofp,BOUT,"wca",0666)) < 0) {

	        bprintf(efp,
	            "%s: could not open output file (%d)\n",
	            progname,rs) ;

	    } else {

	        remote_tsp = localtime(&newclock) ;

	        bprintf(ofp,
	            "%02d/%02d/%d %02d:%02d:%02d\n",
	            remote_tsp->tm_mon + 1,
	            remote_tsp->tm_mday,
	            remote_tsp->tm_year,
	            remote_tsp->tm_hour,
	            remote_tsp->tm_min,
	            remote_tsp->tm_sec) ;

	        if (f_debug) {

	            bprintf(efp,"%s: GMT in decimal '%ld'\n",
	                progname,newclock) ;

	        }
	    }
	}

/* and exit */
done:
	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "usage: %s [-V] [-s] timehost(s)",progname) ;

	bprintf(efp," [-l logfile] [-D]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;
}
/* end subroutine (main) */


int timeout()
{
	return OK ;
}


