/* termstr */

/* terminal display editing control sequences */
/* last modified %G% version %I% */


/* revision history:

	= 1999-08-01, Dave morano
	File was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TERMSTR_INCLUDE
#define	TERMSTR_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<buffer.h>
#include	<localmisc.h>


/* graphic renditions */
#define	TERMSTR_GRNONE	(1<<0)		/* graphic-rendition none */
#define	TERMSTR_GRBOLD	(1<<1)		/* graphic-rendition bold */
#define	TERMSTR_GRUNDER	(1<<2)		/* graphic-rendition underline */
#define	TERMSTR_GRBLINK	(1<<3)		/* graphic-rendition blinking */
#define	TERMSTR_GRREV	(1<<4)		/* graphic-rendition reverse-video */

/* cursor saving and restoring */

#define	TERMSTR_VCURS	"\0337"		/* cursor save-restore (VT) */
#define	TERMSTR_VCURR	"\0338"		/* cursor save-restore (VT) */

#define	TERMSTR_ACURS	"\033[s"	/* cursor save-restore (ASNI) */
#define	TERMSTR_ACURR	"\033[u"	/* cursor save-restore (ANSI) */

#define	TERMSTR_SAVE	TERMSTR_VCURS	/* cursor save-restore (VT) */
#define	TERMSTR_RESTORE	TERMSTR_VCURR	/* cursor save-restore (VT) */

/* cursor positioning */

#define	TERMSTR_CURU	"\033[A"	/* cursor up */
#define	TERMSTR_CURD	"\033[B"	/* cursor down */
#define	TERMSTR_CURR	"\033[C"	/* cursor right */
#define	TERMSTR_CURL	"\033[D"	/* cursor left */
#define	TERMSTR_CURH	"\033[H"	/* cursor home */

#define	TERMSTR_UP	TERMSTR_CURU
#define	TERMSTR_DOWN	TERMSTR_CURD
#define	TERMSTR_RIGHT	TERMSTR_CURR
#define	TERMSTR_LEFT	TERMSTR_CURL
#define	TERMSTR_HOME	TERMSTR_CURH

/* editing */

#define	TERMSTR_ED	"\033[J"	/* erase to end of display */
#define	TERMSTR_EL	"\033[K"	/* erase to end of line */
#define	TERMSTR_EC	"\033[X"	/* erase character */

#define	TERMSTR_EDIS	"\033[J"	/* erase to end of display */
#define	TERMSTR_X	"\033[X"	/* erase character */
#define	TERMSTR_ECH	"\033[X"	/* erase character */

#define	TERMSTR_DL	"\033[M"	/* delete line */
#define	TERMSTR_DC	"\033[P"	/* delete character */
#define	TERMSTR_DCH	"\033[P"	/* delete character */

#define	TERMSTR_IL	"\033[L"	/* insert line */
#define	TERMSTR_IC	"\033[@"	/* insert character */
#define	TERMSTR_ICH	"\033[@"	/* insert character */

/* character renditions */

#define	TERMSTR_NORM	"\033[m"	/* no attributes */
#define	TERMSTR_BOLD	"\033[1m"	/* bold */
#define	TERMSTR_UNDER	"\033[4m"	/* underline */
#define	TERMSTR_BLINK	"\033[5m"	/* blink */
#define	TERMSTR_REVERSE	"\033[7m"	/* reverse */

/* character renditions for advanced terminals */

#define	TERMSTR_NOBOLD	"\033[22m"	/* no-bold */
#define	TERMSTR_NOUNDER	"\033[24m"	/* no-underline */
#define	TERMSTR_NOBLINK	"\033[27m"	/* no-blink */

/* insert-replacement mode */

#define	TERMSTR_S_IRM	"\033[4h"	/* insert-replacement set */
#define	TERMSTR_R_IRM	"\033[4l"	/* insert-replacement clear */

/* cursor visibility */

#define	TERMSTR_S_VCUR	"\033[?25h"	/* set cursor ON (VT) */
#define	TERMSTR_R_VCUR	"\033[?25l"	/* set cursor OFF (VT) */

#define	TERMSTR_S_ACUR	"\033[34h"	/* set cursor ON (ANSI) */
#define	TERMSTR_R_ACUR	"\033[34l"	/* set cursor OFF (ANSI) */

#define	TERMSTR_S_SCUR	"\0337p"	/* set cursor ON (SCREEN) */
#define	TERMSTR_R_SCUR	"\0336p"	/* set cursor OFF (SCREEN) */

#define	TERMSTR_S_CUR	TERMSTR_S_VCUR	/* cursor ON (VT) */
#define	TERMSTR_R_CUR	TERMSTR_R_VCUR	/* cursor OFF (VT) */

/* status display */

#define	TERMSTR_S_SD	"\033[1$\175"	/* status display SET */
#define	TERMSTR_R_SD	"\033[0$\175"	/* status display RESET */


#define	TERMSTR			struct termstr_head


struct termstr_head {
	uint		magic ;
	BUFFER		b ;		/* storage buffer */
	int		ti ;		/* index into database */
} ;


#if	(! defined(TD_MASTER)) || (TD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termstr_start(TERMSTR *,const char *) ;
extern int termstr_clean(TERMSTR *) ;
extern int termstr_char(TERMSTR *,int) ;
extern int termstr_write(TERMSTR *,const char *,int) ;
extern int termstr_writegr(TERMSTR *,int,const char *,int) ;
extern int termstr_get(TERMSTR *,const char **) ;
extern int termstr_ed(TERMSTR *,int) ;
extern int termstr_el(TERMSTR *,int) ;
extern int termstr_ec(TERMSTR *,int) ;
extern int termstr_curu(TERMSTR *,int) ;
extern int termstr_curd(TERMSTR *,int) ;
extern int termstr_curl(TERMSTR *,int) ;
extern int termstr_curr(TERMSTR *,int) ;
extern int termstr_curh(TERMSTR *,int,int) ;
extern int termstr_ssr(TERMSTR *,int,int) ;
extern int termstr_csr(TERMSTR *,int) ;
extern int termstr_il(TERMSTR *,int) ;
extern int termstr_ic(TERMSTR *,int) ;
extern int termstr_dl(TERMSTR *,int) ;
extern int termstr_dc(TERMSTR *,int) ;
extern int termstr_irm(TERMSTR *,int) ;
extern int termstr_cvis(TERMSTR *,int) ;
extern int termstr_finish(TERMSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMSTR_MASTER */

#endif /* TERMSTR_INCLUDE */


