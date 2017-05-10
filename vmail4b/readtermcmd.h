/* readtermcmd */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	UTERM__INCLUDE
#define	UTERM__INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<uterm.h>
#include	<uterm.h>


#define	UTERM_		struct readtermcmd

#define	UTERM__ISIZE	40	/* intermedicate-characters size */
#define	UTERM__DSIZE	140	/* DCS size */


enum termcmdtypes {
	termcmdtype_reg,		/* regular character */
	termcmdtype_esc,		/* "escape" sequence */
	termcmdtype_csi,		/* control-sequence-introducer */
	termcmdtype_dcs,		/* device-control-string */
	termcmdtype_pf,			/* special function key (P,Q,R,S) */
	termcmdtype_overlast
} ;

struct readtermcmd {
	short		type ;		/* terminal-command type */
	short		name ;		/* "final" */
	short		p[16] ;		/* parameters */
	char		istr[UTERM__ISIZE+1] ;
	char		dstr[UTERM__DSIZE+1] ;
	uint		f_private:1 ;	/* private CSI */
	uint		f_iover:1 ;	/* intermediate-string overflow */
	uint		f_dover:1 ;	/* device-control-string overflow */
} ;


#if	(! defined(UTERM__MASTER)) || (READTERMCMD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uterm_readtermcmd(TERMCMD *,UTERM *,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* UTERM__MASTER */


#endif /* UTERM__INCLUDE */


