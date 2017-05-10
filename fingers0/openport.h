/* openport */

/* subroutine to open a priviledged network port */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OPENPORT_INCLUDE
#define	OPENPORT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>

#include	<sockaddress.h>
#include	<openportmsg.h>
#include	<localmisc.h>


#if	(! defined(OPENPORT_MASTER)) || (OPENPORT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int openport(int,int,int,SOCKADDRESS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENPORT_MASTER */

#endif /* OPENPORT_INCLUDE */


