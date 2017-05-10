/* attachentry */

/* extra subroutines for the 'attachentry' object */


#define	F_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history :

	= 1996-03-01, David A­D­ Morano
        The subroutine was written from scratch to do what the previous program
        by the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        These subroutines form extras way to manipulate 'attachentry' objects.
        See the code for details!


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bio.h>
#include	<baops.h>
#include	<field.h>
#include	<ema.h>
#include	<mimetypes.h>
#include	<paramopt.h>
#include	<pcsconf.h>		/* for 'PCS_MSGIDLEN' */
#include	<localmisc.h>

#include	"attach.h"
#include	"headadds.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		200
#endif

#define	CE_7BIT		0
#define	CE_8BIT		1
#define	CE_BINARY	2
#define	CE_BASE64	3


/* external subroutines */

extern int	pcsmsgid(char *,char *,int) ;

extern char	*malloc_str(char *), *malloc_strn(char *,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */

static const char	*encodings[] = {
	"7bit",
	"8bit",
	"binary",
	"base64",
	NULL
} ;


/* exported subroutines */


int attachentry_code(ep,tmpdir)
ATTACH_ENT	*ep ;
char		tmpdir[] ;
{
	int	f_plaintext = FALSE ;

	int	code = -1 ;


#if	F_DEBUGS
	eprintf("attachentry_code: entered, file=%s type=%s\n",
	    ep->filename,ep->type) ;
#endif

	if (ep->type != NULL) {

	    if ((strcasecmp(ep->type,"text") == 0) &&
	        ((ep->subtype == NULL) || 
	        (strcasecmp(ep->subtype,"plain") == 0)))
	        f_plaintext = TRUE ;

	} else
	    f_plaintext = TRUE ;

	if (! f_plaintext)
	    code = CE_BASE64 ;

#if	F_DEBUGS
	eprintf("attachentry_code: f_plaintext=%d default_code=%d\n",
	    f_plaintext,code) ;
#endif

	if (ep->encoding == NULL) {

	    if (f_plaintext) {

	        code = attachentry_analyze(ep,tmpdir) ;

	        if (code >= CE_7BIT)
	            ep->encoding = malloc_str(encodings[code]) ;

	    } else {

	        code = CE_BASE64 ;
	        ep->encoding = malloc_str("base64") ;

	    }

	} else
	    code = matstr(encodings,ep->encoding,-1) ;

	if (code < 0) {

	    if (ep->encoding != NULL)
	        free(ep->encoding) ;

	    code = CE_BASE64 ;
	    ep->encoding = malloc_str("base64") ;

	}

#if	F_DEBUGS
	eprintf("attachentry_code: code=%d encoding=%s\n",
	    code,ep->encoding) ;
#endif

	ep->code = code ;
	return code ;
}
/* end subroutine (attachentry_code) */


int attachentry_analyze(ep,tmpdir)
ATTACH_ENT	*ep ;
char		tmpdir[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	auxfile, *afp = &auxfile ;

	struct ustat	sb ;

	int	clen = 0 ;
	int	rs, i ;
	int	f_needaux = FALSE ;
	int	f_lines = TRUE ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	auxfname[MAXPATHLEN + 1] ;


	if ((ep->filename == NULL) || (ep->filename[0] == '\0') ||
	    (ep->filename[0] == '-'))
	    rs = bopen(ifp,BIO_STDIN,"dr",0666) ;

	else
	    rs = bopen(ifp,ep->filename,"r",0666) ;

	if (rs < 0)
	    return rs ;

	rs = bcontrol(ifp,BC_STAT,&sb) ;

	f_needaux = TRUE ;
	if ((rs >= 0) && S_ISREG(sb.st_mode)) {

	    ep->clen = (int) sb.st_size ;
	    f_needaux = FALSE ;

	}

#if	F_DEBUGS
	eprintf("attachentry_analyze: needaux=%d\n",f_needaux) ;
#endif

	if (f_needaux) {

	    (void) bufprintf(tmpfname,MAXPATHLEN,"%s/mkmsgXXXXXXXXX",
	        tmpdir) ;

	    rs = mktmpfile(tmpfname,0660,auxfname) ;

#if	F_DEBUGS
	    eprintf("attachentry_analyze: mktmpfile rs=%d tmpfname=%s\n",
	        rs,tmpfname) ;
#endif

	    if (rs < 0)
	        goto bad1 ;

	    if ((ep->auxfname = malloc_str(auxfname)) == NULL)
	        goto bad1 ;

	    rs = bopen(afp,ep->auxfname,"wct",0666) ;

	} /* end if (needed an auxillary file) */

	if (rs >= 0) {

	    int	code = 0 ;
	    int	lines = 0 ;
	    int	len ;

	    char	buf[BUFLEN + 1] ;


#if	F_DEBUGS
	    eprintf("attachentry_analyze: looping through file\n") ;
#endif

	    while ((len = bgetline(ifp,buf,BUFLEN)) > 0) {

	        if (code < 2) {

	            for (i = 0 ; i < len ; i += 1) {

#if	F_DEBUGS
	                eprintf("attachentry_analyze: c=%02X (%c)\n",
	                    buf[i],
	                    buf[i]) ;
#endif
	                if (isbinary(buf[i])) break ;

	                if ((code < 1) && is8bit(buf[i])) {

#if	F_DEBUGS
	                    eprintf("attachentry_analyze: got a 8bit c=%02X\n",
	                        buf[i]) ;
#endif

	                    code = 1 ;

	                }

	            } /* end for */

	            if ((i < len) && (isbinary(buf[i]))) {

#if	F_DEBUGS
	                eprintf("attachentry_analyze: got a binary c=%02X\n",
	                    buf[i]) ;
#endif

	                code = 2 ;

	            }

	        } /* end if */

	        if (! f_needaux) {

	            if ((! f_lines) && (code == 2))
	                break ;

	        } else
	            (void) bwrite(afp,buf,len) ;

	        clen += len ;
	        if (buf[len - 1] == '\n')
	            lines += 1 ;

	    } /* end while */

	    rs = code ;
	    ep->lines = lines ;

	} /* end if */

	if (f_needaux) {

	    if (ep->clen < 0)
	        ep->clen = clen ;

	    bclose(afp) ;

	} else
	    bseek(ifp,0L,SEEK_SET) ;

	bclose(ifp) ;

#if	F_DEBUGS
	eprintf("attachentry_analyze: exiting OK rs=%d\n",rs) ;
#endif

	return rs ;

/* bad return */
bad1:
	bclose(ifp) ;

#if	F_DEBUGS
	eprintf("attachentry_analyze: exiting BAD rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (attachentry_analyze) */



