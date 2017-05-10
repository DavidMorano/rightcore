/* jobsched */

/* schedule the jobs around */
/* version %I% last modified %G% */


#define	CF_DEBUG	1


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This subroutine was adopted from the DWD program.


*/



#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/time.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<dirent.h>
#include	<limits.h>
#include	<string.h>
#include	<stropts.h>
#include	<poll.h>
#include	<ftw.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<msg.h>
#include	<msgheaders.h>
#include	<ema.h>
#include	<char.h>
#include	<localmisc.h>

#include	"jobdb.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"srvfile.h"


/* local defines */

#define	W_OPTIONS	(WUNTRACED | WNOHANG)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	matfromenv(const char *,int) ;
extern int	matmsghead(const char *,int,char *,int *) ;
extern int	jobdb_search(), jobdb_findpid() ;
extern int	handlejob_start(), handlejob_end() ;

extern int	path_init(), path_parse() ;

extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */

extern struct global	g ;


/* forwards */

static int	ext_id() ;
static int	handle_enter() ;


/* local global variables */


/* local structures */


/* local data */

/* built-in function table */

#define	FUN_REPORT	0
#define	FUN_EXIT	1


static const char	*funtab[] = {
	"REPORT",
	"EXIT",
	NULL
} ;




/*****************************************************************************

	This subroutine is responsible for handling certain job
	conditions.

	This subroutine is called with the folowing arguments:

	Arguments:
	wsp	watcher state (pointer)
	jep	job entry pointer
	sbp	job's file status block pointer
	slp	daemon's service list pointer 

	Returns:
	OK	doesn't really matter in the current implementation


*****************************************************************************/




int handle_new(wsp,jlp,jep,sbp,slp)
struct watchstate	*wsp ;
JOBDB		*jlp ;
struct jobentry	*jep ;
struct ustat	*sbp ;
struct vecelem	*slp ;
{
	struct sigaction	sigs ;

	MSGMATENV	me ;

	bfile		infile, *ifp = &infile ;

	sigset_t	signalmask ;

	time_t		daytime ;

	offset_t		offset ;

	int	rs, i ;
	int	len, l ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	linebuf[LINELEN + 1] ;
	char	headname[LINELEN + 1] ;
	char	*cp, *cp1 ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_new: entered\n") ;
#endif

	wsp->c[JOBSTATE_WATCH] -= 1 ;

/* can we open and read this file */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_new: filename=%s\n",jep->filename) ;
#endif

	mkpath2(tmpfname, g.directory,jep->filename) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_new: filepath=%s\n",tmpfname) ;
#endif

	len = 0 ;
	rs = bopen(ifp,tmpfname,"r",0666) ;

	if (rs < 0)
		goto ret0 ;

	offset = 0L ;

/* analyze this job */
top:
	while (TRUE) {

	    handle_enter(wsp,jep,sbp,slp,ifp,offset,linebuf,len) ;

/* are we done? */

	    offset = jep->offset + jep->clen ;
	    if (sbp->st_size <= offset) 
		break ;

	while ((rs = breadline(ifp,linebuf,LINELEN)) > 0) {

	    len = rs ;
	    if (! matfromenv(linebuf,len))
		continue ;

	    if (matmsghead(linebuf,len,headname,NULL) > 0)
		continue ;

	    offset += len ;

	} /* end while */

	if (rs < 0)
		break ;

/* we have another job hidden back here, allocate a new job entry */

	    handle_add(wsp,jlp,jep->filename,sbp,&jep) ;

	} /* end while */

done:
ret1:
	bclose(ifp) ;

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (handle_new) */


static int handle_enter(wsp,jep,sbp,slp,ifp,offset,linebuf,linelen)
struct watchstate	*wsp ;
struct jobentry	*jep ;
struct ustat	*sbp ;
struct vecelem	*slp ;
bfile		*ifp ;
offset_t		offset ;
char	linebuf[] ;
int	linelen ;
{
	struct sigaction	sigs ;

	struct srventry		*sep ;

	MSG		jh, *jhp = &jh ;

	MSGMATENV	*mep ;

	MSGHEADERS	*mhp ;

	sigset_t	signalmask ;

	time_t		daytime ;

	int	rs, i ;
	int	len, l ;

	char	**msghvalues ;
	char	queuespec[QUEUESPECLEN + 1] ;
	char	*queue_path, *queue_machine ;
	char	*cp, *cp1 ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: entered, len=%d > %W\n",
			linelen,linebuf,linelen) ;
#endif

/* analyze the job headers */

	l = msg_init(jhp,ifp,offset,linebuf,linelen) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: return from 'msg_init' rs=%d\n",l) ;
#endif

	offset += l ;
	jep->offset = offset ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: offset=%ld size=%ld\n",offset,sbp->st_size) ;
#endif

	msgheaders_init(&msghvalues,jhp) ;

/* what do we have ? */

#if	CF_DEBUG
	if (g.debuglevel > 1) {

	debugprintf("handle_enter: job headers\n") ;

		for (i = 0 ; msg_headget(jhp,i,&mhp) >= 0 ; i += 1) {

			if (mhp == NULL) continue ;

		debugprintf("handle_enter: H> %s: %s\n",
			mhp->name,mhp->value) ;

		}

	}
#endif /* CF_DEBUG */

/* get some initial stuff */

	if (msghvalues[HI_XJOBID] != NULL)
	    strwcpy(jep->jobid,msghvalues[HI_XJOBID],JOBIDLEN) ;

	else
	    strwcpy(jep->jobid,jep->filename,JOBIDLEN) ;


/* get the destination for this job */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: x-queuespec=%s\n",msghvalues[HI_XQUEUESPEC]) ;
#endif

	if (msghvalues[HI_XQUEUESPEC] != NULL)
	    ext_id(queuespec,msghvalues[HI_XQUEUESPEC]) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: queuespec=%s\n",queuespec) ;
#endif

	if ((cp = strpbrk(queuespec,"!:")) != NULL) {

	    *cp++ = '\0' ;
	    queue_machine = strshrink(queuespec) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: queue_machine %s\n",queue_machine) ;
#endif

	    if (cp[0] != '\0')
	        queue_path = strshrink(cp) ;

	    else
	        queue_path = NULL ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: queue_path=%s\n",queue_path) ;
#endif

	} else {

	    queue_path = NULL ;
	    queue_machine = strshrink(queuespec) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: qm=%s qp=%s\n",queue_machine,queue_path) ;
#endif

	if (queue_path != NULL)
	    bufprintf(jep->queuespec,QUEUESPECLEN,"%s!%s",
	        queue_machine,queue_path) ;

	else
	    strcpy(jep->queuespec,queue_machine) ;


	logfile_printf(&g.lh,"dst=%s jobid=%s\n",queue_machine,jep->jobid) ;

/* is this job destined for us ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("handle_enter: about to check if for us %s\n",queue_machine) ;
#endif

	if (! hostequiv(queue_machine,g.nodename,g.domainname))
	    goto handle_routing ;


/* since it is for us, get the rest of the stuff we want from the headers */


/* get the service code */

	if (msghvalues[HI_XSERVICE] == NULL)
	    goto handle_nosrv ;

	l = sfshrink(msghvalues[HI_XSERVICE],-1,&cp1) ;

	if (l <= 0) goto handle_nosrv ;

	cp1[l] = '\0' ;
	vecstrinit(&jep->srvargs,5,VSP_SORTED) ;

	while ((cp = strpbrk(cp1," \t\n\r\v")) != NULL) {

	    vecstradd(&jep->srvargs,cp1,cp - cp1) ;

	    *cp++ = '\0' ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp1 = cp ;

	} /* end while */

	if (cp1[0] != '\0')
	    vecstradd(&jep->srvargs,cp1,-1) ;

	for (i = 0 ; vecstrget(&jep->srvargs,i,&cp) >= 0 ; i += 1)
	    if (cp != NULL) break ;

	if (cp == NULL) {

	    vecstrfree(&jep->srvargs) ;

	    goto handle_nosrv ;

	} else
	    strwcpy(jep->service,cp,MAXNAMELEN) ;

	logfile_printf(&g.lh,"srv=%s\n",jep->service) ;


	jep->orig_machine[0] = '\0' ;
	if (msghvalues[HI_XORIGHOST] != NULL)
	    strwcpy(jep->orig_machine,msghvalues[HI_XORIGHOST],
	        MAXHOSTNAMELEN) ;

	jep->orig_user[0] = '\0' ;
	if (msghvalues[HI_XORIGUSER] != NULL)
	    strwcpy(jep->orig_user,msghvalues[HI_XORIGUSER],
	        MAXUSERNAMELEN) ;

	jep->clen = -1 ;
	if (msghvalues[HI_CLEN] != NULL) {

	    if (cfdeci(msghvalues[HI_CLEN],-1,&jep->clen) < 0)
	        jep->clen = -1 ;

	}

	if (jep->clen < 0)
	    jep->clen = sbp->st_size - offset ;

	jep->date = (time_t) -1 ;
	if (msghvalues[HI_DATE] != NULL) {

	    if ((jep->date = getindate(msghvalues[HI_CLEN],NULL)) ==
	        ((time_t) -1)) {

	        jep->date = getabsdate(msghvalues[HI_CLEN],NULL) ;

	    }
	}


	path_init(&jep->path) ;

	if (msghvalues[HI_PATH] != NULL)
	    path_parse(&jep->path,msghvalues[HI_PATH]) ;


	ema_start(&jep->from) ;

	if (msghvalues[HI_FROM] != NULL)
	    ema_parse(&jep->from,msghvalues[HI_FROM],-1) ;

	ema_start(&jep->sender) ;

	if (msghvalues[HI_SENDER] != NULL)
	    ema_parse(&jep->sender,msghvalues[HI_SENDER],-1) ;

	ema_start(&jep->error_to) ;

	if (msghvalues[HI_ERRORTO] != NULL)
	    ema_parse(&jep->error_to,msghvalues[HI_ERRORTO],-1) ;


/* initialize some stuff */

	jep->ofname[0] = '\0' ;
	jep->efname[0] = '\0' ;


/* do we have this service registered ? */

	if (srvsearch(slp,jep->service,&sep) < 0)
	    goto handle_stale ;


/* are we at the job running limit ? */

	if (wsp->c[JOBSTATE_STARTED] >= g.maxjobs)
	    goto handle_maxjobs ;


/* run the daemon, if we can */

	if ((rs = handlejob_start(jep,ifp,sbp, sep)) < 0)
	    goto handle_noexec ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: started job\n") ;
#endif

	jep->state = JOBSTATE_STARTED ;
	wsp->c[JOBSTATE_STARTED] += 1 ;

	logfile_printf(&g.lh,"job started OK\n") ;

/* finish this one off */
done:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: done OK\n") ;
#endif

	msgheaders_free(&msghvalues) ;

	msg_free(jhp) ;

	return OK ;

/* handle bad things here , the job did not specify a service !! */
handle_nosrv:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: there was no service on this job\n") ;
#endif

	msgheaders_free(&msghvalues) ;

	msg_free(jhp) ;

	return BAD ;

/* this job should be routed further */
handle_routing:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: routing\n") ;
#endif

	jep->state = JOBSTATE_ROUTING ;
	wsp->c[JOBSTATE_ROUTING] += 1 ;
	goto done ;

/* there is currently no service for this job */
handle_stale:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: stale\n") ;
#endif

	jep->state = JOBSTATE_STALE ;
	wsp->c[JOBSTATE_STALE] += 1 ;
	goto done ;

/* we are already at the maximum number of running jobs */
handle_maxjobs:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: maximum jobs\n") ;
#endif

	jep->state = JOBSTATE_MAXJOBS ;
	wsp->c[JOBSTATE_MAXJOBS] += 1 ;
	goto done ;

/* we could not "exec" this program for some reason */
handle_noexec:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_enter: could not EXEC\n") ;
#endif

	jep->state = JOBSTATE_STALE ;
	wsp->c[JOBSTATE_STALE] += 1 ;
	goto done ;

}
/* end subroutine (handle_enter) */


/* add a new job to the job DB */
int handle_add(wsp,jlp,filename,jsbp,jepp)
struct watchstate	*wsp ;
JOBDB		*jlp ;
char		filename[] ;
struct ustat	*jsbp ;
struct jobentry	**jepp ;
{
	struct jobentry		je ;

	time_t		daytime ;

	int		i ;

	char		timebuf[TIMEBUFLEN + 1] ;


/* we have not seen this job before ! */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_add: new job\n") ;
#endif

	strcpy(je.filename,filename) ;

	makelogid(wsp,&je.logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_add: logid=%s\n",je.logid) ;
#endif

	je.size = jsbp->st_size ;
	je.atime = jsbp->st_mtime ;
	je.mtime = jsbp->st_mtime ;

	logfile_setid(&g.lh,je.logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_add: set logid\n") ;
#endif

	(void) time(&daytime) ;

	logfile_printf(&g.lh, "%s job entering\n",
	    timestr_log(daytime,timebuf)) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_add: made log entry\n") ;
#endif

	logfile_printf(&g.lh,
	    "f=%s\n",je.filename) ;

	je.state = JOBSTATE_WATCH ;

	i = jobdb_add(jlp,&je,sizeof(struct jobentry)) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("handle_add: added job to job table\n") ;
#endif

	logfile_setid(&g.lh,g.logid) ;

	jobdb_get(jlp,i,jepp) ;

	return i ;
}
/* end subroutine (handle_add) */


/* delete a job */
int handle_del(jlp,i,jep)
JOBDB		*jlp ;
int		i ;
struct jobentry	*jep ;
{
	int		rs ;


	if (jep == NULL) {

		if ((rs = jobdb_get(jlp,i,&jep)) < 0) return rs ;

	} else if (i < 0) {

		if ((i = jobdb_getp(jlp,jep)) < 0) return i ;

	}

	(void) handlejob_del(jep) ;

	return jobdb_del(jlp,i) ;
}
/* end subroutine (handle_del) */


/* extract an ID type header value */
static int ext_id(articleid,s)
char	s[], articleid[] ;
{
	struct ema		aid ;

	struct ema_ent	*ep ;

	int	i, rs ;


	articleid[0] = '\0' ;
	ema_start(&aid) ;

	if (ema_parse(&aid,s) > 0) {

	    for (i = 0 ; (rs = ema_get(&aid,i,&ep)) >= 0 ; i += 1) {

	        if (ep == NULL) continue ;

	        if (ep->f.error) continue ;

	        if (ep->rlen > 0) {

	            strcpy(articleid,ep->route) ;

	            break ;

	        } else if (ep->alen > 0) {

	            strcpy(articleid,ep->address) ;

	            break ;
	        }

	    } /* end for */

	} /* end if */

	ema_finish(&aid) ;

	return (articleid[0] == '\0') ? BAD : OK ;
}
/* end subroutine (ext_id) */



