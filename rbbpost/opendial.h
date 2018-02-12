/* opendial */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	OPENDIAL_INCLUDE
#define	OPENDIAL_INCLUDE	1


/* options */
#define	OPENDIAL_OLONG		(1<<0)		/* FINGER "long" option */


/* dialers (by the number) */
enum opendialers {
	opendialer_unspec,
	opendialer_udp,
	opendialer_tcp,
	opendialer_tcpmux,
	opendialer_tcpnls,
	opendialer_uss,
	opendialer_ussmux,
	opendialer_ussnls,
	opendialer_ticotsord,
	opendialer_ticotsordnls,
	opendialer_pass,
	opendialer_open,
	opendialer_prog,
	opendialer_finger,
	opendialer_overlast
} ;


#if	(! defined(OPENDIAL_MASTER)) || (OPENDIAL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int opendial(int,int,cchar *,cchar *,cchar *,
		cchar **,cchar **,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENDIAL_MASTER */

#endif /* OPENDIAL_INCLUDE */


