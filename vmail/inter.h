/* inter */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	INTER_INCLUDE
#define	INTER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<uterm.h>
#include	<mailbox.h>
#include	<localmisc.h>

#include	"keysymer.h"
#include	"kbdinfo.h"
#include	"cmdmap.h"
#include	"mbcache.h"
#include	"subprocs.h"
#include	"config.h"
#include	"defs.h"
#include	"display.h"
#include	"mailmsgfile.h"
#include	"mailmsgviewer.h"


#ifndef	PROGINFO
#define	PROGINFO	struct proginfo
#endif
#ifndef	PROGINFO_FL
#define	PROGINFO_FL	struct proginfo_flags
#endif

#define	INTER_MAGIC	0x5CD73921
#define	INTER		struct inter_head
#define	INTER_FL	struct inter_flags
#define	INTER_IPROMPT	"?"
#define	INTER_NUMLEN	10


struct inter_flags {
	uint		cmdmap:1 ;
	uint		keysymer:1 ;
	uint		kbdinfo:1 ;
	uint		display:1 ;	/* DISPLAY */
	uint		msger:1 ;
	uint		mailbox:1 ;	/* mailbox is open */
	uint		mbcache:1 ;
	uint		termmesg:1 ;	/* terminal messages were enabled */
	uint		termbiff:1 ;	/* terminal biffing was enabled */
	uint		ctty:1 ;	/* we have controlling term */
	uint		welcome:1 ;	/* welcome message issued */
	uint		cmdprompt:1 ;	/* prompt issued */
	uint		cmdnumeric:1 ;	/* cmd-numeric in progress */
	uint		mailnew:1 ;	/* new mail arrived */
	uint		mailinfo:1 ;	/* displaying mailinfo */
	uint		mbreadonly:1 ;	/* mailbox opened read-only */
	uint		mv:1 ;
	uint		subprocs:1 ;	/* SUBPROCS started */
	uint		setmbname:1 ;
	uint		exit:1 ;	/* exit interactive mode */
	uint		viewchange:1 ;
	uint		info_msg ;	/* info message */
	uint		info_err ;	/* error-info message */
} ;

struct inter_head {
	uint		magic ;
	INTER_FL	f, open ;
	PROGINFO	*pip ;
	UTERM		*utp ;
	const char	*mbname ;	/* current mailbox */
	const char	*pathprefix ;
	void		*trans ;
	CMDMAP		cm ;
	KEYSYMER	ks ;
	KBDINFO		ki ;
	SUBPROCS	sp ;
	DISPLAY		di ;
	MAILBOX		mb ;
	MBCACHE		mc ;
	MAILMSGFILE	msgfiles ;	/* mail-msg file-name DB */
	MAILMSGVIEWER	mv ;
	time_t		ti_start ;	/* startup */
	time_t		ti_poll ;
	time_t		ti_config ;
	time_t		ti_clock ;
	time_t		ti_mailcheck ;
	time_t		ti_mailinfo ;
	time_t		ti_info ;
	time_t		ti_child ;
	pid_t		pgrp ;		/* controlling term PGID */
	int		termlines ;	/* terminal lines (actual) */
	int		displines ;	/* display lines (requested) */
	int		scanlines ;	/* scan lines */
	int		viewlines ;	/* view lines */
	int		jumplines ;
	int		taginfo ;
	int		nmsgs ;
	int		nmsgdels ;	/* number mail deletions */
	int		miscantop ;	/* msg-index at top of scan display */
	int		miscanpoint ;	/* msg-index pointed to */
	int		miviewpoint ;	/* msg-index being viewed */
	int		lnviewtop ;	/* top line being viewed */
	int		nviewlines ;
	int		plen ;		/* prompt length */
	int		runcount ;	/* testing */
	int		numlen ;
	int		mfd ;		/* mailbox file descriptor */
	int		delmark ;	/* deletion mark character for scans */
	char		numbuf[INTER_NUMLEN + 1] ;
} ;


#if	(! defined(INTER_MASTER)) || (INTER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	inter_start(INTER *,PROGINFO *,UTERM *) ;
extern int	inter_cmd(INTER *) ;
extern int	inter_finish(INTER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* INTER_MASTER */

#endif /* INTER_INCLUDE */


