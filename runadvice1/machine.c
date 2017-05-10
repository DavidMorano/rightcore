/* machine */

/* subroutines to maintain status on machines */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine will allocate structures for maintaining 	
	information on a remote machine.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<rpc/rpc.h>
#include	<rpcsvc/rstat.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include	<math.h>

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<vecitem.h>
#include	<varsub.h>
#include	<field.h>
#include	<logfile.h>
#include	<paramopt.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#ifdef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr() ;
extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;
extern int	cfdouble() ;

#ifdef	BSD
extern int	i_multiply() ;
#endif

extern void	vsaprint() ;


/* forward subroutines */

static int	machineupdateone() ;

static int	lastline() ;


/* external subroutines */

extern struct global	g ;


/* local static data */

static int	f_fieldterms = FALSE ;

static uchar	terms[32] ;


/* exported subroutines */


int machineinit(mhp,n)
vecitem	*mhp ;
int		n ;
{
	int	i ;


	if (! f_fieldterms) {

	    f_fieldterms = TRUE ;
	    fieldterms(terms,FALSE," \t\v\r\n\f") ;

#if	CF_DEBUGS
	    for (i = 0 ; i < 256 ; i += 1) {

	        if (BATST(terms,i)) 
			debugprintf("machineupdateone: character=%d\n",i) ;

	}
#endif

	} /* end if */

	return vecitem_start(mhp,n,0) ;
}
/* end subroutine (machineinit) */


/* add a machine entry to the machine list */
int machineadd(mhp,hostname,loadaverage)
vecitem	*mhp ;
char		hostname[] ;
double		loadaverage ;
{
	struct ustatstime	*rstatp ;

	struct machine		m ;

	int		i, rs, len ;

	char		*cp ;


/* allocate pieces for use in machine structures, et cetera */

	if ((rstatp = 
	    (struct ustatstime *) malloc(sizeof(struct ustatstime))) == NULL)
	    goto badalloc2 ;

	len = strlen(hostname) ;

#if	CF_DEBUGS
	debugprintf("machineadd: hostname=%s\n",
	    hostname) ;
#endif

	if (len > MAXPATHLEN) 
		goto badalloc2 ;

	if ((cp = (char *) malloc(len + 1)) == NULL) 
		goto badalloc3 ;

	m.hostname = cp ;
	strcpy(m.hostname,hostname) ;

#if	CF_DEBUGS
	debugprintf("machineadd: m.hostname=%s\n",
	    m.hostname) ;
#endif

	m.f.rsh = FALSE ;
	m.f.sarfailed = FALSE ;
	m.f.checkeddisk = FALSE ;
	m.f.havedisk = FALSE ;

	m.sp = rstatp ;
	m.minloadaverage = -1000 ;
	m.maxloadaverage = rint(loadaverage * ((double) FSCALE)) ;

#if	CF_DEBUGS
	debugprintf("machineadd: f=%6.3f fscale=%d maxloadaverage=%ld\n",
	    loadaverage,FSCALE,m.maxloadaverage) ;
#endif

/* update this machine with its latest status */

	machineupdateone(&m) ;

/* link it into the list */

	rs = vecitem_add(mhp,&m,sizeof(struct machine)) ;

	if (rs < 0)
	    goto badalloc4 ;

	i = rs ;

#if	CF_DEBUGS
	debugprintf("machineadd: i=%d hostname=%s\n",
	    rs,((struct machine **) mhp->va)[i]->hostname) ;
#endif

#if	CF_DEBUGS
	debugprintf("machineadd: exiting i=%d hostname=%s f_rstat=%d f_rsh=%d\n",
	    i,
	    ((struct machine **) mhp->va)[i]->hostname,
	    ((struct machine **) mhp->va)[i]->f.rstat,
	    ((struct machine **) mhp->va)[i]->f.rsh) ;
#endif

/* end of getting remote machine status */

	return i ;

badalloc4:
	free(m.hostname) ;

badalloc3:
	free(rstatp) ;

badalloc2:
	return BAD ;
}
/* end subroutine (machineadd) */


