/* handlejob */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUG	1
#define	CF_ACCCESSCHECK	0


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This subroutine was originally written.


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
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<limits.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"jobdb.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"srvfile.h"


/* defines */

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_TRUNC)
#define	CLOSEDFDS	4


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*strbasename() ;


/* externals variables */

extern struct global	g ;


/* forwards */

static int	processargs(), expand() ;


/* local global variables */


/* local structures */

struct argparams {
	char	*jobid ;
	char	*service ;
	char	*username ;
	char	*groupname ;
	char	*date ;
	char	*length ;
	char	*version ;
} ;


/* exported subroutines */


int handlejob_start(jep,jfp,sbp,sep)
struct jobentry	*jep ;
bfile		*jfp ;
struct ustat	*sbp ;
struct srventry	*sep ;
{
	struct group	ge, *gp ;

	struct ustat	sb ;

	vecstr	args ;

	struct argparams	subs ;

	pid_t	pid ;

	int	rs, i ;
	int	ifd, fd0, fd1, fd2 ;
	int	len, l ;
	int	clen2 ;
	int	f_infile ;

	char	buf[BUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	jobfname[MAXPATHLEN + 1] ;
	char	ifname[MAXPATHLEN + 1] ;
	char	ofname[MAXPATHLEN + 1] ;
	char	efname[MAXPATHLEN + 1] ;
	char	*cp ;
	char	*program, *arg0 ;
	char	subs_length[33], subs_jobdate[33] ;
	char	subs_username[33], subs_groupname[33] ;


#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_start: entered\n") ;
#endif

/* perform some simple checks for starters */

	if ((sep->args == NULL) && (sep->program == NULL))
	    return SR_BAD ;


/* input file */

	clen2 = 0 ;
	f_infile = FALSE ;
	ifname[0] = '\0' ;
	if (jep->clen != 0) {

	    if (jfp == NULL) {

	        bufprintf(jobfname,MAXPATHLEN,"%s/%s",
	            g.directory,jep->filename) ;

	        if ((rs = bopen(jfp,jobfname,"r",0666)) < 0)
	            goto badret ;

	        f_infile = TRUE ;
	        bseek(jfp,jep->offset,SEEK_SET) ;

	    }

	    mkpath2(tmpfname, g.tmpdir, "dwd0XXXXXXXXXX") ;

	    rs = mktmpfile(ifname,0600,tmpfname)) < 0)
	    if (rs < 0)
	        goto badret2 ;

	    ifd = u_open(ifname,O_RDWR | O_CREAT | O_TRUNC,0666) ;

	    unlink(ifname) ;

	    if (ifd < 0) {

	        rs = ifd ;
	        goto badret3 ;
	    }

	    if (jep->clen > 0) {

	        len = jep->clen ;
	        while ((len > 0) && 
	            ((l = bread(jfp,buf,MIN(BUFLEN,len))) > 0)) {

	            writen(ifd,buf,l) ;

	            len -= l ;
	            clen2 += l ;
	        }

	    } else {

	        while ((l = bread(jfp,buf,BUFLEN)) > 0) {

	            writen(ifd,buf,l) ;

	            clen2 += l ;
	        }

	    }

	    lseek(ifd,0L,SEEK_SET) ;

	} else
	    ifd = open("/dev/null",O_RDONLY,0600) ;

	if (jep->clen < 0)
	    jep->clen = clen2 ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_start: ifname=%s\n",ifname) ;
#endif

#if	CF_DEBUG && 0
	if (g.debuglevel > 0) {

	    if (jep->clen != 0) {

	        if ((fd0 = open(ifname,O_RDONLY,0600)) >= 0) {

	            debugprintf("handlejob_start: could open input file\n") ;

	            close(fd0) ;

	        } else
	            debugprintf("handlejob_start: could NOT open input file\n") ;

	    } else
	        debugprintf("handlejob_start: there was no input for this job\n") ;

	}
#endif /* CF_DEBUG */


