/* termcmd */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TERMCMD_INCLUDE
#define	TERMCMD_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>		/* for "unsigned" types */


#define	TERMCMD		struct termcmd
#define	TERMCMD_FL	struct termcmd_flags
#define	TERMCMD_NP	16	/* number of paramters (as per ANSI) */
#define	TERMCMD_PEOL	0xffff	/* parameter End-Of-Line (EOL) */
#define	TERMCMD_MAXPVAL	9999	/* as per ANSI */
#define	TERMCMD_ISIZE	40	/* intermedicate-characters size */
#define	TERMCMD_DSIZE	140	/* DCS size */


enum termcmdtypes {
	termcmdtype_reg,		/* regular character */
	termcmdtype_esc,		/* "escape" sequence */
	termcmdtype_csi,		/* control-sequence-introducer */
	termcmdtype_dcs,		/* device-control-string */
	termcmdtype_pf,			/* special function key (P,Q,R,S) */
	termcmdtype_overlast
} ;

struct termcmd_flags {
	uint		private:1 ;	/* private CSI */
	uint		iover:1 ;	/* intermediate-string overflow */
	uint		dover:1 ;	/* device-control-string overflow */
} ;

struct termcmd {
	TERMCMD_FL	f ;
	short		type ;		/* terminal-command type */
	short		name ;		/* "final" */
	short		p[TERMCMD_NP] ;	/* parameters */
	char		istr[TERMCMD_ISIZE+1] ;
	char		dstr[TERMCMD_DSIZE+1] ;
} ;

#if	(! defined(TERMCMD_MASTER)) || (TERMCMD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termcmd_clear(TERMCMD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMCMD_MASTER */

#endif /* TERMCMD_INCLUDE */