/* update the status of the machines */
int machineupdate(mhp)
machinehead	*mhp ;
{
	struct ustatstime	*rstatp ;

	struct machine		*mp ;

	int		n, i, rs, len, l, ll ;

	char		*cp ;


	n = 0 ;
	for (i = 0 ; vecitem_get(mhp,i,&mp) >= 0 ; i += 1) {

	    if (mp == NULL) continue ;

	    if (machineupdateone(mp) >= 0)
	        n += 1 ;

	} /* end for */

	return n ;
}
/* end subroutine (machineupdate) */


/* find the least used machine so far */

#define	BEST_DEFSPARE		-10000
#define	BEST_MINFREEPAGES	150
#define	BEST_MINFREETOTAL	1500

int machinebest(mhp,mpp,loadsparep,loadaveragep)
machinehead	*mhp ;
struct machine	**mpp ;
long		*loadsparep ;
long		*loadaveragep ;
{
	struct machine		*mp ;

	time_t		daytime ;

	long		bestspare = BEST_DEFSPARE ;
	long		loadspare = BEST_DEFSPARE ;
	long		loadaverage ;

	int		i, rs ;
	int		freetotal ;
	int		f_freetotal = FALSE ;


	*loadsparep = BEST_DEFSPARE ;
	for (i = 0 ; vecitem_get(mhp,i,&mp) >= 0 ; i += 1) {

	    if (mp == NULL) continue ;

/* calculate the spare load */

	    if (mp->f.rstat) {

#if	CF_DEBUGS
	        if (g.debuglevel > 2)
		    debugprintf("machinebest: h=%s maxloadaverage=%ld load=%ld\n",
	            mp->hostname,
	            mp->maxloadaverage, mp->sp->avenrun[0]) ;
#endif

	        loadaverage = mp->sp->avenrun[0] ;

/* do we have a timed bias outstanding? */

	        if (mp->timeout != 0) {

		    (void) time(&daytime) ;

	            if (daytime < mp->timeout) {

	                if (loadaverage < mp->minloadaverage)
	                    loadaverage = mp->minloadaverage ;

	            } else
	                mp->timeout = 0 ;

	        }

/* OK, get the spare load given the (possibly corrected) loadaverage */

	        loadspare = mp->maxloadaverage - loadaverage ;

#if	CF_DEBUGS
	        if (g.debuglevel > 2)
		    debugprintf("machinebest: 1 h=%s loadspare=%ld int=%d\n",
	            mp->hostname,loadspare,(int) loadspare) ;
#endif

	    }

/* calculate the total free pages on the machine */

	    if (mp->f.freepages && mp->f.freeswap) {

#if	CF_DEBUGS
	        if (g.debuglevel > 2)
		    debugprintf("machinebest: freepages=%d freeswap=%d\n",
	            mp->freepages,mp->freeswap) ;
#endif

	        freetotal = mp->freepages + mp->freeswap ;
	        if (freetotal < BEST_MINFREETOTAL)
	            loadspare = BEST_DEFSPARE ;

#if	CF_DEBUGS
	        if (g.debuglevel > 2)
		    debugprintf("machinebest: 2 h=%s loadspare=%ld\n",
	            mp->hostname,loadspare) ;
#endif

	        if (freetotal < 6000)
	            loadspare -= (+(256 * mp->freepages) / 6000) ;

#if	CF_DEBUGS
	        if (g.debuglevel > 2)
		    debugprintf("machinebest: 3 h=%s loadspare=%ld\n",
	            mp->hostname,loadspare) ;
#endif

	    }

#if	CF_DEBUGS
	    if (g.debuglevel > 2)
		debugprintf("machinebest: 4 h=%s loadspare=%ld\n",
	        mp->hostname,loadspare) ;
#endif

/* weigh it by the number of free pages left */

#ifdef	BSD
	    if (mp->f.freepages && (mp->freepages < 300))
	        loadspare -= (i_multiply(64 * mp->freepages) / 300) ;
#else
	    if (mp->f.freepages && (mp->freepages < 300))
	        loadspare -= (+(64 * mp->freepages) / 300) ;
#endif

#if	CF_DEBUGS
	    if (g.debuglevel > 2)
		debugprintf("machinebest: 5 h=%s loadspare=%ld\n",
	        mp->hostname,loadspare) ;
#endif

/* is this one the best so far? */

	    if (loadspare > bestspare) {

	        *mpp = mp ;
	        *loadsparep = loadspare ;
	        *loadaveragep = loadaverage ;
	        bestspare = loadspare ;

	    }

	} /* end for */

	return (bestspare > BEST_DEFSPARE) ? OK : BAD ;
}
/* end subroutine (machinebest) */