/* output file */

	if (jep->ofname[0] == '\0') {

	    mkpath2(tmpfname, g.tmpdir, "dwd1XXXXXXXXXX") ;

	    if ((rs = mktmpfile(ufname,0600,tmpfname)) < 0) goto badret4 ;

	    cp = strbasename(ofname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_start: ofname=%s\n",cp) ;
#endif

	    strcpy(jep->ofname,cp) ;

	} else
	    mkpath2(ofname,g.tmpdir,jep->ofname) ;

/* error file */

	if (jep->efname[0] == '\0') {

	    mkpath2(tmpfname,g.tmpdir, "dwd2XXXXXXXXXX") ;

	    if ((rs = mktmpfile(efname,0600,tmpfname)) < 0)
	        goto badret5 ;

	    cp = strbasename(efname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_start: efname=%s\n",cp) ;
#endif

	    strcpy(jep->efname,cp) ;

	} else
	    mkpath2(efname,g.tmpdir,jep->efname) ;


/* get the program to execute */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_start: service table args> %s\n",sep->args) ;
#endif

	if (vecstrinit(&args,10,0) < 0)
	    goto badret6 ;

	arg0 = NULL ;
	program = sep->program ;
	if ((sep->args != NULL) && (sep->args[0] != '\0')) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_start: init'ed\n") ;
#endif

/* create the substitutions for the service daemon argument vector */

	    subs.version = VERSION ;
	    subs.service = jep->service ;

	    subs.date = subs_jobdate ;
	    timestr_log(jep->date,subs_jobdate) ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_start: job date=%s\n",subs_jobdate) ;
#endif

/* get the user and group names of the file */

	    subs.username = g.username ;
	    if (sep->username != NULL)
	        subs.username = sep->username ;

	    if (sep->groupname != NULL)
	        subs.groupname = sep->groupname ;

	    else if (g.groupname != NULL)
	        subs.groupname = g.groupname ;

	    else {

#ifdef	SYSV
	        gp = (struct group *) getgrgid_r(sbp->st_gid,
	            &ge,buf,BUFLEN) ;
#else
	        gp = getgrgid(sbp->st_gid) ;
#endif

	        if (gp != NULL) {
	            strcpy(subs_groupname,gp->gr_name) ;
	        } else
	            bufprintf(subs_groupname,USERNAMELEN,
			"GUEST-%d",(int) sbp->st_gid) ;

	        subs.groupname = subs_groupname ;

	    } /* end if */

	    if (jep->clen >= 0) {

	        subs_length[0] = '\0' ;
	        subs.length = subs_length ;
	        ctdeci(subs_length,33,jep->clen) ;

	    } else
	        subs.length = "-1" ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_start: size=%ld length=%s\n",
	            sbp->st_size,subs_length) ;
#endif

/* process them */

	    if ((rs = processargs(&args,sep->args,&subs)) < 0)
	        goto badret7 ;

	    if (vecstrcount(&args) > 0) {

	        arg0 = strbasename(args.va[0]) ;

	        if (program == NULL)
	            program = args.va[0] ;

	    }
	}

	if ((arg0 == NULL) && (program != NULL)) {

	    arg0 = strbasename(program) ;

	    vecstradd(&args,arg0,-1) ;

	}

	if ((program == NULL) && (arg0 == NULL)) {

	    rs = SR_BAD ;
	    goto badret7 ;
	}

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_start: program=%s arg0=%s\n",program,arg0) ;
#endif

#ifdef	CF_ACCESSCHECK

/* can we execute this service daemon ? */

	if (program[0] == '/') {

	    if ((rs = u_access(program,R_OK)) < 0)
	        goto badnotpresent ;

	    if ((rs = u_access(program,X_OK)) < 0)
	        goto badnotexec ;

	} else (getfiledirs(NULL,program,"rx",NULL) < 0) {

	    if (getfiledirs(NULL,program,"r",NULL) < 0)
	        goto badnotexec ;

	    else
	        goto badnotexec ;

	}

	sep->f.notpresent = FALSE ;
	sep->f.notexec = FALSE ;

#endif /* CF_ACCESSCHECK */


