/* gecosname */

/* fix the GECOS name as it comes out of the password file ('/etc/passwd') */


#define	CF_DEBUGS	0


/* revision history:

	= 94/01/12, David A.D. Morano

	This subroutine was originally written.


*/


/****************************************************************************
*									
*	FUNCTIONAL DESCRIPTION:						

	GECOSNAME gets the GECOS version of a user name from the
	fifth field of the system password file ('/etc/passwd').


	Synopsis:

	int gecosname(gn,buf,buflen)
	const char	gn[] ;
	char		buf[] ;
	int		buflen ;


	Arguments:

	gn	input buffer containing the GECOS field string
	buf	user supplied result buffer to contain the full-name
	buflen	length of the user supplied result buffer


	Returns:

	>=0	length of GECOS name
	<0	failed 


*									
*	SUBROUTINES CALLED:						
*		Only system routines are called.
*									
*	GLOBAL VARIABLES USED:						
*		None !!  AS IT SHOULD BE !!
*									

	Extra Notes:

	The GECOS field of the 'passwd' database should be formatted in
	one of the following ways:

	    name,office,work_phone,home_phone
	    organization-name(account,bin)office,work_phone,home_phone,printer
	    name(account)office,work_phone,home_phone,printer
	    name

	Note also that an ampersand character ('&') that appears
	anywhere in the GCOS field is to be logically replaced by the
	corresponding username of the entry.

	The 'name' part of the GCOS entry may contain hyphen ('-')
	characters.

	The original AT&T GECOS field contained:

	    department-name(account,bin)

	and was further put into a 'struct comment' with fields:

	    c_dept
	    c_name
	    c_acct
	    c_bin

	Some suggestions for the GECOS field are:

	    org_dept-name(account,bin)office,work_phone,home_phone
	    org_dept-name(account,bin)office,work_phone,home_phone,printer

	Actual example:

	    XNR64430-d.a.morano(126483,BIN8221)
	    rockwell-d.a.morano(126283,BIN8221)4B-411,5336,6175679484,hp0


*									
************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>

#include	"misc.h"



/* local defines */

#ifndef	CH_LPAREN
#define	CH_LPAREN	0x28
#endif



/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;






int gecosname(gn,buf,buflen)
const char	gn[] ;
char		buf[] ;
int		buflen ;
{
	int	rs = SR_OK, cl, bl ;

	char	*cp, *cp2 ;


#if	CF_DEBUGS
	eprintf("gecosname: entered name=%s\n",gn) ;
#endif

	if ((gn == NULL) || (buf == NULL))
		return SR_FAULT ;

	if (((cp = strchr(gn,'-')) != NULL) &&
	    ((cp2 = strchr(cp,CH_LPAREN)) != NULL)) {

#if	CF_DEBUGS
	    eprintf("gecosname: traditional GECOS\n") ;
#endif

	    cp += 1 ;
	    cl = cp2 - cp ;

	} else if ((cp2 = strchr(gn,CH_LPAREN)) != NULL) {

#if	CF_DEBUGS
	    eprintf("gecosname: partial formed GECOS\n") ;
#endif
	
	    cp = (char *) gn ;
	    cl = cp2 - gn ;

	} else {

#if	CF_DEBUGS
	    eprintf("gecosname: no/bad GECOS\n") ;
#endif

	    cp = (char *) gn ;
	    cl = -1 ;

	} /* end if */

	    if ((buflen < 0) || ((cl >= 0) && (cl <= buflen))) {

	    	bl = strwcpy(buf,cp,cl) - buf ;

	    } else if (cl < 0) {

	    	bl = strwcpy(buf,cp,buflen) - buf ;

	        if (cp[bl] != '\0')
			rs = SR_OVERFLOW ;

	    } else
		rs = SR_OVERFLOW ;

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (gecosname) */



