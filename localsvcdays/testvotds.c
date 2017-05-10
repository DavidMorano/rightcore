/* testvotds */
/* lang=C++98 */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_PHASE1	1		/* phase-1 */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<stdio.h>
#include	<vsystem.h>
#include	<randomvar.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>
#include	"testvotds.h"
#include	"votds.h"

/* local defines */

#define	SHSIZEMULT	100
#define	SHSIZEMOD	256

#ifndef	FD_STDOUT
#define	FD_STDOUT	1
#endif

#define	SUBINFO		struct subinfo


/* external subroutines */

#ifdef	COMMENT
extern "C" int	main(int,const char **,const char **) ;
#endif

#if	CF_DEBUGS
extern "C" int	debugopen(const char *) ;
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	debugclose() ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" const char	*getourenv(const char **,const char *) ;


/* local structures */

struct subinfo {
	FILE		*ofp ;
	VOTDS		*vip ;
	char		*strp ;
	REQUESTS	as ;
	RANDOMVAR	rng ;
	time_t		dt ;
	int		shsize ;
	int		ps ;
} ;


/* forward references */

static int subinfo_start(SUBINFO *,time_t,FILE *,VOTDS *,char *,int,int) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_getrand(SUBINFO *) ;
#if	CF_PHASE1
static int subinfo_phase1(SUBINFO *) ;
#endif /* CF_PHASE1 */
static int subinfo_populate(SUBINFO *) ;
static int subinfo_allfree(SUBINFO *) ;
static int subinfo_phase2(SUBINFO *) ;
static int subinfo_phase3(SUBINFO *) ;
static int subinfo_del(SUBINFO *,int) ;
static int subinfo_acount(SUBINFO *) ;
static int subinfo_delone(SUBINFO *,int) ;
static int subinfo_check(SUBINFO *) ;
static int subinfo_already(SUBINFO *) ;


/* exported subroutines */

