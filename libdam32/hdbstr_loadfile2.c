/* hdbstr_loadfile2 */

/* load strings from a file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

        This subroutine will read (process) a file and put all of the strings
        found into the string (supplied) list, consisting of an HDBSTR object.

	Note: 
        This is NOT the same as 'vecstr_loadfile(3dam)'! This subroutine only
        reads the first TWO strings on the line and puts them into the hash DB
        as key-value pairs! If there is only one string on a line, it is a key
        without a value.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<hdbstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#undef	ITEMLEN
#define	ITEMLEN		MAX(LINEBUFLEN,(2 * MAXPATHLEN))


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x04, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int hdbstr_loadfile2(hsp,fname)
HDBSTR	*hsp ;
char	fname[] ;
{
	bfile	loadfile, *lfp = &loadfile ;

	FIELD	fsb ;

	int	rs ;
	int	len ;
	int	kl, vl ;
	int	n = 0 ;

	const char	*kp, *vp ;

	char	linebuf[ITEMLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("hdbstr_loadfile2: ent=%s\n",fname) ;
#endif

	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

/* try to open it */

	rs = bopen(lfp,fname,"r",0666) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("hdbstr_loadfile2: opened\n") ;
#endif

	while ((rs = breadline(lfp,linebuf,ITEMLEN)) > 0) {
	    len = rs ;

	    if (linebuf[len - 1] == '\n') len -= 1 ;

#if	CF_DEBUGS
	    debugprintf("hdbstr_loadfile2: line> %t\n",linebuf,len) ;
#endif

	    if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

	        if ((kl = field_get(&fsb,fterms,&kp)) > 0) {

	            vp = NULL ;
	            vl = 0 ;
	            if ((fsb.term != '#') && 
	                ((vl = field_get(&fsb,fterms,&vp)) >= 0))

	            n += 1 ;
	            rs = hdbstr_add(hsp,kp,kl,vp,vl) ;

	        } /* end if (got one) */

	        field_finish(&fsb) ;
	    } /* end if (fields) */

	            if (rs < 0) break ;
	} /* end while (reading lines) */

	bclose(lfp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("hdbstr_loadfile2: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (hdbstr_loadfile2) */



