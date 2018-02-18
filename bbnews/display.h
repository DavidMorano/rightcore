/* display */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	DISPLAY_INCLUDE
#define	DISPLAY_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdarg.h>

#include	<varray.h>
#include	<tmtime.h>
#include	<hdrdecode.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ds.h"


/* starting columns where for various things */
#ifndef	COL_SCANFROM
#define	COL_SCANFROM	2
#endif
#ifndef	COL_SCANSUBJECT
#define	COL_SCANSUBJECT	29
#endif
#ifndef	COL_SCANDATE
#define	COL_SCANDATE	60
#endif
#ifndef	COL_SCANLINES
#define	COL_SCANLINES	76
#endif

/* display-area codes here */
#define	DRM_BACKGROUND		0x01
#define	DRM_TOP			0x10
#define	DRM_BOTTOM		0x02
#define	DRM_HEADER		0x04
#define	DRM_MESSAGE		0x08
#define	DRM_ALL			0xFF

/* object defines */
#define	DISPLAY_MAGIC		0x99535152
#define	DISPLAY			struct display_head
#define	DISPLAY_ARGS		struct display_args
#define	DISPLAY_SDATA		struct display_scandata
#define	DISPLAY_SALT		struct display_scanalt
#define	DISPLAY_SS		struct display_scansave
#define	DISPLAY_BOTINFO		struct display_mi
#define	DISPLAY_VALS		struct display_vals
#define	DISPLAY_DATE		struct display_date
#define	DISPLAY_FL		struct display_flags

#define	DISPLAY_DEFTERM		"ansi"
#define	DISPLAY_DEFLINES	24

/* various field widths (in columns) */
#define	DISPLAY_LSCANLINE	80
#define	DISPLAY_LNEWMAIL	16
#define	DISPLAY_LMBSTR		24
#define	DISPLAY_LDATESTR	19
#define	DISPLAY_LTZNAME		6
#define	DISPLAY_LNMSGS		11
#define	DISPLAY_LMOREMSGS	13
#define	DISPLAY_LNLINES		9

#define	DISPLAY_LFROM		(COL_SCANSUBJECT - COL_SCANFROM - 1)
#define	DISPLAY_LSUBJ		(COL_SCANDATE - COL_SCANSUBJECT - 1)
#define	DISPLAY_LDATE		(COL_SCANLINES - COL_SCANDATE - 1)
#define	DISPLAY_LLINES		(DISPLAY_LSCANLINE - COL_SCANLINES)

#define	DISPLAY_LMSGFROM	DISPLAY_LFROM
#define	DISPLAY_LMSGNUM		10
#define	DISPLAY_LMSGLINES	10
#define	DISPLAY_LMSGMORE	10

#define	DISPLAY_FROMLEN		(DISPLAY_LFROM*2)


struct display_mi {
	int		msgnum ;
	int		msgline ;
	int		msglines ;
	char		msgfrom[DISPLAY_LFROM + 1] ;
} ;

struct display_args {
	const char	*termtype ;
	int		tfd ;
	int		termlines ;
	int		displines ;
	int		scanlines ;
} ;

struct display_scanalt {
	const void	*a ;
	char		*fbuf ;
	char		*sbuf ;
	int		flen ;
	int		slen ;
} ;

struct display_scandata {
	DISPLAY_SALT	alt ;
	cchar		*fbuf ;		/* FROM supplied */
	cchar		*sbuf ;		/* SUBJ supplied */
	cchar		*date ;		/* preformatted date string */
	int		lines ;		/* message displable lines */
	int		flen ;
	int		fcol ;
	int		slen ;
	int		scol ;
	int		mark ;		/* optional mark to display */
	int		msgi ;		/* message "scan"? index */
} ;

struct display_scansave {
	cchar		*fbuf ;
	cchar		*sbuf ;
	int		flen ;
	int		slen ;
} ;

struct display_date {
	TMTIME		d ;
	char		datestr[DISPLAY_LDATESTR + 1] ;
} ;

struct display_flags {
	uint		header:1 ;
	uint		check:1 ;
	uint		bol:1 ;
	uint		eol:1 ;
	uint		newmail:1 ;	/* "new-msg" display indicator */
	uint		moremsgs:1 ;	/* "more-msgs" display indicator */
	uint		morelines:1 ;	/* "more-lines" display indicator */
	uint		nmsgs:1 ;	/* "nmsgs" display indicator */
	uint		nlines:1 ;	/* "nlines" display indicator */
	uint		scanfull:1 ;	/* force a full-scan next time */
	uint		hd:1 ;		/* mail-header decode */
} ;

struct display_vals {
	int		total ;		/* total-messages number */
	int		current ;	/* current-message number */
	int		msgnum ;
	int		msgline ;
	int		msglines ;
	int		flen ;		/* actual length */
	char		fbuf[DISPLAY_FROMLEN+1] ;
} ;

