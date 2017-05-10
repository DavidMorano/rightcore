/* progentry */

/* build up a program entry piece-meal as it were */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGS2	0		/* more */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This little object is used to create a program entry and to populate
        aspects of it with different operations on the object. This object is
        used in "server" types of programs. This object is usually created from
        elements taken from the parsing of a server file.


******************************************************************************/


#define	PROGENTRY_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<field.h>
#include	<sbuf.h>
#include	<svcfile.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"progentry.h"
#include	"svckey.h"


/* local defines */

#define	PROGENTRY_MAGIC	0x76452376

#define	STEBUFLEN	(2 * MAXPATHLEN)

#undef	OUTBUFLEN
#define	OUTBUFLEN	(10 * MAXPATHLEN)

#define	FBUFLEN		(2 * MAXPATHLEN)


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* local structures */


/* forward references */

static int	progentry_process(PROGENTRY *,const char *,PROGENTRY_ARGS *,
			char *,int) ;
static int	expand(PROGENTRY_ARGS *,const char *,int,char *,int) ;
static int	vecstr_processargs(vecstr *,char *) ;
static int	mkfile(const char *,const char *,char *) ;

static void	freeit(char **) ;


/* external variables */


/* local variables */

static const char	xes[] = "XXXXXXXXXXXXXX" ;


/* exported subroutines */


