
/* open configuration file */

	rs = bopen(cfp,configfname,"r",0664) ;

	if (rs < 0) {

	    bprintf(efp,"%s: could not open configuration file (%d)\n",
	        progname,rs) ;

	    goto badret ;
	}

/* start processing the configuration file */

	line = 0 ;
	while ((len = breadline(cfp,linebuf,LINELEN)) > 0) {

	    line += 1 ;

	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {

#ifdef	COMMENT
	        f_trunc = TRUE ;
#endif
	        while ((c = bgetc(cfp)) >= 0) if (c == '\n') break ;

		continue ;
	    }

	fsb.rlen = len - 1 ;
	    fsb.lp = linebuf ;
#if	D_FIELD
	    bprintf(efp,"l: %W\n",fsb.lp,fsb.rlen) ;
#endif
	    field(&fsb,fterms) ;

#if	D_FIELD
	    bprintf(efp,"f0: %W\n",fsb.fp,fsb.flen) ;
#endif


/* empty or comment only line */

	    if (fsb.flen == 0) continue ;

/* convert key to lower case */

	    clow(fsb.flen,fsb.fp,buf) ;

	    buf[fsb.flen] = '\0' ;

/* check if key is a valid one */

	    for (i = 0 ; i < KEY_OVERLAST ; i += 1) {

#if	DFIELD
	        bprintf(efp,"user key %W\n",buf,fsb.flen) ;
#endif

	        if (fsb.flen < cfk[i].min) continue ;

#if	DFIELD
	        bprintf(efp,"about to check for min\n") ;
#endif

/* insure that 'min' characters match */

	        if (strncmp(buf,cfk[i].namep,cfk[i].min) != 0) continue ;

#if	DFIELD
	        bprintf(efp,"about to check for max\n") ;
#endif

	        len = MIN(fsb.flen,cfk[i].max) ;

	        if (strncmp(buf,cfk[i].namep,len) == 0) goto key_match ;

#if	DFIELD
	        bprintf(efp,"bottom of loop\n") ;
#endif

	    }

	    bprintf(efp,"%s: unknown configuration file key \"%s\"\n",
	        progname,buf) ;

	    continue ;

key_match:
	    switch (i) {

	    case KEY_LOG:
	    case KEY_CON:
	    case KEY_LIB:
	    case KEY_FILT:
	    case KEY_MAP:
	    case KEY_DICT:
	    case KEY_DEF:
	    case KEY_REL:
	        field(&fsb,fterms) ;

	        if (fsb.flen == 0) goto badconfig ;

	        len = MIN(fsb.flen,(PATHLEN - 1)) ;

	        if ((REALNAMELEN - (nbp - namebuf)) < (len + 1))
	            goto badnamestore ;

	        strncpy(nbp,fsb.fp,len) ;

	        nbp[len] = '\0' ;

	        switch (i) {

	        case KEY_CON:
	            condirp = nbp ;
	            nbp += (len + 1) ;
	            break ;

	        case KEY_LOG:
	            logfname = nbp ;
	            nbp += (len + 1) ;
	            break ;

	        case KEY_LIB:
	            libdirp = nbp ;
	            nbp += (len + 1) ;
	            break ;

	        case KEY_FILT:
	            if (! f_filt) {

	                filtfname = nbp ;
	                nbp += (len + 1) ;
	            }
	            break ;

	        case KEY_MAP:
	            if (! f_map) {

	                mapfname = nbp ;
	                nbp += (len + 1) ;
	            }
	            break ;

	        case KEY_DICT:
	            if (! f_dict) {

	                dictfname = nbp ;
	                nbp += (len + 1) ;
	            }
	            break ;

	        case KEY_DEF:
	            if (sysname == NULL) {

	                sysname = nbp ;
	                nbp += (len + 1) ;
	            }
	            break ;

	        case KEY_REL:
	            if (f_release < 1) {

	                releasep = nbp ;
	                nbp += (len + 1) ;
	                f_release = 1 ;
	            }
	            break ;
	        }

	        break ;

	    case KEY_SYS:
	        if (nsys > NSYSTEM) {

	            bprintf(efp,
	                "%s: too many system entries in configuration file",
	                progname) ;

	            bprintf(efp," - extras ignored\n") ;

	        } else {

/* get the system name */

	            field(&fsb,fterms) ;

	            if ((fsb.flen == 0) || (fsb.term == '#'))
	                goto badconfig ;

	            len = MIN((NAMELEN - 1),fsb.flen) ;

	            strncpy(system[nsys].name,fsb.fp,len) ;

	            system[nsys].name[len] = '\0' ;

/* get the 'gtdev' field */

	            field(&fsb,fterms) ;

	            if ((fsb.flen == 0) || (fsb.term == '#')) goto badconfig ;

	            len = MIN((PATHLEN - 1),fsb.flen) ;

	            strncpy(system[nsys].gtdev,fsb.fp,len) ;

	            system[nsys].gtdev[len] = '\0' ;

/* get the 'gtdev2' field */

	            field(&fsb,fterms) ;

	            if (fsb.flen == 0) goto badconfig ;

	            len = MIN((PATHLEN - 1),fsb.flen) ;

	            strncpy(system[nsys].gtdev2,fsb.fp,len) ;

	            system[nsys].gtdev2[len] = '\0' ;

/* null out the remaining optional fields in this record */

	            system[nsys].filtfile[0] = '\0' ;
	            system[nsys].mapfile[0] = '\0' ;
	            system[nsys].dictfile[0] = '\0' ;

/* get the SAT filter file name, if exists */

	            field(&fsb,fterms) ;

	            if (fsb.flen == 0) goto sysrecdone ;

	            if ((fsb.flen == 1) && (fsb.fp[0] == '-')) {

	                len = MIN((PATHLEN - 1),fsb.flen) ;

	                strncpy(system[nsys].filtfile,fsb.fp,len) ;

	            } else {

	                len = MIN((PATHLEN - 1),fsb.flen) ;

	                strncpy(system[nsys].filtfile,fsb.fp,len) ;

	            }

	            system[nsys].filtfile[len] = '\0' ;

/* get the SAT mapping file name, if exists */

	            field(&fsb,fterms) ;

	            if (fsb.flen == 0) goto sysrecdone ;

	            len = MIN((PATHLEN - 1),fsb.flen) ;

	            strncpy(system[nsys].mapfile,fsb.fp,len) ;

	            system[nsys].mapfile[len] = '\0' ;

/* get the SAT dictionary file name, if exists */

	            field(&fsb,fterms) ;

	            if (fsb.flen == 0) goto sysrecdone ;

	            len = MIN((PATHLEN - 1),fsb.flen) ;

	            strncpy(system[nsys].dictfile,fsb.fp,len) ;

	            system[nsys].dictfile[len] = '\0' ;

/* done parsing this 'system' entry line */
sysrecdone:
	            nsys += 1 ;

	        } /* end if */

	        break ;

/* find our function name */
	    case KEY_FUN:
	        if (f_builtin) break ;

/* get the function options field */

	        field(&fsb,fterms) ;

	        if ((fsb.flen == 0) || (fsb.term == '#')) goto badconfig ;

	        cp = fsb.fp ;
	        len = fsb.flen ;

/* get the function name field */

	        field(&fsb,fterms) ;

	        if ((fsb.flen == 0) || (fsb.term == '#')) goto badconfig ;

	        if (strncmp(fsb.fp,funname,fsb.flen) == 0) {

/* process the option field */

	            funopt = 0 ;
	            while (len > 0) {

	                switch (c = *cp++) {

	                case 'l':
	                    funopt |= FO_LOCK ;
	                    break ;

	                case 'r':
	                    funopt |= FO_READ ;
	                    break ;

	                case 'w':
	                    funopt |= FO_WRITE ;
	                    break ;

	                case '-':
	                    break ;

	                default:
	                    bprintf(efp,
	                        "%s: unknown function option code - %c\n",
	                        progname,c) ;

	                } /* end switch */

	                len -= 1 ;

	            } /* end while */

/* read the quoted field constituting the command string */

	            funlen = quoted(funbuf,LINELEN,fsb.lp,fsb.rlen) ;

	            if (funlen <= 0) {

	                bflush(efp) ;

	                bprintf(efp,
	                    "%s: bad function command string ",
	                    progname) ;

	                bprintf(efp,
	                    "specified in configuration file\n") ;

	                bprintf(efp,
	                    " on line %d\n",line) ;

	                goto badconfig ;
	            }
	        }

	        break ;

	    case KEY_EXP:
	        bp = cp = ebp ;

/* get first part */

	        field(&fsb,fterms) ;

	        if ((fsb.flen == 0) || (fsb.term == '#')) goto badconfig  ;

	        if (fsb.flen > (ENVLEN - (bp - envbuf) - 2)) goto badconfig ;

	        strncat(bp,fsb.fp,fsb.flen) ;

	        bp += fsb.flen ;
	        strcat(bp,"=");

	        bp += 1 ;

/* get second part */

	        field(&fsb,fterms) ;

	        if (fsb.flen == 0) goto badconfig ;

	        if (fsb.flen > (ENVLEN - (bp - envbuf) - 1)) goto badconfig ;

	        strncat(bp,fsb.fp,fsb.flen) ;

	        bp += fsb.flen ;
	        *bp++ = '\0' ;

/* put it in the environment */

	        ebp = bp ;
	        putenv(cp) ;

	        if (nlenv < NLENV) lenviron[nlenv++] = cp ;

	        break ;

	    case KEY_WAIT:
	        field(&fsb,fterms) ;

	        if (fsb.flen == 0) goto badconfig ;

	        if (cfdec(fsb.fp,fsb.flen,&wtime) < 0) goto badconfig ;

	        break ;

	    default:
	        bprintf(efp,
	            "%s: bad case on switch for config key\n",
	            progname) ;

	        goto badconfig ;
	    }

	} /* end while */

	bclose(cfp) ;

	if (len < 0) {

	    bprintf(efp,
	        "%s: bad read from configuration file (%d)\n",
	        progname,len) ;

	    goto badret ;
	}

/* done with configuration file processing */

