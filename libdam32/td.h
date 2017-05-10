/* td */

/* terminal display manager */


/* revision history:

	= 1999-08-01, Dave morano
	File was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TD_INCLUDE
#define	TD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdarg.h>

#include	<vecitem.h>
#include	<termstr.h>
#include	<localmisc.h>


#define	TD		struct td_head
#define	TD_POSITION	struct td_position

/* control codes */

#define	TD_CEOL		0		/* set "erase to EOL" mode */
#define	TD_CLINES	1		/* change number of lines in window */
#define	TD_CSCROLL	2		/* scroll the window up or down */
#define	TD_CSUBWIN	3		/* create a subwindow */
#define	TD_CCURSOR	4		/* position the cursor */

/* graphic renditions */

#define	TD_GRBOLD	(1<<0)		/* graphic-rendition bold */
#define	TD_GRUNDER	(1<<1)		/* graphic-rendition underline */
#define	TD_GRBLINK	(1<<2)		/* graphic-rendition blinking */
#define	TD_GRREV	(1<<3)		/* graphic-rendition reverse-video */


struct td_flags {
	uint		statusdisplay:1 ;
	uint		eol:1 ;		/* erase-to-EOL mode */
	uint		linebuf:1 ;	/* line buffer mode */
	uint		smallscroll:1 ;
	uint		nlcr:1 ;	/* NL-character display behavior */
} ;

struct td_position {
	uint		timecount ;
	int		row ;
	int		col ;
} ;

struct td_window {
	struct td_position	move ;	/* move is outstanding */
	struct td_position	cur ;	/* current position */
	int		srow ;		/* starting absolute row */
	int		scol ;		/* starting absolute col */
	int		rows ;		/* rows */
	int		cols ;		/* cols */
} ;

struct td_head {
	uint		magic ;
	struct td_position	cur ;	/* current position */
	struct td_flags		f ;
	const char	*termtype ;	/* given terminal type-name */
	char		*buf ;		/* storage buffer */
	TERMSTR		enter ;
	vecitem		wins ;		/* sub windows */
	uint		timecounter ;
	int		tfd ;
	int		termcap ;	/* terminal capabilities */
	int		buflen ;	/* buffer extent */
	int		curlen ;	/* current buffer occupancy */
	int		rows ;
	int		cols ;
} ;


#if	(! defined(TD_MASTER)) || (TD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int td_start(TD *,int,const char *,int,int) ;
extern int td_finish(TD *) ;
extern int td_flush(TD *) ;
extern int td_subnew(TD *,int,int,int,int) ;
extern int td_subdel(TD *,int) ;
extern int td_getlines(TD *,int) ;
extern int td_setlines(TD *,int,int) ;
extern int td_move(TD *,int,int,int) ;
extern int td_scroll(TD *,int,int) ;
extern int td_control(TD *,int,int,int) ;
extern int td_setsize(TD *,int,int) ;
extern int td_suspend(TD *,int,int) ;
extern int td_write(TD *,int,const char *,int) ;
extern int td_pwrite(TD *,int,int,int,const char *,int) ;
extern int td_pwritegr(TD *,int,int,int,int,const char *,int) ;
extern int td_vprintf(TD *,int,const char *,va_list) ;
extern int td_vpprintf(TD *,int,int,int,const char *,va_list) ;
extern int td_printf(TD *,int,const char *,...) ;
extern int td_pprintf(TD *,int,int,int,const char *,...) ;
extern int td_clear(TD *,int) ;
extern int td_ew(TD *,int,int,int) ;
extern int td_el(TD *,int,int) ;
extern int td_ec(TD *,int,int) ;
extern int td_check(TD *) ;
extern int td_position(TD *,int,TD_POSITION *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TD_MASTER */

#endif /* TD_INCLUDE */