int progentry_init(pep,ssp,sep,esap)
PROGENTRY	*pep ;
varsub		*ssp ;
SVCFILE_ENT	*sep ;
PROGENTRY_ARGS	*esap ;
{
	struct svckey	sk ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	sl ;

	char	outbuf[OUTBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("progentry_init: entered OUTBUFLEN=%d\n",OUTBUFLEN) ;
#endif

	if (pep == NULL)
	    return SR_FAULT ;

	if (sep == NULL)
	    return SR_FAULT ;

#ifdef	OPTIONAL
	memset(pep,0,sizeof(PROGENTRY)) ;
#endif

	pep->magic = 0 ;
	pep->program = NULL ;
	pep->username = NULL ;
	pep->groupname = NULL ;
	pep->options = NULL ;
	pep->access = NULL ;
	pep->atime = 0 ;
	pep->pid = 0 ;
	pep->interval = 0 ;

	memset(&pep->f,0,sizeof(struct progentry_flags)) ;

	pep->name[0] = '\0' ;
	pep->ofname[0] = '\0' ;
	pep->efname[0] = '\0' ;
	pep->jobid[0] = '\0' ;

/* store what we will need later! */

	pep->ssp = ssp ;
	pep->atime = esap->daytime ;

/* process the service-entry in a way that makes it how we want it */

	svckey_load(&sk,sep) ;

/* load the service name */

	strwcpy(pep->name,sk.svc,MAXNAMELEN) ;

/* process the access field */

	if ((rs >= 0) && (sk.acc != NULL)) {

	    rs = progentry_process(pep,sk.acc,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad0 ;

	    rs = uc_mallocstrw(outbuf,sl,&pep->access) ;
	    if (rs < 0)
	        goto bad0 ;

	} /* end if (access field) */

/* process the interval field */

	if ((rs >= 0) && (sk.interval != NULL)) {

	    rs = progentry_process(pep,sk.interval,esap,outbuf,OUTBUFLEN) ;
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
		uc_free(pep->access) ;
		pep->access = NULL ;
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

	return (pep->access != NULL) ? SR_OK : SR_EMPTY ;
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
	    vecstr_finish(&pep->srvargs) ;

	if (pep->program != NULL) {
	    uc_free(pep->program) ;
	    pep->program = NULL ;
	}

	if (pep->username != NULL) {
	    uc_free(pep->username) ;
	    pep->username = NULL ;
	}

	if (pep->groupname != NULL) {
	    uc_free(pep->groupname) ;
	    pep->groupname = NULL ;
	}

	if (pep->options != NULL) {
	    uc_free(pep->options) ;
	    pep->options = NULL ;
	}

	if (pep->access != NULL) {
	    uc_free(pep->access) ;
	    pep->access = NULL ;
	}

	pep->program = NULL ;
	pep->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (progentry_free) */


/* expand out the server entry */
int progentry_expand(pep,sep,esap)
PROGENTRY	*pep ;
SVCFILE_ENT	*sep ;
PROGENTRY_ARGS	*esap ;
{
	struct svckey	sk ;

	varsub	*ssp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	sl, cl ;
	int	opts ;

	const char	*oldservice, *oldinterval ;

	char	outbuf[OUTBUFLEN + 1] ;
	char	*argz ;
	char	*tmpdname ;
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

	svckey_load(&sk,sep) ;

	esap->service = sk.svc ;
	esap->interval = sk.interval ;

/* load the job ID if one was supplied */

	if (esap->logid != NULL)
	    strwcpy(pep->jobid,esap->logid,PROGENTRY_IDLEN) ;

/* did they supply a TMPDIR? */

	tmpdname = (esap->tmpdir != NULL) ? esap->tmpdir : PROGENTRY_TMPDIR ;

/* make some temporary files for program file input and output */

	rs = mkfile(tmpdname,"o",pep->ofname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = mkfile(tmpdname,"e",pep->efname) ;
	if (rs < 0)
	    goto bad1 ;

/* process them */

	if (sk.p != NULL) {

	    rs = progentry_process(pep,sk.p,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad2 ;

	    cl = sfshrink(outbuf,sl,&cp) ;

	    rs = uc_mallocstrw(cp,cl,&pep->program) ;
	    if (rs < 0)
	        goto bad2 ;

	} /* end if (program path) */

	argz = NULL ;
	if (sk.a != NULL) {

	    rs = progentry_process(pep,sk.a,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad3 ;

	    opts = VECSTR_OCOMPACT ;
	    rs = vecstr_start(&pep->srvargs,6,opts) ;
	    if (rs < 0)
	        goto bad3 ;

	    pep->f.srvargs = TRUE ;
	    if ((rs = vecstr_processargs(&pep->srvargs,outbuf)) > 0) {

	        rs1 = vecstr_get(&pep->srvargs,0,&argz) ;
		if (rs1 < 0)
		    argz = NULL ;

	    } /* end if */

	} /* end if (program arguments) */

	if (sk.u != NULL) {

	    rs = progentry_process(pep,sk.u,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad4 ;

	    rs = uc_mallocstrw(outbuf,sl,&pep->username) ;
	    if (rs < 0)
	        goto bad4 ;

	} /* end if (username field) */

	if (sk.g != NULL) {

	    rs = progentry_process(pep,sk.g,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad5 ;

	    rs = uc_mallocstrw(outbuf,sl,&pep->groupname) ;
	    if (rs < 0)
	        goto bad5 ;

	}

	if (sk.opts != NULL) {

	    rs = progentry_process(pep,sk.opts,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad6 ;

	    rs = uc_mallocstrw(outbuf,sl,&pep->options) ;
	    if (rs < 0)
	        goto bad6 ;

	}

/* OK, perform some fixups */


	if ((pep->program == NULL) && (argz != NULL)) {

	    cl = sfshrink(argz,-1,&cp) ;

	    rs = uc_mallocstrw(cp,cl,&pep->program) ;
	    if (rs < 0)
	        goto bad7 ;

	}

/* are we OK for a go? */

	if (pep->program == NULL)
	    goto bad7 ;

/* set at least one program argument if we have none so far */

	rs = SR_OK ;
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

	if (rs >= 0)
	    goto retok ;

/* bad things */
bad7:
	freeit(&pep->options) ;

bad6:
	freeit(&pep->groupname) ;

bad5:
	freeit(&pep->username) ;

bad4:
	if (pep->f.srvargs)
	    vecstr_finish(&pep->srvargs) ;

	pep->f.srvargs = FALSE ;

bad3:
	if (pep->program != NULL)
	    freeit(&pep->program) ;

bad2:
	u_unlink(pep->efname) ;

bad1:
	u_unlink(pep->ofname) ;

bad0:
retok:
	esap->interval = oldinterval ;
	esap->service = oldservice ;
	return rs ;
}
/* end subroutine (progentry_expand) */



/* INTERNAL SUBROUTINES */



/* expand out one program string entry */
static int progentry_process(pep,inbuf,esap,outbuf,outlen)
PROGENTRY	*pep ;
const char	inbuf[] ;		/* input string */
PROGENTRY_ARGS	*esap ;			/* key-type arguments */
char		outbuf[] ;		/* output buffer */
int		outlen ;		/* output buffer length */
{
	int	vlen, elen ;

	const char	*ibp ;
	char	vbuf[OUTBUFLEN + 1] ;
	char	ebuf[OUTBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("progentry_process: entered, outlen=%d\n",outlen) ;
	debugprintf("progentry_process: inbuf=>%s<\n",inbuf) ;
#endif

	if (inbuf == NULL)
	    return SR_FAULT ;

	ibp = inbuf ;
	if (pep->ssp != NULL) {

	    vlen = varsub_buf(pep->ssp,inbuf,-1,vbuf,OUTBUFLEN) ;
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
const char	buf[] ;
int		len ;
char		rbuf[] ;
int		rlen ;
{
	int	elen, sl ;

	const char	*bp = buf ;
	const char	*cp ;

	char	hostbuf[MAXHOSTNAMELEN + 1] ;
	char	*rbp = rbuf ;


#if	CF_DEBUGS2
	debugprintf("progentry/expand: entered >%w<\n",buf,len) ;
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
	        if (len == 0) 
			return elen ;

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
	debugprintf("progentry/expand: normal exit >%w<\n",rbuf,elen) ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */


static void freeit(pp)
char	**pp ;
{


	if (*pp != NULL) {
	    uc_free(*pp) ;
	    *pp = NULL ;
	}

}
/* end subroutine (freeit) */


/* process an argument list */
static int vecstr_processargs(alp,args)
vecstr		*alp ;
char		args[] ;
{
	FIELD	fsb ;

	int	rs = SR_OK ;
	int	i = 0 ;

	uchar	terms[32] ;


	if (alp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("progentry/processargs: args=>%s<\n",args) ;
#endif

	if ((args == NULL) || (args[0] == '\0')) goto ret0 ;

	fieldterms(terms,FALSE," \t") ;

	if ((rs = field_init(&fsb,args,-1)) >= 0) {
	    const int	flen = FBUFLEN ;
	    int		fl ;
	    char	fbuf[FBUFLEN + 1] ;

	    while ((fl = field_sharg(&fsb,terms,fbuf,flen)) > 0) {

	        i += 1 ;
	        rs = vecstr_add(alp,fbuf,fl) ;

	        if (rs < 0) break ;
	    } /* end while */

	    field_free(&fsb) ;
	} /* end if (field) */

ret0:

#if	CF_DEBUGS
	debugprintf("progentry/processargs: ret rs=%d i=%u\n",i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (processargs) */


/* make our little files for input and output of the server programs */
static int mkfile(tmpdname,in,outbuf)
const char	tmpdname[] ;
const char	in[] ;
char		outbuf[] ;
{
	SBUF	b ;

	int	rs ;
	int	len = 0 ;

	char	template[MAXPATHLEN + 2] ;


	if ((rs = sbuf_start(&b,template,MAXPATHLEN)) >= 0) {
	int	nx ;

	sbuf_strw(&b,tmpdname,-1) ;

	sbuf_char(&b,'/') ;

	sbuf_strw(&b,"pcspoll",-1) ;

	sbuf_strw(&b,in,7) ;

	nx = MAX((7 - strlen(in)),0) ;
	sbuf_strw(&b,xes,nx) ;

	len = sbuf_finish(&b) ;
	if (rs >= 0) rs = len ;
	} /* end if (sbuf) ;

	if (rs >= 0)
	    rs = mktmpfile(outbuf,0600,template) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkfile) */


