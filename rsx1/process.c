/* process */


#define	CF_DEBUGS	1


/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
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



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/ioctl.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/filio.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<shadow.h>
#include	<errno.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<limits.h>
#include	<crypt.h>

#ifdef	COMMENT
#include	<paths.h>
#endif

#include	<vsystem.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local program defines */

#define	_PATH_DEFPATH	"/usr/bin:/usr/ccs/bin:/usr/ucb:/usr/openwin/bin:/usr/dt/bin"
#define	_PATH_BSHELL	"/bin/sh"



/* external subroutines */

extern char	*timestr_log() ;


/* external variables */

extern struct global	g ;


/* forward references */

int	perform() ;

void	getstr() ;


/*VARARGS1*/
void	ourerror() ;



/* Protocol Exchange :

out to client :

 * input from client :

*	port\0
 *	username\0
 *	password\0
 *	command\0
 *	data


******************************************************************************/




int process(s,elp)
int		s ;
vecstr	*elp ;
{
	struct sockaddr_in	from ;

	time_t	daytime ;

	int	fromlen, efd ;
	int	rs ;

	char	timebuf[100] ;
	char	peername[MAXHOSTNAMELEN + 1] ;


#if	CF_DEBUGS
	if (g.debuglevel > 1) {

	    debugprintf("rexecd: entered, s=%d\n",s) ;

	    (void) time(&daytime) ;

	    debugprintf("rexecd: %s\n",
	        timestr_log(daytime,timebuf)) ;

	}
#endif /* CF_DEBUGS */

	fromlen = sizeof(struct sockaddr_in) ;
	if ((rs = u_getpeername(s,(struct sockaddr *) &from,&fromlen)) >= 0) {

#ifdef	COMMENT
		if (getchostnameaddr(&from,peername) >= 0)
	    		logfile_printf(&g.lh,
	        		"DNS peer host=%d\n",peername) ;
#endif /* COMMENT */

		rs = perform(s, &from) ;

	} /* end if (getpeername) */

	return rs ;
}
/* end subroutine (rexecd) */



/* external variables */

extern char	**environ ;


/* local global variables */

static struct	sockaddr_in asin = { 
	AF_INET } ;

static char	username[33] = "USER=" ;
static char	logname[48] = "LOGNAME=" ;
static char	homedir[MAXPATHLEN + 1] = "HOME=" ;
static char	shell[MAXPATHLEN + 1] = "SHELL=" ;
static char	path[sizeof(_PATH_DEFPATH) + sizeof("PATH=")] = "PATH=" ;
static char	*envinit[] = {
	homedir, shell, path, username, logname, NULL 
} ;




int perform(f, fromp)
int			f ;
struct sockaddr_in	*fromp ;
{
	struct passwd	*pwd ;

	struct spwd	*sp ;

	int	s ;
	int	pv[2], pid, ready, readfrom, cc ;
	int	one = 1 ;
	int	rs ;

	u_short port ;

	char cmdbuf[NCARGS + 1], *cp, *namep ;
	char user[16 + 1], pass[16 + 1] ;
	char buf[BUFSIZ + 1], sig ;


#if	CF_DEBUGS
	if (g.debuglevel > 1)
	    debugprintf("perform: entered\n") ;
#endif

	(void) signal(SIGINT, SIG_DFL) ;

	(void) signal(SIGQUIT, SIG_DFL) ;

	(void) signal(SIGTERM, SIG_DFL) ;

#ifdef	COMMENT
	dup2(f, 0) ;
#endif

	close(1) ;

	close(2) ;

	dup2(f, 1) ;

	dup2(f, 2) ;

	(void) alarm(60) ;

	port = 0 ;
	for (;;) {

	    char c ;


	    if (read(f, &c, 1) != 1)
	        exit(1) ;

	    if (c == 0)
	        break ;

	    port = port * 10 + c - '0' ;

	} /* end for */

	(void) alarm(0) ;

	if (port != 0) {

#if	CF_DEBUGS
	    debugprintf("rexecd: user supplied port=%d\n",port) ;
#endif

	    s = u_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP) ;

	    if (s < 0) {

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("rexecd: bad return from socket, rs=%d\n",s) ;
#endif

	        return s ;
	    }

	    memset((char *) &asin,0,sizeof(struct sockaddr)) ;

	    asin.sin_family = AF_INET ;

	    if ((rs = u_bind(s, &asin, sizeof(struct sockaddr))) < 0) {

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("rexecd: bad return from bind, rs=%d\n",rs) ;
#endif

	        return rs ;
	    }

	    (void) alarm(60) ;

	    fromp->sin_port = htons(port) ;

	    if ((rs = u_connect(s,fromp, sizeof(struct sockaddr))) < 0) {

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("rexecd: bad return from connect, rs=%d\n",rs) ;
#endif

	        return rs ;
	    }

	    (void) alarm(0) ;

	} /* end if (user supplied additional port) */

	getstr(f,user, sizeof(user), "username") ;

	getstr(f,pass, sizeof(pass), "password") ;

	getstr(f,cmdbuf, sizeof(cmdbuf), "command") ;

#if	CF_DEBUGS
	if (g.debuglevel > 0) {

	debugprintf("perform: u=\"%s\" p=\"%s\"\n",user,pass) ;

	debugprintf("perform: cmd> %s\n",cmdbuf) ;

	}
#endif

/* check if this is a valid login */

#ifdef	COMMENT
	setpwent() ;
