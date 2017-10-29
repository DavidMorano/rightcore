

int progconfig(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;


/* prepare to store configuration variable lists */

	if ((rs = vecstr_start(&defines,10,VECSTR_PORDERED)) < 0)
	    goto badlistinit ;

	if ((rs = vecstr_start(&unsets,10,VECSTR_PNOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    goto badlistinit ;
	}

	if ((rs = vecstr_start(&exports,10,VECSTR_PNOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    vecstr_finish(&unsets) ;

	    goto badlistinit ;
	}

	oo.lists = TRUE ;

/* load up some initial environment that everyone should have! */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: about to do DEFINITFNAME=%s\n",DEFINITFNAME) ;
#endif

	procfileenv(pip->pr,DEFINITFNAME,&exports) ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: checking for configuration file\n") ;
	    debugprintf("main: 0 CF=%s\n",configfname) ;
	}
#endif /* CF_DEBUG */

	rs = SR_NOEXIST ;
	if ((configfname == NULL) || (configfname[0] == '\0')) {

	    configfname = CONFIGFNAME ;

	    if ((sl = permsched(sched1,&svars,configfname,R_OK,
	        tmpfname,MAXPATHLEN)) < 0) {

	        if ((sl = permsched(sched3,&svars,configfname,R_OK,
	            tmpfname,MAXPATHLEN)) > 0)
	            configfname = tmpfname ;

	    } else if (sl > 0)
	        configfname = tmpfname ;

	    rs = sl ;

	} else {

	    sl = getfname(pip->pr,configfname,1,tmpfname) ;

	    if (sl > 0)
	        configfname = tmpfname ;

	    rs = sl ;		/* cause an open failure later */

	} /* end if */

	if (configfname == tmpfname)
	    f_freeconfigfname = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: find rs=%d\n",sl) ;
	    debugprintf("main: 1 CF=%s\n",configfname) ;
	}
#endif /* CF_DEBUG */

/* read in the configuration file if we have one */

	if ((rs >= 0) || (perm(configfname,-1,-1,NULL,R_OK) >= 0)) {

	    FIELD	fsb ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: configuration file \"%s\"\n",
	            configfname) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: conf=%s\n",
	            pip->progname,configfname) ;

	    if ((rs = configfile_start(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we have a good configuration file\n") ;
#endif

/* we must set this mode to 'VARSUB_MBADNOKEY' so that a miss is noticed */

	    varsub_start(&vsh_d,VARSUB_MBADNOKEY) ;

	    varsub_start(&vsh_e,0) ;

	    varsub_loadenv(&vsh_e,(const char **) envv) ;

/* program root from configuration file */

	    if ((cf.root != NULL) && (! f_programroot)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: processing CF programroot\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            proginfo_setprogroot(pip,buf2,l2) ;

	        }

	    } /* end if (configuration file program root) */

/* loop through the DEFINEd variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf( "main: 0 top, cp=%s\n",cp) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cp,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	                debugprintf("main: 0 about to merge\n") ;
#endif

	            varsub_merge(&vsh_d,&defines,buf2,l2) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	                debugprintf("main: 0 out of merge\n") ;
#endif

	        } /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: done w/ defines\n") ;
#endif


/* all of the rest of the configuration file stuff */

	    if ((cf.workdir != NULL) && (pip->workdname == NULL)) {

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.workdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            proginfo_setentry(pip,&pip->workdname,buf2,l2) ;

	        }

	    } /* end if (config file working directory) */

	    if ((cf.pidfname != NULL) && (pidfname[0] == '\0')) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF pidfile=%s\n",cf.pidfname) ;
#endif

	        if ((cf.pidfname[0] != '\0') && (cf.pidfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.pidfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(pip,buf,sl, buf2,BUFLEN)) > 0)) {

	                snwcpy(pidfname,MAXPATHLEN,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: pidfname=%s\n",pidfname) ;
#endif

	    } /* end if (configuration file PIDFNAME) */

	    if ((cf.lockfname != NULL) && (lockfname[0] == '\0')) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF lockfile=%s\n",cf.lockfname) ;
#endif

	        if ((cf.lockfname[0] != '\0') && (cf.lockfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.lockfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(pip,buf,sl, buf2,BUFLEN)) > 0)) {

	                snwcpy(lockfname,MAXPATHLEN,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: lockfile=%s\n",lockfname) ;
#endif

	    } /* end if (configuration file LOCKFNAME) */


	    if ((cf.logfname != NULL) && (logfile_type < 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

		if (logfname == NULL) {

	            proginfo_setentry(pip,&pip->logfname,buf2,l2) ;

	            if (strchr(logfname,'/') != NULL)
	                logfile_type = 1 ;

		}

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: processed CF logfilename=%s\n",
	                logfname) ;
#endif

	    } /* end if (configuration file log filename) */

	    if ((cf.paramfname != NULL) && (ptfile_type < 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF pingtab filename=%s\n",
	                cf.paramfname) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.paramfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            snwcpy(ptfname,MAXPATHLEN,buf2,l2) ;

	            if (strchr(ptfname,'/') != NULL)
	                ptfile_type = 1 ;

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: processed CF ptfname=%s\n",
	                ptfname) ;
#endif

	    } /* end if (configuration file 'pingtab' filename) */


	    if ((cf.statfname != NULL) && (psfile_type < 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF statfilename=%s\n",cf.statfname) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.statfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            snwcpy(psfname,MAXPATHLEN,buf2,l2) ;

	            if (strchr(psfname,'/') != NULL)
	                psfile_type = 1 ;

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: processed CF pingstat filename=%s\n",
	                psfname) ;
#endif

	    } /* end if (configuration file 'pingstat' filename) */

/* timeout */

	    if ((cf.timeout != NULL) && (to < 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF timeout=%s\n",cf.timeout) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.timeout,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            if (cfdeci(buf2,l2,&val) >= 0)
	                to = val ;

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: processed CF timeout=%s\n",
	                buf2) ;
#endif

	    } /* end if (configuration file timeout) */

/* what about an 'environ' file? */

	    if (cf.envfname != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: 1 envfile=%s\n",cf.envfname) ;
#endif

	        procfileenv(pip->pr,cf.envfname,&exports) ;

	    } else
	        procfile(pip,procfileenv,pip->pr,&svars,
	            ENVFNAME,&exports) ;

	    f_procfileenv = TRUE ;


/* "do" any 'paths' file before we process the environment variables */

	    if (cf.pathfname != NULL)
	        procfilepaths(pip->pr,cf.pathfname,&exports) ;

	    else
	        procfile(pip,procfilepaths,pip->pr,&svars,
	            PATHSFNAME,&exports) ;

	    f_procfilepaths = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

/* OK, now loop through any options that we have */

	    if (cf.options != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: CF options\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.options,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            if ((rs = field_start(&fsb,buf2,sl2)) >= 0) {

	                while ((fl = field_get(&fsb,oterms,&fp)) >= 0) {

	                    if (fl == 0) continue ;

	                    if ((sl = sfkey(fp,fl,&cp)) < 0) {
	                        cp = fp ;
	                        sl = fl ;
	                    }

	                    if ((i = matostr(configopts,1,cp,sl)) >= 0) {

	                        char	*cp2 ;


	                        cp2 = strnchr((cp + sl),(fl - sl),'=') ;

	                        sl2 = 0 ;
	                        if (cp2 != NULL) {
	                            cp2 += 1 ;
	                            sl2 = fl - (cp2 - fp) ;
	                        }

	                        switch (i) {

	                        case configopt_marktime:
	                            if ((pip->markint <= 0) && (cp2 != NULL) &&
	                                (cfdecti(cp2,sl2,&val) >= 0)) {

	                                pip->markint = val ;

	                            } /* end if */

	                            break ;

	                        case configopt_sumfile:
	                            if (pip->sumfname == NULL) {

	                                if (cp2 != NULL)
	                                    proginfo_setentry(pip,
	                                        &pip->sumfname,
	                                        cp2,sl2) ;

	                            }

	                            break ;

	                        case configopt_minpingint:
	                            if ((pip->minpingint < 0) && 
					(cp2 != NULL) &&
	                                (cfdecti(cp2,sl2,&val) >= 0)) {

	                                pip->minpingint = val ;

	                            } /* end if */

	                            break ;

	                        case configopt_minupdate:
	                            if ((pip->minupdate < 0) && 
					(cp2 != NULL) &&
	                                (cfdecti(cp2,sl2,&val) >= 0)) {

	                                pip->minupdate = val ;

	                            } /* end if */

	                            break ;

	                        } /* end switch */

	                    } /* end if */

	                } /* end while */

	                field_finish(&fsb) ;

	            } /* end if (field) */

	        } /* end if */

	    } /* end if (options) */

/* loop through the UNSETs in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.unsets,i,&cp) >= 0 ; i += 1) {

	        rs = vecstr_finder(&exports,cp,vstrkeycmp,NULL) ;

	        if (rs >= 0)
	            vecstr_del(&exports,rs) ;

	    } /* end for */

/* loop through the EXPORTed variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: 1 about to sub> %s\n",cp) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cp,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	                debugprintf("main: 1 about to merge> %t\n",buf,l2) ;
#endif

	            varsub_merge(NULL,&exports,buf2,l2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 1 done merging\n") ;
#endif

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(2)) {
	                debugprintf("varsub_merge: VSA_D so far \n") ;
	                varsub_dump(&vsh_d) ;
	                debugprintf("varsub_merge: VSA_E so far \n") ;
	                varsub_dump(&vsh_e) ;
	            } /* end if (debuglevel) */
#endif /* CF_DEBUG */

	        } /* end if (merging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: done subbing & merging\n") ;
#endif

	    } /* end for (exports) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ exports\n") ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: processing CF freeing data structures\n") ;
#endif

	    varsub_finish(&vsh_d) ;

	    varsub_finish(&vsh_e) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: processing CF free\n") ;
#endif

	    configfile_finish(&cf) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	        debugprintf("main: dumping defines\n") ;
	        for (i = 0 ; vecstr_get(&defines,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: define> %s\n",cp) ;
	        debugprintf("main: dumping exports\n") ;
	        for (i = 0 ; vecstr_get(&exports,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: export> %s\n",cp) ;
	    } /* end if (CF_DEBUG) */
#endif /* CF_DEBUG */

	} /* end if (accessed the configuration file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished with any configfile stuff\n") ;
#endif


	return rs ;
}
/* end subroutine (progconfig) */