int machinemin(mhp,hostname,min,ttl)
machinehead	*mhp ;
char		hostname[] ;
int		min ;
int		ttl ;
{
	struct machine	*mp ;

	time_t		daytime ;

	int		i, rs ;


/* get the machine entry pointer for this host */

	for (i = 0 ; (rs = vecitem_get(mhp,i,&mp)) >= 0 ; i += 1) {

	    if (mp == NULL) continue ;

	    if (strcmp(mp->hostname,hostname) == 0) break ;

	} /* end for */

	if (rs < 0) return BAD ;

/* OK, get current time and let's do it */

	mp->timeout = time(NULL) + ttl ;
	mp->minloadaverage = min ;
	return OK ;
}
/* end subroutine (machinebias) */


/* local subroutines */


/* update the status of one machine */
static int machineupdateone(mp)
struct machine	*mp ;
{
	struct ustatstime	*rstatp ;

	bfile		infile ;
	bfile		outfile ;
	bfile		*fpa[3] ;

	FIELD	fsb ;

	pid_t	pid ;

	int	rs = SR_OK ;
	int	i, len, l, ll ;
	int	childstat ;
	int	lb ;

	char	cmd[(MAXPATHLEN * 2) + 1] ;
	char	linebuf[2][LINEBUFLEN + 1], *lbp ;
	char	*cp ;


	if (mp == NULL) return BAD ;

#if	CF_DEBUGS
	debugprintf("machineupdateone: host=%s f_fieldterms=%d\n",
	    mp->hostname,f_fieldterms) ;
#endif

/* can we execute remotely on it at this time? */

	if (! mp->f.sarfailed) {

/* continue to get SAR information */

	    mp->f.freepages = FALSE ;
	    mp->f.freeswap = FALSE ;
	    fpa[0] = &infile ;
	    fpa[1] = &outfile ;
	    fpa[2] = NULL ;
	    sprintf(cmd,"/usr/bin/sar -r 1 1 2> /dev/null") ;

#if	CF_DEBUGS
	    debugprintf("machineupdateone: about to spawn 'sar'\n%s\n",cmd) ;
#endif

	    if ((pid = bopenrcmd(fpa,mp->hostname,cmd)) < 0) {
	        mp->f.sarfailed = TRUE ;

	    } else
	        bclose(fpa[0]) ;

#if	CF_DEBUGS
	    debugprintf("machineupdateone: spawn done, pid=%d\n",pid) ;
#endif

	}

/* can we get remote status at this time? */

	mp->f.rstat = FALSE ;
	if ((rs = rstat(mp->hostname,mp->sp)) == RPC_SUCCESS)
	    mp->f.rstat = TRUE ;

#if	CF_DEBUGS
	debugprintf("machineupdateone: rstat=%d\n",mp->f.rstat) ;
#endif

/* does the remote system have any local disk? */

	if (! mp->f.checkeddisk) {

	    mp->f.havedisk = FALSE ;
	    if ((rs = havedisk(mp->hostname)) != -1) {

	        mp->f.checkeddisk = TRUE ;
	        if (rs == 1)
			mp->f.havedisk = TRUE ;

	    }
	}

#if	CF_DEBUGS
	debugprintf("machineupdateone: havedisk=%d\n",mp->f.havedisk) ;
#endif

/* only execute if we are not in SAR fail mode */