#endif

	pwd = getpwnam(user) ;

	if (pwd == NULL) {

	    ourerror("Login incorrect.\n") ;

#if	CF_DEBUGS
	    if (g.debuglevel > 1)
	        debugprintf("perform: user does not exist\n") ;
#endif

	    return BAD ;
	}

#ifdef	COMMENT
	endpwent() ;
#endif

/* check if this is a valid password */

	if (pwd->pw_passwd[0] != '\0') {

	    char	*password ;


	    password = pwd->pw_passwd ;
	    if (access("/etc/shadow",R_OK) >= 0) {

	        sp = getspnam(user) ;

	        if (sp != NULL)
	            password = sp->sp_pwdp ;

	    } /* end if (we have a shadow password file) */

	    namep = crypt(pass, password) ;

	    if (strcmp(namep, password)) {

	        ourerror("Password incorrect.\n") ;

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("perform: inccorect password\n") ;
#endif

	        return BAD ;
	    }

	} /* end if */

/* change to the user's home directory */

	if (chdir(pwd->pw_dir) < 0) {

	    ourerror("No remote directory.\n") ;

#if	CF_DEBUGS
	    if (g.debuglevel > 1)
	        debugprintf("perform: could not change directories\n") ;
#endif

	    return BAD ;
	}

	(void) write(2, "\0", 1) ;

	if (port) {

	    (void) pipe(pv) ;

	    pid = uc_fork() ;

	    if (pid == -1)  {

	        ourerror("Try again.\n") ;

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("perform: could not fork\n") ;
#endif
	        return BAD ;
	    }

	    if (pid > 0) {

#ifdef	COMMENT
	        (void) close(0) ;
#endif
	        (void) close(1); 
	        (void) close(2) ;

	        (void) close(f); 
	        (void) close(pv[1]) ;

	        readfrom = (1<<s) | (1<<pv[0]) ;
	        ioctl(pv[1], FIONBIO, (char *)&one) ;

/* should set s nbio! */
	        do {

	            ready = readfrom ;
	            (void) select(16, (fd_set *)&ready,
	                (fd_set *)NULL, (fd_set *)NULL,
	                (struct timeval *)NULL) ;

	            if (ready & (1<<s)) {

	                if (read(s, &sig, 1) <= 0)
	                    readfrom &= ~(1<<s) ;
	                else
	                    killpg(pid, sig) ;

	            }

	            if (ready & (1<<pv[0])) {

	                cc = read(pv[0], buf, sizeof (buf)) ;

	                if (cc <= 0) {
	                    shutdown(s, 1+1) ;

	                    readfrom &= ~(1<<pv[0]) ;

	                } else
	                    (void) write(s, buf, cc) ;

	            }

	        } while (readfrom) ;

#if	CF_DEBUGS
	if (g.debuglevel > 1)
	        debugprintf("perform: parent exit (two socket case)\n") ;
#endif

	        logfile_close(&g.lh) ;

	        exit(0) ;

	    } /* end if (parent) */

#ifdef	COMMENT
	    setpgrp(0, getpid()) ;
#else
	    setpgrp() ;
#endif

	    (void) close(s); 
	    (void)close(pv[0]) ;

	    dup2(pv[1], 2) ;

	} /* end if (secondary port specified) */

	if (pwd->pw_shell[0] == '\0')
	    pwd->pw_shell = _PATH_BSHELL ;

	if (f > 2)
	    (void) close(f) ;

#ifdef	COMMENT
	setlogin(pwd->pw_name) ;
#endif

	(void) setgid((gid_t)pwd->pw_gid) ;

	initgroups(pwd->pw_name, pwd->pw_gid) ;

	(void) setuid((uid_t)pwd->pw_uid) ;

	(void) strcat(path, _PATH_DEFPATH) ;

	environ = envinit ;

	strncat(homedir, pwd->pw_dir, sizeof(homedir)-6) ;

	strncat(shell, pwd->pw_shell, sizeof(shell)-7) ;

	strncat(username, pwd->pw_name, sizeof(username)-6) ;

	strcat(logname, pwd->pw_name) ;

	cp = (char *) rindex(pwd->pw_shell, '/') ;

	if (cp)
	    cp += 1 ;

	else
	    cp = pwd->pw_shell ;

#if	CF_DEBUGS
	if (g.debuglevel > 1)
	    debugprintf("perform: parent exit (spawning child)\n") ;
#endif

	logfile_close(&g.lh) ;

	execl(pwd->pw_shell, cp, "-c", cmdbuf, NULL) ;

	perror(pwd->pw_shell) ;

	exit(1) ;
}
/* end subroutine (perform) */


/*VARARGS1*/
void ourerror(fmt, a1, a2, a3)
char *fmt ;
int a1, a2, a3 ;
{
	char buf[BUFSIZ] ;


	buf[0] = 1 ;
	(void) sprintf(buf+1, fmt, a1, a2, a3) ;

	(void) write(2, buf, strlen(buf)) ;

}


void getstr(s,buf, cnt, err)
int	s ;
char *buf ;
int cnt ;
char *err ;
{
	int	rs ;

	char c ;


	do {

	    if ((rs = u_read(s, &c, 1)) != 1) {

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("getstr: bad return from read, rs=%d\n",rs) ;
#endif

	        exit(1) ;
	    }

	    *buf++ = c ;
	    if (--cnt == 0) {

#if	CF_DEBUGS
	        if (g.debuglevel > 1)
	            debugprintf("getstr: something too long\n") ;
#endif

	        ourerror("%s too long\n", err) ;

	        exit(1) ;

	    }

	} while (c != 0) ;

}
/* end subroutine (getstr) */



