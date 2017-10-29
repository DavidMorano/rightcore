/* ns_getserial */

/* get the serial number for logging references */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_LOG		1


/* revision history:

	= 1995-11-01, David A­D­ Morano
	This program was originally written.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to get a canonical INET hostname for a supplied
        name. Note carefully that the returned hostname, if

	Synopsis:

	int ns_getserial(filename)
	char	filename[] ;

	Arguments:

	- filename	filename of file containing the serial number

	Returns:

	>=0		the serial number
	<0		could not get it !


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fnctl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>

#if	CF_LOG
#include	<logfile.h>
#endif

#include	<localmisc.h>


/* local defines */

#ifndef	LOGIDLEN
#define	LOGIDLEN	80
#endif

#define	LOCKTIMEOUT	4	/* seconds */


/* external subroutines */


/* external variables */


/* global data */


/* local structures */


/* forward references */


/* exported subroutines */


int ns_getserial(filename)
char	filename[] ;
{
	bfile	sf, *sfp = &sf ;

	int	rs ;
	int	len, serial ;

	char	logid[LOGIDLEN + 1] ;


	if ((rs = bopen(sfp,filename,"rwc",0666)) >= 0) {

#if	CF_DEBUGS
	debugprintf("ns_getserial: rs=%d\n") ;
#endif

	    serial = 0 ;
	    if (rs == SR_CREATED)
	        bcontrol(sfp,BC_CHMOD,0666) ;

/* try to lock it but with a timeout */

	    if (bcontrol(sfp,BC_LOCK,LOCKTIMEOUT) >= 0) {

	        if ((len = breadline(sfp,logid,LOGIDLEN)) > 0) {

	            if (cfdeci(logid,len,&serial) < 0)
	                serial = 0 ;

	        }

	    }

	    bseek(sfp,0L,SEEK_SET) ;

	    bprintf(sfp,"%d\n",serial + 1) ;

	    bclose(sfp) ;

	    rs = serial ;

	} /* end if (opened successfully) */

	return rs ;
}
/* end subroutine (ns_getserial) */



