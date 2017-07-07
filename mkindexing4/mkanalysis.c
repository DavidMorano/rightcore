/* mkanalysis */

/* invert the data (keys to pointers) */


#define	CF_DEBUG 	0		/* compile-time debugging */
#define	CF_TESTSLEEP	0


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int mkanalysis(pip,aip,dbname)
	struct proginfo	*pip ;
	struct arginfo	*aip ;
	const char	dbname[] ;

	Arguments:

	- pip		program information pointer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<hdbstr.h>
#include	<vecint.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TABLEN
#define	TABLEN		(2 * 1024)
#endif

#define	DEFLISTLEN	8


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfname2(char *,const char *,const char *) ;
extern int	mkfname3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;

extern int	procanalysis(struct proginfo *,vecint *,int,bfile *,
			struct keyinfo *,HDBSTR *,const char *) ;
extern int	mkhashfile(struct proginfo *,vecint *,int, struct keyinfo *,
			const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkanalysis(pip,aip,dbname)
struct proginfo	*pip ;
struct arginfo	*aip ;
const char	dbname[] ;
{
	struct keyinfo	ki ;

	HDBSTR	stab ;

	bfile	hashfile, tagfile ;

	vecint	*table ;

	uint	c_tagref = 0 ;

	int	rs, rs1, n, i, j, pan, cl ;
	int	ai ;
	int	tablen ;
	int	size ;
	int	f_bad ;
	int	f ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	hashfname[MAXPATHLEN + 1] ;
	char	tagfname[MAXPATHLEN + 1] ;
	char	*cp ;


	if (aip == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("mkanalysis: dbname=%s\n",dbname) ;
#endif

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    return SR_FAULT ;

/* initialize the data structures we need */

	tablen = (pip->tablen > 0) ? pip->tablen : TABLEN ;
	size = tablen * sizeof(vecint) ;
	rs = uc_malloc(size,&table) ;

	if (rs < 0)
	    goto ret0 ;

	for (i = 0 ; i < tablen ; i += 1) {

	    rs = vecint_start((table + i),DEFLISTLEN,VECINT_PORDERED) ;

	    if (rs < 0)
	        break ;

	}

	if (rs < 0) {

	    for (j = 0 ; j < i ; j += 1)
	        vecint_finish(table + j) ;

	    goto ret1 ;
	}

/* can we open the two output data files? */

	mkfname3(hashfname,dbname,FE_HASH,ENDIANSTR) ;

	rs = u_creat(hashfname,0666) ;

	if (rs >= 0)
		u_close(rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "mkanalysis: u_creat() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

	mkfname3(tagfname,dbname,FE_TAG,ENDIANSTR) ;

	sncpy2(tmpfname,MAXPATHLEN,tagfname,"n") ;

	rs = bopen(&tagfile,tmpfname,"wct",0666) ;

	if (rs < 0)
	    goto ret3 ;

	rs = hdbstr_start(&stab,6000) ;

	if (rs < 0)
		goto ret4 ;

	memset(&ki,0,sizeof(struct keyinfo)) ;

	ki.minlen = INT_MAX ;

/* process the positional arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "mkanalysis: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    f = (ai <= aip->ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (! f) continue ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	            debugprintf("mkanalysis: ai=%d pan=%u arg=%s\n",
	                ai,pan,aip->argv[ai]) ;
#endif

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: processing input file=%s\n",
	                pip->progname,aip->argv[ai]) ;

		cp = aip->argv[ai] ;
	        pan += 1 ;
	        rs = procanalysis(pip,table,tablen,&tagfile,&ki,&stab,
			cp) ;

		if (rs > 0)
			c_tagref += rs ;

	        if (rs < 0) {

	            if (*cp == '-')
	                cp = "*stdinput*" ;

	            bprintf(pip->efp,
			"%s: error processing input file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;

	        }

	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkanalysis: done processing argument files\n") ;
#endif

/* process any files in the argument filename list file */

	if ((rs >= 0) && (aip->afname != NULL) && 
		(aip->afname[0] != '\0')) {

		bfile	argfile ;


	    if (strcmp(aip->afname,"-") != 0)
	        rs = bopen(&argfile,aip->afname,"r",0666) ;

	    else
	        rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {

			len = rs ;
	            if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	            linebuf[len] = '\0' ;
			cp = linebuf ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

		pan += 1 ;
	        rs = procanalysis(pip,table,tablen,&tagfile,&ki,&stab,
			cp) ;

		if (rs > 0)
			c_tagref += rs ;

	        if (rs < 0) {

	            if (*cp == '-')
	                cp = "*stdinput*" ;

	            bprintf(pip->efp,
	                "%s: error processing input file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;

	        } /* end if */

			if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(&argfile) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done processing the argument file list\n") ;
#endif

	    } else if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(pip->efp,"%s: \trs=%d argfile=%s\n",
			pip->progname,rs,aip->afname) ;

	    }

	} /* end if (argument file) */

	if ((rs >= 0) && (pan == 0)) {

		cp = "-" ;

		pan += 1 ;
	        rs = procanalysis(pip,table,tablen,&tagfile,&ki,&stab,
			cp) ;

		if (rs > 0)
			c_tagref += rs ;

	        if (rs < 0) {

	            if (*cp == '-')
	                cp = "*stdinput*" ;

	            bprintf(pip->efp,
	                "%s: error processing input file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;

	        } /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("mkanalysis: done processing STDIN\n") ;
#endif

	} /* end if (argument file) */

	if ((pip->debuglevel > 0) && (rs >= 0)) {

		rs1 = hdbstr_count(&stab) ;

		if (rs1 >= 0)
		bprintf(pip->efp,"%s: unique keys=%u\n",
			pip->progname,rs1) ;

		else
		bprintf(pip->efp,"%s: unique keys=NA\n",
			pip->progname) ;

		bprintf(pip->efp,"%s: tag references=%u\n",
			pip->progname,c_tagref) ;

	}


/* create the output hash file */

#if	CF_TESTSLEEP
	sleep(20) ;
#endif

	{


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	        debugprintf("mkanalysis: min=%u max=%u\n",ki.minlen,ki.maxlen) ;
	        debugprintf("mkanalysis: tablen=%d\n",tablen) ;
	}
#endif

		rs = mkhashfile(pip,table,tablen,&ki,
			hashfname) ;

		if (pip->debuglevel > 0) {

			if (rs >= 0) {

				double	fn, fd, percent ;


				fn = rs ;
				fd = tablen ;
				percent = 100.0 * fn / fd ;
				bprintf(pip->efp,
				"%s: hash table occupancy=%5.1f%%\n",
				pip->progname,percent) ;

			} else
				bprintf(pip->efp,
				"%s: error in creating hash file (%d)\n",
				pip->progname,rs) ;

		} /* end if (debugging turned on) */

	} /* end block */


/* close the output files */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("mkanalysis: files processed=%u\n",pan) ;
	}
#endif

	f_bad = (rs < 0) ;

/* return here */
ret5:
	hdbstr_finish(&stab) ;

ret4:
	rs1 = bclose(&tagfile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mkanalysis: tagfile bclose() rs=%d\n",rs1) ;
#endif

	if (! f_bad)
		rs = u_rename(tmpfname,tagfname) ;

ret3:
	if (f_bad && (tmpfname[0] != '\0'))
		u_unlink(tmpfname) ;

ret2:
	for (i = 0 ; i < tablen ; i += 1)
	    vecint_finish(table + i) ;

ret1:
	if (table != NULL)
	uc_free(table) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mkanalysis: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (mkanalysis) */



/* LOCAL SUBROUTINES */



