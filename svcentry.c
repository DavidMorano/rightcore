/* svcentry */

/* build up a program entry piece-meal as it were */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGS2	0		/* more */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object is used to create a program entry and to populate
        aspects of it with different operations on the object. This object is
        used in "server" types of programs. This object is usually created from
        elements taken from the parsing of a server file.


*******************************************************************************/


#define	SVCENTRY_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<field.h>
#include	<sbuf.h>
#include	<svcfile.h>
#include	<localmisc.h>

#include	"svcentry.h"
#include	"svckey.h"


/* local defines */

#define	SVCENTRY_MAGIC	0x76452376

#define	STEBUFLEN	(2 * MAXPATHLEN)

#undef	OUTBUFLEN
#define	OUTBUFLEN	(10 * MAXPATHLEN)

#define	FBUFLEN		(2 * MAXPATHLEN)


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	svcentry_process(SVCENTRY *,const char *,SVCENTRY_ARGS *,
			char *,int) ;
static int	svcentry_mkfile(SVCENTRY *,const char *,int) ;

static int	expand(SVCENTRY_ARGS *,const char *,int,char *,int) ;
static int	vecstr_procargs(vecstr *,char *) ;
static int	mkfile(char *,const char *,int) ;

static void	freeit(const char **) ;


/* external variables */


/* local variables */

static const char	xes[] = "XXXXXXXXXXXXXX" ;


/* exported subroutines */