/* fork it ! */

	handlejob_setstat(jep,0,0) ;

	while ((pid = uc_fork()) < 0) {
	    rs = pid ;

	    if ((rs != SR_INTR) && (rs != SR_NOMEM)) break ;

	    logfile_printf(&g.lh,
	        "trouble forking, rs=%d\n",rs) ;

	    sleep(2) ;

	} /* end while */

	if (pid < 0) {

	    logfile_printf(&g.lh,
	        "could not fork (for service daemon), rs=%d\n",rs) ;

	    goto badret7 ;

	} else if (pid == 0) {

	    if ((g.workdir != NULL) && (g.workdir[0] != '\0') &&
	        (strcmp(g.workdir,".") != 0)) {

	        if ((rs = u_chdir(g.workdir)) < 0) {

	            handlejob_setstat(jep,1,rs) ;

	            exit(BAD) ;

	        }

	    }

	    if (ifd < CLOSEDFDS) {

	        i = ifd ;
	        ifd = dupup(ifd,3) ;

	        close(i) ;

	    }

/* close the shared memory FD that the parent keeps open */

	    close(jep->sfd) ;

/* close some other common FDs */

	    for (i = 0 ; i < CLOSEDFDS ; i += 1) {

	        if (i != ifd) close(i) ;

	    }

/* make the FDs that we want for the child */

	    dup(ifd) ;

	    close(ifd) ;

	    fd1 = open(ofname,O_FLAGS,0600) ;

	    fd2 = open(efname,O_FLAGS,0600) ;

	    handlejob_setstat(jep,2,SR_OK) ;

	    rs = u_execvp(program,args.va) ;

	    handlejob_setstat(jep,2,rs) ;

	    exit(BAD) ;

	} /* end if (child) */

	jep->pid = pid ;

	logfile_printf(&g.lh,"job executing, daemon=%s pid=%d\n",
	    arg0,pid) ;


	if (f_infile) bclose(jfp) ;

	close(ifd) ;

	vecstrfree(&args) ;

	return pid ;

/* handle bad returns */
badret7:
	vecstrfree(&args) ;

badret6:
	unlink(efname) ;

badret5:
	unlink(ofname) ;

badret4:
	if (ifd >= 0) close(ifd) ;

	if ((jfp != NULL) && (jep->clen > 0))
	    bseek(jfp,jep->offset,SEEK_SET) ;

badret3:

badret2:
	if (f_infile) bclose(jfp) ;

badret:
	return rs ;

badnopresent:
	if (! sep->f.notpresent) {

	    sep->f.notpresent = TRUE ;
	    logfile_printf(&g.lh,"service daemon program is not present\n") ;

	}

	goto badret7 ;

badnotexec:
	if (! sep->f.notexec) {

	    sep->f.notexec = TRUE ;
	    logfile_printf(&g.lh,"service daemon program is not executable\n") ;

	}

	goto badret7 ;

}
/* end subroutine (handlejob_start) */


/* end a job */
void handlejob_end(jep,child_stat)
struct jobentry	*jep ;
int		child_stat ;
{
	bfile		file, *fp = &file ;

	struct ustat	sb ;

	int		len ;

	char		linebuf[LINELEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_end: entered, file=%s\n",jep->filename) ;
#endif

#ifdef	COMMENT
	logfile_printf(&g.lh,"process exit status %d\n",child_stat) ;
#endif

/* standard output file */

	mkpath2(tmpfname,g.tmpdir,jep->ofname) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_end: ofname=%s\n",tmpfname) ;
#endif

	if ((stat(tmpfname,&sb) >= 0) && (sb.st_size > 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_end: got some output, size=%d\n",
	            sb.st_size) ;
#endif

	    logfile_printf(&g.lh,"job standard output\n") ;

	    if (bopen(fp,tmpfname,"r",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,LINELEN)) > 0) {

	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&g.lh," | %W\n",linebuf,len) ;

	        }

	        bclose(fp) ;

	    } /* end if (opened file) */
	}

	unlink(tmpfname) ;

/* standard error file */

	mkpath2(tmpfname,g.tmpdir,jep->efname) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("handlejob_end: efname=%s\n",tmpfname) ;
#endif

	if ((stat(tmpfname,&sb) >= 0) && (sb.st_size > 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("handlejob_end: goto some error output, size=%d\n",
	            sb.st_size) ;
#endif

	    logfile_printf(&g.lh,"job standard error\n") ;

	    if (bopen(fp,tmpfname,"r",0666) >= 0) {

	        while ((len = breadline(fp,linebuf,LINELEN)) > 0) {

	            if (linebuf[len - 1] == '\n')
	                linebuf[--len] = '\0' ;

	            logfile_printf(&g.lh," | %W\n",linebuf,len) ;

	        }

	        bclose(fp) ;

	    } /* end if (opened file) */

	}

	unlink(tmpfname) ;


}
/* end subroutine (handlejob_end) */


