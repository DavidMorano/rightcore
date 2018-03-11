/* onckeygetset */

/* get and set the ONC private key */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
        This subroutine was written to deal with ONC key authorization issues
        when using Solaris 2.x.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will decrypt the private key by getting the encrypted
	version from the key database and then decrypting it.  This is "getting
	the key."  We then give the decrypted private key to the key-server for
	subsequent (whatever the user wants) ONC secure operations.

	Synopsis:

	int onckeygetset(netname,passwd)
	const char	netname[] ;
	const char	passwd[] ;

	Arguments:

	netname		user supplied netname to use
	passwd		user supplied password used to decrypt the private key

	Returns:

	>=0		OK
	<0		error


	Program notes:

	The 'key_setnet()' subroutine is part of the KEYSERV version 2 API.
	There does not appear to be a manual page on it so we guess at its use
	where necessary!  It must be used by 'login(1)', 'keylogin(1)', and
	other such programs for setting the ONC netname on login.

	The 'key_setnet()' subroutine returns are:

	1	succeeded in setting ONC private key to KEYSERV
	-1	failed to set ONC private key with KEYSERV

	Other needed subroutines are:

	Synopsis:

	getsecretkey(cchar *netname,char result[]) ;

	Arguments:

	netname		given net-name
	result		buffer to retrieve secret key (in HEX)

	Returns:

	0		failed
	1		succeeded


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"stubrpc.h"


/* local defines */

#define	PASSPHRASELEN	8		/* maximum ONC password length */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int onckeygetset(cchar *netname,cchar *passwd)
{
	key_netstarg	sna ;
	int		rs = SR_OK ;
	cchar		*nnp = netname ;
	char		passphrase[PASSPHRASELEN + 1] ;

	if (netname == NULL) return SR_FAULT ;
	if (passwd == NULL) return SR_FAULT ;

	if (netname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("niskeylogin: ent netname=%s\n",netname) ;
	debugprintf("niskeylogin: passwd=%s\n",passwd) ;
#endif

	strncpy(passphrase,passwd,PASSPHRASELEN) ;

	passphrase[PASSPHRASELEN] = '\0' ;	/* truncate to maximum length */

#ifdef	COMMENT
	memset(&sna,0,sizeof(key_netstarg)) ;
#endif

	memset(sna.st_priv_key,0,HEXKEYBYTES) ;
	sna.st_pub_key[0] = '\0' ;
	sna.st_netname = (char *) netname ;

/* decrypt and retrieve the private key */

	if (rs >= 0) {
	    if (getsecretkey(nnp,sna.st_priv_key,passphrase) > 0) {
	        if (sna.st_priv_key[0] != '\0') {

#if	CF_DEBUGS
	        debugprintf("niskeylogin: privkey=%W\n",
	            sna.st_priv_key,HEXKEYBYTES) ;
#endif

/* we have successfully decrypted our private ONC key, give it to KEYSERV */

	            if ((rs = key_setnet(&sna)) > 0) {
	                memset(sna.st_priv_key,0,HEXKEYBYTES) ;
		    } else if (rs == 0) {
		        rs = SR_PROTO ;
		    } else
	                rs = SR_SRCH ;

		} else
		     rs = SR_PROTO ;
	    } else
	        rs = SR_NOENT ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("niskeylogin: exiting rs=%d\n",rs) ;
#endif

	memset(passphrase,0,PASSPHRASELEN) ;

	return rs ;
}
/* end subroutine (oncskeygetset) */


