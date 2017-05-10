/* job */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_PCSPROG	1		/* assume a PCS program first */
#define	CF_UGETPW	1		/* use 'ugetpw(3uc)' */


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine is responsible for processing a job that
	we have received in full.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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
#include	<time.h>

#include	<vsystem.h>
#include	<ctdec.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"srvtab.h"
#include	"job.h"


/* local defines */

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_TRUNC)
#define	NCLOSEFD	3
#define	MAXOUTLEN	62

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#define	GETPW_UID	ugetpw_uid
#else
#define	GETPW_NAME	getpw_name
#define	GETPW_UID	getpw_uid
#endif /* CF_UGETPW */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* externals variables */


/* forward references */

static int	processargs(), expand() ;


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





int job_init(jhp)
JOB	*jhp ;
{
	int	rs ;


	if (jhp == NULL)
		return SR_FAULT ;

	rs = vecitem_start(jhp,10,0) ;

	return rs ;
}
/* end subroutine (job_init) */


int job_free(jhp)
JOB	*jhp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (jhp == NULL) return SR_FAULT ;

	rs1 = vecitem_finish(jhp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (job_free) */


int job_add(jhp,jep)
JOB		*jhp ;
struct jobentry	*jep ;
{
	int	rs ;


	if (jhp == NULL)
		return SR_FAULT ;

	if (jep == NULL)
		return SR_FAULT ;

	rs = vecitem_add(jhp,jep,sizeof(struct jobentry)) ;

	return rs ;
}
/* end subroutine (job_free) */


int job_del(jhp,i)
JOB	*jhp ;
int	i ;
{
	int	rs ;


	if (jhp == NULL)
		return SR_FAULT ;

	rs = vecitem_del(jhp,i) ;

	return rs ;
}
/* end subroutine (job_del) */


int job_start(jhp,jep,pip,sbp,sfp)
JOB		*jhp ;
struct jobentry	*jep ;
struct proginfo	*pip ;
struct ustat	*sbp ;
SRVTAB		*sfp ;
{
	struct passwd	pe ;

	struct group	ge, *gp = &ge ;

	struct argparams	subs ;

	SRVTAB_ENT	*sep ;

	vecstr	args ;

	pid_t	pid ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	ifd, fd0, fd1, fd2 ;
	int	sl ;
	int	plen = -1 ;
	int	f ;

	char	buf[BUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	progfname[MAXPATHLEN + 1] ;
	char	ifname[MAXPATHLEN + 1] ;
	char	ofname[MAXPATHLEN + 1] ;
	char	efname[MAXPATHLEN + 1] ;
	char	progbuf[MAXPATHLEN + 1] ;
	char	rootname[MAXNAMELEN + 1] ;
	char	length[LOGNAMELEN + 1], mtime[LOGNAMELEN + 1] ;
	char	username[LOGNAMELEN + 1], groupname[LOGNAMELEN + 1] ;
	char	*program = NULL, *arg0 = NULL ;
	char	*sp, *cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: entered\n") ;
#endif

	rs = vecstr_start(&args,10,0) ;

	if (rs < 0)
	    return rs ;

/* create the substitutions for the service daemon argument vector */

	memset(&subs,0,sizeof(struct argparams)) ;

	subs.version = VERSION ;
	subs.directory = pip->directory ;
	subs.filename = jep->filename ;

	subs.rootname = rootname ;
	if ((cp = strrchr(jep->filename,'.')) != NULL) {
	    strwcpy(rootname,jep->filename,(cp - jep->filename)) ;
	} else
	    strcpy(rootname,jep->filename) ;

	subs.length = length ;
	ctdeci(length,LOGNAMELEN,sbp->st_size) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: size=%d length=%s\n",sbp->st_size,length) ;
#endif

	subs.mtime = mtime ;
	ctdeci(mtime,LOGNAMELEN,sbp->st_mtime) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: mtime=%s\n",mtime) ;
#endif

/* get the user and group names of the file */

	if ((rs1 = GETPW_UID(&pe,buf,BUFLEN,sbp->st_uid) >= 0) {

	    sncpy1(username,LOGNAMELEN,pe.pw_name) ;

	} else {

#ifdef	COMMENT
	    bufprintf(username,LOGNAMELEN,"U%u",sbp->st_uid) ;
#else

		username[0] = 'U' ;
		ctdeci((username + 1),(LOGNAMELEN - 1),sbp->st_uid) ;

#endif /* COMMENT */

	}

	subs.username = username ;


	if (getgr_gid(&ge,buf,BUFLEN,sbp->st_gid) >= 0) {

	    sncpy1(groupname,LOGNAMELEN,ge.gr_name) ;

	} else {

#ifdef	COMMENT
	    bufprintf(groupname,LOGNAMELEN,"G%d",(int) sbp->st_gid) ;
#else

		groupname[0] = 'G' ;
		ctdeci((groupname + 1),(LOGNAMELEN - 1),sbp->st_gid) ;

#endif /* COMMENT */

	}

	subs.groupname = groupname ;


/* search for a match in the service table */

	if (pip->f.srvtab && 
		((i = srvtab_match(sfp,jep->filename,&sep)) >= 0)) {

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        debugprintf("job_start: service table args> %s\n",sep->args) ;
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
	            debugprintf("job_start: expanded\n") ;
#endif

	    } /* end if */

	    if ((sep->args != NULL) && (sep->args[0] != '\0')) {

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            debugprintf("job_start: we have service args\n") ;
#endif

	        rs1 = processargs(pip,&args,sep->args,&subs) ;

		if (rs1 < 0)
	            goto badret ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            debugprintf("job_start: we processed them\n") ;
#endif

	        if ((rs = vecstr_count(&args)) > 0) {

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	                debugprintf("job_start: we have non-zero count %d, a0=%s\n",
	                    rs,args.va[0]) ;

	                for (i = 0 ; vecstr_get(&args,i,&cp) >= 0 ; i += 1) {

	                    if (cp == NULL) continue ;

	                    debugprintf("job_start: arg%d> %s\n",i,cp) ;

	                }
	            }
#endif /* CF_DEBUG */

	            arg0 = strbasename(args.va[0]) ;

	            if (program == NULL)
	                program = args.va[0] ;

	        } /* end if (we had some arguments) */

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	            debugprintf("job_start: done w/ counting args, arg0=%s\n",
	                arg0) ;
#endif

	    } /* end if (processing any ARGuments) */

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        debugprintf("job_start: is ARG0 NULL ?, arg0=%s program=%s\n",
	            arg0,program) ;
#endif

	    if ((arg0 == NULL) && (program != NULL)) {

	        arg0 = strbasename(program) ;

	        vecstr_add(&args,arg0,-1) ;

	    }

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        debugprintf("job_start: is both ARG0 and PROGRAM NULL ?\n") ;
#endif

	    if ((program == NULL) && (arg0 == NULL))
	        goto badret ;

	} else if (pip->command != NULL) {

/* command match */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("job_start: we have a COMMAND only\n") ;
#endif

	    i = processargs(pip,&args,pip->command,&subs) ;

	    if (i < 0)
	        goto badret ;

	    rs1 = vecstr_get(&args,0,&program) ;

	    if (rs1 >= 0)
	    arg0 = strbasename(program) ;

	} else {

/* there was no match at all ! */

	    logfile_printf(&pip->lh,
		"could not find a service for this job\n") ;

	    jep->state = STATE_WAIT ;
	    goto badret ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_start: out of IF\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_start: program=%s arg0=%s\n",program,arg0) ;
#endif

/* can we execute this service daemon ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_start: starting X check\n") ;
#endif

	if (plen < 0)
	    sl = strlen(program) ;

	else
	    sl = plen ;

/* apply some appropriate trailing stripping */

	sl = sfshrink(program,-1,&sp) ;

	while ((sl > 0) && (sp[sl - 1] == '/')) 
		sl -= 1 ;

	sp[sl] = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("job_start: program=%s\n",sp) ;
#endif

	f = (strchr(sp,'/') != NULL) ;

	if ((f && (u_access(sp,X_OK) < 0)) ||
	    ((! f) && (getfiledirs(NULL,sp,"x",NULL) <= 0))) {

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	        debugprintf("job_start: could not execute\n") ;
#endif

	    logfile_printf(&pip->lh,
		"cannot find service daemon\n") ;

	    jep->state = STATE_WAIT ;
	    goto badret ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: past X check\n") ;
#endif

/* program file */

#if	CF_PCSPROG
	rs = prgetprogpath(pip->pr,progfname,sp,-1) ;
#else
	rs = findfilepath(NULL,progfname,sp,X_OK) ;
#endif

	if (rs == 0) {

	    if (program[0] != '/') {
	        mkpath2(progfname, pip->pwd,sp) ;
	    } else
		mkpath1(progfname,sp) ;

	} /* end if */

	if (rs < 0) {

	    logfile_printf(&pip->lh,
		"cannot find service daemon\n") ;

	    jep->state = STATE_WAIT ;
	    goto badret ;
	}

/* input file */

	mkpath2(ifname, pip->directory,jep->filename) ;

	rs = u_open(ifname,O_RDONLY,0600) ;

	ifd = rs ;
	if (rs < 0) {

	    logfile_printf(&pip->lh,
	        "could not open the input file (%d)\n",ifd) ;

	    goto badret ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: ifname=%s\n",ifname) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	    if ((fd0 = open(ifname,O_RDONLY,0600)) >= 0) {

	        debugprintf("job_start: could open input file\n") ;

	        close(fd0) ;

	    } else
	        debugprintf("job_start: could NOT open input file\n") ;

	}
#endif /* CF_DEBUG */

/* output file */

	mkpath2(tmpfname, pip->tmpdname, "dwd1XXXXXXXXXX") ;

	if ((rs = mktmpfile(ofname,0600,tmpfname)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open an output file (%d)\n",rs) ;

	    goto badret1 ;
	}

	cp = strbasename(ofname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: ofname=%s\n",cp) ;
#endif

	strcpy(jep->ofname,cp) ;

/* error file */

	mkpath2(tmpfname, pip->tmpdname, "dwd2XXXXXXXXXX") ;

	if ((rs = mktmpfile(efname,0600,tmpfname)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open an error file (%d)\n",rs) ;

	    goto badret2 ;
	}

	cp = strbasename(efname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job_start: efname=%s\n",cp) ;
#endif

	strcpy(jep->efname,cp) ;

/* fork it ! */

	if (pip->f.log)
	    logfile_flush(&pip->lh) ;

	for (i = 0 ; (i < NFORKS) && ((rs = uc_fork()) < 0) ; i += 1) {

	    if (i == 0)
	        logfile_printf(&pip->lh,
	            "trouble forking (%d)\n",rs) ;

	    sleep(2) ;

	} /* end for */

	if (rs < 0) {

	    logfile_printf(&pip->lh,
	        "cannot fork for service daemon (%d)\n",rs) ;

	    goto badret3 ;
	}

	pid = rs ;
	if (pid == 0) {

	    if ((pip->workdname != NULL) && (pip->workdname[0] != '\0') &&
	        (strcmp(pip->workdname,".") != 0))
	        u_chdir(pip->workdname) ;

		if (ifd < 3) {

			rs = uc_moveup(ifd,3) ;

			if (rs >= 0)
				ifd = rs ;

		}

	    for (i = 0 ; i < NCLOSEFD ; i += 1) 
		u_close(i) ;

	    u_dup(ifd) ;

	    u_close(ifd) ;

	    fd1 = u_open(ofname,O_FLAGS,0600) ;

	    fd2 = u_open(efname,O_FLAGS,0600) ;

	    {
		const char	**eav = (const char **) args.va ;
		const char	**eev = (const char **) pip->exports.va ;
	        rs = u_execve(progfname,eav,eev) ;
	    }

/* deleting the error output file will be the signal that we couldn't 'exec' */

	    u_unlink(efname) ;

	    uc_exit(EX_NOEXEC) ;

	} /* end if (child) */

	jep->pid = pid ;

	logfile_printf(&pip->lh,"server=%s pid=%d\n",
	    arg0,pid) ;

ret2:
	vecstr_finish(&args) ;

ret1:
	close(ifd) ;

ret0:
	return (rs >= 0) ? ((int) pid) : rs ;

/* handle bad returns */
badret3:
	u_unlink(efname) ;

badret2:
	u_unlink(ofname) ;

badret1:
badret:
	rs = SR_INVALID ;
	goto ret2 ;
}
/* end subroutine (job_start) */


/* end a job */
int job_end(jhp,jep,pip,child_stat)
JOB		*jhp ;
struct jobentry	*jep ;
struct proginfo	*pip ;
int		child_stat ;
{
	struct ustat	sb ;

	bfile		file, *fp = &file ;

	int	rs = SR_OK ;
	int	len ;
	int	lines ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_end: entered !, file=%s\n",jep->filename) ;
#endif

#ifdef	COMMENT
	logfile_printf(&pip->lh,"process exit status %d\n",child_stat) ;
#endif

/* standard output file */

	mkpath2(tmpfname, pip->tmpdname,jep->ofname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_end: ofname=%s\n",tmpfname) ;
#endif

	if ((u_stat(tmpfname,&sb) >= 0) && (sb.st_size > 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("job_end: got some output, size=%d\n",
	            sb.st_size) ;
#endif

	    logfile_printf(&pip->lh,"job standard output, size=%d\n",
		sb.st_size) ;

	    lines = 0 ;
	    if (bopen(fp,tmpfname,"r",0666) >= 0) {

	        while ((rs = breadline(fp,linebuf,MAXOUTLEN)) > 0) {

		    len = rs ;
	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %t\n",linebuf,len) ;

	            lines += 1 ;

	        }

	        bclose(fp) ;

	    } /* end if (opened file) */

	    logfile_printf(&pip->lh,"lines=%d\n",lines) ;

	} /* end if (non-zero file size) */

	u_unlink(tmpfname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_end: after 1st unlink\n") ;
#endif

/* standard error file */

	mkpath2(tmpfname, pip->tmpdname,jep->efname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("job_end: efname=%s\n",tmpfname) ;
#endif

	if ((u_stat(tmpfname,&sb) >= 0) && (sb.st_size > 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("job_end: goto some error output, size=%d\n",
	            sb.st_size) ;
#endif

	    logfile_printf(&pip->lh,"job standard error, size=%d\n",
			sb.st_size) ;

	    lines = 0 ;
	    if (bopen(fp,tmpfname,"r",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,MAXOUTLEN)) > 0) {

	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&pip->lh,"| %t\n",linebuf,len) ;

	            lines += 1 ;

	        }

	        bclose(fp) ;

	    } /* end if (opened file) */

	    logfile_printf(&pip->lh,"lines=%d\n",lines) ;

	} /* end if (non-zero file size) */

	if (tmpfname[0] != '\0')
	    u_unlink(tmpfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("job_end: ret\n") ;
#endif

	return 0 ;
}
/* end subroutine (job_end) */


/* search the job table for a PID match */
int job_findpid(jlp,pip,pid,jepp)
JOB		*jlp ;
struct proginfo	*pip ;
int		pid ;
struct jobentry	**jepp ;
{
	int	i ;


	for (i = 0 ; vecitem_get(jlp,i,jepp) >= 0 ; i += 1) {

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
JOB		*jsp ;
struct proginfo	*pip ;
char		filename[] ;
struct jobentry	**jepp ;
{
	int	i ;


	for (i = 0 ; vecitem_get(jsp,i,jepp) >= 0 ; i += 1) {

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
JOB		*jsp ;
struct proginfo	*pip ;
int		i ;
struct jobentry	**jepp ;
{
	int	rs ;


	rs = vecitem_get(jsp,i,jepp) ;

	return rs ;
}
/* end subroutine (job_get) */


/* local subroutines */


/* process an argument list */
static int processargs(pip,alp,command,sp)
struct proginfo	*pip ;
vecstr		*alp ;
const char		command[] ;
struct argparams	*sp ;
{
	FIELD	fsb ;

	int	rs = SR_OK ;
	int	i = 0 ;
	int	elen ;

	uchar	terms[32] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("processargs: entered> %s\n",command) ;
#endif

	if ((command == NULL) || (command[0] == '\0')) goto done ;

	fieldterms(terms,0," \t") ;

	if ((rs = field_start(&fsb,command,-1)) >= 0) {
	    const int	flen = BUFLEN ;
	    const int	elen = BUFLEN ;
	    int		fl ;
	    int		el ;
	    char	fbuf[BUFLEN+1] ;
	    char	ebuf[BUFLEN + 1] ;

	    while ((fl = field_sharg(&fsb,terms,fbuf,flen)) > 0) {

	        if ((el = expand(pip,fbuf,fl,sp,ebuf,elen)) >= 0) {
	            i += 1 ;
	            rs = vecstr_add(alp,ebuf,el) ;
	        } else
		    rs = SR_TOOBIG ;

	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

done:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("processargs: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (processargs) */


/* expand out a program argument with the substitution parameters */

/*
#	The following substitutions are made on command strings :

#		%V	Directory Watcher version string
#		%S	search name
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
	int	rs = SR_OK ;
	int	elen = 0, sl ;

	char	hostbuf[MAXHOSTNAMELEN + 1] ;
	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job/expand: entered\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	    if (buf == NULL)
	        debugprintf("job/expand: buf is NULL\n") ;

	    if (rbuf == NULL)
	        debugprintf("job/expand: rbuf is NULL\n") ;

	}
#endif /* CF_DEBUG */

	rbuf[0] = '\0' ;
	if (len == 0)
	    return 0 ;

	if (len < 0)
	    len = strlen(buf) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job/expand: top of while\n") ;
#endif

	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 2)
	        debugprintf("job/expand: switching on >%c<\n",*bp) ;
#endif

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) 
			break ;

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
	            cp = pip->version ;
	            sl = strlen(cp) ;

	            break ;

	        case 'S':
	            cp = pip->searchname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'R':
	            cp = pip->pr ;
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
	            sl = -1 ;
	            cp = pip->nodename ;
	            if ((pip->domainname != NULL) && 
				(pip->domainname[0] != '\0')) {

	                cp = hostbuf ;
	                sl = snsds(hostbuf,BUFLEN,
	                    pip->nodename,pip->domainname) ;

	            }

	            if (sl < 0)
	                sl = strlen(cp) ;

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
	        debugprintf("job/expand: bottom while\n") ;
#endif

	} /* end while */

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("job/expand: ret\n") ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */



