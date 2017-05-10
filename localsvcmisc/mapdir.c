
static int mapdir_processor(ep,ev,groupname,fd)
ISSUE_MAPDIR	*ep ;
const char	*ev[] ;
const char	groupname[] ;
int		fd ;
{
	struct ustat	sb ;

	VECSTR		nums ;

	FSDIR		d ;
	FSDIR_ENT	de ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	wlen = 0 ;

	const char	*dn ;
	const char	*gn ;
	const char	*defname = ISSUE_DEFGROUP ;
	const char	*allname = ISSUE_ALLGROUP ;
	const char	*name = ISSUE_NAME ;


#if	CF_DEBUGS
	debugprintf("issue/mapdir_processor: dir=%s\n",ep->dirname) ;
	debugprintf("issue/mapdir_processor: kn=%s\n",groupname) ;
#endif

	dn = ep->dirname ;
	if (dn[0] == '~') {
	    dn = ep->dname ;
	    if ((dn == NULL) || (dn[0] == '\0'))
	        goto ret0 ;
	}

	rs1 = u_stat(dn,&sb) ;
	if (rs1 < 0) goto ret0 ;

	gn = groupname ;
	if ((rs = vecstr_start(&nums,0,0)) >= 0) {
	    int	i ;
	    const char	*strs[5] ;

	    loadstrs(strs,gn,defname,allname,name) ;

	    if ((rs = fsdir_open(&d,dn)) >= 0) {
	        const char	*tp ;

	        while (fsdir_read(&d,&de) > 0) {
	            const char	*den = de.name ;
	            if (den[0] == '.') continue ;

#if	CF_DEBUGS
	            debugprintf("issue/mapdir_processor: den=%s\n",den) ;
#endif

	            tp = strchr(den,'.') ;
	            if ((tp != NULL) && (strcmp((tp+1),name) == 0)) {
	                const char	*digp ;
	                int	f = TRUE ;

	                digp = strnpbrk(den,(tp-den),"0123456789") ;
	                if (digp != NULL)
	                    f = hasalldig(digp,(tp-digp)) ;

	                if (f) {
	                    for (i = 0 ; i < 3 ; i += 1) {
	                        f = isBaseMatch(den,strs[i],digp) ;
	                        if (f) break ;
	                    }
	                }

	                if (f)
	                    rs = vecstr_add(&nums,den,(tp-den)) ;

	            } /* end if (have an ISSUE file) */

	            if (rs < 0) break ;
	        } /* end while (reading directory entries) */

	        fsdir_close(&d) ;
	    } /* end if (fsdir) */

	    if (rs >= 0) {

	        vecstr_sort(&nums,NULL) ;

#if	CF_DEBUGS
	        {
	            const char	*bep ;
	            for (i = 0 ; vecstr_get(&nums,i,&bep) >= 0 ; i += 1)
	                debugprintf("issue/mapdir_processor: i=%u bep=%s\n",
				i,bep) ;
	        }
#endif

	        rs = mapdir_processorthem(ep,ev,dn,&nums,strs,fd) ;
	        wlen += rs ;

	    } /* end if */

	    vecstr_finish(&nums) ;
	} /* end if (nums) */

ret0:

#if	CF_DEBUGS
	debugprintf("issue/mapdir_processor: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processor) */


static int mapdir_processorthem(ep,ev,dn,blp,strs,fd)
ISSUE_MAPDIR	*ep ;
const char	*ev[] ;
const char	dn[] ;
VECSTR		*blp ;
const char	**strs ;
int		fd ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	wlen = 0 ;

	const char	*kn ;


	kn = strs[0] ;
	rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;

	if (isNotPresent(rs1)) {

	    kn = strs[1] ;
	    rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;

	} else
	    rs = rs1 ;

	if (rs > 0) wlen += rs ;

	if (rs >= 0) {
	    kn = strs[2] ;
	    rs1 = mapdir_processorone(ep,ev,dn,blp,kn,fd) ;
	    if (! isNotPresent(rs1)) rs = rs1 ;
	    if (rs > 0) wlen += rs ;
	}


ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processorthem) */


static int mapdir_processorone(ep,ev,dn,blp,kn,fd)
ISSUE_MAPDIR	*ep ;
const char	*ev[] ;
const char	*dn ;
VECSTR		*blp ;
const char	*kn ;
int		fd ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	kl = strlen(kn) ;
	int	c = 0 ;
	int	wlen = 0 ;

	const char	*bep ;


#if	CF_DEBUGS
	debugprintf("issue/mapdir_processorone: kn=%s\n",kn) ;
#endif

	for (i = 0 ; vecstr_get(blp,i,&bep) >= 0 ; i += 1) {
	    if (bep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("issue/mapdir_processorone: bep=%s\n",bep) ;
#endif

	    if (strncmp(bep,kn,kl) == 0) {
	        c += 1 ;
	        rs1 = mapdir_procout(ep,ev,dn,bep,fd) ;

#if	CF_DEBUGS
	        debugprintf("issue/mapdir_processorone: _procout() rs=%d\n",
			rs1) ;
#endif

	        if (rs1 >= 0) {
	            wlen += rs1 ;
	        } else if (! isNotPresent(rs1))
	            rs = rs1 ;
	    }

	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (c == 0)) rs = SR_NOENT ;

ret0:

#if	CF_DEBUGS
	debugprintf("issue/mapdir_processorone: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mapdir_processorone) */

