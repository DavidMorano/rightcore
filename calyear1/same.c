
#ifdef	COMMENT
int calent_same(CALENT *ep,CALYEARS *op,CALENT *oep)
{
	WORDER		w1, w2 ;
	int		rs ;
	int		c1l, c2l ;
	int		f = FALSE ;
	const char	*c1p, *c2p ;

	if ((rs = worder_start(&w1,op,ep)) >= 0) {

	    if ((rs = worder_start(&w2,op,oep)) >= 0) {

	        while ((rs >= 0) && (! f)) {

	            c1l = worder_get(&w1,&c1p) ;

	            c2l = worder_get(&w2,&c2p) ;

	            if (c1l != c2l)
	                break ;

	            if ((c1l == 0) && (c2l == 0)) {
	                f = TRUE ;
	                break ;
	            }

	            if (c1p[0] != c2p[0])
	                break ;

	            if (strncmp(c1p,c2p,c1l) != 0)
	                break ;

	        } /* end while */

	        worder_finish(&w2) ;
	    } /* end if (w2) */

	    worder_finish(&w1) ;
	} /* end if (w1) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (calent_same) */
#endif /* COMMENT */

