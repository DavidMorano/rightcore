/* job */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUG	1		/* run-time debugging */


/* revision history :

	= 1991-09-10, David Morano

	This program was originally written.


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for processing a job that
	we have received in full.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<srvtab.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_TRUNC)
#define	CLOSEDFDS	4
#define	MAXOUTLEN	62


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	ctdeci(char *,int,int) ;

extern char	*strbasename(char *) ;


/* externals variables */


/* forward references */

static int	processargs(), expand() ;


/* local global variables */


/* local structures */

struct argparams {
	char	*filename ;
	char	*rootname ;
	char	*username ;
	char	*groupname ;
	char	*mtime ;
	char	*directory ;
	char	*length ;
	char	*version ;
} ;




int job_start(jhp,jep,pip,sbp,sfp)
vecelem		*jhp ;
struct jobentry	*jep ;
struct proginfo	*pip ;
struct ustat	*sbp ;
SRVTAB		*sfp ;
{
	struct passwd	pe, *pp = &pe ;

	struct group	ge, *gp = &ge ;

	vecstr	args ;

	struct argparams	subs ;

	SRVTAB_ENTRY	*sep ;

	pid_t	pid ;

	int	rs, i, l1, l2 ;
	int	ifd, fd0, fd1, fd2 ;
	int	sl ;
	int	plen = -1 ;
	int	f ;

	char	buf[BUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	ifname[MAXPATHLEN + 1] ;
	char	ofname[MAXPATHLEN + 1] ;
	char	efname[MAXPATHLEN + 1] ;
	char	*cp ;
	char	*program, *arg0 ;
	char	progbuf[MAXPATHLEN + 1] ;
	char	rootname[MAXNAMELEN + 1] ;
	char	length[33], mtime[33] ;
	char	username[33], groupname[33] ;


#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: entered\n") ;
#endif

	if ((rs = vecstr_start(&args,10,0)) < 0)
	    return rs ;

/* create the substitutions for the service daemon argument vector */

	subs.version = VERSION ;
	subs.directory = pip->directory ;
	subs.filename = jep->filename ;

	subs.rootname = rootname ;
	if ((cp = strrchr(jep->filename,'.')) != NULL)
	    strwcpy(rootname,jep->filename,cp - jep->filename) ;

	else
	    strcpy(rootname,jep->filename) ;

	subs.length = length ;
	ctdeci(length,33,sbp->st_size) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: size=%d length=%s\n",sbp->st_size,length) ;
#endif

	subs.mtime = mtime ;
	ctdeci(mtime,33,sbp->st_mtime) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: mtime=%s\n",mtime) ;
#endif

/* get the user and group names of the file */

	if (passwd_getuid(sbp->st_uid,&pe,buf,BUFLEN) >= 0)
	    strcpy(username,pp->pw_name) ;

	else
	    sprintf(username,"GUEST-%d",sbp->st_uid) ;

	subs.username = username ;


	if (group_getgid(sbp->st_gid,&ge,buf,BUFLEN) >= 0)
	    strcpy(groupname,gp->gr_name) ;

	else
	    sprintf(groupname,"GUEST-%d",(int) sbp->st_gid) ;

	subs.groupname = groupname ;


/* search for a match in the service table */

	if (pip->f.srvtab && 
		((i = srvtab_match(sfp,jep->filename,&sep)) >= 0)) {

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        eprintf("job_start: service table args> %s\n",sep->args) ;
#endif

	    arg0 = NULL ;
	    program = sep->program ;
	    if (sep->program != NULL) {

	        program = progbuf ;
	        plen = expand(pip,sep->program,-1,&subs,progbuf,MAXPATHLEN) ;

	        if (plen < 0)
	            goto badret ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            eprintf("job_start: expanded\n") ;
#endif

	    } /* end if */

	    if ((sep->args != NULL) && (sep->args[0] != '\0')) {

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            eprintf("job_start: we have service args\n") ;
#endif

	        if (processargs(pip,&args,sep->args,&subs) < 0)
	            goto badret ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            eprintf("job_start: we processed them\n") ;
#endif

	        if ((rs = vecstr_count(&args)) > 0) {

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	                eprintf("job_start: we have non-zero count %d, a0=%s\n",
	                    rs,args.va[0]) ;

	                for (i = 0 ; vecstr_get(&args,i,&cp) >= 0 ; i += 1) {

	                    if (cp == NULL) continue ;

	                    eprintf("job_start: arg%d> %s\n",i,cp) ;

	                }
	            }
#endif /* CF_DEBUG */

	            arg0 = strbasename(args.va[0]) ;

	            if (program == NULL)
	                program = args.va[0] ;

	        } /* end if (we had some arguments) */

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            eprintf("job_start: done w/ counting args, arg0=%s\n",
	                arg0) ;
#endif

	    } /* end if (processing any ARGuments) */

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        eprintf("job_start: is ARG0 NULL ?, arg0=%s program=%s\n",
	            arg0,program) ;
#endif

	    if ((arg0 == NULL) && (program != NULL)) {

	        arg0 = strbasename(program) ;

	        vecstr_add(&args,arg0,-1) ;

	    }

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        eprintf("job_start: is both ARG0 and PROGRAM NULL ?\n") ;
#endif

	    if ((program == NULL) && (arg0 == NULL))
	        goto badret ;

	} else if (pip->command != NULL) {

/* command match */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("job_start: we have a COMMAND only\n") ;
#endif

	    if ((i = processargs(pip,&args,pip->command,&subs)) < 0)
	        goto badret ;

	    vecstr_get(&args,0,&program) ;

	    arg0 = strbasename(program) ;

	} else {

/* there was no match at all ! */

	    logfile_printf(&pip->lh,"could not find a service for this job\n") ;

	    jep->state = STATE_WAIT ;
	    goto badret ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_start: out of IF\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_start: program=%s arg0=%s\n",program,arg0) ;
#endif


/* can we execute this service daemon ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_start: starting X check\n") ;
#endif

	if (plen < 0)
	    sl = strlen(program) ;

	else
	    sl = plen ;

	while ((sl > 0) && (program[sl - 1] == '/')) 
		sl -= 1 ;

	program[sl] = '\0' ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_start: program=%s\n",program) ;
#endif

	f = (strchr(program,'/') != NULL) ;

	if ((f && (u_access(program,X_OK) < 0)) ||
	    ((! f) && (getfiledirs(NULL,program,"x",NULL) <= 0))) {

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        eprintf("job_start: could not execute\n") ;
#endif

	    logfile_printf(&pip->lh,"cannot find or execute service daemon\n") ;

	    jep->state = STATE_WAIT ;
	    goto badret ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: past X check\n") ;
#endif


/* input file */

	mkpath2(ifname,pip->directory,jep->filename) ;

	if ((ifd = u_open(ifname,O_RDONLY,0600)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open the input file, rs=%d\n",ifd) ;

	    goto badret ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: ifname=%s\n",ifname) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	    if ((fd0 = open(ifname,O_RDONLY,0600)) >= 0) {

	        eprintf("job_start: could open input file\n") ;

	        close(fd0) ;

	    } else
	        eprintf("job_start: could NOT open input file\n") ;

	}
#endif /* CF_DEBUG */


/* output file */

	mkpath2(tmpfname,pip->tmpdir,"dwd1XXXXXXXXXX") ;

	if ((rs = mktmpfile(ofname,0600,tmpfname)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open an output file, rs=%d\n",rs) ;

	    goto badret1 ;
	}

	cp = strbasename(ofname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: ofname=%s\n",cp) ;
#endif

	strcpy(jep->ofname,cp) ;


/* error file */

	mkpath2(tmpfname, pip->tmpdir, "dwd2XXXXXXXXXX") ;

	if ((rs = mktmpfile(efname,0600,tmpfname)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open an error file, rs=%d\n",rs) ;

	    goto badret2 ;
	}

	cp = strbasename(efname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job_start: efname=%s\n",cp) ;
#endif

	strcpy(jep->efname,cp) ;


/* fork it ! */

	for (i = 0 ; (i < NFORKS) && ((pid = uc_fork()) < 0) ; i += 1) {

	    rs = pid ;
	    if (i == 0)
	        logfile_printf(&pip->lh,
	            "trouble forking, rs=%d\n",rs) ;

	    sleep(2) ;

	} /* end for */

	if (pid < 0) {

	    rs = pid ;
	    logfile_printf(&pip->lh,
	        "cannot fork for service daemon, rs=%d\n",rs) ;

	    goto badret3 ;
	}

	if (pid == 0) {

	    if ((pip->workdir != NULL) && (pip->workdir[0] != '\0') &&
	        (strcmp(pip->workdir,".") != 0))
	        chdir(pip->workdir) ;

	    for (i = 0 ; i < CLOSEDFDS ; i += 1) close(i) ;

	    dup(ifd) ;

	    close(ifd) ;

	    fd1 = open(ofname,O_FLAGS,0600) ;

	    fd2 = open(efname,O_FLAGS,0600) ;

	    rs = execvp(program,args.va) ;

/* deleting the error output file will be the signal that we couldn't 'exec' */

	    unlink(efname) ;

	    exit(1) ;

	} /* end if (child) */

	jep->pid = pid ;

	logfile_printf(&pip->lh,"server=%s pid=%d\n",
	    arg0,pid) ;

	vecstr_finish(&args) ;

	close(ifd) ;

	return pid ;

/* handle bad returns */
badret3:
	unlink(efname) ;

badret2:
	unlink(ofname) ;

badret1:
badret:
	close(ifd) ;

	vecstr_finish(&args) ;

	return BAD ;
}
/* end subroutine (job_start) */


/* end a job */
int job_end(jhp,jep,pip,child_stat)
vecelem		*jhp ;
struct jobentry	*jep ;
struct proginfo	*pip ;
int		child_stat ;
{
	bfile		file, *fp = &file ;

	struct ustat	sb ;

	int		len ;
	int		lines ;

	char		linebuf[LINELEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_end: entered !, file=%s\n",jep->filename) ;
#endif

#ifdef	COMMENT
	logfile_printf(&pip->lh,"process exit status %d\n",child_stat) ;
#endif

/* standard output file */

	sprintf(tmpfname,"%s/%s",pip->tmpdir,jep->ofname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_end: ofname=%s\n",tmpfname) ;
#endif

	if ((stat(tmpfname,&sb) >= 0) && (sb.st_size > 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("job_end: got some output, size=%d\n",
	            sb.st_size) ;
#endif

	    logfile_printf(&pip->lh,"job standard output, size=%d\n",
		sb.st_size) ;

	    lines = 0 ;
	    if (bopen(fp,tmpfname,"r",0666) >= 0) {

	        while ((len = bgetline(fp,linebuf,MAXOUTLEN)) > 0) {

	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %W\n",linebuf,len) ;

	            lines += 1 ;

	        }

	        bclose(fp) ;

	    } /* end if (opened file) */

	    logfile_printf(&pip->lh,"lines=%d\n",lines) ;

	} /* end if (non-zero file size) */

	unlink(tmpfname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_end: after 1st unlink\n") ;
#endif

/* standard error file */

	mkpath2(tmpfname,pip->tmpdir,jep->efname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_end: efname=%s\n",tmpfname) ;
#endif

	if ((u_stat(tmpfname,&sb) >= 0) && (sb.st_size > 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("job_end: goto some error output, size=%d\n",
	            sb.st_size) ;
#endif

	    logfile_printf(&pip->lh,"job standard error, size=%d\n",
			sb.st_size) ;

	    lines = 0 ;
	    if (bopen(fp,tmpfname,"r",0666) >= 0) {

	        while ((len = bgetline(fp,linebuf,MAXOUTLEN)) > 0) {

	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %W\n",linebuf,len) ;

	            lines += 1 ;

	        }

	        bclose(fp) ;

	    } /* end if (opened file) */

	    logfile_printf(&pip->lh,"lines=%d\n",lines) ;

	} /* end if (non-zero file size) */

	unlink(tmpfname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("job_end: exiting\n") ;
#endif

	return 0 ;
}
/* end subroutine (job_end) */


/* search the job table for a PID match */
int job_findpid(jlp,pip,pid,jepp)
vecelem		*jlp ;
struct proginfo	*pip ;
pid_t		pid ;
struct jobentry	**jepp ;
{
	int	i ;


	for (i = 0 ; vecelem_get(jlp,i,jepp) >= 0 ; i += 1) {

	    if ((*jepp) == NULL) continue ;

	    if (((*jepp)->state == STATE_STARTED) && ((*jepp)->pid == pid))
	        return i ;

	} /* end for */

	(*jepp) = NULL ;
	return -1 ;
}
/* end subroutine (job_findpid) */


/* search for a job in the job table via its filename */
int job_search(jsp,pip,filename,jepp)
vecelem		*jsp ;
struct proginfo	*pip ;
char		filename[] ;
struct jobentry	**jepp ;
{
	int	i ;


	for (i = 0 ; vecelem_get(jsp,i,jepp) >= 0 ; i += 1) {

	    if ((*jepp) == NULL) continue ;

	    if (strcmp((*jepp)->filename,filename) == 0) 
		return i ;

	}

	(*jepp) = NULL ;
	return -1 ;
}
/* end subroutine (job_search) */


/* enumerate all of the jobs */
int job_get(jsp,pip,i,jepp)
vecelem		*jsp ;
struct proginfo	*pip ;
int		i ;
struct jobentry	**jepp ;
{


	return vecelem_get(jsp,i,jepp) ;
}
/* end subroutine (job_search) */



/* INTERNAL SUBROUTINES */



/* process an argument list */
static int processargs(pip,alp,command,sp)
struct proginfo	*pip ;
vecstr		*alp ;
char			command[] ;
struct argparams	*sp ;
{
	FIELD	fsb ;

	int	flen, elen ;
	int	i ;

	uchar	terms[32] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("processargs: entered> %s\n",command) ;
#endif

	if ((command == NULL) || (command[0] == '\0'))
	    return 0 ;

	fieldterms(terms,0," \t") ;

	field_init(&fsb,command,-1) ;

/* loop through the arguments */

	i = 0 ;
	while ((flen = field_sharg(&fsb,terms,buf,BUFLEN)) > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("processargs: TOL f=%W fl=%d\n",
	            fsb.fp,fsb.flen,fsb.flen) ;
#endif

	    if ((elen = expand(pip,buf,flen,sp,buf2,BUFLEN)) < 0)
	        return -1 ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("processargs: buf2> %s\n",buf2) ;
#endif

	    if (vecstr_add(alp,buf2,elen) < 0)
	        return -1 ;

	    i += 1 ;

	} /* end while */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("processargs: exiting, %d args\n",i) ;
#endif

	return i ;
}
/* end subroutine (processargs) */


/* expand out a program argument with the substitution parameters */

/*
#	The following substitutions are made on command strings :

#		%V	Directory Watcher version string
#		%R	program root
#		%N	machine nodename
#		%D	machine DNS domain
#		%H	machine hostname

#		%f	file name argument
#		%r	file name root (part before the last dot)
#		%u	file username
#		%g	file groupname
#		%l	file length in bytes
#		%m	UNIX modification time (integer)
#		%d	watched directory path
#
*/

static int expand(pip,buf,len,sep,rbuf,rlen)
struct proginfo		*pip ;
char			rbuf[], buf[] ;
int			rlen, len ;
struct argparams	*sep ;
{
	int	elen = 0, sl ;

	char	hostbuf[MAXHOSTNAMELEN + 1] ;
	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job/expand: entered\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	    if (buf == NULL)
	        eprintf("job/expand: buf is NULL\n") ;

	    if (rbuf == NULL)
	        eprintf("job/expand: rbuf is NULL\n") ;

	}
#endif
	rbuf[0] = '\0' ;
	if (len == 0)
	    return 0 ;

	if (len < 0)
	    len = strlen(buf) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job/expand: top of while\n") ;
#endif

	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        eprintf("job/expand: switching on >%c<\n",*bp) ;
#endif

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) return elen ;

	        switch ((int) *bp) {

	        case 'f':
	            cp = sep->filename ;
	            sl = strlen(cp) ;

	            break ;

	        case 'r':
	            cp = sep->rootname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'u':
	            cp = sep->username ;
	            sl = strlen(cp) ;

	            break ;

	        case 'g':
	            cp = sep->groupname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'l':
	            cp = sep->length ;
	            sl = strlen(cp) ;

	            break ;

	        case 'm':
	            cp = sep->mtime ;
	            sl = strlen(cp) ;

	            break ;

	        case 'd':
	            cp = sep->directory ;
	            sl = strlen(cp) ;

	            break ;

	        case 'V':
	            cp = sep->version ;
	            sl = strlen(cp) ;

	            break ;

	        case 'R':
	            cp = pip->programroot ;
	            sl = strlen(cp) ;

	            break ;

	        case 'N':
	            cp = pip->nodename ;
	            sl = strlen(cp) ;

	            break ;

	        case 'D':
	            cp = pip->domainname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'H':
		    {
			const char	*nn = pip->nodename ;
			const char	*dn = pip->domainname ;
	                sl = -1 ;
	                cp = pip->nodename ;
	                if ((dn != NULL) && (dn[0] != '\0')) {
	                    cp = hostbuf ;
	                    sl = snsds(hostbuf,BUFLEN,nn,dn) ;
	                }
	                if (sl < 0) sl = strlen(cp) ;
		    }
	            break ;

	        default:
	            cp = bp ;
	            sl = 1 ;

	        } /* end switch */

	        bp += 1 ;
	        len -= 1 ;

	        if ((elen + sl) > rlen)
	            return BAD ;

	        strncpy(rbp,cp,sl) ;

	        rbp += sl ;
	        elen += sl ;
	        break ;

	    default:
	        *rbp++ = *bp++ ;
	        elen += 1 ;
	        len -= 1 ;

	    } /* end switch */

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        eprintf("job/expand: bottom while\n") ;
#endif

	} /* end while */

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    eprintf("job/expand: normal exit\n") ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */



