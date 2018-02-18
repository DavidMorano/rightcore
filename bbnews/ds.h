/* ds */

/* low-level terminal-display manager */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DS_INCLUDE
#define	DS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"td.h"


/* object define */
#define	DS_MAGIC	0x24182137
#define	DS		struct ds_head
#define	DS_FL		struct ds_flags

/* windows */
#define	DS_WROOT	0
#define	DS_WHEADER	1
#define	DS_WVIEWER	2

/* graphic renditions */
#define	DS_GRBOLD	(1<<0)		/* graphic-rendition bold */
#define	DS_GRUNDER	(1<<1)		/* graphic-rendition underline */
#define	DS_GRBLINK	(1<<2)		/* graphic-rendition blinking */
#define	DS_GRREV	(1<<3)		/* graphic-rendition reverse-video */


struct ds_flags {
	uint		update:1 ;
	uint		mailnew:1 ;		/* new mail arrived */
} ;

struct ds_head {
	const char	*termtype ;
	TD		dm ;		/* terminal display manager */
	DS_FL		f ;
	uint		magic ;
	int		tfd ;
	int		rows ;
	int		cols ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int ds_start(DS *,int,const char *,int,int) ;
extern int ds_subnew(DS *,int,int,int,int) ;
extern int ds_subdel(DS *,int) ;
extern int ds_getlines(DS *,int) ;
extern int ds_setlines(DS *,int,int) ;
extern int ds_clear(DS *,int) ;
extern int ds_move(DS *,int,int,int) ;
extern int ds_ew(DS *,int,int,int) ;
extern int ds_el(DS *,int,int) ;
extern int ds_ec(DS *,int,int) ;
extern int ds_printf(DS *,int,const char *,...) ;
extern int ds_pprintf(DS *,int,int,int,const char *,...) ;
extern int ds_vprintf(DS *,int,const char *,va_list) ;
extern int ds_vpprintf(DS *,int,int,int,const char *,va_list) ;
extern int ds_write(DS *,int,const char *,int) ;
extern int ds_pwrite(DS *,int,int,int,const char *,int) ;
extern int ds_pwritegr(DS *,int,int,int,int,const char *,int) ;
extern int ds_scroll(DS *,int,int) ;
extern int ds_flush(DS *) ;
extern int ds_suspend(DS *,int,int) ;
extern int ds_done(DS *) ;
extern int ds_finish(DS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DS_INCLUDE */


