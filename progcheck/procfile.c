static int procfile(PROGINFO *pip,cchar *fn)
{
	bfile		cfile, *cfp = &cfile ;
	int		rs ;
	cchar		*pn = pip->progname ;
	ccha		*fmt ;
	if ((rs = bopen(cfp,fn,"r",0666)) < 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    while ((rs = breadline(cfp,lbuf,llen)) > 0) {
	        len = rs ;

	        for (j = 0 ; j < len ; j += 1) {
	            for (k = 0 ; k < NFUN ; k += 1) {
	                if (lbuf[j] == cca[k].c_open) {
	                    counts[k].c_open += 1 ;
	                } else if (lbuf[j] == cca[k].c_close) {
	                    counts[k].c_close += 1 ;
			}
	            } /* end for */
	        } /* end for */

	    } /* end while */

	    if (rs >= 0) {
	    for (k = 0 ; k < NFUN ; k += 1) {

	        if (counts[k].c_open != counts[k].c_close) {

	            bprintf(ofp,"file \"%s\" %s> open %d	close %d\n",
	                fname[i],cca[k].funcname,
	                counts[k].c_open, counts[k].c_close) ;

	        } else {

			if (pip->verboselevel > 0)
	            bprintf(ofp,"file \"%s\" %s> is clean\n",
	                fname[i],cca[k].funcname) ;

			if (pip->debuglevel > 0)
		    bprintf(pip->efp,"%s: file \"%s\" %s> is clean\n",
			pip->progname,
	                fname[i],cca[k].funcname) ;

	        } /* end if */

	    } /* end for */
	    } /* end if */

	    bclose(cfp) ;
	} else {
	    fmt = "%s: inaccessible file (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: file=%s\n",pn,fn) ;
	} /* end for */
	return rs ;
}
/* end subroutine (procfile) */