int handlejob_setstat(jep,w,s)
struct jobentry	*jep ;
int	w, s ;
{
	struct job_status	*table ;


	if (jep == NULL) return BAD ;

	table = *(jep->jspp) ;
	table[jep->index].what = w ;
	table[jep->index].status = s ;
	return OK ;
}


int handlejob_getstat(jep,wp,sp)
struct jobentry	*jep ;
int	*wp, *sp ;
{
	struct job_status	*table ;


	if (jep == NULL) return BAD ;

	table = *(jep->jspp) ;
	*wp = table[jep->index].what ;
	*sp = table[jep->index].status ;
	return OK ;
}


int handlejob_del(jep)
struct jobentry	*jep ;
{
	struct job_status	*table ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if (jep == NULL) return BAD ;

	if (jep->state != JOBSTATE_WATCH) {

	    vecstrfree(&jep->path) ;

	    vecstrfree(&jep->srvargs) ;

	    ema_finish(&jep->from) ;

	    ema_finish(&jep->error_to) ;

	    ema_finish(&jep->sender) ;

	}

	if (jep->filename[0] != '\0') {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        g.directory,jep->filename) ;

	    unlink(tmpfname) ;

	}

	return OK ;
}
/* end subroutine (handlejob_del) */



/* INTERNAL SUBROUTINES */



/* process an argument list */
static int processargs(alp,command,app)
vecstr		*alp ;
char			command[] ;
struct argparams	*app ;
{
	FIELD	fsb ;

	int	fl, elen ;
	int	i ;

	char	terms[32] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;


#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("processargs: entered> %s\n",command) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("processargs: length=%s\n",app->length) ;
#endif

	if ((command == NULL) || (command[0] == '\0')) return 0 ;

	fieldterms(terms,0," \t") ;

	field_start(&fsb,command,-1) ;

/* loop through the arguments */

	i = 0 ;
	while ((fl = field_sharg(&fsb,terms,buf,BUFLEN)) > 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("processargs: TOL f=%W fl=%d\n",
	            fsb.fp,fsb.flen,fsb.flen) ;
#endif

	    if ((elen = expand(buf,fl,app,buf2,BUFLEN)) < 0) return -1 ;

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("processargs: buf2> %s\n",buf2) ;
#endif

	    if (vecstradd(alp,buf2,elen) < 0) return -1 ;

	    i += 1 ;

	} /* end while */

	field_finish(&fsb) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("processargs: exiting, %d args\n",i) ;
#endif

	return i ;
}
/* end subroutine (processargs) */


/* expand out a program argument with the substitution parameters */

/*
#	The following substitutions are made on command strings :
#		%j	job ID
#		%s	service name
#		%u	service username
#		%g	service groupname
#		%l	program's input length in bytes
#		%d	date job was initiated (in UNIX integer time format)
#		%V	Directory Watcher version string
#
*/

static int expand(buf,len,app,rbuf,rlen)
char			rbuf[], buf[] ;
int			rlen, len ;
struct argparams	*app ;
{
	int	elen = 0, l ;

	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;


	if (len == 0) return 0 ;

	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) return elen ;

	        switch ((int) *bp) {

	        case 'j':
	            cp = app->jobid ;
	            l = strlen(cp) ;

	            break ;

	        case 's':
	            cp = app->service ;
	            l = strlen(cp) ;

	            break ;

	        case 'u':
	            cp = app->username ;
	            l = strlen(cp) ;

	            break ;

	        case 'g':
	            cp = app->groupname ;
	            l = strlen(cp) ;

	            break ;

	        case 'l':

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                debugprintf("expand: length=%s\n",app->length) ;
#endif

	            cp = app->length ;
	            l = strlen(cp) ;

	            break ;

	        case 'd':
	            cp = app->date ;
	            l = strlen(cp) ;

	            break ;

	        case 'V':
	            cp = app->version ;
	            l = strlen(cp) ;

	            break ;

	        default:
	            cp = bp ;
	            l = 1 ;
	        }

	        bp += 1 ;
	        len -= 1 ;

	        if ((elen + l) > rlen) return BAD ;

	        strncpy(rbp,cp,l) ;

	        rbp += l ;
	        elen += l ;
	        break ;

	    default:
	        *rbp++ = *bp++ ;
	        elen += 1 ;
	        len -= 1 ;

	    } /* end switch */

	} /* end while */

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */



