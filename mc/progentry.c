/* progentry */

/* build up a program entry piece-meal as it were */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGS2	0		/* more */


/* revision history:

	= 1996-09-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little object is used to create a program entry and to
	populate aspects of it with different operations on the
	object.  This object is used in "server" types of programs.
	This object is usually created from elements taken from the
	parsing of a server file.


******************************************************************************/


#define	PROGENTRY_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<field.h>
#include	<sbuf.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"srvtab.h"
#include	"progentry.h"

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif


/* local defines */

#undef	BUFLEN
#define	BUFLEN		(10 * MAXPATHLEN)

#define	PROGENTRY_MAGIC	0x76452376


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	cfdecti(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* forward references */

static int progentry_process(PROGENTRY *,char *,PROGENTRY_ARGS *,char *,int) ;
static int expand(PROGENTRY_ARGS *,char *,int,char *,int) ;
static int processargs(vecstr *,char *) ;
static int mkfile(const char *,const char *,char *) ;

static void	freeit(char **) ;


/* external variables */


/* local variables */

static const char	xes[] = "XXXXXXXXXXXXXX" ;


/* exported subroutines */


int progentry_init(pep,ssp,sep,esap)
PROGENTRY	*pep ;
varsub		*ssp ;
SRVTAB_ENT	*sep ;
PROGENTRY_ARGS	*esap ;
{
	int	rs ;
	int	sl ;

	char	outbuf[BUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("progentry_init: entered, BUFLEN=%d\n",BUFLEN) ;
#endif

	if (pep == NULL)
	    return SR_FAULT ;

	pep->magic = 0 ;
	if (sep == NULL)
		return SR_FAULT ;

#ifdef	OPTIONAL
	(void) memset(pep,0,sizeof(PROGENTRY)) ;
#endif

	(void) memset(&pep->f,0,sizeof(struct progentry_flags)) ;

	pep->program = NULL ;
	pep->username = NULL ;
	pep->groupname = NULL ;
	pep->options = NULL ;
	pep->access = NULL ;

	pep->ofname[0] = '\0' ;
	pep->efname[0] = '\0' ;
	pep->jobid[0] = '\0' ;

/* store what we will need later ! */

	pep->ssp = ssp ;
	pep->atime = esap->daytime ;

/* load the service name */

	strwcpy(pep->name,sep->service,MAXNAMELEN) ;

	rs = SR_OK ;

/* process the access field */

	if ((rs >= 0) && (sep->access != NULL)) {

	    rs = progentry_process(pep,sep->access,esap,outbuf,BUFLEN) ;

#if	CF_DEBUGS
	    debugprintf("progentry_init: access process() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad0 ;

	    sl = rs ;
	    if ((rs = uc_mallocstrw(outbuf,sl,&pep->access)) < 0)
	        goto bad0 ;

#ifdef	MALLOCLOG
		malloclog_alloc(pep->access,sl,"progentry_init:access") ;
#endif

	} /* end if (access field) */

/* process the interval field */

	if ((rs >= 0) && (sep->interval != NULL)) {

#if	CF_DEBUGS
	    debugprintf("progentry_init: srv interval=%s BUFLEN=%d\n",
	        sep->interval,BUFLEN) ;
#endif

	    rs = progentry_process(pep,sep->interval,esap,outbuf,BUFLEN) ;

#if	CF_DEBUGS
	    debugprintf("progentry_init: interval process() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad1 ;

/* convert the interval string to an integer */

	    rs = cfdecti(outbuf,rs,&pep->interval) ;

#if	CF_DEBUGS
	    debugprintf("progentry_init: cfdecti() rs=%d v=%d\n",
	        rs,pep->interval) ;
#endif

	    pep->interval = (rs >= 0) ? pep->interval : -1 ;
		rs = SR_OK ;

	} else
	    pep->interval = -1 ;

	pep->magic = PROGENTRY_MAGIC ;

ret0:
	return rs ;

/* bad stuff happened */
bad1:
	if (pep->access != NULL) {

		free(pep->access) ;

#ifdef	MALLOCLOG
	malloclog_free(pep->access,"progentry_init:access") ;
#endif

	}

bad0:
	goto ret0 ;
}
/* end subroutine (progentry_init) */


/* retrieve for caller the access groups for this entry */
int progentry_getaccess(pep,rpp)
PROGENTRY	*pep ;
char		**rpp ;
{


	if (pep == NULL)
	    return SR_FAULT ;

	if (pep->magic != PROGENTRY_MAGIC)
		return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = pep->access ;

	return ((pep->access != NULL) ? SR_OK : SR_EMPTY) ;
}
/* end subroutine (progentry_getaccess) */


/* retrieve for caller the execution interval for this entry */
int progentry_getinterval(pep,rp)
PROGENTRY	*pep ;
int		*rp ;
{


	if (pep == NULL)
	    return SR_FAULT ;

	if (pep->magic != PROGENTRY_MAGIC)
		return SR_NOTOPEN ;

	if (rp != NULL)
	    *rp = pep->interval ;

	return SR_OK ;
}
/* end subroutine (progentry_getinterval) */


/* free up this entry */
int progentry_free(pep)
PROGENTRY	*pep ;
{


#if	CF_DEBUGS
	debugprintf("progentry_free: entered\n") ;
#endif

	if (pep == NULL)
	    return SR_FAULT ;

	if (pep->magic != PROGENTRY_MAGIC)
		return SR_NOTOPEN ;

	if (pep->ofname[0] != '\0')
	    u_unlink(pep->ofname) ;

	if (pep->efname[0] != '\0')
	    u_unlink(pep->efname) ;

	if (pep->f.srvargs)
	    (void) vecstr_finish(&pep->srvargs) ;

	if (pep->program != NULL) {

	    free(pep->program) ;

#ifdef	MALLOCLOG
	malloclog_free(pep->program,"progentry_free:program") ;
#endif

	}

	if (pep->username != NULL) {

	    free(pep->username) ;

#ifdef	MALLOCLOG
	malloclog_free(pep->username,"progentry_free:username") ;
#endif

	}

	if (pep->groupname != NULL) {

	    free(pep->groupname) ;

#ifdef	MALLOCLOG
	malloclog_free(pep->groupname,"progentry_free:groupname") ;
#endif

	}

	if (pep->options != NULL) {

	    free(pep->options) ;

#ifdef	MALLOCLOG
	malloclog_free(pep->options,"progentry_free:options") ;
#endif

	}

	if (pep->access != NULL) {

	    free(pep->access) ;

#ifdef	MALLOCLOG
	malloclog_free(pep->access,"progentry_free:access") ;
#endif

	}

	pep->program = NULL ;
	pep->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (progentry_free) */


/* expand out the server entry */
int progentry_expand(pep,sep,esap)
PROGENTRY	*pep ;
SRVTAB_ENT	*sep ;
PROGENTRY_ARGS	*esap ;
{
	varsub	*ssp ;

	int	rs, sl, cl ;

	char	outbuf[BUFLEN + 1] ;
	char	*arg0 ;
	char	*oldservice, *oldinterval ;
	char	*tmpdir ;
	char	*cp ;


#if	CF_DEBUGS
	debugprintf("progentry_expand: entered\n") ;
#endif

	if (pep == NULL)
	    return SR_FAULT ;

	if (pep->magic != PROGENTRY_MAGIC)
		return SR_NOTOPEN ;

	ssp = pep->ssp ;

	oldservice = esap->service ;
	oldinterval = esap->interval ;

	esap->service = sep->service ;
	esap->interval = sep->interval ;

/* load the job ID if one was supplied */

	if (esap->logid != NULL)
	    strwcpy(pep->jobid,esap->logid,PROGENTRY_IDLEN) ;

/* did they supply a TMPDIR ? */

	tmpdir = (esap->tmpdir != NULL) ? esap->tmpdir : PROGENTRY_TMPDIR ;

#if	CF_DEBUGS
	debugprintf("progentry_expand: tmpdir=%s\n",tmpdir) ;
#endif

/* make some temporary files for program file input and output */

	rs = mkfile(tmpdir,"o",pep->ofname) ;

#if	CF_DEBUGS
	debugprintf("progentry_expand: mkfile() 1 rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = mkfile(tmpdir,"e",pep->efname) ;

#if	CF_DEBUGS
	debugprintf("progentry_expand: mkfile() 2 rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	    debugprintf("progentry_expand: p=%s\n",
		sep->program) ;
	    debugprintf("progentry_expand: args=>%s<\n",
		sep->args) ;
#endif

/* pop them */

	if (sep->program != NULL) {

#if	CF_DEBUGS
	    debugprintf("progentry_expand: srventry program=>%s<\n",
		sep->program) ;
#endif

	    rs = progentry_process(pep,sep->program,esap,outbuf,BUFLEN) ;

	    if (rs < 0)
	        goto bad2 ;

#if	CF_DEBUGS
	    debugprintf("progentry_expand: outbuf=>%w<\n",
		outbuf,rs) ;
#endif

	    sl = rs ;
	    cl = sfshrink(outbuf,sl,&cp) ;

#if	CF_DEBUGS
	    debugprintf("progentry_expand: cleaned-up outbuf=>%w<\n",
		cp,cl) ;
#endif

	    rs = uc_mallocstrw(cp,cl,&pep->program) ;

		if (rs < 0)
	        goto bad2 ;

#ifdef	MALLOCLOG
		malloclog_alloc(pep->program,sl,"progentry_expand:program") ;
#endif

#if	CF_DEBUGS
	    debugprintf("progentry_expand: processed=>%s<\n",
		pep->program) ;
#endif

	} /* end if (program path) */

	arg0 = NULL ;
	if (sep->args != NULL) {

#if	CF_DEBUGS
	    debugprintf("progentry_expand: srvargs args=>%s<\n",
		sep->args) ;
#endif

	    rs = progentry_process(pep,sep->args,esap,outbuf,BUFLEN) ;

#if	CF_DEBUGS
	    debugprintf("progentry_process: outbuf=>%s<\n",
	        outbuf) ;
#endif

	    if (rs < 0)
	        goto bad3 ;

	    sl = rs ;
	    rs = vecstr_start(&pep->srvargs,6,0) ;

	    if (rs < 0)
	        goto bad3 ;

	    pep->f.srvargs = TRUE ;
	    if ((rs = processargs(&pep->srvargs,outbuf)) > 0) {

	        rs = vecstr_get(&pep->srvargs,0,&arg0) ;

		if (rs < 0)
			arg0 = NULL ;

#if	CF_DEBUGS
	    debugprintf("progentry_expand: processed arg0=%s\n",arg0) ;
#endif

	    } /* end if */

	} /* end if (program arguments) */

	if (sep->username != NULL) {

#if	CF_DEBUGS
	    debugprintf("progentry_expand: username\n") ;
#endif

	    rs = progentry_process(pep,sep->username,esap,outbuf,BUFLEN) ;

	    if (rs < 0)
	        goto bad4 ;

	    sl = rs ;
	    if ((rs = uc_mallocstrw(outbuf,sl,&pep->username)) < 0)
	        goto bad4 ;

#ifdef	MALLOCLOG
		malloclog_alloc(pep->username,sl,"progentry_expand:username") ;
#endif

	} /* end if (username field) */

	if (sep->groupname != NULL) {

#if	CF_DEBUGS
	    debugprintf("progentry_expand: groupname\n") ;
#endif

	    rs = progentry_process(pep,sep->groupname,esap,outbuf,BUFLEN) ;

	    if (rs < 0)
	        goto bad5 ;

	    sl = rs ;
	    if ((rs = uc_mallocstrw(outbuf,sl,&pep->groupname)) < 0)
	        goto bad5 ;

#ifdef	MALLOCLOG
		malloclog_alloc(pep->groupname,sl,
			"progentry_expand:groupname") ;
#endif

	}

	if (sep->options != NULL) {

#if	CF_DEBUGS
	    debugprintf("progentry_expand: options\n") ;
#endif

	    rs = progentry_process(pep,sep->options,esap,outbuf,BUFLEN) ;

	    if (rs < 0)
	        goto bad6 ;

	    sl = rs ;
	    if ((rs = uc_mallocstrw(outbuf,sl,&pep->options)) < 0)
	        goto bad6 ;

#ifdef	MALLOCLOG
		malloclog_alloc(pep->options,sl,"progentry_expand:options") ;
#endif

	}

/* OK, perform some fixups */

#if	CF_DEBUGS
	debugprintf("progentry_expand: program fixup p=%s\n",
		pep->program) ;
#endif

	if ((pep->program == NULL) && (arg0 != NULL)) {

#if	CF_DEBUGS
	debugprintf("progentry_expand: arg0=>%s<\n",
		arg0) ;
#endif

	    cl = sfshrink(arg0,-1,&cp) ;

#if	CF_DEBUGS
	debugprintf("progentry_expand: arg0 program=>%w<\n",
		cp,cl) ;
#endif

	    rs = uc_mallocstrw(cp,cl,&pep->program) ;

	    if (rs < 0)
	        goto bad7 ;

#ifdef	MALLOCLOG
		malloclog_alloc(pep->program,-1,"progentry_expand:program2") ;
#endif

	}

/* are we OK for a go ? */

	if (pep->program == NULL)
	    goto bad7 ;

/* set at least one program argument if we have none so far */

#if	CF_DEBUGS
	debugprintf("progentry_expand: srvargs fixup\n") ;
#endif

	rs = 0 ;
	if (pep->f.srvargs)
	    rs = vecstr_count(&pep->srvargs) ;

	if ((rs == 0) && (pep->program != NULL)) {

	    if ((cp = strbasename(pep->program)) != NULL) {

	        if (! pep->f.srvargs) {

	            rs = vecstr_start(&pep->srvargs,2,0) ;

		    if (rs >= 0)
	    		pep->f.srvargs = TRUE ;

		}

		if (pep->f.srvargs)
	            rs = vecstr_add(&pep->srvargs,cp,-1) ;

	    }

	} /* end if (setting 'argv[0]') */

/* we're out of here */

#if	CF_DEBUGS
	debugprintf("progentry_expand: exiting OK rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    goto goodret ;

/* bad things */
bad7:
	freeit(&pep->options) ;

bad6:
	freeit(&pep->groupname) ;

bad5:
	freeit(&pep->username) ;

bad4:
	if (pep->f.srvargs)
	    (void) vecstr_finish(&pep->srvargs) ;

	pep->f.srvargs = FALSE ;

bad3:
	if (pep->program != NULL)
	    freeit(&pep->program) ;

bad2:
	u_unlink(pep->efname) ;

bad1:
	u_unlink(pep->ofname) ;

bad0:
goodret:
	esap->interval = oldinterval ;
	esap->service = oldservice ;
	return rs ;
}
/* end subroutine (progentry_expand) */



/* INTERNAL SUBROUTINES */



/* expand out one program string entry */
static int progentry_process(pep,inbuf,esap,outbuf,outlen)
PROGENTRY	*pep ;
char		inbuf[] ;		/* input string */
PROGENTRY_ARGS	*esap ;			/* key-type arguments */
char		outbuf[] ;		/* output buffer */
int		outlen ;		/* output buffer length */
{
	int	vlen, elen ;

	char	vbuf[BUFLEN + 1] ;
	char	ebuf[BUFLEN + 1] ;
	char	*ibp ;


#if	CF_DEBUGS
	debugprintf("progentry_process: entered, outlen=%d\n",outlen) ;
	debugprintf("progentry_process: inbuf=>%s<\n",inbuf) ;
#endif

	if (inbuf == NULL)
	    return SR_FAULT ;

	ibp = inbuf ;
	if (pep->ssp != NULL) {

	    vlen = varsub_buf(pep->ssp,inbuf,-1,vbuf,BUFLEN) ;

	    if (vlen < 0)
	        return SR_TOOBIG ;

	    ibp = vbuf ;

	} else
	    vlen = strlen(ibp) ;

#if	CF_DEBUGS
	debugprintf("progentry_process: vlen=%d\n",vlen) ;
#endif

	elen = expand(esap,ibp,vlen,outbuf,outlen) ;

	if (elen < 0)
	    return SR_TOOBIG ;

#if	CF_DEBUGS
	debugprintf("progentry_process: elen=%d\n",elen) ;
#endif

	return elen ;
}
/* end subroutine (progentry_process) */


/* expand out a program argument with the substitution parameters */

/*
#	The following substitutions are made on command strings :

#		%V	Directory Watcher version string
#		%R	program root
#		%N	machine nodename
#		%D	machine DNS domain
#		%H	machine hostname
#		%U	invoking username
#		%G	invoking groupname

#		%s	service
#		%i	interval (decimal seconds)
#
*/

static int expand(esap,buf,len,rbuf,rlen)
PROGENTRY_ARGS	*esap ;
char		buf[], rbuf[] ;
int		len, rlen ;
{
	int	elen, sl ;

	char	hostbuf[MAXHOSTNAMELEN + 1] ;
	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;


#if	CF_DEBUGS2
	debugprintf("progentry/expand: entered >%W<\n",buf,len) ;
	debugprintf("progentry/expand: rbuflen=%d\n",rlen) ;
#endif

#if	CF_DEBUGS2

	if (buf == NULL)
	    debugprintf("progentry/expand: buf is NULL\n") ;

	if (rbuf == NULL)
	    debugprintf("progentry/expand: rbuf is NULL\n") ;

#endif /* CF_DEBUGS2 */

	rbuf[0] = '\0' ;
	if (len == 0)
	    return 0 ;

	if (len < 0)
	    len = strlen(buf) ;

#if	CF_DEBUGS2
	debugprintf("progentry/expand: before while\n") ;
#endif

	elen = 0 ;
	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

#if	CF_DEBUGS2
	    debugprintf("progentry/expand: switching on >%c<\n",*bp) ;
#endif

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) return elen ;

	        sl = 0 ;
	        switch ((int) *bp) {

	        case 'V':
	            cp = esap->version ;
	            sl = strlen(cp) ;

	            break ;

	        case 'R':
	            cp = esap->programroot ;
	            sl = strlen(cp) ;

	            break ;

	        case 'N':
	            cp = esap->nodename ;
	            sl = strlen(cp) ;

	            break ;

	        case 'D':
	            cp = esap->domainname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'H':
	            sl = -1 ;
	            if (esap->hostname == NULL) {

	                cp = hostbuf ;
	                sl = snsds(hostbuf,MAXHOSTNAMELEN,
	                    esap->nodename,esap->domainname) ;

	            } else
	                cp = esap->hostname ;

	            if (sl < 0)
	                sl = strlen(cp) ;

	            break ;

	        case 'U':
	            cp = esap->username ;
	            sl = strlen(cp) ;

	            break ;

	        case 'G':
	            cp = esap->groupname ;
	            sl = strlen(cp) ;

	            break ;

	        case 's':
	            cp = esap->service ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'i':
		    if (esap->interval != NULL) {

	            cp = esap->interval ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

		    } else {

			cp = "1" ;
			sl = 1 ;

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

#if	CF_DEBUGS2
	    debugprintf("progentry/expand: bottom while\n") ;
#endif

	} /* end while */

#if	CF_DEBUGS2
	debugprintf("progentry/expand: normal exit >%W<\n",rbuf,elen) ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */


static void freeit(pp)
char	**pp ;
{


	if (*pp != NULL) {

	    free(*pp) ;

#ifdef	MALLOCLOG
	malloclog_free(*pp,"progentry/freeit") ;
#endif

	    *pp = NULL ;
	}
}
/* end subroutine (freeit) */


/* process an argument list */
static int processargs(alp,args)
vecstr		*alp ;
char		args[] ;
{
	FIELD	fsb ;

	int	rs = SR_OK ;
	int	i = 0 ;

	uchar	terms[32] ;


#if	CF_DEBUGS
	debugprintf("progentry/processargs: entered> %s\n",args) ;
#endif

	if ((args == NULL) || (args[0] == '\0')) goto ret ;

	fieldterms(terms,0," \t") ;

	if ((rs = field_start(&fsb,args,-1)) >= 0) {
	    const int	flen = BUFLEN ;
	    int		fl ;
	    char	fbuf[BUFLEN + 1] ;

	    while ((fl = field_sharg(&fsb,terms,fbuf,flen)) > 0) {

	        i += 1 ;
	        rs = vecstr_add(alp,fbuf,fl) ;

	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

ret0:

#if	CF_DEBUGS
	debugprintf("progentry/processargs: ret rs=%d i=%u\n",i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (processargs) */


/* make our little files for input and output of the server programs */
static int mkfile(tmpdir,in,outbuf)
const char	tmpdir[], in[] ;
char		outbuf[] ;
{
	SBUF	b ;

	int	rs, len ;

	char	template[MAXPATHLEN + 2] ;


#if	CF_DEBUGS2
	debugprintf("progentry/mkfile: entered, in=%s\n",in) ;
#endif

	rs = sbuf_init(&b,template,MAXPATHLEN) ;

	if (rs < 0)
		goto ret0 ;

	sbuf_strw(&b,tmpdir,-1) ;

	sbuf_char(&b,'/') ;

	sbuf_strw(&b,"pcspoll",-1) ;

#if	CF_DEBUGS2
	rs = sbuf_getlen(&b) ;

	debugprintf("progentry/mkfile: sbuf_getlen() 0 rs=%d\n",
	    rs) ;
#endif

	sbuf_strw(&b,in,7) ;

	len = MAX((7 - strlen(in)),0) ;

#if	CF_DEBUGS2
	rs = sbuf_getlen(&b) ;

	debugprintf("progentry/mkfile: len=%d sbuf_getlen() 1 rs=%d\n",
	    len,rs) ;
#endif

	sbuf_strw(&b,xes,len) ;

	rs = sbuf_getlen(&b) ;

#if	CF_DEBUGS2
	debugprintf("progentry/mkfile: sbuf_getlen() 2 rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	rs = mktmpfile(outbuf,0600,template) ;

#if	CF_DEBUGS2
	debugprintf("progentry/mkfile: mktmpfile() rs=%d\n",rs) ;
#endif

	sbuf_free(&b) ;

ret0:
	return rs ;
}
/* end subroutine (mkfile) */



