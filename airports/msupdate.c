static int msupdate(pip,lp)
struct proginfo	*pip ;
LFM		*lp ;
{
	struct pollfd	fds[2] ;

	MSFILE		ms ;

	MSFILE_ENT	e, etmp ;

	KINFO		ki ;

	KINFO_DATA	d ;

	time_t	ti_start ;
	time_t	ti_log ;

	long	lw ;

	uint	ppm ;
	uint	pagesize = getpagesize() ;

	int	rs, rs1, i, c ;
	int	nfds, oflags ;
	int	f ;

	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/msupdate: runint=%s\n",
	        timestr_elapsed((time_t) pip->runint,timebuf)) ;
#endif

	nfds = 0 ;
	fds[nfds].fd = -1 ;
	fds[nfds].events = 0 ;
	fds[nfds].revents = 0 ;

	oflags = O_RDWR ;
	rs = msfile_open(&ms,pip->msfname,oflags,0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/msupdate: msfile_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	pip->daytime = time(NULL) ;

	ti_log = pip->daytime ;
	ti_start = pip->daytime ;

	rs1 = msfile_match(&ms,pip->daytime,pip->nodename,-1,&e) ;

	if (rs1 < 0) {

	    memset(&e,0,sizeof(MSFILE_ENT)) ;

	    strwcpy(e.nodename,
	        pip->nodename,MSFILEE_LNODENAME) ;

	}

/* get some updated information */

	if (! pip->f.zerospeed) {

#if	F_CPUSPEED
	    f = (e.boottime == 0) || (e.speed == 0) || pip->f.speed ;

	    if (! f)
	        f = (e.stime == 0) || 
	            ((pip->daytime - e.stime) >= pip->speedint) ;

	    if (f) {

	        if (pip->debuglevel > 0)
	            shio_printf(pip->efp,
	                "%s: speed recalculation is indicated\n",
	                pip->progname) ;

	        shio_flush(pip->efp) ;

	        rs1 = cpuspeed(pip->pr,pip->speedname,0) ;

	        pip->daytime = time(NULL) ;

	        if (rs1 < 0) {

	            if ((! pip->f.quiet) && (pip->efp != NULL)) {

	                shio_printf(pip->efp,
	                    "%s: speed name=%s\n",
	                    pip->progname,pip->speedname) ;

	                shio_printf(pip->efp,
	                    "%s: speed subsystem is not available (%d)\n",
	                    pip->progname,rs1) ;

	            }

	        } else {
	            e.speed = rs1 ;
	            e.stime = pip->daytime ;
	        }

	    } /* end if (needed speed update) */
#endif /* F_CPUSPEED */

	} else {

	    e.speed = 0 ;
	    e.stime = pip->daytime ;

	}

/* were we requested to do a disable ? */

	if (pip->f.disable) {

	    e.flags |= MSFLAG_MDISABLED ;
	    if (pip->disint > 0)
	        e.dtime = pip->daytime + pip->disint ;

	}

/* do some load-ave updates */

	c = 0 ;
	rs = kinfo_open(&ki,pip->daytime) ;

	if (rs >= 0) {

	    while (! if_int) {

	        LFM_CHECK	ci ;


	        lw = pip->runint - (pip->daytime - ti_start) ;
	        if (lw <= 0)
	            break ;

	        if (pip->f.daemon) {

	            if (pip->have.pidfile && (lp != NULL)) {

	                rs = lfm_check(lp,&ci,pip->daytime) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("b_msu/msupdate: lfm_check() rs=%d\n",rs) ;
	                    if (rs < 0) {
	                        debugprintf("b_msu/msupdate: pid=%d\n",ci.pid) ;
	                        debugprintf("b_msu/msupdate: node=%s\n",
					ci.nodename) ;
	                        debugprintf("b_msu/msupdate: user=%s\n",
					ci.username) ;
	                        debugprintf("b_msu/msupdate: banner=%s\n",
					ci.banner) ;
	                    }
	                }
#endif /* CF_DEBUG */

	            } /* end if (had a lock file) */

#ifdef	COMMENT
	            if ((rs < 0) && (rs != SR_AGAIN))
	                break ;

	            if (rs == SR_AGAIN) {
	                rs = SR_OK ;
	                break ;
	            }
#else
	            if (rs < 0)
	                break ;
#endif /* COMMENT */

	        } /* end if (daemon mode) */

/* continue with the update */

	        rs = kinfo_sysmisc(&ki,pip->daytime,&d) ;

	        e.boottime = (uint) d.boottime ;
	        e.ncpu = d.ncpu ;
	        e.nproc = d.nproc ;

	        e.la[0] = d.la_1min ;
	        e.la[1] = d.la_5min ;
	        e.la[2] = d.la_15min ;

/* calculate pages-per-megabyte */

	        ppm = (1024 * 1024) / pagesize ;

/* OK, now calculate the megabytes of each type of memory */

#if	defined(SOLARIS)
	        rs1 = uc_sysconf(_SC_PHYS_PAGES,&lw) ;

		e.pmtotal = 1 ;
	        if ((rs1 >= 0) && (ppm > 0))
	            e.pmtotal = (lw / ppm) ;

	        rs1 = uc_sysconf(_SC_AVPHYS_PAGES,&lw) ;

		e.pmavail = 1 ;
	        if ((rs1 >= 0) && (ppm > 0))
	            e.pmavail = (lw / ppm) ;
#else
		e.pmavail = 1 ;
#endif /* defined(SOLARIS) */

/* finish off with time stamps */

	        e.atime = (uint) 0 ;
	        e.utime = (uint) pip->daytime ;

/* write-back the update */

	        rs = msfile_update(&ms,pip->daytime,&e) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_msu/msupdate: msfile_update() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            break ;

	        c += 1 ;

	        if (! pip->f.daemon)
	            break ;

/* sleep for daemon mode */

	        for (i = 0 ; (! if_int) && (i < pip->pollint) ; i += 1) {

	            rs1 = u_poll(fds,nfds,POLLINTMULT) ;

	            if (rs1 < 0)
	                if_int = TRUE ;

	        } /* end for */

	        if (if_int)
	            break ;

	        pip->daytime = time(NULL) ;

/* maintenance */

	        if ((c & 15) == 3)
	            kinfo_check(&ki,pip->daytime) ;

	        if (pip->have.configfile && ((c & 3) == 4))
	            progconfig_check(pip) ;

	        if ((c & 15) == 5)
	            msfile_check(&ms,pip->daytime) ;

	        if (pip->open.logfile && ((c & 7) == 1)) {

	            if ((pip->daytime - ti_log) >= pip->markint) {

	                ti_log = pip->daytime ;
	                lw = labs(pip->runint - (pip->daytime - ti_start)) ;

	                logfile_printf(&pip->lh,
	                    "%s mark> %s",
	                    timestr_logz(pip->daytime,timebuf),
	                    pip->nodename) ;

	                logfile_printf(&pip->lh,
	                    "remaining=%s",
	                    timestr_elapsed(lw,timebuf)) ;

	                logfile_flush(&pip->lh) ;

	            }
	        }

/* periodically close and rdebugopen the log file (precaution ?) */

	        if ((c & 31) == 1) {

		    if (pip->open.logfile) {

		        pip->open.logfile = FALSE ;
	                logfile_close(&pip->lh) ;

		    }

		    if (pip->have.logfile) {

	                rs1 = logfile_open(&pip->lh,pip->logfname,0,0666,
	                    pip->logid) ;

	                pip->open.logfile = (rs1 >= 0) ;

		    }

	        } /* end if (logging) */

	        if (pip->cmd[0] != '\0') {

	            if (strcmp(pip->cmd,"exit") == 0)
	                break ;

	        }

/* get a flesh copy of the entry */

	        rs1 = msfile_match(&ms,pip->daytime,pip->nodename,-1,&etmp) ;

	        if (rs1 >= 0)
	            e = etmp ;

/* check disabled state only in daemon mode */

	        if ((e.dtime != 0) && (pip->daytime >= e.dtime)) {

	            e.dtime = 0 ;
	            e.flags &= (~ MSFLAG_MDISABLED) ;

	        }

	    } /* end while (end time not reached) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("b_msu/msupdate: white-out if_int=%u\n",
	            if_int) ;
	        debugprintf("b_msu/msupdate: start=%s\n",
	            timestr_log(ti_start,timebuf)) ;
	        debugprintf("b_msu/msupdate: now=%s\n",
	            timestr_log(pip->daytime,timebuf)) ;
	    }
#endif

	    kinfo_close(&ki) ;

	} /* end if (opened kernel channel) */

	msfile_close(&ms) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_msu/msupdate: ret rs=%d c=%u\n",rs,c) ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msupdate) */



