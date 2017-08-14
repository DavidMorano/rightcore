/* varsub_loadfile */

/* process a file with variable substitutions */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1994-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will read a file of variable substitutions and put them
	into the passed VARSUB object.

	Synopsis:

	int varsub_loadfile(vsp,fname)
	VARSUB		*vsp ;
	const char	fname[] ;

	Arguments:

	vsp		pointer to VARSUB object to accumulate results
	fname		file to process

	Returns:

	>=0		count of environment variables loaded
	<0		bad


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>

#include	"varsub.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif

#undef	BUFLEN
#define	BUFLEN		LINEBUFLEN


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	vstrkeycmp(char **,char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */


/* local structures */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int varsub_loadfile(VARSUB *vsp,cchar *fname)
{
	FIELD		fsb ;
	bfile		vfile, *vfp = &vfile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (vsp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("varsub_loadfile: fname=%s\n",fname) ;
#endif

	if ((fname == NULL) || (fname[0] == '\0') || (fname[0] == '-'))
	    fname = BFILE_STDIN ;

	if ((rs = bopen(vfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		kl ;
	    int		bl, cl ;
	    const char	*cp ;
	    const char	*kp ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    char	buf[BUFLEN + 1] ;
	    char	*bp ;

	    while ((rs = breadlines(vfp,lbuf,llen,NULL)) > 0) {
	        int	len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

	        cp = lbuf ;
	        cl = len ;
	        while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if ((cp[0] == '\0') || (cp[0] == '#'))
	            continue ;

	        if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	            if ((kl = field_get(&fsb,fterms,&kp)) > 0) {
	                int	blen = BUFLEN ;

	                if ((kl == 6) && (strncasecmp(kp,"export",kl) == 0)) {
	                    kl = field_get(&fsb,fterms,&kp) ;
	                }

	                bp = buf ;
	                while ((blen > 0) &&
	                    ((bl = field_sharg(&fsb,fterms,bp,blen)) >= 0)) {

	                    if (bl > 0)
	                        bp += bl ;

	                    blen = (buf + BUFLEN - bp) ;

	                    if (fsb.term == '#') break ;
	                } /* end while */

	                *bp = '\0' ;
	                rs = varsub_add(vsp,kp,kl,buf,(bp - buf)) ;
			if (rs < INT_MAX) c += 1 ;

#if	CF_DEBUGS
	                debugprintf("varsub_loadfile: varsub_add() rs=%d\n",
				rs) ;
#endif

	            } /* end if (have a variable keyname) */

	            rs1 = field_finish(&fsb) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (field) */

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = bclose(vfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open-file) */

#if	CF_DEBUGS
	debugprintf("varsub_loadfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varsub_loadfile) */


