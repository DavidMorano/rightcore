/* fingerd */


#define	CF_DEBUG	1
#define	CF_DEBUGS	0


/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n" ;
#endif /* not lint */

#ifndef lint
/*
static char sccsid[] = "@(#)fingerd.c	8.1 (Berkeley) 6/4/93";
*/
static const char rcsid[] =
"$Id: fingerd.c,v 1.5 1995/12/17 20:25:28 wollman Exp $" ;
#endif /* not lint */



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include	"localmisc.h"
#include "pathnames.h"



/* local defines */

#define	VERSION		"0a"
#define	ENTRIES		50
#define	LINELEN		1024



/* external subroutines */

extern char	*strbasename(char *) ;
extern char	*sfshrink(const char *,int,char **) ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	register FILE *fp ;

	struct hostent *hp ;
	struct sockaddr_in sin ;

	pid_t		cpid ;

	register int	ch ;

	int p[2], logging, secure, sval ;
	int	i, sl ;
	int	debuglevel = 0 ;
	int	f_version = FALSE ;
	int	err_fd ;

	char	**ap, *av[ENTRIES + 1], **comp, line[LINELEN + 1], *prog ;
	char	*lp ;
	char	*progname ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;

	progname = strbasename(argv[0]) ;

	prog = PROG_FINGER ;
	logging = secure = FALSE ;
	for (i = 0 ; i < ENTRIES ; i += 1)
		av[i] = NULL ;

	openlog("fingerd", LOG_PID | LOG_CONS, LOG_DAEMON) ;

	opterr = 0 ;
	while ((ch = getopt(argc, argv, "DVslp:")) != EOF) {

	    switch (ch) {

	    case 'D':
	        debuglevel = 1 ;
	        break ;

	    case 'V':
	        f_version = TRUE ;
	        break ;

	    case 'l':
	        logging = 1 ;
	        break ;
	    case 'p':
	        prog = optarg ;
	        break ;
	    case 's':
	        secure = 1 ;
	        break ;

	    case '?':
	    default:
	        syslog(LOG_NOTICE,"illegal option -- %c", ch) ;

	    } /* end switch */

	} /* end while */

	if (f_version) {

	    fprintf(stderr,"%s: version %s\n",
	        progname,VERSION) ;

	    fclose(stderr) ;

	    goto earlyret ;
	}

	if (logging || (debuglevel > 0)) {

	    sval = sizeof(sin) ;
	    if (getpeername(0, (struct sockaddr *)&sin, &sval) >= 0) {

	        if (hp = gethostbyaddr((char *)&sin.sin_addr.s_addr,
	            sizeof(sin.sin_addr.s_addr), AF_INET))
	            lp = hp->h_name ;

	        else
	            lp = inet_ntoa(sin.sin_addr) ;

	        syslog(LOG_INFO, "from %s", lp) ;

	    } else
	        syslog(LOG_INFO,"getpeername() failed - %s", 
	            strerror(errno)) ;

	} /* end if */

#ifdef	COMMENT
/* * Enable server-side Transaction TCP */
	{
	    int one = 1 ;
	    if (setsockopt(STDOUT_FILENO, IPPROTO_TCP, TCP_NOPUSH, &one, 
	        sizeof(one)) < 0) {

	        logerr("setsockopt(TCP_NOPUSH) failed: %m") ;

	    }
	}
#endif /* COMMENT */

	if (fgets(line, LINELEN, stdin) == NULL)
	    goto earlyret ;

	sl = sfshrink(line,-1,&lp) ;

#if	CF_DEBUGS
	if (debuglevel > 0)
	    debugprintf("main: line> %s\n",lp) ;
#endif

	lp[sl] = '\0' ;
	if (debuglevel > 0)
	    syslog(LOG_DEBUG,"query> %s [%d]\n",lp,sl) ;

	av[0] = "finger" ;
	if (sl > 0) {

	    comp = &av[1] ;
	    av[2] = "--" ;
	    for ((ap = &av[3]) ; ; ) {

#if	CF_DEBUG
	        if (debuglevel > 0)
	            syslog(LOG_DEBUG,"top loop\n") ;
#endif /* CF_DEBUG */

	        if ((*ap = strtok(lp, " \t\r\n")) != NULL) {

	            if (secure && (ap == &av[3])) {

	                if (debuglevel > 0)
	                    syslog(LOG_DEBUG,
	                        "security - no user name suppied\n") ;

	                puts("must provide username\r\n") ;

	                goto earlyret ;
	            }

	            break ;

	        } else
	            *ap = "" ;

	        if (secure && (strchr(*ap, '@') != NULL)) {

	            if (debuglevel > 0)
	                syslog(LOG_DEBUG,
	                    "security - forwarding requested and denied\n") ;

	            puts("forwarding service denied\r\n") ;

	            goto earlyret ;
	        }

/* RFC742: "/[Ww]" == "-l" */
	        if (((*ap)[0] == '/') && (((*ap)[1] == 'W') || 
			((*ap)[1] == 'w'))) {

	            av[1] = "-l" ;
	            comp = &av[0] ;

	        } else if (++ap == (av + ENTRIES))
	            break ;

	        lp = NULL ;

#if	CF_DEBUG
	        if (debuglevel > 0)
	            syslog(LOG_DEBUG,"bottom loop\n") ;
#endif

	    } /* end for */

	} else {

		av[1] = NULL ;
	    comp = &av[0] ;

	}

	if ((lp = strrchr(prog, '/')) != NULL) {
	    *comp = lp + 1 ;

	} else
	    *comp = prog ;

/* some debugging ? */

	if (debuglevel > 0) {

	    syslog(LOG_DEBUG,"prog=\"%s\"\n",
	        ((prog == NULL) ? STDNULLFNAME : prog)) ;

	    for (i = 0 ; av[i] != NULL ; i += 1) {

	        if (av[i] != NULL)
	            syslog(LOG_DEBUG,"av[%d]=\"%s\"\n",
	                i,av[i]) ;

	        if (comp[i] != NULL)
	            syslog(LOG_DEBUG,"comp[%d]=\"%s\"\n",
	                i,comp[i]) ;

	    } /* end for */

	} /* end if (debugging) */

/* continue */

	if (pipe(p) < 0) {

	    syslog(LOG_ERR,"pipe() failed - %s", 
	        strerror(errno)) ;

	    goto earlyret ;
	}

	if ((cpid = vfork()) == 0) {

	    (void) close(p[0]) ;

	    if (p[1] != 1) {

	        (void)dup2(p[1], 1) ;

	        (void)close(p[1]) ;

	    }

	    execv(prog, comp) ;

	    syslog(LOG_ERR,"execv() of %s failed - %s", 
	        prog, strerror(errno)) ;

	    goto done ;

	} else if (cpid < 0) {

	    syslog(LOG_ERR,"fork() failed - %s", strerror(errno)) ;

	    goto earlyret ;

	} /* end switch */

	(void) close(p[1]) ;

	if ((fp = fdopen(p[0], "r")) == NULL) {

	    syslog(LOG_ERR,"fdopen() failed - %s", 
	        strerror(errno)) ;

	    goto earlyret ;
	}

	while ((ch = getc(fp)) != EOF) {

	    if (ch == '\n')
	        putchar('\r') ;

	    putchar(ch) ;

	} /* end while */

/* we are done, man! */
done:
	closelog() ;

	return 0 ;

earlyret:
	syslog(LOG_DEBUG,"early return\n") ;

	goto done ;
}
/* end subroutine (main) */



