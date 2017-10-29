

static int procname(PROGINFO *pip,void *ofp,cchar *qs)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if ((qs != NULL) && (qs[0] != '\0')) {
	    int		sl = strlen(qs) ;
	    int		cl ;
	    cchar	*sp = qs ;
	    cchar	*tp ;
	    cchar	*cp ;

	    while ((tp = strnchr(sp,sl,'&')) != NULL) {
	        cp = sp ;
	        cl = (tp-sp) ;
	        if (cl > 0) {
	            rs = procnamer(pip,ofp,cp,cl) ;
	            wlen += rs ;
	        }
	        sl -= ((tp-1)-sp) ;
	        sp = (tp+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (sl > 0)) {
	        rs = procnamer(pip,ofp,sp,sl) ;
	        wlen += rs ;
	    }

	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (procname) */


static int procnamer(PROGINFO *pip,void *ofp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		kl ;
	int		vl = 0 ;
	int		wlen = 0 ;
	const char	*tp ;
	const char	*kp ;
	const char	*vp = NULL ;
	if (sl < 0) sl = strlen(sp) ;

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	} /* end while */
	kp = sp ;
	kl = sl ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) {
	    kl = (tp - sp) ;
	    vp = (tp + 1) ;
	    vl = ((sp + sl) - vp) ;
	}
	if (kl > 0) {
	    const int	vlen = MAXNAMELEN ;
	    char	vbuf[MAXNAMELEN+1] ;
	    while ((vl > 0) && CHAR_ISWHITE(*vp)) {
	        vp += 1 ;
	        vl -= 1 ;
	    }
	    if ((vl > 0) && ((strnpbrk(vp,vl,"% +\t")) != NULL)) {
	        rs = procfixval(pip,vbuf,vlen,vp,vl) ;
	        vl = rs ;
	        vp = vbuf ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_querystring/procnamer: "
			"vl=%d f=>%t<\n",vl,vp,vl) ;
#endif
	    } /* end if (value) */
	} /* end if (key) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procnamer) */


static int procfixval(PROGINFO *pip,char *rbuf,int rlen,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		i = 0 ;
	if (vl > 0) {
	    cchar	*tp ;
	    char	*rp = rbuf ;
	    if (vl > rlen) vl = rlen ;
	    while ((tp = strnpbrk(vp,vl,"%+\t")) != NULL) {
	        const int	sch = MKCHAR(*tp) ;
	        if ((tp-vp) > 0) {
	            rp = strwcpy(rp,vp,(tp-vp)) ;
	        }
	        switch (sch) {
	        case '+':
	            if (((rp-rbuf) == 0) || (rp[-1] != ' ')) *rp++ = ' ' ;
	            break ;
	        case '\t':
	            if (((rp-rbuf) == 0) || (rp[-1] != ' ')) *rp++ = ' ' ;
	            break ;
	        case '%':
	            {
	                const int	tl = (vl-(tp-vp)) ;
	                rp = strwebhex(rp,tp,tl) ;
	                tp += MIN(2,tl) ;
	            }
	            break ;
	        } /* end switch */
	        vl -= ((tp+1)-vp) ;
	        vp = (tp+1) ;
	    } /* end while */
	    if ((rs >= 0) && (vl > 0)) {
	        while ((vl > 0) && CHAR_ISWHITE(vp[vl-1])) vl -= 1 ;
	        rp = strwcpy(rp,vp,vl) ;
	    }
	    i = (rp-rbuf) ;
	} /* end if (positive) */
	rbuf[i] = '\0' ;
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (procfixval) */


static char *strwebhex(char *rp,cchar *tp,int tl)
{
	if ((tl >= 3) && (*tp == '%')) {
	    const int	ch1 = MKCHAR(tp[1]) ;
	    const int	ch2 = MKCHAR(tp[2]) ;
	    if (ishexlatin(ch1) && ishexlatin(ch2)) {
	        int	v ;
	        if (cfhexi((tp+1),2,&v) >= 0) {
	            *rp++ = v ;
	        }
	    }
	}
	return rp ;
}
/* end subroutine (strwebhex) */


