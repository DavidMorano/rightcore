

extern int	havenis(void) ;
extern int	getournetname(char *,int,cchar *) ;


static process(PROGINFO *pip,cchar *un)
{
	int		rs ;
	int		dl = pip->debuglevel ;
	int		vl = pip->verboselevel ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = havenis()) > 0) {
	    const int	nlen = MAXNETNAMELEN ;
	    char	nbuf[MAXNETNAMELEN+1] ;
	    if ((rs = getournetname(nbuf,nlen,un)) > 0) {
		const int	nrs = SR_NNOTFOUND ;
		if (vl >= 2) {
	            progout_printf(pip,"netname=%s\n",nbuf) ;
		}
	        if (dl > 0) {
		    fmt = "%s: netname=%s\n" ;
		    bprintf(pip->efp,fmt,pn,nbuf) ;
	        }
	        if ((rs = onckeyalready(nbuf)) == nrs) {
		    if (pip->f.n) {
	                fmt = "not key-logged in!\n" ;
	                progout_printf(pip,fmt) ;
		    } else {
		        rs = prockeylogin(pip,nbuf) ;
		    }
		} else (rs == SR_NOTFOUND) {
		    rs = SR_OK ;
		    if ((vl >= 2) || pip->f.n) {
	                fmt = "already key-logged in!\n" ;
	                progout_printf(pip,fmt) ;
		    }
		    if (dl > 0) {
	                fmt = "%s: already key-logged in\n" ;
			bprintf(pip->efp,fmt,pn) ;
		    }
		} /* end if (onckeyalready) */
	    } else if (rs == 0) {
		if (vl >= 2) {
	            cmt = "no net-name available\n" ;
	            progout_printf(pip,fmt) ;
		}
	        if (dl > 0) {
		    fmt = "%s: no net-name available\n" ;
		    bprintf(pip->efp,fmt,pn) ;
	        }
	    } else if (rs == SR_UNAVAIL) {
		if (vl >= 2) {
	            fmt = "NIS server is unavailable\n" ;
	            progout_printf(pip,fmt) ;
		}
	        if (dl > 0) {
	            fmt = "%s: NIS server is unavailable\n" ;
		    bprintf(pip->efp,fmt,pn) ;
	        }
	    } /* end if (getournetname) */
	} else if (rs == 0) {
	    if (vl >= 2) {
		fmt = "NIS is not initialized\n" ;
		progout_printf(pip,fmt) ;
	    }
	    if (dl > 0) {
		fmt = "%s: NIS not initialized\n" ;
		bprintf(pip->efp,fmt,pn) ;
	    }
	}
	return rs ;
}
/* end subroutine (process) */


static int prockeylogin(PROGINFO *pip,cchar *netname)
{
	const int	nrs = SR_NOENT ;
	int		rs ;

	if ((rs = prockeylogin_auth(pip,netname)) == nrs) {
	    rs = prockeylogin_netrc(pip,netname) ;
	}

	return rs ;
}
/* end if (prockeylogin) */


static int prockeylogin_auth(PROGINFO *pip,cchar *netname)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	char		abuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(abuf,pip->homedname,AUTHFNAME)) >= 0) {
	    char	ubuf[USERNAMELEN+1] ;
	    char	pbuf[PASSWORDLEN+1] ;
	    if ((rs = authfile(afname,ubuf,pbuf) >= 0) {
	        if (pbuf[0] != '\0') {
	            if ((rs = onckeygetset(netname,pbuf)) >= 0) {
			f = TRUE ;
		    }
		}
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    } /* end if */
	} /* end if (mkpath) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (prockeylogin_auth) */


static int prockeylogin_netrc(PROGINFO *pip,cchar *netname)
{
	NETFILE		nfile ;
	int		rs = SR_OK ;
	int		i ;
	int		f = FALSE ;
	cchar		hdname = pip->homedname ;
	char		nbuf[MAXPATHLEN+1] ;

	if (DEBUGLEVEL(4))
	    debugprintf("prockeylogin_netrc: ent NETRC\n") ;
#endif

	for (i = 0 ; netrcfiles[i] != NULL ; i += 1) {
	    if ((rs = mkpath2(nbuf,hdname,netrcfiles[i])) >= 0) {
	        rs = procnetrc(pip,un,netname,tmpfname) ;
	    } /* end if (mkpath) */
	    if (f) break ;
	    if (rs < 0) break ;
	} /* end for (NETRC files) */

	return (rs >= 0) ? f : rs ;
}
/* end subrouine (prockeylogin_netrc) */