int main(int argc,const char **argv,const char **envv)
{
	struct subinfo	si, *sip = &si ;
	VOTDS		v ;
	time_t		dt = time(NULL) ;
	FILE		*ofp = stdout ;
#if	CF_DEBUGS && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif
	const int	ps = getpagesize() ;
	const int	of = O_CREAT ;
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;
	cchar		*pr = "/usr/add-on/local" ;
	cchar		*lang = "English" ;
	cchar		*pn = argv[0] ;
	char		*strp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((rs = votds_open(&v,pr,lang,of)) >= 0) {
	        if ((rs = subinfo_start(sip,dt,ofp,&v,strp,shsize,ps)) >= 0) {

#if	CF_PHASE1
	            if (rs >= 0) {
	                rs = subinfo_phase1(sip) ;
		    }
#endif /* CF_PHASE1 */

	            if (rs >= 0) {
	                rs = subinfo_phase2(sip) ;
		    }

	            if (rs >= 0) {
	                rs = subinfo_phase3(sip) ;
		    }

	            rs1 = subinfo_finish(sip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (subinfo) */
	        rs1 = votds_close(&v) ;
	        if (rs >= 0) rs = rs1 ;
	} /* end if (votds) */

	if (rs < 0) {
	    fprintf(stderr,"%s: extiing (%d)\n",pn,rs) ;
	}

#if	CF_DEBUGS
	debugprintf("main: ret rs=%d\n",rs) ;
#endif

	if (rs < 0) ex = EX_DATAERR ;

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	CF_DEBUGS
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,time_t dt,FILE *ofp,
	VOTDS *cip,char *strp,int shsize,int ps)
{
	int		rs ;
	int		seed ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->dt = dt ;
	sip->ofp = ofp ;
	sip->vip = vip ;
	sip->strp = strp ;
	sip->shsize = shsize ;
	sip->ps = ps ;
	seed = (dt & UINT_MAX) ;
	rs = randomvar_start(&sip->rng,0,seed) ;
	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (sip == NULL) return SR_FAULT ;

	rs1 = randomvar_finish(&sip->rng) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_getrand(SUBINFO *sip)
{
	uint		x ;
	int		rs ;
	if ((rs = randomvar_getuint(&sip->rng,&x)) >= 0) {
	    rs = (int) x ;
	    rs &= INT_MAX ;
	}
	return rs ;
}
/* end subroutine (subinfo_getrand) */


#if	CF_PHASE1
static int subinfo_phase1(SUBINFO *sip)
{
	FILE		*ofp = sip->ofp ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("subinfo_phase1: ent\n") ;
#endif
	if ((rs = subinfo_populate(sip)) >= 0) {
	    c = rs ;
	    if ((rs = votds_used(sip->shp)) >= 0) {
	        fprintf(ofp,"used=%d\n",rs) ;
#if	CF_DEBUGS
	        debugprintf("subinfo_phase1: votds_used() rs=%d\n",rs) ;
#endif
	        if ((rs = votds_audit(sip->shp)) >= 0) {
	            if ((rs = subinfo_allfree(sip)) >= 0) {
	                if ((rs = subinfo_check(sip)) >= 0) {
#if	CF_DEBUGS
	                    debugprintf("subinfo_phase1: "
	                        "votds_check() rs=%d\n",rs) ;
#endif
	                }
	            } /* end if (subinfo_allfree) */
	        } /* end if (votds_audit) */
	    } /* end if votds_used) */
	} /* end if (subinfo_populate) */

	if (rs >= 0)
	    fprintf(sip->ofp,"phase1=%u\n",c) ;

#if	CF_DEBUGS
	debugprintf("subinfo_phase1: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_phase1) */
#endif /* CF_PHASE1 */


static int subinfo_populate(SUBINFO *sip)
{
	FILE		*ofp = sip->ofp ;
	SHMALLOC	*shp = sip->shp ;
	REQUESTS	*asp = &sip->as ;
	int		rs = SR_OK ;
	int		asize ;
	int		tsize = 0 ;
	int		c = 0 ;

	while (rs >= 0) {
	    if ((rs = subinfo_getrand(sip)) >= 0) {
	        asize = rs ;
	        asize = (asize % SHSIZEMOD) ;
	        if (asize == 0) asize = 1 ;
#if	CF_DEBUGS
	        debugprintf("subinfo_populate: votds_alloc() asz=%u\n",
	            asize) ;
#endif
	        if ((rs = votds_alloc(shp,asize)) >= 0) {
	            REQUESTS_ITEM	ai ;
	            tsize += asize ;
		    c += 1 ;
#if	CF_DEBUGS
	            debugprintf("subinfo_populate: votds_alloc() rs=%d\n",
	                rs) ;
#endif
	            fprintf(ofp,"asz=%u a=%d\n",asize,rs) ;
	            ai.ro = rs ;
	            ai.rs = asize ;
	            if ((rs = requests_add(asp,&ai)) >= 0) {
	                rs = subinfo_already(sip) ;
	            }
	        } else if (rs == SR_NOMEM) {
	            rs = SR_OK ;
	            break ;
	        }
	        if (rs < 0) break ;
	    } /* end if (subinfo_getrand) */
	} /* end while */

#if	CF_DEBUGS
	debugprintf("subinfo_populate: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo__populate) */


static int subinfo_allfree(SUBINFO *sip)
{
	FILE		*ofp = sip->ofp ;
	SHMALLOC	*shp = sip->shp ;
	REQUESTS	*asp = &sip->as ;
	int		rs ;
#if	CF_DEBUGS
	    debugprintf("subinfo_allfree: ent\n") ;
#endif
	if ((rs = requests_count(asp)) >= 0) {
	    REQUESTS_ITEM	ai ;
	    int			i ;
#if	CF_DEBUGS
	    debugprintf("subinfo_allfree: c=%u\n",rs) ;
#endif
	for (i = 0 ; requests_get(asp,i,&ai) >= 0 ; i += 1) {
	    if (ai.ro >= 0) {
#if	CF_DEBUGS
	        debugprintf("subinfo_allfree: bi=%u r=%d:%d\n",
		i,ai.ro,ai.rs) ;
#endif
	        fprintf(ofp,"free ro=%d\n",ai.ro) ;
	        if ((rs = votds_free(shp,ai.ro)) >= 0) {
	            if ((rs = subinfo_check(sip)) >= 0) {
#if	CF_DEBUGS
	                if (rs < 0)
	                    debugprintf("subinfo_allfree: "
	                        "votds_check() rs=%d\n",rs) ;
#endif
	                rs = requests_del(asp,i--) ;
	            }
	        }
	    }
	    if (rs < 0) break ;
	} /* end for */
	} /* end if (requests_count) */
#if	CF_DEBUGS
	debugprintf("subinfo_allfree: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_allfree) */


static int subinfo_phase2(SUBINFO *sip)
{
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_phase2: ent\n") ;
#endif
	if ((rs = subinfo_populate(sip)) >= 0) {
	    c = rs ;
	    if ((rs = subinfo_already(sip)) >= 0) {
	        if ((rs = subinfo_acount(sip)) > 0) {
	            const int	to = (10*rs) ;
	            const int	c = rs ;
	            int		i = 0 ;
	            while ((rs = subinfo_acount(sip)) > 0) {

	                if ((rs = subinfo_delone(sip,c)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("subinfo_phase2: "
	                        "subinfo_delone() rs=%d\n", rs) ;
#endif
	                    rs = subinfo_check(sip) ;
	                }

	                if (i++ >= to) break ;
	                if (rs < 0) break ;
	            } /* end while */
	            if (rs >= 0) {
	                rs = subinfo_allfree(sip) ;
	            }
	        } /* end if (a-count) */
	    } /* end if (subinfo_already) */
	} /* end if (phase1a) */
	if (rs >= 0)
	    fprintf(sip->ofp,"phase2=%u\n",c) ;
#if	CF_DEBUGS
	debugprintf("subinfo_phase2: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_phase2) */


static int subinfo_phase3(SUBINFO *sip)
{
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_phase3: ent\n") ;
#endif
	if ((rs = subinfo_populate(sip)) >= 0) {
	    c = rs ;
	    if ((rs = subinfo_already(sip)) >= 0) {
	        if ((rs = subinfo_acount(sip)) > 0) {
	            int		bi ;
#if	CF_DEBUGS
	            debugprintf("subinfo_phase3: c=%u\n",c) ;
#endif
	            for (bi = 0 ; (rs = subinfo_del(sip,bi)) >= 0 ; bi += 2) {

#if	CF_DEBUGS
	                debugprintf("subinfo_phase3: bo=%u\n",bi) ;
#endif

			if (rs > 0) {
#if	CF_DEBUGS
	                    debugprintf("subinfo_phase3: "
	                        "subinfo_del() rs=%d\n", rs) ;
#endif
	                    rs = subinfo_check(sip) ;
	                }

	                if (rs < 0) break ;
	            } /* end for */
		    if (rs == SR_NOTFOUND) rs = SR_OK ;
	            if (rs >= 0) {
	                rs = subinfo_allfree(sip) ;
	            }
	        } /* end if (a-count) */
	    } /* end if (subinfo_already) */
	} /* end if (populate) */
	if (rs >= 0)
	    fprintf(sip->ofp,"phase3=%u\n",c) ;
#if	CF_DEBUGS
	debugprintf("subinfo_phase3: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_phase3) */


static int subinfo_del(SUBINFO *sip,int bi)
{
	SHMALLOC	*shp = sip->shp ;
	REQUESTS	*asp = &sip->as ;
	REQUESTS_ITEM	ai ;
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("subinfo_del: ent bi=%u\n",bi) ;
#endif
	if ((rs = requests_get(asp,bi,&ai)) >= 0) {
	    if (ai.ro >= 0) {
#if	CF_DEBUGS
	        debugprintf("subinfo_del: del r=%u:%u\n",ai.ro,ai.rs) ;
#endif
	        if ((rs = votds_already(shp,ai.ro)) == 0) {
	            if ((rs = votds_free(shp,ai.ro)) >= 0) {
	                if ((rs = votds_avail(shp)) >= 0) {
	                    f = TRUE ;
	                    rs = requests_del(asp,bi) ;
#if	CF_DEBUGS
	                    if (rs < 0)
	                        debugprintf("subinfo_del: "
	                            "del() rs=%d bi=%d\n",
	                            rs,bi) ;
#endif
	                }
	            }
#if	CF_DEBUGS
	            if (rs < 0)
	                debugprintf("subinfo_del: "
	                    "votds_free() rs=%d\n",
	                    rs) ;
#endif
	        } else if (rs > 0) {
#if	CF_DEBUGS
	            debugprintf("subinfo_del: already ro=%u\n",
	                ai.ro) ;
#endif
	            rs = SR_BADFMT ;
	        }
	    } /* end if (non-negative) */
	} /* end if (requests_get) */
#if	CF_DEBUGS
	debugprintf("subinfo_del: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_del) */


static int subinfo_already(SUBINFO *sip)
{
	SHMALLOC	*shp = sip->shp ;
	REQUESTS	*asp = &sip->as ;
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo_already: ent\n") ;
#endif
	if ((rs = requests_count(asp)) > 0) {
	    REQUESTS_ITEM	ai ;
	    int			i ;
#if	CF_DEBUGS
	    debugprintf("subinfo_already: c=%u\n",rs) ;
#endif
	    for (i = 0 ; requests_get(asp,i,&ai) >= 0 ; i += 1) {
	        if (ai.ro >= 0) {
	            c += 1 ;
	            if ((rs = votds_already(shp,ai.ro)) > 0) {
	                rs = SR_BADFMT ;
	            }
	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (requests_count) */
#if	CF_DEBUGS
	debugprintf("subinfo_already: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_already) */


static int subinfo_acount(SUBINFO *sip)
{
	return requests_count(&sip->as) ;
}
/* end subroutine (subinfo_acount) */


static int subinfo_delone(SUBINFO *sip,int c)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("subinfo_delone: ent\n") ;
#endif
	if ((rs = subinfo_getrand(sip)) >= 0) {
	    REQUESTS_ITEM	ai ;
	    const int		bi = (rs%c) ;
#if	CF_DEBUGS
	    debugprintf("subinfo_delone: bi=%u\n",bi) ;
#endif
	    if ((rs = requests_get(&sip->as,bi,&ai)) >= 0) {
	        if (ai.ro >= 0) {
#if	CF_DEBUGS
	            debugprintf("subinfo_delone: del r=%u:%u\n",ai.ro,ai.rs) ;
#endif
	            if ((rs = votds_already(sip->shp,ai.ro)) == 0) {
	                if ((rs = votds_free(sip->shp,ai.ro)) >= 0) {
	                    if ((rs = votds_avail(sip->shp)) >= 0) {
	                        rs = requests_del(&sip->as,bi) ;
#if	CF_DEBUGS
	                        if (rs < 0)
	                            debugprintf("subinfo_delone: "
	                                "del() rs=%d bi=%d\n",
	                                rs,bi) ;
#endif
	                    }
	                }
#if	CF_DEBUGS
	                if (rs < 0)
	                    debugprintf("subinfo_delone: "
	                        "votds_free() rs=%d\n",
	                        rs) ;
#endif
	            } else if (rs > 0) {
#if	CF_DEBUGS
	                debugprintf("subinfo_delone: already ro=%u\n",
	                    ai.ro) ;
#endif
	                rs = SR_BADFMT ;
	            }
	        } /* end if (non-negative) */
	    } else if (rs == SR_NOENT)
	        rs = SR_OK ;
#if	CF_DEBUGS
	    if (rs < 0)
	        debugprintf("subinfo_delone: requests_get() rs=%d\n",rs) ;
#endif
	} /* end if (get-rand) */
#if	CF_DEBUGS
	debugprintf("subinfo_delone: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_delone) */


static int subinfo_check(SUBINFO *sip)
{
	int		rs ;
	int		usz = 0 ;
	int		fsz = 0 ;
	if ((rs = votds_used(sip->shp)) >= 0) {
	    usz = rs ;
	    if ((rs = votds_audit(sip->shp)) >= 0) {
	        fsz = rs ;
	        if ((fsz+usz) != sip->shsize) rs = SR_NOMSG ;
	    }
	}
#if	CF_DEBUGS
	if (rs < 0) {
	    debugprintf("subinfo_check: sdz=%d usz=%d fsz=%d\n",
	        sip->shsize,usz,fsz) ;
	}
	debugprintf("subinfo_check: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_check) */