int svcentry_start(pep,ssp,sep,esap)
SVCENTRY	*pep ;
varsub		*ssp ;
SVCFILE_ENT	*sep ;
SVCENTRY_ARGS	*esap ;
{
	struct svckey	sk ;
	const int	outlen = OUTBUFLEN ;
	int		rs = SR_OK ;
	int		sl ;
	char		outbuf[OUTBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("svcentry_start: ent OUTBUFLEN=%d\n",outlen) ;
#endif

	if (pep == NULL) return SR_FAULT ;
	if (sep == NULL) return SR_FAULT ;

#ifdef	OPTIONAL
	memset(pep,0,sizeof(SVCENTRY)) ;
#endif

	pep->magic = 0 ;
	pep->pid = 0 ;
	pep->interval = 0 ;

	pep->program = NULL ;
	pep->username = NULL ;
	pep->groupname = NULL ;
	pep->options = NULL ;
	pep->access = NULL ;
	pep->efname = NULL ;
	pep->ofname = NULL ;

	memset(&pep->f,0,sizeof(struct svcentry_flags)) ;

	pep->name[0] = '\0' ;
	pep->jobid[0] = '\0' ;

/* store what we will need later! */

	pep->ssp = ssp ;
	pep->atime = esap->daytime ;	/* job arrival time */

/* process the service-entry in a way that makes it how we want it */

	svckey_load(&sk,sep) ;

/* load the service name */

	strwcpy(pep->name,sk.svc,MAXNAMELEN) ;

/* process the access field */

	if ((rs >= 0) && (sk.acc != NULL)) {

	    rs = svcentry_process(pep,sk.acc,esap,outbuf,outlen) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad0 ;

	    rs = uc_mallocstrw(outbuf,sl,&pep->access) ;
	    if (rs < 0)
	        goto bad0 ;

	} /* end if (access field) */

/* process the interval field */

	if ((rs >= 0) && (sk.interval != NULL)) {

	    rs = svcentry_process(pep,sk.interval,esap,outbuf,outlen) ;
	    if (rs < 0)
	        goto bad1 ;

/* convert the interval string to an integer */

	    rs = cfdecti(outbuf,rs,&pep->interval) ;

#if	CF_DEBUGS
	    debugprintf("svcentry_start: cfdecti() rs=%d v=%d\n",
	        rs,pep->interval) ;
#endif

	    pep->interval = (rs >= 0) ? pep->interval : -1 ;
	    rs = SR_OK ;

	} else {
	    pep->interval = -1 ;
	}

	if (rs >= 0)
	    pep->magic = SVCENTRY_MAGIC ;

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
/* end subroutine (svcentry_start) */


/* retrieve for caller the access groups for this entry */
int svcentry_getaccess(pep,rpp)
SVCENTRY	*pep ;
const char	**rpp ;
{

	if (pep == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL) *rpp = pep->access ;

	return (pep->access != NULL) ? SR_OK : SR_EMPTY ;
}
/* end subroutine (svcentry_getaccess) */


/* retrieve for caller the execution interval for this entry */
int svcentry_getinterval(pep,rp)
SVCENTRY	*pep ;
int		*rp ;
{

	if (pep == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	if (rp != NULL) *rp = pep->interval ;

	return SR_OK ;
}
/* end subroutine (svcentry_getinterval) */


int svcentry_getargs(pep,avp)
SVCENTRY	*pep ;
const char	***avp ;
{
	int		rs ;

	if (pep == NULL) return SR_FAULT ;
	if (avp == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_getvec(&pep->srvargs,avp) ;

	return rs ;
}
/* end subroutine (svcentry_getargs) */


/* free up this entry */
int svcentry_finish(pep)
SVCENTRY	*pep ;
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("svcentry_finish: ent\n") ;
#endif

	if (pep == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	if (pep->ofname != NULL) {
	    if (pep->ofname[0] != '\0') u_unlink(pep->ofname) ;
	    rs1 = uc_free(pep->ofname) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->ofname = NULL ;
	}

	if (pep->efname != NULL) {
	    if (pep->efname[0] != '\0') u_unlink(pep->efname) ;
	    rs1 = uc_free(pep->efname) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->efname = NULL ;
	}

	if (pep->f.srvargs) {
	    pep->f.srvargs = FALSE ;
	    rs1 = vecstr_finish(&pep->srvargs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->program != NULL) {
	    rs1 = uc_free(pep->program) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->program = NULL ;
	}

	if (pep->username != NULL) {
	    rs1 = uc_free(pep->username) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->username = NULL ;
	}

	if (pep->groupname != NULL) {
	    rs1 = uc_free(pep->groupname) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->groupname = NULL ;
	}

	if (pep->options != NULL) {
	    rs1 = uc_free(pep->options) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->options = NULL ;
	}

	if (pep->access != NULL) {
	    rs1 = uc_free(pep->access) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->access = NULL ;
	}

	pep->program = NULL ;
	pep->magic = 0 ;
	return rs ;
}
/* end subroutine (svcentry_finish) */


/* expand out the service entry */
int svcentry_expand(pep,sep,esap)
SVCENTRY	*pep ;
SVCFILE_ENT	*sep ;
SVCENTRY_ARGS	*esap ;
{
	struct svckey	sk ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl, cl ;
	int		opts ;
	const char	*oldservice, *oldinterval ;
	const char	*argz ;
	const char	*tmpdname ;
	const char	*ccp ;
	const char	*cp ;
	char		outbuf[OUTBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("svcentry_expand: ent\n") ;
	debugprintf("svcentry_expand: tmpdname=%s\n",esap->tmpdname) ;
#endif

	if (pep == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	oldservice = esap->service ;
	oldinterval = esap->interval ;

	svckey_load(&sk,sep) ;

	esap->service = sk.svc ;
	esap->interval = sk.interval ;

/* load the job ID if one was supplied */

	if (esap->jobid != NULL)
	    strwcpy(pep->jobid,esap->jobid,SVCENTRY_IDLEN) ;

/* did they supply a TMPDIR? */

	tmpdname = (esap->tmpdname != NULL) ? esap->tmpdname : SVCENTRY_TMPDIR ;

#if	CF_DEBUGS
	debugprintf("svcentry_expand: tmpfname=%s\n",tmpdname) ;
#endif

/* make some temporary files for program file input and output */

	rs = svcentry_mkfile(pep,tmpdname,'o') ;
	if (rs < 0) goto bad0 ;

	rs = svcentry_mkfile(pep,tmpdname,'e') ;
	if (rs < 0) goto bad1 ;

#if	CF_DEBUGS
	debugprintf("svcentry_expand: process rs=%d\n",rs) ;
#endif

/* process them */

	if (sk.p != NULL) {

	    rs = svcentry_process(pep,sk.p,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad2 ;

	    if ((cl = sfshrink(outbuf,sl,&cp)) > 0) {
	        if ((rs = uc_mallocstrw(cp,cl,&ccp)) >= 0) {
		    pep->program = ccp ;
		}
	    } else {
		rs = SR_INVALID ;
	    }
	    if (rs < 0) goto bad2 ;

	} /* end if (program path) */

	argz = NULL ;
	if (sk.a != NULL) {

	    rs = svcentry_process(pep,sk.a,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad3 ;

	    opts = VECSTR_OCOMPACT ;
	    rs = vecstr_start(&pep->srvargs,6,opts) ;
	    if (rs < 0)
	        goto bad3 ;

	    pep->f.srvargs = TRUE ;
	    if ((rs = vecstr_procargs(&pep->srvargs,outbuf)) > 0) {

	        rs1 = vecstr_get(&pep->srvargs,0,&argz) ;
	        if (rs1 < 0)
	            argz = NULL ;

	    } /* end if */

	} /* end if (program arguments) */

#if	CF_DEBUGS
	debugprintf("svcentry_expand: 4 rs=%d\n",rs) ;
#endif

	if (rs < 0) goto bad4 ;

	if (sk.u != NULL) {

	    rs = svcentry_process(pep,sk.u,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad4 ;

	    if ((rs = uc_mallocstrw(outbuf,sl,&ccp)) >= 0) {
		pep->username = ccp ;
	    }
	    if (rs < 0)
	        goto bad4 ;

	} /* end if (username field) */

	if (sk.g != NULL) {

	    rs = svcentry_process(pep,sk.g,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad5 ;

	    if ((rs = uc_mallocstrw(outbuf,sl,&ccp)) >= 0) {
		pep->groupname = ccp ;
	    }
	    if (rs < 0)
	        goto bad5 ;

	}

	if (sk.opts != NULL) {

	    rs = svcentry_process(pep,sk.opts,esap,outbuf,OUTBUFLEN) ;
	    sl = rs ;
	    if (rs < 0)
	        goto bad6 ;

	    if ((rs = uc_mallocstrw(outbuf,sl,&ccp)) >= 0) {
		pep->options = ccp ;
	    }
	    if (rs < 0)
	        goto bad6 ;

	}

#if	CF_DEBUGS
	debugprintf("svcentry_expand: fixups rs=%d\n",rs) ;
#endif

/* OK, perform some fixups */

	if ((pep->program == NULL) && (argz != NULL)) {

	    cl = sfshrink(argz,-1,&cp) ;

	    if ((rs = uc_mallocstrw(cp,cl,&ccp)) >= 0) {
		pep->program = ccp ;
	    }
	    if (rs < 0)
	        goto bad7 ;

	}

/* are we OK for a go? */

	if (pep->program == NULL)
	    goto bad7 ;

#if	CF_DEBUGS
	debugprintf("svcentry_expand: go rs=%d\n",rs) ;
#endif

/* set at least one program argument if we have none so far */

	rs = SR_OK ;
	if (pep->f.srvargs) {
	    rs = vecstr_count(&pep->srvargs) ;
	}

	if ((rs == 0) && (pep->program != NULL)) {

	    if (sfbasename(pep->program,-1,&cp) > 0) {

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
	if (pep->f.srvargs) {
	    pep->f.srvargs = FALSE ;
	    vecstr_finish(&pep->srvargs) ;
	}

bad3:
	if (pep->program != NULL)
	    freeit(&pep->program) ;

bad2:
	if ((pep->efname != NULL) && (pep->efname[0] != '\0')) {
	    u_unlink(pep->efname) ;
	}

bad1:
	if ((pep->ofname != NULL) && (pep->ofname[0] != '\0')) {
	    u_unlink(pep->ofname) ;
	}

bad0:
retok:
	esap->interval = oldinterval ;
	esap->service = oldservice ;

#if	CF_DEBUGS
	debugprintf("svcentry_expand: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (svcentry_expand) */


int svcentry_arrival(pep,tp)
SVCENTRY	*pep ;
time_t		*tp ;
{
	int	rs = SR_OK ;


	if (pep == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	if (tp != NULL) *tp = pep->atime ;

	return rs ;
}
/* end subroutine (svcentry_arrival) */


int svcentry_stime(pep,daytime)
SVCENTRY	*pep ;
time_t		daytime ;
{
	int		rs = SR_OK ;

	if (pep == NULL) return SR_FAULT ;

	if (pep->magic != SVCENTRY_MAGIC) return SR_NOTOPEN ;

	pep->stime = daytime ;
	return rs ;
}
/* end subroutine (svcentry_stime) */


/* private subroutines */


/* expand out one program string entry */
static int svcentry_process(pep,inbuf,esap,outbuf,outlen)
SVCENTRY	*pep ;
const char	inbuf[] ;		/* input string */
SVCENTRY_ARGS	*esap ;			/* key-type arguments */
char		outbuf[] ;		/* output buffer */
int		outlen ;		/* output buffer length */
{
	int		rs = SR_OK ;
	int		vlen ;
	int		elen = 0 ;
	const char	*ibp ;
	char		vbuf[OUTBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("svcentry_process: ent, outlen=%d\n",outlen) ;
	debugprintf("svcentry_process: inbuf=>%s<\n",inbuf) ;
#endif

	if (inbuf == NULL) return SR_FAULT ;

	ibp = inbuf ;
	if (pep->ssp != NULL) {

	    rs = varsub_expand(pep->ssp, vbuf,OUTBUFLEN, inbuf,-1) ;
	    vlen = rs ;
	    if (rs >= 0)
	        ibp = vbuf ;

	} else {
	    vlen = strlen(ibp) ;
	}

#if	CF_DEBUGS
	debugprintf("svcentry_process: vlen=%d\n",vlen) ;
#endif

	if (rs >= 0) {
	    elen = expand(esap,ibp,vlen,outbuf,outlen) ;
	    if (elen < 0)
	        rs = SR_TOOBIG ;
	}

#if	CF_DEBUGS
	debugprintf("svcentry_process: ret rs=%d elen=%d\n",rs,elen) ;
#endif

	return (rs >= 0) ? elen : rs ;
}
/* end subroutine (svcentry_process) */


static int svcentry_mkfile(SVCENTRY *pep,const char *tmpdname,int type)
{
	int		rs ;
	char		tfname[MAXPATHLEN+1] ;

	if ((rs = mkfile(tfname,tmpdname,type)) >= 0) {
	    int	fl = rs ;
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(tfname,fl,&cp)) >= 0) {
		switch (type) {
		case 'o':
		    pep->ofname = cp ;
		    break ;
		case 'e':
		    pep->efname = cp ;
		    break ;
		} /* end switch */
	    } /* end if (malloc-str) */
	} /* end if (mkfile) */

	return rs ;
}
/* end subroutine (svcentry_mkfile) */


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
SVCENTRY_ARGS	*esap ;
const char	buf[] ;
int		len ;
char		rbuf[] ;
int		rlen ;
{
	int		elen, sl ;
	int		ch ;
	const char	*bp = buf ;
	const char	*cp ;
	char		hostbuf[MAXHOSTNAMELEN + 1] ;
	char		*rbp = rbuf ;

#if	CF_DEBUGS2
	debugprintf("svcentry/expand: ent >%t<\n",buf,len) ;
	debugprintf("svcentry/expand: rbuflen=%d\n",rlen) ;
#endif

#if	CF_DEBUGS2
	if (buf == NULL)
	    debugprintf("svcentry/expand: buf is NULL\n") ;
	if (rbuf == NULL)
	    debugprintf("svcentry/expand: rbuf is NULL\n") ;
#endif /* CF_DEBUGS2 */

	rbuf[0] = '\0' ;
	if (len == 0)
	    return 0 ;

	if (len < 0)
	    len = strlen(buf) ;

#if	CF_DEBUGS2
	debugprintf("svcentry/expand: before while\n") ;
#endif

	elen = 0 ;
	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

#if	CF_DEBUGS2
	    debugprintf("svcentry/expand: switching on >%c<\n",*bp) ;
#endif

	    ch = MKCHAR(*bp) ;
	    switch (ch) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0)
	            return elen ;

	        sl = 0 ;
		ch = (*bp & 0xff) ;
	        switch (ch) {
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
			const int	hlen = MAXHOSTNAMELEN ;
			cchar		*nn = esap->nodename ;
			cchar		*dn = esap->domainname ;
	                cp = hostbuf ;
	                sl = snsds(hostbuf,hlen,nn,dn) ;
	            } else {
	                cp = esap->hostname ;
		    }
	            if (sl < 0) sl = strlen(cp) ;
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
		    break ;
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
		break ;

	    } /* end switch */

#if	CF_DEBUGS2
	    debugprintf("svcentry/expand: bottom while\n") ;
#endif

	} /* end while */

#if	CF_DEBUGS2
	debugprintf("svcentry/expand: normal exit >%t<\n",rbuf,elen) ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */


static void freeit(cchar **pp)
{
	if (*pp != NULL) {
	    uc_free(*pp) ;
	    *pp = NULL ;
	}
}
/* end subroutine (freeit) */


/* process an argument list */
static int vecstr_procargs(alp,abuf)
vecstr		*alp ;
char		abuf[] ;
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (alp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("svcentry/processargs: args=>%s<\n",abuf) ;
#endif

	if ((abuf != NULL) && (abuf[0] != '\0')) {
	    FIELD	fsb ;
	    const int	alen = strlen(abuf) ;
	    uchar	terms[32] ;

	    fieldterms(terms,FALSE," \t") ;

	    if ((rs = field_start(&fsb,abuf,alen)) >= 0) {
	        const int	flen = alen ;
	        char		*fbuf ;
		if ((rs = uc_malloc((flen+1),&fbuf)) >= 0) {
	            int		fl ;
	            while ((fl = field_sharg(&fsb,terms,fbuf,flen)) > 0) {
	                i += 1 ;
	                rs = vecstr_add(alp,fbuf,fl) ;
		        if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while */
		    uc_free(fbuf) ;
		} /* end if (m-a) */
	        field_finish(&fsb) ;
	    } /* end if (field) */

	} /* end if (non-empty arguments) */

#if	CF_DEBUGS
	debugprintf("svcentry/processargs: ret rs=%d i=%u\n",i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (processargs) */


/* make our little files for input and output of the server programs */
static int mkfile(outbuf,tmpdname,type)
char		outbuf[] ;
const char	tmpdname[] ;
int		type ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	char		template[MAXPATHLEN + 2] ;

	if ((rs = sbuf_start(&b,template,MAXPATHLEN)) >= 0) {

	    sbuf_strw(&b,tmpdname,-1) ;

	    sbuf_char(&b,'/') ;

	    {
		int	nu = 0 ;
	        nu += sbuf_strw(&b,"pcspoll",-1) ;
	        nu += sbuf_char(&b,type) ;
	        sbuf_strw(&b,xes,(14-nu)) ;
	    }

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	if (rs >= 0) {
#if	CF_DEBUGS
	debugprintf("svcentry/mkfile: template=%s\n",template) ;
#endif
	    rs = mktmpfile(outbuf,0600,template) ;
#if	CF_DEBUGS
	debugprintf("svcentry/mkfile: mkfile() rs=%d\n",rs) ;
#endif
	}

#if	CF_DEBUGS
	debugprintf("svcentry/mkfile: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mkfile) */


