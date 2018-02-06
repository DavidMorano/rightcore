/* authfile */

/* subroutine to read the authorization file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-12, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine reads the authorization file (if supplied by the
        invoking user) and returns (possibly) a username, a password, one, none,
        or both of these.

	Synospsis:

	int authfile(char *username,char *password,cchar *fname)

	Arguments:

	username	result sername
	password	result passwordA
	fname		authorization file

	Returns:

	>0		success
	==		not-successful
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
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

#ifndef	MAXAUTHLEN
#define	MAXAUTHLEN	64
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,cchar *,int) ;


/* forward subroutines */


/* external variables */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x02, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int authfile(char *username,char *password,cchar *fname)
{
	bfile		afile, *afp = &afile ;
	int		rs ;
	int		rs1 ;

	if (fname == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;
	if (password == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	username[0] = '\0' ;
	password[0] = '\0' ;

	if ((rs = bopen(afp,fname,"r",0666)) >= 0) {
	    FIELD	fsb ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		type ;
	    int		fl ;
	    const char	*fp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(afp,lbuf,LINEBUFLEN)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

	        if ((len > 0) && (lbuf[0] != '#')) {
	            if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	                while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {
	                    if (fl > 0) {
	                        type = fp[0] ;
	                        if ((fl = field_get(&fsb,fterms,&fp)) > 0) {
	                            switch (type) {
	                            case 'u':
	                            case 'l':
	                                strdcpy1w(username,MAXAUTHLEN,fp,fl) ;
	                                break ;
	                            case 'p':
	                                strdcpy1w(password,MAXAUTHLEN,fp,fl) ;
	                                break ;
	                            } /* end switch */
	                        } /* end if (had a value for the key) */
	                    } /* end if (positive) */
	                    if (fsb.term == '#') break ;
	                } /* end while (parsing fields) */
	                field_finish(&fsb) ;
	            } /* end if (field) */
		} /* end if (positive) */

	        if ((username[0] != '\0') && (password[0] != '\0')) break ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = bclose(afp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

	return rs ;
}
/* end subroutine (authfile) */


