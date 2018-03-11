/* vecstr_loadfile */

/* load strings from a file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will read (process) a file and put all of the strings
	found into the string (supplied) list, consisting of a VECSTR object.

	Synopsis:

	int vecstr_loadfile(vsp,fu,fname)
	vecstr		*vsp ;
	int		fu ;
	const char	fname[] ;

	Arguments:

	vsp		pointer to VECSTR object
	fu		flag specifying uniqueness
	fname		file to load

	Returns:

	>=0		number of elements loaded
	<0		error

	Notes:

        Why use FILEBUF over BFILE? Yes, FILEBUF is a tiny bit more lightweight
        than BFILE -- on a good day. But the real reason may be so that we don't
        need to load BFILE in code that resides very deep in a software stack if
        we don't need it -- like deep inside loadable modules. Anyway, just a
        thought!

        Why are we using FIELD as opposed to 'nextfield()' or something similar?
        Because our sematics are to process quoted strings as a single VECSTR
        entry!

	Note_on_uniqueness:

	The 'fu' argument:
		0=no_unique
		1=unique


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<filebuf.h>
#include	<field.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	FD_STDIN
#define	FD_STDIN	0
#endif

#define	DEFBUFLEN	1024

#define	TO_READ		-1		/* read timeout */


/* external subroutines */

extern int	vecstr_adduniq(vecstr *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* forward references */

static int	vecstr_loadfd(VECSTR *,int,int) ;
static int	vecstr_loadline(VECSTR *,int,const char *,int) ;


/* local structures */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x04, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int vecstr_loadfile(vecstr *vsp,int fu,cchar *fname)
{
	int		rs = SR_OK ;
	int		fd = FD_STDIN ;
	int		c = 0 ;
	int		f_opened = FALSE ;

	if (vsp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

/* try to open it */

	if (strcmp(fname,"-") != 0) {
	    if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	        fd = rs ;
	        f_opened = TRUE ;
	    }
	}

	if (rs >= 0) {
	    rs = vecstr_loadfd(vsp,fu,fd) ;
	    c = rs ;
	}

	if (f_opened && (fd >= 0)) {
	    u_close(fd) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadfile) */


/* local subroutines */


static int vecstr_loadfd(vecstr *vsp,int fu,int fd)
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		to = -1 ;
	int		c = 0 ;

	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    if (! S_ISDIR(sb.st_mode)) {
		FILEBUF	loadfile, *lfp = &loadfile ;
	        int	fbsize = DEFBUFLEN ;
	        int	fbo = 0 ;

	        if (S_ISREG(sb.st_mode)) {
	            int	fs = ((sb.st_size == 0) ? 1 : (sb.st_size & INT_MAX)) ;
	            int	cs ;
	            cs = BCEIL(fs,512) ;
	            fbsize = MIN(cs,1024) ;
	        } else {
	            to = TO_READ ;
	            if (S_ISSOCK(sb.st_mode)) fbo |= FILEBUF_ONET ;
	        }

	        if ((rs = filebuf_start(lfp,fd,0L,fbsize,fbo)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = filebuf_readline(lfp,lbuf,llen,to)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;

#if	CF_DEBUGS
	                debugprintf("vecstr_loadfile: line=>%t<\n",
	                    lbuf,strlinelen(lbuf,len,60)) ;
#endif

			if ((len > 0) && (lbuf[0] != '#')) {
			    rs = vecstr_loadline(vsp,fu,lbuf,len) ;
			    c += rs ;
			}

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = filebuf_finish(lfp) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (filebuf) */

	    } else {
	        rs = SR_ISDIR ;
	    }
	} /* end if (stat) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadfd) */


static int vecstr_loadline(VECSTR *vsp,int fu,const char *lbuf,int len)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {
		if (fl > 0) {
		    if (fu) {
			rs = vecstr_adduniq(vsp,fp,fl) ;
			if (rs < INT_MAX) c += 1 ;
		    } else {
			c += 1 ;
			rs = vecstr_add(vsp,fp,fl) ;
		    }
		} /* end if (got one) */
		if (fsb.term == '#') break ;
		if (rs < 0) break ;
	    } /* end while (fields) */
	    field_finish(&fsb) ;
	} /* end if (fields) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadline) */