	if (! mp->f.sarfailed) {

#if	CF_DEBUGS
	    debugprintf("machineupdateone: continuing with 'sar'\n") ;
#endif

/* assume a failure */

	    mp->f.sarfailed = TRUE ;

/* read all of the lines, saving the last (non-blank) line reliably */

	    ll = -1 ;
	    lb = 0 ;
	    lbp = linebuf[lb] ;
	    while ((l = breadline(fpa[1],lbp,LINEBUFLEN)) > 0) {

#if	CF_DEBUGS
	        debugprintf("machineupdateone: line\n%W",lbp,l) ;
#endif

/* do not "turn" the buffer if we have a blank line */

	        if (sfshrink(lbp,l,&cp) > 0) {

	            ll = l ;
	            lb = (lb + 1) % 2 ;
	            lbp = linebuf[lb] ;

	        }

	    } /* end while */

/* if we have something in the last line, process for free pages */

	    if (ll > 0) {

#if	CF_DEBUGS
	        debugprintf(
	            "machineupdateone: got a last line, len=%d\n",
	            ll) ;
#endif

/* switch to the buffer with the last line */

	        lb = (lb + 1) % 2 ;
	        lbp = linebuf[lb] ;

/* OK, pick off the fields in the line that we want */

	        if ((ll > 0) && (lbp[ll - 1] == '\n')) 
			ll -= 1 ;

#if	CF_DEBUGS
	        debugprintf(
	            "machineupdateone: line>\n%W\n",
	            lbp,ll) ;
#endif

	        field_start(&fsb,lbp,ll) ;

/* skip the first field */

	        field_get(&fsb,terms) ;

/* this second field should be the free page count */

	        l = field_get(&fsb,terms) ;

#if	CF_DEBUGS
	        debugprintf(
	            "machineupdateone: len=%d field=%W\n",
	            l,fsb.fp,l) ;
#endif

	        if ((l > 0) && 
	            ((rs = cfdeci(fsb.fp,l,&mp->freepages)) >= 0)) {

#if	CF_DEBUGS
	            debugprintf(
	                "machineupdateone: got some free pages %d\n",
	                mp->freepages) ;
#endif

	            mp->f.sarfailed = FALSE ;
	            mp->f.rsh = TRUE ;
	            mp->f.freepages = TRUE ;

	        }

/* this third field is the total swap pages available */

	        l = field_get(&fsb,terms) ;

	        if ((l > 0) && 
	            ((rs = cfdeci(fsb.fp,l,&mp->freeswap)) >= 0)) {

#if	CF_DEBUGS
	            debugprintf(
	                "machineupdateone: got some free swap %d\n",
	                mp->freeswap) ;
#endif

	            mp->f.freeswap = TRUE ;

	        }

		field_finish(&fsb) ;

#if	CF_DEBUGS
	        debugprintf("machineupdateone: cfdec rs=%d f_rsh=%d\n",
	            rs,mp->f.rsh) ;
#endif

	    } /* end if (got some output) */

#if	CF_DEBUGS
	    debugprintf("machineupdateone: out of last line %d\n",ll) ;
#endif

	    bclose(fpa[1]) ;

	    waitpid(pid,&childstat,WUNTRACED) ;

	} /* end if (processing remote command) */

#if	CF_DEBUGS
	debugprintf("machineupdateone: about to exit rstat=%d rsh=%d\n",
	    mp->f.rstat,mp->f.rsh) ;
#endif

/* end of getting remote machine status */

	return (mp->f.rstat) ? OK : BAD ;
}
/* end subroutine (machineupdateone) */


/* get the last line off of this file pointer before it goes EOF */
static int lastline(fp,buf,buflen)
bfile		*fp ;
char		buf[] ;
int		buflen ;
{
	int	l, ll, lb ;

	char	linebuf[2][LINEBUFLEN + 1], *lbp ;
	char	*cp ;


/* read all of the lines, saving the last (non-blank) line reliably */

	ll = -1 ;
	lb = 0 ;
	lbp = linebuf[lb] ;
	while ((l = breadline(fp,lbp,LINEBUFLEN)) > 0) {

#if	CF_DEBUGS
	    debugprintf("machineupdateone: line\n%W",lbp,l) ;
#endif

/* do not "turn" the buffer if we have a blank line */

	    if (sfshrink(lbp,l,&cp) > 0) {

	        ll = l ;
	        lb = (lb + 1) % 2 ;
	        lbp = linebuf[lb] ;

	    }

	} /* end while */

/* if we have something in the last line, process for free pages */

	if (ll > 0) {

#if	CF_DEBUGS
	    debugprintf("machineupdateone: got a last line %d\n",ll) ;
#endif

/* switch to the buffer with the last line */

	    lb = (lb + 1) % 2 ;
	    lbp = linebuf[lb] ;

	    if (ll > (buflen - 1)) ll = buflen - 1 ;

	    strwcpy(buf,lbp,ll) ;

	}

	return ll ;
}
/* end subroutine (lastline) */



