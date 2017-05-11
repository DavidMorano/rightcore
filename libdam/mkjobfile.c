/* mkjobfile */

/* make a temporary job file */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_BADTOOLONG	1		/* return "bad" if dirname too long */


/* revision history:

	= 1998-08-10, David A­D­ Morano
	This subroutine was originally written.

	= 1999-04-19, David A­D­ Morano
	I made a correction for short filenames when the host name is less than
	4 characters long!

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Make a file which serves as a job submital file.  The file will have a
	name which is, as nearly as possible, totally unique throughout the
	world when it is always made by this routine.

	Synopsis:

	int mkjobfile(tmpdname,mode,outname)
	const char	tmpdname[] ;
	int		mode ;
	char		outname[] ;

	Arguments:

	- buffer holding the name of a directory to make the file in
	- file creation mode for created file
	- a buffer which the user supplies to receive the resultant file name

	Returns:

	>=0	GOOD
	<0	BAD


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#define	O_MODE		(O_CREAT | O_EXCL | O_RDWR)
#define	MAXLOOP		100

#define	NAMEBREAK	4


/* external subroutines */

extern int	getnodename(char *,int) ;
extern int	base64_e() ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern uchar	base64_dt[], base64_et[] ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkjobfile(tmpdname,mode,outname)
const char	tmpdname[] ;
mode_t		mode ;
char		outname[] ;
{
	struct ustat	sb ;
	const time_t	daytime = time(NULL) ;
	ulong		nodev ;
	ulong		rv0, rv1 ;
	int		rs ;
	int		fd ;
	int		loop = 0 ;
	int		i, sl, cl ;
	int		blen, rlen ;
	int		len = 0 ;
	int		f_exit = FALSE ;
	char		nodename[NODENAMELEN + 1] ;
	char		cvtbuf[16 + 1] ;
	char		namebuf[24 + 1] ;
	char		*bnp ;
	char		*sp ;
	char		*cp ;

#if	CF_DEBUGS
	debugprintf("mkjobfile: dname=%s\n",tmpdname) ;
#endif

	if (outname == NULL)
	    return SR_FAULT ;

	if ((tmpdname == NULL) || (tmpdname[0] == '\0'))
	    tmpdname = getenv(VARTMPDNAME) ;

	if (tmpdname == NULL)
	    tmpdname = TMPDNAME ;

	outname[0] = '\0' ;
	rs = u_stat(tmpdname,&sb) ;
	if (rs < 0) goto bad0 ;

	if (! S_ISDIR(sb.st_mode)) {
	    rs = SR_NOTDIR ;
	    goto bad0 ;
	}

/* check that it isn't too long */

	sp = outname ;

#if	CF_BADTOOLONG
	sl = strlen(tmpdname) ;

	if ((sl == 0) || (sl > (MAXPATHLEN - 6)) || 
	    ((sl == 1) && (sp[0] == '/'))) {

	    tmpdname = TMPDNAME ;
	}
#endif /* CF_BADTOOLONG */

/* put it in the output buffer */

	rlen = (MAXPATHLEN - 2) ;
	cp = strwcpy(outname,tmpdname,rlen) ;

	if (((cp - outname) > 0) && (cp[-1] != '/'))
	    *cp++ = '/' ;

	rlen -= (cp - outname) ;
	sp = cp ;

#if	CF_DEBUGS
	debugprintf("mkjobfile: outname=%t\n",outname,rlen) ;
#endif

/* let's go! */

	rs = getnodename(nodename,NODENAMELEN) ;
	if (rs < 0) goto bad0 ;

	cp = strwcpy(sp,nodename,rlen) ;

	while ((cp - sp) < NAMEBREAK)
	    *cp++ = '_' ;

	*cp = '\0' ;
	cl = (cp - sp) ;

/* copy anything over NAMEBREAK characters into the 'nodev' variable */

	nodev = 0 ;
	if (cl > NAMEBREAK)
	    strwcpy((char *) &nodev,(sp + NAMEBREAK),MIN(cl,sizeof(ulong))) ;

	rlen -= NAMEBREAK ;
	sp += NAMEBREAK ;

/* set up to loop */

	blen = (sp - outname) ;
	bnp = sp ;

	len = blen + 10 ;

#if	CF_DEBUGS
	debugprintf("mkjobfile: early len=%d\n",blen) ;
#endif

/* go through the loops! */

	cp = bnp ;
	while ((rs >= 0) && (loop < MAXLOOP) && (! f_exit)) {

#if	CF_DEBUGS
	    debugprintf("mkjobfile: top of while @ loop %d\n",loop) ;
#endif

	    f_exit = TRUE ;
	    rs = SR_TIMEDOUT ;
	    rv0 = nodev ^ ((daytime >> 18) & 0x0003FFFF) ;
	    rv1 = ((daytime << 14) ^ loop) << 4 ;

#if	CF_DEBUGS
	    debugprintf("mkjobfile: %08X %08X\n",rv0,rv1) ;
#endif

	    for (i = 0 ; i < 4 ; i += 1)
	        cvtbuf[i] = (rv0 >> ((3 - i) * 8)) ;

	    for (i = 0 ; i < 4 ; i += 1)
	        cvtbuf[i + 4] = (rv1 >> ((3 - i) * 8)) ;

#if	CF_DEBUGS
	    debugprintf("mkjobfile:") ;
	    for (i = 0 ; i < 8 ; i += 1)
	        debugprintf(" %02X",cvtbuf[i]) ;
	    debugprintf("\n") ;
#endif /* CF_DEBUGS */

/* put it all together */

	    base64_e(cvtbuf,8,namebuf) ;

	    strwcpy(bnp,namebuf,10) ;

/* ensure that there are no '/' characters in the result! */

	    sp = bnp ;
	    while ((cp = strchr(sp,'/')) != NULL) {

	        *cp = '_' ;
	        sp = cp + 1 ;

	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("mkjobfile: fname=%s\n",outname) ;
#endif

/* OK, try and make the thing */

	    if (S_ISDIR(mode)) {

	        if ((rs = u_mkdir(outname,mode)) < 0) {

	            if ((rs == SR_EXIST) || (rs == SR_INTR))
	                f_exit = FALSE ;

	        } /* end if */

	    } else if (S_ISREG(mode) || ((mode & (~ 0777)) == 0)) {

	        if ((rs = u_open(outname,O_MODE,mode & 0777)) < 0) {

	            if ((rs == SR_EXIST) || (rs == SR_INTR))
	                f_exit = FALSE ;

	        } else {

	            fd = rs ;
	            u_close(fd) ;

	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("mkjobfile: u_open() rs=%d\n",
	            rs) ;
#endif

	    } else if (S_ISFIFO(mode)) {

#if	CF_DEBUGS
	        debugprintf("mkjobfile: got a FIFO\n") ;
#endif

	        if ((rs = u_mknod(outname,mode,0)) < 0) {

	            if ((rs == SR_EXIST) || (rs == SR_INTR))
	                f_exit = FALSE ;

#if	CF_DEBUGS
	            debugprintf("mkjobfile: u_mknod() rs=%d\n",
	                rs) ;
#endif

	        } /* end if */

	    } else if (S_ISSOCK(mode)) {

	        rs = SR_NOTSUP ;

	    } else {

#if	CF_DEBUGS
	        debugprintf("mkjobfile: got an unknown request mode=%08o\n",
	            mode) ;
#endif

	        rs = SR_INVALID ;

	    } /* end if */

	    loop += 1 ;

	} /* end while (trying for success) */

	if ((rs >= 0) && (loop >= MAXLOOP))
	    rs = SR_TIMEDOUT ;

ret0:
	return (rs >= 0) ? len : rs ;

/* bad stuff */
bad0:
	goto ret0 ;
}
/* end subroutine (mkjobfile) */