struct display_head {
	uint		magic ;
	PROGINFO	*pip ;
	cchar		*termtype ;
	wchar_t		*wbuf ;
	DS		ds ;		/* display-terminal manager */
	DISPLAY_VALS	v ;
	DISPLAY_DATE	date ;
	DISPLAY_FL	f, open ;
	HDRDECODE	hd ;		/* mail-header decode */
	varray		scans ;		/* scan-line objects */
	time_t		ti_info ;	/* timestamp for 'info' */
	int		tfd ;		/* terminal FD */
	int		wlen ;
	int		termlines ;	/* number of terminal lines */
	int		displines ;	/* total lines */
	int		scanlines ;	/* scan lines */
	int		viewlines ;	/* view lines */
	int		wscan ;		/* sub-window for scan lines */
	int		wview ;		/* sub-window for viewing */
	int		rl_top ;	/* top screen line */
	int		rl_scantitle ;	/* the header-area title line */
	int		rl_scan ;	/* the top of the viewer area */
	int		rl_middle ;	/* middle screen line */
	int		rl_viewer ;	/* the top of the viewer area */
	int		rl_bottom ;	/* bottom line */
	int		rl_info ;	/* info line */
	int		rl_input ;	/* input line */
	int		si_top ;	/* scan-index at scan-display top */
	int		si_point ;	/* scan-index current pointer */
	int		si_ppoint ;	/* scan-index previous pointer */
	int		nnblank ;	/* non-blank displayed scan lines */
	int		margin ;	/* line margin on HEADER lines */
	int		cp_info ;	/* cursor position (info) */
	int		cp_input ;	/* cursor position (input) */
	int		hl_cur ;
	char		newmail[DISPLAY_LNEWMAIL + 1] ;
	char		mbstr[DISPLAY_LMBSTR + 1] ;
	char		datestr[DISPLAY_LDATESTR + 1] ;
	char		tzname[DISPLAY_LTZNAME + 1] ;
	char		nmsgs[DISPLAY_LNMSGS + 1] ;
	char		nlines[DISPLAY_LNLINES + 1] ;
	char		scantitle[DISPLAY_LSCANLINE + 1] ;
	char		linebot[DISPLAY_LSCANLINE + 1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int display_start(DISPLAY *,PROGINFO *,DISPLAY_ARGS *) ;
extern int display_finish(DISPLAY *) ;
extern int display_setdate(DISPLAY *,int) ;
extern int display_setmbname(DISPLAY *,const char *,int) ;
extern int display_setnewmail(DISPLAY *,int) ;
extern int display_info(DISPLAY *,const char *,...) ;
extern int display_winfo(DISPLAY *,const char *,int) ;
extern int display_vinfo(DISPLAY *,const char *,va_list) ;
extern int display_infots(DISPLAY *,time_t *) ;
extern int display_input(DISPLAY *,const char *,...) ;
extern int display_vinput(DISPLAY *,const char *,va_list) ;
extern int display_done(DISPLAY *) ;
extern int display_flush(DISPLAY *) ;
extern int display_cmddig(DISPLAY *,int,int) ;
extern int display_winadj(DISPLAY *,int,int) ;
extern int display_setscanlines(DISPLAY *,int) ;

extern int display_refresh(DISPLAY *) ;
extern int display_rframe(DISPLAY *) ;
extern int display_rtop(DISPLAY *) ;
extern int display_rscantitle(DISPLAY *) ;
extern int display_rmid(DISPLAY *) ;
extern int display_rbot(DISPLAY *) ;

extern int display_scanclear(DISPLAY *) ;
extern int display_scanload(DISPLAY *,int,DISPLAY_SDATA *) ;
extern int display_scanloadlines(DISPLAY *,int,int,int) ;
extern int display_scanblank(DISPLAY *,int) ;
extern int display_scanblanks(DISPLAY *,int) ;
extern int display_scanfull(DISPLAY *) ;
extern int display_scandel(DISPLAY *,int) ;
extern int display_scanmark(DISPLAY *,int,int) ;
extern int display_scanpoint(DISPLAY *,int) ;
extern int display_scancheck(DISPLAY *,int) ;
extern int display_scandisplay(DISPLAY *,int) ;

extern int display_midmsgs(DISPLAY *,int,int) ;

extern int display_viewclear(DISPLAY *) ;
extern int display_viewload(DISPLAY *,int,const char *,int) ;
extern int display_viewscroll(DISPLAY *,int) ;

extern int display_botclear(DISPLAY *) ;
extern int display_botinfo(DISPLAY *,DISPLAY_BOTINFO *) ;
extern int display_botline(DISPLAY *,int) ;

extern int display_allclear(DISPLAY *) ;

extern int display_suspend(DISPLAY *) ;
extern int display_resume(DISPLAY *) ;


extern int scandata_init(DISPLAY_SDATA *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DISPLAY_INCLUDE */


