


/* local defines */

#define	SUBINFO		struct subinfo


/* local structures */

typedef const void	cvoid ;

struct subinfo {
	cchar		*fn ;
	cchar		*id ;
	char		*bp ;
	int		mc ;
	int		bl ;
	int		wl ;
	int		blen ;
	int		ilen ;
} ;


/* forward references */

static int subinfo_start(SUBINFO *,char *,cchar *,cchar *,int,cvoid *,int) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_wrline(SUBINFO *,cchar *,int) ;
static int subinfo_flush(SUBIFO *,int) ;


/* exported subroutines */


int nprinthexblock(cchar *fn,cchar *id,int mc,const void *vp,int vl)
{
	int		rs ;
	int		sl = vl 
	int		wlen = 0 ;
	cchar		*sp = (cchar *) vp ;
	char		b[PRINTBUFLEN + 1] ;

	if (fn == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (fn[0] == '\0') return SR_INVALID ;

	if (mc < 0) mc = COLUMNS ;
	if (sl < 0) sl = strlen(sp) ;

	if ((rs = subinfo_start(&si,b,fn,id,mc,vp,vl)) >= 0) {
	    while (sl > 0) {
		rs = subinfo_wrline(&si,sp,sl) ;
		sp += rs ;
		sl -= rs ;
		if (rs < 0) break ;
	    } /* end while */
	    wlen = subinfo_finish(&si) ;
	    if (rs >= 0) rs = wlen ;
	} /* end if (subinfo) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (nprinthexblock) */


/* private subroutines */


static int subinfo_start(SUBINFO *sip,char *bp,cchar *fn,cchar *id,int mc)
{
	int		sl = vl ;
	cchar		*cp = (cchar *) vp ;
	if (sl < 0) sl = strlen(sp) ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->bp = bp ;
	sip->fn = fn ;
	sip->mc = mc ;
	sip->blen = PRINTBUFLEN ;
	if (id != NULL) {
	    sip->id = id ;
	    sip->ilen = strlen(id) ;
	}
	return SR_OK ;
}
/* end subroutine (subfino_start) */


static int subinfo_finish(SUBINFO *sip)
{
	return sip->wl ;
}
/* end subroutine (subfino_finish) */


static int subinfo_wrline(SUBINFO *sip,cchar *sp,int sl)
{
	const int	mlen = MIN((3*sl),(sip->mc - sip->ilen)) ;
	int		rs ;
	int		ul = 0 ;

	if ((rs = subinfo_flush(sip,mlen)) >= 0) {
	    if (sip->id != NULL) {
		rs = subinfo_write(sip,sip->id,sip->ilen) ;
	    }
	    if (rs >= 0) {
		const int	alen = (sip->blen-sip->bl) ;
		const int	n = (mlen/3) ;
	        if ((rs = mkhexstr(sip->bp,alen,sp,n)) >= 0) {
			sip->bl += rs ;
		        ul = rs ;
		}
	    }
	}

	return (rs >= 0) ? wl : rs ;
}
/* end subroutine (subinfo_wrline) */


static int subinfo_flush(SUBIFO *sip,int mlen)
{
	int		rs = SR_OK ;
	if (mlen > (sip->blen-sip->bl)) {
	    if ((rs = nprint(sip->fn,sip->bp,sip->nl)) >= 0) {
	        sip->wl += rs ;
		sip->bl = 0 ;
	    }
	}
	return rs ;
}
/* end subroutine (subfino_flush) */


