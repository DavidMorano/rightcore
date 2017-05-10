/* redomsg */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
		David A.D. Morano
 *									


 * 'redo_message' creates a new temproary file, puts the current state of
 * headers into it, copies the rest of the old temp file into the new one
 * and then unlinks the old tempfile and returns the name of the new tempfile
 * in tempfile


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>
#include	<errno.h>

#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"header.h"


/* local defines */

#define	MSGFNAME	tempfile


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	writeheader() ;


/* external variables */

extern struct global	g ;


/* forward references */


/* exported subroutines */


int redo_message()
{
	bfile	msgfile, *mfp = &msgfile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	*fpa[3] ;

	int	rs ;
	int	len, l ;
	int	f_bol, f_eol ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	linebuf[LINELEN + 1] ;


	if ((rs = mktmpfile( tmpfname, 0644, "/tmp/smailXXXXXXXXX")) < 0) {

	    bprintf(g.efp,"%s: can't create temporary file\n",
	        g.progname) ;

	    logfile_printf(&g.lh,
		"redo_message: ERROR couldn't make TMPFILE\n") ;

	    logfile_printf(&g.eh,
		"redo_message: ERROR couldn't make TMPFILE\n") ;

	    return BAD ;
	}

	if ((rs = bopen(mfp,MSGFNAME,"r",0664)) < 0) {

	    unlink(tmpfname) ;

	    bprintf(g.efp,"%s: can't open message file\n",
	        g.progname) ;

	    logfile_printf(&g.lh,"redo_message: couldn't open 'tempfile'\n") ;

	    logfile_printf(&g.eh,"redo_message: couldn't open tempfile=%s (rs %d)\n",
	        MSGFNAME,rs) ;

	    rmtemp(TRUE,"redomsg") ;

	    return BAD ;
	}

	if ((rs = bopen(tfp,tmpfname,"wct",0664)) < 0) {

	    unlink(tmpfname) ;

	    bprintf(g.efp,"%s: can't open temp file\n",
	        g.progname) ;

	    logfile_printf(&g.lh,"redo_message: couldn't open the 'tempfile'\n") ;

	    logfile_printf(&g.eh,"redo_message: couldn't open the 'tempfile'\n") ;

	    rmtemp(TRUE,"redomsg") ;

	    return BAD ;
	}

#if	CF_DEBUG
	            if (g.f.debug)
	fileinfo(tempfile,"redo_message enter") ;
#endif

/* we survived making or opening files ! */

	ambiguous = 0 ;
	chown(tmpfname,g.uid,g.gid_mail) ;

/* don't let anyone snoop !! */

	chmod(tmpfname,0640) ;

/* start writing */

#if	CF_DEBUG
	            if (g.f.debug)
	logfile_printf(&g.eh,"redo_message: about to write the header\n") ;
#endif

	if ((rs = writeheader(tfp, TRUE)) < 0) {

	    unlink(tmpfname) ;

	    bclose(mfp) ;

	    bclose(tfp) ;

	    logfile_printf(&g.lh,
	        "redo_message: bad filecopy in 'redomsg' at 'writeheader'\n") ;

	    logfile_printf(&g.eh,
	        "redo_message: bad filecopy in 'redomsg' at 'writeheader'\n") ;

	    return rs ;
	}

/* skip the headers including the blank line */

#if	CF_DEBUG
	            if (g.f.debug)
	logfile_printf(&g.eh,"redo_message: about to get-rid of the old headers\n") ;
#endif

	len = 0 ;
	f_bol = TRUE ;
	while ((l = breadline(mfp,linebuf,LINELEN)) > 0) {

	    linebuf[l] = '\0' ;
	    len += l ;

	    f_eol = FALSE ;
	    if (linebuf[l - 1] == '\n') f_eol = TRUE ;

	    if (f_bol) {

/* some people are used to a much more lenient end-of-headers definition */

#ifdef	COMMENT
	        if (linebuf[0] == '\n') break ;
#else
	        if (! isheader(linebuf)) break ;
#endif

	    }

	    f_bol = f_eol ;

	} /* end while */

/* write out the last line if it was not a header line */

	if ((l > 0) && (linebuf[0] != '\n')) {

		bwrite(tfp,linebuf,l) ;

	}

#if	CF_DEBUG
	            if (g.f.debug)
	logfile_printf(&g.eh,"redo_message: %d header bytes skipped, EOF=%d\n",
	    len,l == 0) ;
#endif

/* copy the message body to the temporary file */

#if	CF_DEBUG
	            if (g.f.debug)
	logfile_printf(&g.eh,"redo_message: about to copy the message body\n") ;
#endif

	len = 0 ;
	while ((l = bcopyblock(mfp,tfp,BUFLEN)) > 0) len += l ;

	bclose(mfp) ;

	bclose(tfp) ;

	logfile_printf(&g.eh,"redo_message: %d body bytes copied\n",len) ;

	if (l < 0) {

	    rs = l ;
	    unlink(tmpfname) ;

	    logfile_printf(&g.lh,
	        "redo_message: bad filecopy at 'bcopyblock' (rs %d)\n",
	        rs) ;

	    logfile_printf(&g.eh,
	        "redo_message: bad filecopy at 'bcopyblock' (rs %d)\n",
	        rs) ;

	    return rs ;
	}

	unlink(tempfile) ;

	strcpy(tempfile,tmpfname) ;

#if	CF_DEBUG
	            if (g.f.debug)
	fileinfo(tempfile,"redo_message exit") ;
#endif

	return rs ;
}
/* end subroutine (redomsg) */


