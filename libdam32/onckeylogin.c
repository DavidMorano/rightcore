/* onckeylogin */

/* key-login into the system using ONC secure services */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
        This subroutine was written to deal with ONC key authorization issues
        when using Solaris 2.x.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is a sort of "all in one" key-login subroutine.  We
	will get the user's ONC netname, get the encrypted private key from the
	key database, decrypt it with the supplied password (the missing
	piece), and give the resulting private key to the KEYSERV server.

	Synopsis:

	int onckeylogin(passwd)
	const char	passwd[] ;

	Returns:

	>=0	OK
	<0	error


	Program notes:

	The 'key_setnet()' subroutine is part of the KEYSERV version 2 API.
	There does not appear to be a manual page on it so we guess at its use
	where necessary!  It must be used by 'login(1)', 'keylogin(1)', and
	other such programs for setting the ONC netname on login.

	The 'key_setnet()' subroutine returns are:

	1	succeeded in setting ONC private key to KEYSERV
	-1	failed to set ONC private key with KEYSERV


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"stubrpc.h"


/* local defines */

#define	PASSPHRASELEN	8		/* maximum current allowed */


/* external subroutines */


/* exported subroutines */


int onckeylogin(cchar *passwd)
{
	key_netstarg	sna ;
	int		rs = SR_OK ;
	int		f_netname ;
	char		netname[MAXNETNAMELEN + 1] ;

	if (passwd == NULL)
	    return SR_FAULT ;

	sna.st_netname = netname ;
	if ((f_netname = getnetname(netname)) == 0) {
	    int		rc ;

#if	CF_DEBUGS
	    debugprintf("onckeylogin: netname=%s\n",netname) ;
#endif

	    if ((rc = key_secretkey_is_set()) == 0) {
	        char		netname3[MAXNETNAMELEN + 1] ;
	        char		passphrase[PASSPHRASELEN + 1] ;
	        char		*nnp = netname ;

	        strncpy(passphrase, passwd,PASSPHRASELEN) ;

	        passphrase[PASSPHRASELEN] = '\0' ;/* truncate at maximum */
	        strcpy(netname3,nnp) ;

#ifdef	COMMENT
	        memset(&sna,0,sizeof(key_netstarg)) ;
#endif

	        memset(sna.st_priv_key,0,HEXKEYBYTES) ;
	        sna.st_pub_key[0] = '\0' ;
	        sna.st_netname = netname3 ;

	        if (getsecretkey(netname,sna.st_priv_key,passphrase) &&
	            (sna.st_priv_key[0] != '\0')) {

/* decrypted private ONC key, give it to KEYSERV */

	            if ((rs = key_setnet(&sna)) < 0)
	                rs = SR_ACCESS ;

/* destroy the private key since we do not really need or want it! */

	            memset(sna.st_priv_key,0,HEXKEYBYTES) ;

	        } /* end if (decrypted and retrieved private key) */

	        memset(passphrase,0,PASSPHRASELEN) ;

	    } /* end if (key-is-already-set) */
	} else
	    rs = SR_NOTSUP ;

	return rs ;
}
/* end subroutine (onckeylogin) */


