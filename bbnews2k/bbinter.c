/* bbinter */

/* the user bbinterface (command bbinterpreter) for VMAIL */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DISPLAY	1		/* compile-in 'display()' */
#define	CF_BBINTERPOLL	1		/* call 'bbinter_poll()' */
#define	CF_INITTEST	0		/* initialization test */
#define	CF_WELCOME	1		/* issue welcome info */
#define	CF_KBDINFO	1		/* activate KBDINFO */


/* revision history:

	= 2009-01-20, David A­D­ Morano
        This is a complete rewrite of the trash that performed this function
        previously.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the main bbinteractive loop.

	Implimentation notes:

	Caching: We cache scanlines (scan-line data) in two places.
	This is probably needless but we are doing it anyway.  It is first
	cached in the mailbox-cache (MBCACHE) object.  It is secondarily
	cached in our own DISPLAY object.  Mail-message viewing data is
	cached in the MAILMSGFILE object.


*******************************************************************************/


#define	BBINTER_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<uterm.h>
#include	<ascii.h>
#include	<sbuf.h>
#include	<spawnproc.h>
#include	<keysym.h>
#include	<toxc.h>
#include	<localmisc.h>

#include	"mailmsg.h"
#include	"keysymer.h"
#include	"config.h"
#include	"defs.h"
#include	"display.h"
#include	"bbinter.h"
#include	"mailmsgfile.h"
#include	"mailmsgviewer.h"
#include	"cmdmap.h"
#include	"readcmdkey.h"
#include	"termcmd.h"


/* local defines */

#define	BBINTER_MAGIC	0x5CD73921
#define	BBINTER_IPROMPT	"?"

#ifndef	SCANMARGIN
#define	SCANMARGIN	2
#endif

#ifndef	DISBUFLEN
#define	DISBUFLEN	80
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		100
#endif

#define	TR_OPTS		(FM_NOFILTER | FM_NOECHO | FM_RAWIN | FM_TIMED)

#undef	CNTRL
#define CNTRL(x)	((x) & 037)

#ifndef	CH_BELL
#define CH_BELL		'\07'
#endif

#ifndef	CH_DELETE
#define CH_DELETE	'\177'
#endif

#ifndef	TO_INFO
#define	TO_INFO		4
#endif

#ifndef	TO_LOCK
#define	TO_LOCK		4
#endif


/* external subroutines */

extern int	snsdd(char *,int,const char *,uint) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfnext(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	pcsmailcheck(const char *,char *,int,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	spawncmdproc(SPAWNPROC *,const char *,const char *) ;
extern int	uterm_readcmd(UTERM *,TERMCMD *,int,int) ;
extern int	hasallalnum(const char *,int) ;
extern int	hasprintbad(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	iscmdstart(int) ;

extern int	mailboxappend(const char *,int,int) ;
extern int	mkdisplayable(char *,int,const char *,int) ;
extern int	compactstr(char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	mkhexstr(char *,int,const void *,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */

static const char	*syscmdmap[] = {
	"%p/etc/%s/%s.%f",
	"%p/etc/%s/%f",
	"%p/etc/%s.%f",
	NULL
} ;

static const char	*usrcmdmap[] = {
	"%h/etc/%s/%s.%f",
	"%h/etc/%s/%f",
	"%h/etc/%s.%f",
	NULL
} ;

struct bbinter_fieldstr {
	const char	*fp ;
	int		fl ;
} ;

enum infotags {
	infotag_empty,
	infotag_unspecified,
	infotag_mailfrom,
	infotag_welcome,
	infotag_overlast
} ;


/* forward references */

static int	bbinter_calclines(BBINTER *) ;
static int	bbinter_sigbegin(BBINTER *) ;
static int	bbinter_sigend(BBINTER *) ;
static int	bbinter_utermbegin(BBINTER *) ;
static int	bbinter_utermend(BBINTER *) ;
static int	bbinter_loadcmdmap(BBINTER *) ;
static int	bbinter_loadcmdmapsc(BBINTER *,vecstr *) ;
static int	bbinter_loadcmdmapfile(BBINTER *,const char *) ;
static int	bbinter_loadcmdkey(BBINTER *,const char *,int) ;
static int	bbinter_loadcmdkeyone(BBINTER *,struct bbinter_fieldstr *) ;
static int	bbinter_refresh(BBINTER *) ;
static int	bbinter_cmdin(BBINTER *) ;
static int	bbinter_cmdinesc(BBINTER *,int) ;
static int	bbinter_cmddig(BBINTER *,int) ;
static int	bbinter_cmdhandle(BBINTER *,int) ;
static int	bbinter_charin(BBINTER *,const char *,...) ;
static int	bbinter_done(BBINTER *) ;
static int	bbinter_welcome(BBINTER *) ;
static int	bbinter_version(BBINTER *) ;
static int	bbinter_user(BBINTER *) ;
static int	bbinter_poll(BBINTER *) ;
static int	bbinter_checkclock(BBINTER *) ;
static int	bbinter_checkmail(BBINTER *) ;
static int	bbinter_checkmailinfo(BBINTER *,const char *) ;
static int	bbinter_info(BBINTER *,int,const char *,...) ;

static int	bbinter_mailstart(BBINTER *,const char *,int) ;
static int	bbinter_mailscan(BBINTER *) ;
static int	bbinter_mailend(BBINTER *,int) ;

static int	bbinter_mbopen(BBINTER *,const char *,int) ;
static int	bbinter_mbclose(BBINTER *) ;

static int	bbinter_msgnum(BBINTER *) ;
static int	bbinter_msgpoint(BBINTER *,int) ;
static int	bbinter_scancheck(BBINTER *,int,int) ;
static int	bbinter_change(BBINTER *) ;
static int	bbinter_input(BBINTER *,char *,int,const char *,...) ;
static int	bbinter_response(BBINTER *,char *,int,const char *,...) ;
static int	bbinter_havemb(BBINTER *,const char *,int) ;
static int	bbinter_mailempty(BBINTER *) ;
static int	bbinter_pathprefix(BBINTER *,const char *) ;
static int	bbinter_viewtop(BBINTER *,int) ;
static int	bbinter_viewnext(BBINTER *,int) ;

static int	bbinter_cmdpathprefix(BBINTER *) ;
static int	bbinter_cmdwrite(BBINTER *,int,int) ;
static int	bbinter_cmdpipe(BBINTER *,int,int) ;
static int	bbinter_cmdpage(BBINTER *,int) ;
static int	bbinter_cmdbody(BBINTER *,int) ;
static int	bbinter_cmdmove(BBINTER *,int) ;
static int	bbinter_cmddel(BBINTER *,int,int) ;
static int	bbinter_cmddelnum(BBINTER *,int,int) ;
static int	bbinter_cmdsubject(BBINTER *,int) ;
static int	bbinter_cmdshell(BBINTER *) ;

static int	bbinter_msgoutfile(BBINTER *,const char *,int,offset_t,int) ;
static int	bbinter_msgoutpipe(BBINTER *,const char *,offset_t,int) ;
static int	bbinter_msgoutview(BBINTER *,const char *,const char *) ;
static int	bbinter_msgmove(BBINTER *,const char *,offset_t,int) ;
static int	bbinter_msgdel(BBINTER *,int,int) ;
static int	bbinter_msgdelnum(BBINTER *,int,int,int) ;

static int	bbinter_msgviewopen(BBINTER *) ;
static int	bbinter_msgviewtop(BBINTER *,int) ;
static int	bbinter_msgviewadj(BBINTER *,int) ;
static int	bbinter_msgviewnext(BBINTER *,int) ;
static int	bbinter_msgviewclose(BBINTER *) ;
static int	bbinter_msgviewrefresh(BBINTER *) ;
static int	bbinter_msgviewload(BBINTER *,int,int,int) ;
static int	bbinter_msgviewsetlines(BBINTER *,int) ;

static int	bbinter_mbviewopen(BBINTER *) ;
static int	bbinter_mbviewclose(BBINTER *) ;

static int	bbinter_filecopy(BBINTER *,const char *,const char *) ;

#if	CF_INITTEST
static int	bbinter_test(BBINTER *) ;
#endif

static void	sighand_int(int) ;
static void	sighand_win(int) ;


/* local variables */

static volatile int	if_term = FALSE ;
static volatile int	if_quit = FALSE ;
static volatile int	if_int = FALSE ;
static volatile int	if_win = FALSE ;
static volatile int	if_def = FALSE ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	SIGHUP,
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	SIGWINCH,
	0
} ;

static const char	*cmds[] = {
	"quitquick",
	"refresh",
	"msginfo",
	"username",
	"version",
	"welcome",
	"quit",
	"zero",
	"scanfirst",
	"scanlast",
	"scannext",
	"scanprev",
	"scannextmult",
	"scanprevmult",
	"scantop",
	"return",
	"space",
	"viewtop",
	"viewnext",
	"viewprev",
	"viewnextmult",
	"viewprevmult",
	"change",
	"mbend",
	"cwd",
	"msgwrite", /* w */
	"bodywrite", /* B */
	"pagewrite", /* b */
	"msgpipe", /* | */
	"bodypipe", /* ¦ */
	"pagebody", /* v */
	"shell",
	"msgmove", /* m */
	"goto",
	"msgundeletenum",
	"msgdeletenum",
	"msgundelete",
	"msgdelete",
	"msgsubject",
	"help",
	"searchnext",
	NULL
} ;

enum cmds {
	cmd_quitquick,
	cmd_refresh,
	cmd_msginfo,
	cmd_username,
	cmd_version,
	cmd_welcome,
	cmd_quit,
	cmd_zero,
	cmd_scanfirst,
	cmd_scanlast,
	cmd_scannext,
	cmd_scanprev,
	cmd_scannextmult,
	cmd_scanprevmult,
	cmd_scantop,
	cmd_return,
	cmd_space,
	cmd_viewtop,
	cmd_viewnext,
	cmd_viewprev,
	cmd_viewnextmult,
	cmd_viewprevmult,
	cmd_change,
	cmd_mbend,
	cmd_cwd,
	cmd_msgwrite, /* w */
	cmd_bodywrite, /* B */
	cmd_pagewrite, /* b */
	cmd_msgpipe, /* | */
	cmd_bodypipe , /* ¦ */
	cmd_pagebody, /* v */
	cmd_shell,
	cmd_msgmove, /* m */
	cmd_goto , /* t */
	cmd_msgundeletenum,
	cmd_msgdeletenum,
	cmd_msgundelete,
	cmd_msgdelete,
	cmd_msgsubject,
	cmd_help,
	cmd_searchnext,
	cmd_overlast
} ;

static const CMDMAP_E	defcmds[] = {
	{ 'p',			cmd_scanprev },
	{ 'n',			cmd_scannext },
	{ 'P',			cmd_scanprevmult },
	{ 'N',			cmd_scannextmult },
	{ 'F',			cmd_scanfirst },
	{ 'L',			cmd_scanlast },
	{ 'd',			cmd_msgdelete },
	{ 'u',			cmd_msgundelete },
	{ 'm',			cmd_msgmove },
	{ 't',			cmd_goto },
	{ 'v',			cmd_pagebody },
	{ 'c',			cmd_change },
	{ 'S',			cmd_msgsubject },
	{ 'w',			cmd_msgwrite },
	{ 'B',			cmd_bodywrite },
	{ 'b',			cmd_pagewrite },
	{ 'C',			cmd_cwd },
	{ 'W',			cmd_welcome },
	{ 'V',			cmd_version },
	{ 'U',			cmd_username },
	{ 'E',			cmd_mbend },
	{ 'q',			cmd_quit },
	{ 'z',			cmd_zero },
	{ 'g',			cmd_viewtop },
	{ '=',			cmd_msginfo },
	{ '.',			cmd_msginfo },
	{ '|',			cmd_msgpipe },
	{ 0xA6,			cmd_bodypipe },
	{ '!',			cmd_shell },
	{ CH_CR,		cmd_return },
	{ CH_SP,		cmd_space },
	{ KEYSYM_EOT,		cmd_quitquick },
	{ KEYSYM_FormFeed,	cmd_refresh },
	{ KEYSYM_Plus,		cmd_viewnext },
	{ KEYSYM_Minus,		cmd_viewprev },
	{ KEYSYM_Next,		cmd_viewnextmult },
	{ KEYSYM_Previous,	cmd_viewprevmult },
	{ KEYSYM_Help,		cmd_help },
	{ KEYSYM_CurLeft,	cmd_scanprev },
	{ KEYSYM_CurRight,	cmd_scannext },
	{ KEYSYM_CurUp,		cmd_viewprev },
	{ KEYSYM_CurDown,	cmd_viewnext },
	{ KEYSYM_PF1,		cmd_scanprev },
	{ KEYSYM_PF2,		cmd_scannext },
	{ KEYSYM_PF3,		cmd_scanfirst },
	{ KEYSYM_PF4,		cmd_scanlast },
	{ KEYSYM_Find,		cmd_searchnext },
	{ KEYSYM_Remove,	cmd_msgdelete },
	{ -1, -1 },
} ;


/* exported subroutines */


int bbinter_start(iap,pip)
BBINTER		*iap ;
struct proginfo	*pip ;
{
	struct ustat	sb ;

	DISPLAY_ARGS	da ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	rows, cols ;
	int	f ;

	const char	*pn = pip->progname ;
	const char	*un = pip->username ;
	const char	*ccp ;

	char	tmpdname[MAXPATHLEN + 1] = { 0 } ;
	char	tmpfname[MAXPATHLEN + 1] = { 0 } ;


	if (iap == NULL)
	    return SR_FAULT ;

	if_term = 0 ;
	if_quit = 0 ;
	if_int = 0 ;
	if_win = 0 ;
	if_def = 0 ;

	memset(iap,0,sizeof(struct bbinter_head)) ;
	iap->pip = pip ;
	iap->ti_start = pip->daytime ;
	iap->plen = strlen(BBINTER_IPROMPT) ;
	iap->mfd = -1 ;
	iap->miviewpoint = -1 ;
	iap->delmark = '*' ; 

	rs = bbinter_calclines(iap) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_start: bbinter_calclines() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = bbinter_sigbegin(iap) ;
	if (rs < 0)
	    goto bad0 ;

	if (pip->vmdname == NULL) {
	    if ((rs = mktmpuserdir(tmpdname,un,pn,VMDMODE)) >= 0)
	        rs = proginfo_setentry(pip,&pip->vmdname,tmpdname,rs) ;
	}
	if (rs < 0) goto badvmdname ;

	if (pip->pwd == NULL) rs = proginfo_pwd(iap->pip) ;

	if (rs >= 0)
	    rs = bbinter_pathprefix(iap,pip->pwd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_start: bbinter_pathprefix() rs=%d\n",rs) ;
	    debugprintf("bbinter_start: pwd=%s\n",pip->pwd) ;
	}
#endif

	if (rs < 0)
	    goto badpathprefix ;

	rs = bbinter_utermbegin(iap) ;
	if (rs < 0) goto baduterm ;

	rs = cmdmap_start(&iap->cm,defcmds) ;
	iap->f.cminit = (rs >= 0) ;
	if (rs < 0) goto badcmdmap ;

	rs1 = keysymer_open(&iap->ks,pip->pr) ;
	iap->f.ksinit = (rs1 >= 0) ;
	if (rs < 0) goto badkeysymer ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_start: keysymer_open() rs1=%d\n",rs1) ;
#endif

	ccp = pip->kbdtype ;
	if ((ccp != NULL) && (ccp[0] != '\0') && iap->f.ksinit) {
	    rs1 = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_start: ccp=%s\n",ccp) ;
#endif
	    rs = mkpath3(tmpfname,pip->pr,KBDNAME,ccp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_start: tmpfname=%s\n",tmpfname) ;
#endif

	    if (rs >= 0) {
		rs1 = u_stat(tmpfname,&sb) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_start: u_stat() rs1=%d\n",rs1) ;
#endif

	    }

	    if ((rs >= 0) && (rs1 >= 0))
		rs1 = sperm(&pip->id,&sb,R_OK) ;

#if	CF_KBDINFO
	    if ((rs >= 0) && (rs1 >= 0)) {
	        rs = kbdinfo_open(&iap->ki,&iap->ks,tmpfname) ;
	        iap->f.kiinit = (rs >= 0) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_start: kbdinfo_open() rs=%d\n",rs) ;
#endif
	    }
#endif /* CF_KBDINFO */

	}
	if (rs < 0)
	    goto badkbdinfo ;

	if (iap->f.kiinit) {
	    rs = bbinter_loadcmdmap(iap) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_start: bbinter_loadcmdmap() rs=%d\n",rs) ;
#endif
	}

	if (rs < 0)
	    goto badloadcmdmap ;

/* message-directory setup */

	rs = mkpath2(tmpdname,iap->vmdname,MSGDNAME) ;
	if (rs >= 0)
	    rs = mkdirs(tmpdname,VMDMODE) ;

	if (rs >= 0) {
	    rs = mailmsgfile_start(&iap->msgfiles,tmpdname,pip->linelen,-1) ;
	    iap->f.mfinit = (rs >= 0) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_start: mailmsgfile_start() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badmailmsgfile ;

/* setup the display */

	rows = iap->displines ;
	cols = pip->linelen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_start: rows=%d cols=%d\n",rows,cols) ;
#endif

/* initialize display manager */

#if	CF_DISPLAY

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_start: displines=%u\n",iap->displines) ;
	    debugprintf("bbinter_start: scanlines=%u\n",iap->scanlines) ;
	}
#endif

	memset(&da,0,sizeof(DISPLAY_ARGS)) ;
	da.termtype = pip->termtype ;
	da.tfd = iap->tfd ;
	da.displines = iap->displines ;
	da.scanlines = iap->scanlines ;
	rs = display_start(&iap->di,pip,&da) ;
#endif /* CF_DISPLAY */

	if (rs < 0)
	    goto bad4 ;

/* put up the standard display */

#ifdef	COMMENT
	display_setdate(&iap->di,FALSE) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_start: bbinter_test() rs=%d\n",rs) ;
#endif

/* put the "welcome" message into the "info" area */

/* setup the display frame */

	if (rs >= 0)
	    rs = display_rframe(&iap->di) ;

#if	CF_INITTEST
	if (rs >= 0)
	    rs = bbinter_test(iap) ;
#endif

	if (rs >= 0) {
	    rs = bbinter_mailstart(iap,pip->mbname_cur,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_start: bbinter_mailstart() rs=%d\n",rs) ;
#endif

	}

#if	CF_WELCOME 
	if (rs >= 0) {
	    time_t	ts ;
	    rs1 = display_infots(&iap->di,&ts) ;
	    f = (rs1 >= 0) ;
	    f = f && ((ts == 0) || (pip->daytime > (ts + 5))) ;
	    if (f && (! iap->f.welcome)) {
	        iap->f.welcome = TRUE ;
	        rs = bbinter_welcome(iap) ;
	    }
	}
#endif /* CF_WELCOME */

	if (rs < 0)
	    goto bad5 ;

	iap->magic = BBINTER_MAGIC ;

/* done */
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad5:

#if	CF_DISPLAY
	display_finish(&iap->di) ;
#endif /* CF_DISPLAY */

bad4:
bad3:
	if (iap->f.mfinit) {
	    iap->f.mfinit = FALSE ;
	    mailmsgfile_finish(&iap->msgfiles) ;
	}

badmailmsgfile:
badloadcmdmap:
	if (iap->f.kiinit) {
	    iap->f.kiinit = FALSE ;
	    kbdinfo_close(&iap->ki) ;
	}

badkbdinfo:
	if (iap->f.ksinit) {
	    iap->f.ksinit = FALSE ;
	    keysymer_close(&iap->ks) ;
	}

badkeysymer:
	if (iap->f.uterminit) {
	    bbinter_utermend(iap) ;
	}

baduterm:
	if (iap->f.cminit) {
	    iap->f.cminit = FALSE ;
	    cmdmap_finish(&iap->cm) ;
	}

badcmdmap:
	if (iap->pathprefix != NULL) {
	    uc_free(iap->pathprefix) ;
	    iap->pathprefix = NULL ;
	}

badvmdname:
badpathprefix:
	bbinter_sigend(iap) ;

bad0:
	goto ret0 ;
}
/* end subroutine (bbinter_start) */


int bbinter_finish(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap == NULL) return SR_FAULT ;

	if (iap->magic != BBINTER_MAGIC) return SR_NOTOPEN ;

	rs1 = bbinter_mbviewclose(iap) ;
	if (rs >= 0) rs = rs1 ;

	if (iap->f.mvinit) {
	    iap->f.mvinit = FALSE ;
	    rs1 = mailmsgviewer_close(&iap->mv) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->mbname != NULL) {
	    rs1 = uc_free(iap->mbname) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->mbname = NULL ;
	}

	if (iap->f.mcinit || iap->f.mbopen) {
	    rs1 = bbinter_mbclose(iap) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DISPLAY
	rs1 = display_finish(&iap->di) ;
	if (rs >= 0) rs = rs1 ;
#endif /* CF_DISPLAY */

	if (iap->f.mfinit) {
	    iap->f.mfinit = FALSE ;
	    rs1 = mailmsgfile_finish(&iap->msgfiles) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->f.kiinit) {
	    iap->f.kiinit = FALSE ;
	    rs1 = kbdinfo_close(&iap->ki) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->f.ksinit) {
	    iap->f.ksinit = FALSE ;
	    rs1 = keysymer_close(&iap->ks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->f.cminit) {
	    iap->f.cminit = FALSE ;
	    rs1 = cmdmap_finish(&iap->cm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bbinter_utermend(iap) ;
	if (rs >= 0) rs = rs1 ;

	if (iap->pathprefix != NULL) {
	    rs1 = uc_free(iap->pathprefix) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->pathprefix = NULL ;
	}

	rs1 = bbinter_sigend(iap) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_finish: ret rs=%d\n",rs) ;
#endif

	iap->magic = 0 ;
	return rs ;
}
/* end subroutine (bbinter_finish) */


int bbinter_cmd(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cmd ;
	int	rc = 1 ;
	int	f ;

	const char	*pp = BBINTER_IPROMPT ;


	if (iap == NULL)
	    return SR_FAULT ;

	if (iap->magic != BBINTER_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_cmd: entered\n") ;
#endif

#if	CF_WELCOME
	if (rs >= 0) {
	    time_t	ts ;
	    rs1 = display_infots(&iap->di,&ts) ;
	    f = (rs1 >= 0) ;
	    f = f && (pip->daytime > (ts + 7)) ;
	    f = f && (pip->daytime > (iap->ti_start + 3)) ;
	    if (f && (! iap->f.welcome)) {
	        iap->f.cmdprompt = FALSE ;
	        iap->f.welcome = TRUE ;
	        rs = bbinter_welcome(iap) ;
	    }
	}
#endif /* CF_WELCOME */

	if ((rs >= 0) && if_int) {
	    if_int = 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("bbinter_cmd: bbinterrupt\n") ;
#endif

	    iap->f.cmdprompt = FALSE ;
	    rs = bbinter_refresh(iap) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_cmd: f_cmdprompt=%u\n",iap->f.cmdprompt) ;
#endif

	if ((rs >= 0) && (! iap->f.cmdprompt)) {
	    iap->f.cmdprompt = TRUE ;
	    rs = display_input(&iap->di,"%s %t\v",
	        pp,iap->numbuf,iap->numlen) ;
	}

	if (rs >= 0) {
	    rs = bbinter_cmdin(iap) ;
	    cmd = rs ;
	    if (rs > 0) {
	        if (isdigitlatin(cmd) || (cmd == CH_DEL) || (cmd == CH_BS)) {
	            rs = bbinter_cmddig(iap,cmd) ;
	        } else {
	            iap->f.cmdprompt = FALSE ;
	            rs = bbinter_cmdhandle(iap,cmd) ;
	        }
	    }
	}

	if (if_term) {
	    iap->f.exit = TRUE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_cmd: SIGTERM rs=%d\n",rs) ;
#endif
	}

#if	CF_BBINTERPOLL
	if ((rs >= 0) && (! iap->f.exit)) {
	    rs = bbinter_poll(iap) ;
	    if (rs > 0)
	        iap->f.cmdprompt = FALSE ;
	}
#endif

	if ((rs >= 0) && iap->f.exit)
	    rs = bbinter_done(iap) ;

	rc = (iap->f.exit) ? 0 : 1 ;

	if ((rs >= 0) && if_term) {
	    rs = SR_INTR ;
	    if (! pip->f.quiet) {
		const char	*pn = pip->progname ;
		bprintf(pip->efp,"%s: exiting on termination\n",pn) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_cmd: ret rs=%d rc=%d\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (bbinter_cmd) */


/* private subroutines */


static int bbinter_calclines(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	double	percent = 0.0 ;

	int	rs = SR_OK ;
	int	f_percent ;


/* display-lines */

	iap->displines = pip->lines ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_calclines: displines=%u\n",iap->displines) ;
#endif

/* scan-lines */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_calclines: svlines=%u\n",pip->svlines) ;
#endif

	iap->scanlines = pip->svlines ;
	f_percent = FALSE ;
	if (pip->svlines == 0) {
	    f_percent = TRUE ;
	    percent = SCANPERCENT ;
	} else if (pip->f.svpercent) {
	    f_percent = TRUE ;
	    percent = pip->svlines ;
	}

	if (f_percent)
	    iap->scanlines = (int) (iap->displines * percent / 100.0) ;

	if (iap->scanlines < MINSCANLINES)
	    iap->scanlines = MINSCANLINES ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_calclines: scanlines=%u%c\n",
		iap->scanlines,((f_percent) ? '%' : ' ')) ;
#endif

/* jump-lines */

	f_percent = FALSE ;
	iap->jumplines = pip->sjlines ;
	if (pip->sjlines == 0) {
	    iap->jumplines = (iap->scanlines - 1) ;
	} else if (pip->f.sjpercent) {
	    f_percent = TRUE ;
	    percent = pip->sjlines ;
	}

	if (f_percent)
	    iap->jumplines = (int) (iap->scanlines * percent / 100.0) ;

	if (iap->jumplines < 1)
	    iap->jumplines = 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_calclines: jumplines=%u%c\n",
		iap->jumplines,((f_percent) ? '%' : ' ')) ;
#endif

/* view-lines */

	iap->viewlines = (iap->displines - iap->scanlines - FRAMELINES) ;

	return rs ;
}
/* end subroutine (bbinter_calclines) */


static int bbinter_sigbegin(iap)
BBINTER		*iap ;
{
	struct sigaction	san ;

	sigset_t	newsigmask ;

	int	rs = SR_OK ;
	int	n, i, j ;
	int	size ;


	n = nelem(sigints) + nelem(sigignores) ;
	size = n * sizeof(struct sigaction) ;
	rs = uc_malloc(size,&iap->sao) ;
	if (rs < 0)
	    goto ret0 ;

	memset(iap->sao,0,size) ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;

	for (i = 0 ; sigblocks[i] != 0 ; i += 1)
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;

#if	defined(PTHREAD) && PTHREAD
	pthread_sigmask(SIG_BLOCK,&newsigmask,&iap->oldsigmask) ;
#else
	u_sigprocmask(SIG_BLOCK,&newsigmask,&iap->oldsigmask) ;
#endif

/* ignore these signals */

	uc_sigsetempty(&newsigmask) ;

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;
	    san.sa_handler = SIG_IGN ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigignores[i],&san,(iap->sao + j)) ;

	    j += 1 ;

	} /* end for */

/* bbinterrupt on these signals */

	for (i = 0 ; sigints[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;
	    san.sa_handler = sighand_int ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigints[i],&san,(iap->sao + j)) ;

	    j += 1 ;

	} /* end for */

	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	{
	    uc_free(iap->sao) ;
	    iap->sao = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (bbinter_sigbegin) */


static int bbinter_sigend(iap)
BBINTER		*iap ;
{
	int	rs = SR_OK ;
	int	i, j ;


	if (iap->sao == NULL)
	    return SR_NOANODE ;

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1)
	    u_sigaction(sigignores[i],(iap->sao + j++),NULL) ;

	for (i = 0 ; sigints[i] != 0 ; i += 1)
	    u_sigaction(sigints[i],(iap->sao + j++),NULL) ;

#if	defined(PTHREAD) && PTHREAD
	pthread_sigmask(SIG_SETMASK,&iap->oldsigmask,NULL) ;
#else
	u_sigprocmask(SIG_SETMASK,&iap->oldsigmask,NULL) ;
#endif

	{
	    uc_free(iap->sao) ;
	    iap->sao = NULL ;
	}

	return rs ;
}
/* end subroutine (bbinter_sigend) */


static int bbinter_utermbegin(BBINTER *iap)
{
	struct proginfo	*pip = iap->pip ;
	int	rs ;

	if ((rs = uterm_start(&iap->ut,iap->tfd)) >= 0) {
	    if ((rs = uterm_control(&iap->ut,fc_setmode,fm_notecho)) >= 0) {
		iap->f.uterminit = TRUE ;
	    }
	    if (rs < 0)
		uterm_finish(&iap->ut) ;
	}

	return rs ;
}
/* end subroutine (bbinter_utermbegin) */


static int bbinter_utermend(BBINTER *iap)
{
	struct proginfo	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->f.uterminit) {
	    iap->f.uterminit = FALSE ;
	    rs1 = uterm_finish(&iap->ut) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (bbinter_utermend) */


static int bbinter_loadcmdmap(BBINTER *iap)
{
	struct proginfo	*pip = iap->pip ;

	VECSTR	sc ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*cmfname = CMDMAPFNAME ;


	if ((rs = vecstr_start(&sc,3,0)) >= 0) {
	    const int	flen = MAXPATHLEN ;
	    const char	**sa ;
	    char	fbuf[MAXPATHLEN + 1] ;

	    if ((rs = bbinter_loadcmdmapsc(iap,&sc)) >= 0) {

		sa = syscmdmap ; /* first: system (S) CMDMAP */
		if (rs >= 0) {
	            rs1 = permsched(sa,&sc, fbuf,flen,cmfname,R_OK) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmap: S rs1=%d fname=%s\n",rs1,fbuf) ;
#endif
		    if (rs1 >= 0) rs = bbinter_loadcmdmapfile(iap,fbuf) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmap: S bbinter_loadcmdmapfile() rs=%d\n",rs) ;
#endif
		}

		sa = usrcmdmap ; /* second: user (U) CMDMAP */
		if (rs >= 0) {
	            rs1 = permsched(sa,&sc, fbuf,flen,cmfname,R_OK) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmap: U rs1=%d fname=%s\n",rs1,fbuf) ;
#endif
		    if (rs1 >= 0) rs = bbinter_loadcmdmapfile(iap,fbuf) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmap: U bbinter_loadcmdmapfile() rs=%d\n",rs) ;
#endif
		}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmap: mid rs=%d\n",rs) ;
#endif
	    } /* end if (bbinter_loadcmdmapsc) */

	    vecstr_finish(&sc) ;
	} /* end if (sched-comp) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmap: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_loadcmdmap) */


static int bbinter_loadcmdmapsc(BBINTER *iap,vecstr *scp)
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	i ;

	const char	*keys = "hps" ;
	const char	*vp ;
	char	kbuf[2] = { 0, 0 } ;

	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    kbuf[0] = (keys[i] & 0xff) ;
	    kbuf[1] = '\0' ;
	    vp = NULL ;
	    switch (i) {
	    case 0:
		vp = pip->homedname ;
		break ;
	    case 1:
		vp = pip->pr ;
		break ;
	    case 2:
		vp = pip->searchname ;
		break ;
	    } /* end switch */
	    if (vp != NULL) rs = vecstr_envadd(scp,kbuf,vp,-1) ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (bbinter_loadcmdmapsc) */


static int bbinter_loadcmdmapfile(BBINTER *iap,const char *fname)
{
	struct proginfo	*pip = iap->pip ;

	bfile	cfile, *cfp = &cfile ;

	int	rs = SR_OK ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmapfile: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if (! iap->f.kiinit) goto ret0 ;

	if (pip->kbdtype == NULL) goto ret0 ;

	if ((rs = bopen(cfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		sl ;
	    const char	*sp, *tp ;
	    char	lbuf[LINEBUFLEN+1] ;

	    while ((rs = breadline(cfp,lbuf,llen)) > 0) {
	        len = rs ;

		if (lbuf[len-1] == '\n') len -= 1 ;
		lbuf[len] = '\0' ;

		if ((tp = strnrchr(lbuf,len,'#')) != NULL) len = (tp-lbuf) ;
		
		sl = sfshrink(lbuf,len,&sp) ;

		if (sl && (sp[0] == '#')) continue ;

		if (sl > 0)
	            rs = bbinter_loadcmdkey(iap,sp,sl) ;

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
		debugprintf("bbinter_loadcmdmapfile: reading sl=%u rs=%d\n",
		sl,rs) ;
#endif

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    bclose(cfp) ;
	} /* end if (CMDMAP-file) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_loadcmdmapfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_loadcmdmapfile) */


static int bbinter_loadcmdkey(iap,sp,sl)
BBINTER		*iap ;
const char	sp[] ;
int		sl ;
{
	struct proginfo	*pip = iap->pip ;

	struct bbinter_fieldstr	fs[4] ;

	int	rs = SR_OK ;
	int	i ;
	int	fl ;
	int	f_loaded = FALSE ;

	const char	*fp ;


#ifdef	COMMENT
	for (i = 0 ; i < nelem(fs) ; i += 1) fs[i].fl = 0 ;
#else
	memset(fs,0,sizeof(struct bbinter_fieldstr)*4) ;
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkey: v=>%t<\n",sp,sl) ;
#endif

	if (sl < 0) sl = ((sp != NULL) ? strlen(sp) : 0) ;

	for (i = 0 ; (i < nelem(fs)) && sl && *sp &&
		((fl = sfnext(sp,sl,&fp)) >= 0) ; i += 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkey: f=>%t<\n",fp,fl) ;
#endif

	    fs[i].fp = fp ;
	    fs[i].fl = fl ;

	    sl -= ((fp+fl)-sp) ;
	    sp = (fp+fl) ;

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkey: mid rs=%d i=%u\n",rs,i) ;
#endif

	if (i >= 3) {
	    f_loaded = TRUE ;
	    rs = bbinter_loadcmdkeyone(iap,fs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkey: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? f_loaded : rs ;
}
/* end subroutine (bbinter_loadcmdkey) */


static int bbinter_loadcmdkeyone(iap,fs)
BBINTER		*iap ;
struct bbinter_fieldstr	*fs ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 = 0 ;
	int	key = 0 ;
	int	cmd = 0 ;
	int	cl ;
	int	f ;

	const char	*cp ;


	cp = fs[1].fp ;
	cl = fs[1].fl ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkeyone: termtype=%t\n",cp,cl) ;
#endif

#ifdef	COMMENT
	m = nleadstr(pip->kbdtype,cp,cl) ;
	f = ((m > 0) && (cl == m) && (pip->kbdtype[m] == '\0')) ;
#else
	f = (strncmp(pip->kbdtype,cp,cl) == 0) && (pip->kbdtype[cl] == '\0') ;
#endif /* COMMENT */

	if (! f)
	    goto ret0 ;

	if (fs[2].fl == 1) {

	    key = (fs[2].fp[0] & 0xff) ;

	} else {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkeyone: keyname=%t\n",
	        fs[2].fp,fs[2].fl) ;
#endif

	    rs1 = SR_NOTFOUND ;
	    if (iap->f.ksinit) {
	        rs1 = keysymer_lookup(&iap->ks,fs[2].fp,fs[2].fl) ;
	        key = rs1 ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkeyone: mid rs1=%d key=\\x%04X\n",
		rs1,key) ;
#endif

	if (rs1 < 0)
	    goto ret0 ;

	if ((cmd = matstr(cmds,fs[0].fp,fs[0].fl)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_loadcmdkeyone: "
		"cmdmap_load() key=\\x%04X cmd=%s(%u)\n",
		key,cmds[cmd],cmd) ;
	}
#endif

	    rs = cmdmap_load(&iap->cm,key,cmd) ;
	} /* end if (found command) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_loadcmdkeyone: ret rs=%d cmd=%u\n",
		rs,(cmd>=0)) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_loadcmdkeyone) */


#if	CF_INITTEST

static int bbinter_test(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	w = 0 ;

	rs = display_input(&iap->di,"initializing ... \v") ;
	if (rs >= 0)
	    display_flush(&iap->di) ;

	sleep(1) ;

	if (rs >= 0) {
	    display_scanclear(&iap->di) ;
	    display_viewclear(&iap->di) ;
	}

	return rs ;
}
/* end subroutine (bbinter_test) */

#endif /* CF_INITTEST */


static int bbinter_refresh(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs ;


	rs = display_refresh(&iap->di) ;

	if (rs >= 0) {
	    if (iap->f.mvinit) {
	        rs = bbinter_msgviewrefresh(iap) ;
	    } else
		rs = display_viewclear(&iap->di) ;
	}

	return rs ;
}
/* end subroutine (bbinter_refresh) */


static int bbinter_cmdin(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs ;
	int	to ;
	int	ropts = TR_OPTS ;
	int	cmd = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;


	to = pip->to_read ;
	rs = display_flush(&iap->di) ;
	if (rs < 0)
	    goto ret0 ;

	linebuf[0] = '\0' ;
	rs = uterm_reade(&iap->ut,linebuf,1,to,ropts,NULL,NULL) ;

	if (rs > 0) {
	    cmd = (linebuf[0] & 0xff) ;
	    if (iscmdstart(cmd)) {
		rs = bbinter_cmdinesc(iap,cmd) ;
		cmd = rs ;
	    }
	}

	pip->daytime = time(NULL) ;
	pip->now.time = pip->daytime ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_cmdin: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (bbinter_cmdin) */


static int bbinter_cmdinesc(iap,ch)
BBINTER		*iap ;
int		ch ;
{
	struct proginfo	*pip = iap->pip ;

	TERMCMD	ck ;

	int	rs ;
	int	keynum = 0 ;
	int	to ;


	to = pip->to_read ;
	rs = uterm_readcmd(&iap->ut,&ck,to,ch) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("bbinter_cmdinesc: readcmdkey() rs=%d\n",rs) ;
	debugprintf("bbinter_cmdinesc: ktype=%d kname=%d\n",
		ck.type,ck.name) ;
	}
#endif

	if (rs < 0) goto ret0 ;

	if (ck.type < 0) goto ret0 ;

	if (! iap->f.kiinit) goto ret0 ;

	rs = kbdinfo_lookup(&iap->ki,NULL,0,&ck) ;
	keynum = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_cmdinesc: kbdinfo_lookup() rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	    keynum = 0 ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_cmdinesc: ret rs=%d keynum=\\x%04x\n",rs,keynum) ;
#endif

	return (rs >= 0) ? keynum : rs ;
}
/* end subroutine (bbinter_cmdinesc) */


static int bbinter_cmddig(iap,cmd)
BBINTER		*iap ;
int		cmd ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	pl = (iap->plen + 1) ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int ch = (isprintlatin(cmd)) ? cmd : ' ' ;
	    debugprintf("bbinter_cmddig: numlen=%u dig=>%c< (%02X)\n",
	        iap->numlen,ch,cmd) ;
	}
#endif

	if (cmd == CH_BS)
	    cmd = CH_DEL ;

	if ((cmd == CH_DEL) && (iap->numlen > 0)) {
	    iap->numlen -= 1 ;
	    rs = display_cmddig(&iap->di,(iap->numlen+pl),cmd) ;
	    iap->numbuf[iap->numlen] = '\0' ;
	} else if (isdigitlatin(cmd) && (iap->numlen < BBINTER_NUMLEN)) {
	    iap->numbuf[iap->numlen] = cmd ;
	    rs = display_cmddig(&iap->di,(iap->numlen+pl),cmd) ;
	    iap->numlen += 1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (bbinter_cmddig) */


static int bbinter_cmdhandle(iap,key)
BBINTER		*iap ;
int		key ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	argnum = -1 ;
	int	cmd ;
	int	mult = 1 ;
	int	nmsg = 0 ;
	int	f_nomailbox = FALSE ;
	int	f_errinfo = iap->f.info_err ;
	int	f = FALSE ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int ch = (isprintlatin(key)) ? key : ' ' ;
	    debugprintf("bbinter_cmdhandle: key=»%c« (\\x%04X)\n",
	        ch,key) ;
	}
#endif

	iap->f.info_err = FALSE ;
	if (iap->numlen > 0) {
	    rs1 = cfdeci(iap->numbuf,iap->numlen,&argnum) ;
	    if (rs1 < 0)
	        argnum = -1 ;
	} /* end if */

	cmd = cmdmap_lookup(&iap->cm,key) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3) && (cmd >= 0)) {
	    debugprintf("bbinter_cmdhandle: cmdmap_lookup() cmd=%s (%d)\n",
		((cmd < cmd_overlast) ? cmds[cmd] : ""),cmd) ;
	}
#endif

	switch (cmd) {

	case cmd_refresh:
	    rs = bbinter_refresh(iap) ;
	    if (rs >= 0)
	        rs = bbinter_info(iap,FALSE,"refreshed\v") ;
	    break ;

	case cmd_msginfo:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    rs = bbinter_msgnum(iap) ;
	    break ;

	case cmd_username:
	    rs = bbinter_user(iap) ;
	    break ;

	case cmd_version:
	    rs = bbinter_version(iap) ;
	    break ;

	case cmd_welcome:
	    rs = bbinter_welcome(iap) ;
	    break ;

	case cmd_quitquick:
	case cmd_quit:
	    iap->f.exit = TRUE ;
	    if (iap->f.mcinit) {
		int f = (cmd == cmd_quitquick) ;
	        rs = bbinter_mailend(iap,f) ;
	    }
	    if (rs >= 0) rs = display_setdate(&iap->di,TRUE) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_cmdhandle: display_setdate() rs=%d\n",rs) ;
#endif
	    break ;

	case cmd_zero:
	    rs = display_scanclear(&iap->di) ;
	    if (rs >= 0)
	        rs = display_viewclear(&iap->di) ;
	    break ;

	case cmd_scanfirst:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    nmsg = 0 ;
	    if (nmsg != iap->miscanpoint)
	        rs = bbinter_msgpoint(iap,nmsg) ;
	    break ;

	case cmd_scanlast:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    nmsg = (iap->miscanpoint + 10000) ;
	    if (nmsg >= iap->nmsgs)
	        nmsg = (iap->nmsgs - 1) ;
	    if (nmsg != iap->miscanpoint)
	        rs = bbinter_msgpoint(iap,nmsg) ;
	    break ;

	case cmd_scannext:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg += (argnum > 0) ? argnum : 1 ;
	    if (nmsg < 0)
	        nmsg = 0 ;
	    if ((nmsg != iap->miscanpoint) && (nmsg >= 0))
	        rs = bbinter_msgpoint(iap,nmsg) ;
	    break ;

	case cmd_scanprev:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg -= (argnum > 0) ? argnum : 1 ;
	    if (nmsg < 0)
	        nmsg = 0 ;
	    if ((nmsg != iap->miscanpoint) && (nmsg >= 0))
	        rs = bbinter_msgpoint(iap,nmsg) ;
	    break ;

	case cmd_scannextmult:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    mult = iap->jumplines ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg += (argnum > 0) ? (argnum * mult) : mult ;
	    if (nmsg >= iap->nmsgs)
	        nmsg = (iap->nmsgs - 1) ;
	    if ((nmsg != iap->miscanpoint) && (nmsg < iap->nmsgs))
	        rs = bbinter_msgpoint(iap,nmsg) ;
	    break ;

	case cmd_scanprevmult:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    mult = iap->jumplines ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg -= (argnum > 0) ? (argnum * mult) : mult ;
	    if (nmsg < 0)
	        nmsg = 0 ;
	    if ((nmsg != iap->miscanpoint) && (nmsg >= 0))
	        rs = bbinter_msgpoint(iap,nmsg) ;
	    break ;

	case cmd_goto:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox)
		break ;

/* FALLTHROUGH */
	case cmd_return:
	    if (argnum > 0) {
	        nmsg = (argnum - 1) ;
	        if (nmsg >= iap->nmsgs)
	            nmsg = (iap->nmsgs - 1) ;
	        if (nmsg != iap->miscanpoint) {
		    iap->f.viewchange = TRUE ;
	            rs = bbinter_msgpoint(iap,nmsg) ;
	        }
	    }
	    f = (cmd == cmd_return) || (cmd == cmd_viewnextmult) ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0) && f) {
		rs = bbinter_viewnext(iap,(iap->viewlines-1)) ;
	    }
	    break ;

	case cmd_space:
	    if ((rs >= 0) && (iap->miscanpoint >= 0))
		rs = bbinter_viewnext(iap,(iap->viewlines-1)) ;
	    break ;

	case cmd_viewtop:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0))
		rs = bbinter_viewtop(iap,argnum) ;
	    break ;

	case cmd_viewnextmult:
		f = TRUE ;

/* FALLTHROUGH */
	case cmd_viewnext:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->f.mvinit) {
	        if (argnum < 1) argnum = 1 ;
		if (f)
		    argnum = argnum * (iap->viewlines - 1) ;
	    } else
		argnum = 0 ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0))
		rs = bbinter_viewnext(iap,(+ argnum)) ;
	    break ;

	case cmd_viewprevmult:
		f = TRUE ;

/* FALLTHROUGH */
	case cmd_viewprev:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->f.mvinit) {
	        if (argnum < 1) argnum = 1 ;
		if (f)
		    argnum = argnum * (iap->viewlines - 1) ;
	    } else
		argnum = 0 ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0))
		rs = bbinter_viewnext(iap,(- argnum)) ;
	    break ;

	case cmd_change:
	    rs = bbinter_change(iap) ;
	    break ;

	case cmd_mbend:
	    rs = bbinter_mailend(iap,FALSE) ;
	    if (rs >= 0)
		rs = bbinter_mailempty(iap) ;
	    break ;

	case cmd_cwd:
	    rs = bbinter_cmdpathprefix(iap) ;
	    break ;

	case cmd_msgwrite:
	case cmd_bodywrite:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmdwrite(iap,(cmd == cmd_msgwrite),argnum) ;
	    break ;

	case cmd_pagewrite:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmdbody(iap,argnum) ;
	    break ;

	case cmd_msgpipe:
	case cmd_bodypipe:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmdpipe(iap,(cmd == cmd_msgpipe),argnum) ;
	    break ;

	case cmd_pagebody:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmdpage(iap,argnum) ;
	    break ;

	case cmd_shell:
	    rs = bbinter_cmdshell(iap) ;
	    break ;

	case cmd_msgmove:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmdmove(iap,argnum) ;
	    break ;

	case cmd_msgundelete:
	case cmd_msgdelete:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    f = (cmd == cmd_msgdelete) ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmddel(iap,f,argnum) ;
	    break ;

	case cmd_msgundeletenum:
	case cmd_msgdeletenum:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    f = (cmd == cmd_msgdeletenum) ;
	    if (iap->miscanpoint >= 0)
	        rs = bbinter_cmddelnum(iap,f,argnum) ;
	    break ;

	case cmd_msgsubject:
	    f_nomailbox = (! iap->f.mcinit) ;
	    if (f_nomailbox) break ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0))
	        rs = bbinter_cmdsubject(iap,argnum) ;
	    break ;

	default:
	    rs = bbinter_info(iap,TRUE,"invalid command\v") ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int ch = (isprintlatin(cmd)) ? key : ' ' ;
	    debugprintf("bbinter_cmdhandle: unknown key=»%c« (\\x%04X)\n",
	        ch,key) ;
	}
#endif
	    break ;

	} /* end switch */

/* error-info message handling */

	if (f_nomailbox)
	    bbinter_info(iap,TRUE,"no current mailbox\v") ;

	if (f_errinfo && (! iap->f.info_msg))
	    display_info(&iap->di,"\v") ;

	iap->f.info_msg = FALSE ;

/* done */

	iap->numlen = 0 ;
	iap->numbuf[0] = '\0' ;
	return rs ;
}
/* end subroutine (bbinter_cmdhandle) */


static int bbinter_info(BBINTER *iap,int f_err,const char *fmt,...)
{
	struct proginfo	*pip = iap->pip ;

	va_list	ap ;

	int	rs = SR_OK ;


	iap->f.info_msg = TRUE ;
	iap->f.info_err = f_err ;

	{
		va_begin(ap,fmt) ;
		rs = display_vinfo(&iap->di,fmt,ap) ;
		va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (bbinter_info) */


static int bbinter_done(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs ;


	rs = display_done(&iap->di) ;

	return rs ;
}
/* end subroutine (bbinter_done) */


static int bbinter_welcome(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;


	iap->ti_info = pip->daytime ;
	pip->to_info = 3 ;
	if (iap->tag_info == infotag_welcome)
	    goto ret0 ;

	iap->tag_info = infotag_welcome ;
	rs = bbinter_info(iap,FALSE,
	    "welcome %s - type ? for help\v",pip->name) ;

ret0:
	return rs ;
}
/* end subroutine (bbinter_welcome) */


static int bbinter_version(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;


	iap->tag_info = infotag_unspecified ;
	rs = bbinter_info(iap,FALSE,
	    "%s PCS VMAIL(%s) v=%s\v",
	    pip->org,pip->progname,
	    VERSION) ;

	return rs ;
}
/* end subroutine (bbinter_version) */


static int bbinter_user(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	const char	*org = pip->org ;
	char		buf[BUFLEN + 1] ;

	if ((rs = sbuf_start(&b,buf,BUFLEN)) >= 0) {

	    if ((org != NULL) && (org[0] != '\0')) {

	        sbuf_strw(&b,org,-1) ;

	        sbuf_strw(&b," - ",3) ;
	    }

	    sbuf_printf(&b,"%s!%s",pip->nodename,pip->username) ;

	    if (pip->name != NULL)
	        sbuf_printf(&b," (%s)",pip->name) ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	if ((rs >= 0) && (buf[0] != '\0')) {
	    iap->ti_info = pip->daytime ;
	    pip->to_info = 7 ;
	    iap->tag_info = infotag_unspecified ;
	    rs = bbinter_info(iap,FALSE,"%s\v",buf) ;
	    len = rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bbinter_user) */


static int bbinter_poll(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = mailcheck(pip,pip->now.time) ;
	    rc = rs ;
	    if (rs >= 0)
	        rs = display_newmail(&iap->di,rc) ;
	}
#endif /* COMMENT */

	if ((rs >= 0) && (pip->to_mailcheck > 0)) {
	    rs = bbinter_checkmail(iap) ;
	    c += rs ;
	}

	if ((rs >= 0) && pip->f.clock) {
	    rs = bbinter_checkclock(iap) ;
	    c += rs ;
	}

/* what is this (next part) supposed to do? (don't know!) */

	if ((rs >= 0) && iap->f.setmbname && (iap->mbname == NULL)) {
	    iap->f.setmbname = FALSE ;
	    rs = display_setmbname(&iap->di,"",-1) ;
	    c += 1 ;
	}

	if ((rs >= 0) && (! iap->f.mcinit) && (! iap->f.exit)) {
	     rs = bbinter_mailempty(iap) ;
	}

/* tinmeout any outstanding "info" messages */

	if ((pip->to_info > 0) && 
	    (pip->daytime >= (iap->ti_info + pip->to_info))) {
		pip->to_info = 0 ;
		iap->tag_info = 0 ;
	}

/* get out */

	iap->ti_poll = pip->daytime ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_poll) */


static int bbinter_checkclock(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	f = FALSE ;


	if (pip->daytime >= (iap->ti_clock + pip->to_clock)) {
	    iap->ti_clock = pip->daytime ;

	    f = TRUE ;
	    rs = display_setdate(&iap->di,FALSE) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("bbinter_checkclock: display_setdate() rs=%d\n",
			rs) ;
#endif

	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bbinter_checkclock) */


static int bbinter_checkmail(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	to ;
	int	f = FALSE ;

	char	buf[BUFLEN + 1] ;


	to = pip->to_mailcheck ;
	if (to > 0) {
	if (pip->daytime >= (iap->ti_mailcheck + to)) {

	    iap->ti_mailcheck = pip->daytime ;
	    rs1 = pcsmailcheck(pip->pr,buf,BUFLEN,pip->username) ;
	    if (rs1 > 0) {

	        f = TRUE ;
	        iap->f.mailnew = TRUE ;
	        rs = bbinter_checkmailinfo(iap,buf) ;

	        if (rs >= 0)
	            rs = display_setnewmail(&iap->di,rs1) ;

	    } else {
	        if (iap->f.mailnew) {
	            iap->f.mailnew = FALSE ;
	            buf[0] = '\0' ;
	            rs = bbinter_checkmailinfo(iap,buf) ;

	            if (rs >= 0)
	                rs = display_setnewmail(&iap->di,0) ;
	        }
	    } /* end if (new mail) */

	} /* end if */
	} /* end if (mailcheck) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bbinter_checkmail) */


static int bbinter_checkmailinfo(iap,buf)
BBINTER		*iap ;
const char	buf[] ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	f ;


	f = (pip->daytime >= (iap->ti_info + pip->to_info)) ;
	if (! f)
	    goto ret0 ;

	if (buf[0] != '\0') {

	    if (pip->daytime >= (iap->ti_mailinfo + TO_MAILCHECK)) {

	        iap->ti_mailinfo = pip->daytime ;
	        iap->f.mailinfo = TRUE ;
	        iap->tag_info = infotag_mailfrom ;
	        rs = bbinter_info(iap,FALSE,"mailfrom> %s\v",buf) ;

	    } /* end if */

	} else if (iap->f.mailinfo) {

	    iap->f.mailinfo = FALSE ;
	    if (iap->tag_info == infotag_mailfrom) {
	        iap->tag_info = 0 ;
	        rs = bbinter_info(iap,FALSE,"\v") ;
	    }

	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (bbinter_checkmailinfo) */


static int bbinter_mailstart(iap,mbname,mblen)
BBINTER		*iap ;
const char	mbname[] ;
int		mblen ;
{
	struct proginfo	*pip = iap->pip ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f_access = FALSE ;
	int	f_readonly = FALSE ;

	char	mbfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_mailstart: mbname=%s\n",mbname) ;
	    debugprintf("bbinter_mailstart: folderdname=%s\n",
		pip->folderdname) ;
	}
#endif

	if (mbname == NULL) return SR_FAULT ;

	iap->f.viewchange = TRUE ;

/* form mailbox filename */

	rs1 = mkpath2w(mbfname,pip->folderdname,mbname,mblen) ;

/* is it there? and acccessable? */

	if (rs1 >= 0)
	rs1 = u_stat(mbfname,&sb) ;

	f_access = ((rs1 >= 0) && S_ISREG(sb.st_mode)) ;
	if (f_access) {
	    rs1 = sperm(&pip->id,&sb,(R_OK | W_OK)) ;
	    if (rs1 == SR_ACCES) {
	        rs1 = sperm(&pip->id,&sb,(R_OK)) ;
	        f_readonly = (rs1 >= 0) ;
	    }
	    f_access = (rs1 >= 0) ;
	}

	if (! f_access)
	    goto ret1 ;

	if (iap->f.mcinit) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_mailstart: bbinter_mailend() rs=%d\n",rs) ;
#endif

	    rs = bbinter_mailend(iap,FALSE) ;
	} /* end if */

	if (rs >= 0) {
	    rs = display_input(&iap->di,"mb=%t\v", mbname,mblen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mailstart: display_input() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            iap->f.setmbname = TRUE ;
	            rs = display_setmbname(&iap->di,mbname,mblen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mailstart: display_setmbname() rs=%d\n",
		rs) ;
#endif

	        }

	    if (rs >= 0)
	        display_flush(&iap->di) ;

	} /* end if */

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_mailstart: bbinter_mbopen() \n") ;
#endif

	    rs = bbinter_mbopen(iap,mbfname,f_readonly) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_mailstart: bbinter_mbopen() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        rs = uc_mallocstrw(mbname,mblen,&iap->mbname) ;

	        if (rs >= 0) {
		    iap->miscanpoint = (iap->nmsgs > 0) ? 0 : -1 ;
	            rs = display_midmsgs(&iap->di,iap->nmsgs,iap->miscanpoint) ;
		}

	        if (rs >= 0) {

		    iap->miscantop = 0 ;
	            rs = bbinter_mailscan(iap) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("bbinter_mailstart: "
				"bbinter_mailscan() rs=%d\n",rs) ;
#endif

	        }

		if (rs >= 0)
			rs = display_viewclear(&iap->di) ;

	        if (rs < 0) {
		    iap->miscantop = -1 ;
		    iap->miscanpoint = -1 ;
	            if (iap->mbname != NULL) {
	                uc_free(iap->mbname) ;
	                iap->mbname = NULL ;
		    }
	        }

	    } /* end if (mailbox opened) */

	} /* end if (mailbox) */

ret1:
	if ((rs >= 0) && (! f_access))
	    rs = bbinter_info(iap,TRUE,"inaccessible mb=%s\v",
	        mbname) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mailstart: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_mailstart) */


static int bbinter_mailscan(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	si ;
	int	n ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	si = iap->miscantop ;
	n = iap->scanlines ;
	if ((rs >= 0) && (si >= 0))
	    rs = bbinter_scancheck(iap,si,n) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mailscan: bbinter_scancheck() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_scanpoint(&iap->di,iap->miscanpoint) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mailscan: display_scanpoint() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_scandisplay(&iap->di,iap->miscantop) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mailscan: display_scandisplay() rs=%d\n",rs) ;
#endif

ret0:
	return rs ;
}
/* end subroutine (bbinter_mailscan) */


static int bbinter_mailend(iap,f_quick)
BBINTER		*iap ;
int		f_quick ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rl, cl ;
	int	i ;
	int	f_yes = TRUE ;

	const char	*ccp ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;


	if (! iap->f.mcinit)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bbinter_mailend: f_quick=%u\n",f_quick) ;
#endif

	if (f_quick)
	    f_yes = FALSE ;

	if ((rs >= 0) && (! f_quick) && (iap->nmsgdels > 0)) {
	    f_yes = TRUE ;
	    ccp = "delete marked messages? [yes] " ;
	    rs = bbinter_input(iap,response,LINEBUFLEN, ccp) ;
	    rl = rs ;
	    if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) rl -= 1 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bbinter_mailend: delete? _input() rs=%d\n",rs) ;
#endif
	    if ((rs >= 0) && (rl > 0)) {
		cl = sfshrink(response,rl,&cp) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("bbinter_mailend: sfshrink() cl=%d c=>%t<\n",
		cl,cp,cl) ;
	    debugprinthex("bbinter_mailend: c=",40,cp,cl) ;
	}
#endif
	        f_yes = ((cl == 0) || (tolc(cp[0]) == 'y')) ;
	    }
	    pip->daytime = time(NULL) ;
	} /* end if (user input) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("bbinter_mailend: rs=%d ndels=%u f_yes=%u\n",
		rs,iap->nmsgdels,f_yes) ;
#endif

	if ((rs >= 0) && (! f_yes) && (iap->nmsgdels > 0)) {

	    for (i = 0 ; (rs >= 0) && (i < iap->nmsgs) ; i += 1) {
		rs = mbcache_msgdel(&iap->mc,i,FALSE) ;
	    } /* end for */

	    if (rs >= 0) 
	        rs = bbinter_info(iap,FALSE,"deletions canceled\v") ;

	} /* end if */

/* blank out all scan lines (in case of refresh) */

	if (! f_quick) {

	    if (rs >= 0) {
		iap->miscantop = -1 ;
	        rs = display_scanblank(&iap->di,-1) ;
	    }

	    if (rs >= 0) {
		iap->miscanpoint = -1 ;
	        rs = display_scanpoint(&iap->di,-1) ;
	    }

	} /* end if (not quick quit) */

/* close out MB resources */

	if (rs >= 0) 
	    rs = bbinter_mbviewclose(iap) ;

	if (rs >= 0)
	    rs = bbinter_mbclose(iap) ;

ret1:
	if (iap->mbname != NULL) {
	    uc_free(iap->mbname) ;
	    iap->mbname = NULL ;
	}

ret0:
	return rs ;
}
/* end subroutine (bbinter_mailend) */


int bbinter_charin(BBINTER *iap,const char *fmt,...)
{
	struct proginfo	*pip = iap->pip ;

	va_list	ap ;

	int	rs = SR_OK ;
	int	len = 0 ;
	int	to ;
	int	ropts = TR_OPTS ;
	int	cmd = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;


	to = pip->to_read ;
	{
	    va_begin(ap,fmt) ;
	    rs = vbufprintf(linebuf,LINEBUFLEN,fmt,ap) ;
	    len += rs ;
	    va_end(ap) ;
	}

	if (rs >= 0)
	    rs = display_input(&iap->di,linebuf) ;

	if (rs >= 0) {
	    linebuf[0] = '\0' ;
	    rs = uterm_reade(&iap->ut,linebuf,1,to,ropts,NULL,NULL) ;
	    if (rs > 0) cmd = (linebuf[0] & 0xff) ;
	}

	if (rs > 0) {
	    if (iscmdstart(cmd)) {
		rs = bbinter_cmdinesc(iap,cmd) ;
		cmd = rs ;
	    }
	}

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (bbinter_charin) */


static int bbinter_mbopen(iap,mbfname,f_ro)
BBINTER		*iap ;
const char	mbfname[] ;
int		f_ro ;
{
	MBCACHE_INFO	mcinfo ;

	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	mflags ;


	if (iap->f.mbopen)
	    return SR_NOANODE ;

	mflags = 0 ;
	mflags |= (f_ro) ? MAILBOX_ORDONLY : MAILBOX_ORDWR ;
	mflags |= (! pip->f.useclen) ? MAILBOX_ONOCLEN : 0 ;
	mflags |= (pip->f.useclines) ? MAILBOX_OUSECLINES : 0 ;
	rs1 = mailbox_open(&iap->mb,mbfname,mflags) ;
	iap->f.mbopen = (rs1 >= 0) ;
	iap->f.mbreadonly = f_ro ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_mbopen: mailbox_open() rs1=%d\n",rs1) ;
#endif

	if (rs1 < 0)
	    goto ret0 ;

	rs = mbcache_start(&iap->mc,mbfname,mflags,&iap->mb) ;
	iap->f.mcinit = (rs >= 0) ;
	if (rs < 0)
	   goto bad1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_mbopen: mbcache_start() rs=%d\n",rs) ;
#endif

	rs = mbcache_mbinfo(&iap->mc,&mcinfo) ;
	if (rs < 0)
	    goto bad2 ;

	iap->nmsgs = mcinfo.nmsgs ;
	iap->miscanpoint = (mcinfo.nmsgs > 0) ? 0 : -1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_mbopen: mbcache_mbinfo() rs=%d nmsgs=%u\n",
	            rs,iap->nmsgs) ;
#endif

ret0:
	return (rs >= 0) ? iap->f.mbopen : rs ;

/* bad stuff */
bad2:
	if (iap->f.mvinit) {
	    iap->f.mcinit = FALSE ;
	    mbcache_finish(&iap->mc) ;
	}

bad1:
	if (iap->f.mbopen) {
	    iap->f.mbopen = FALSE ;
	    mailbox_close(&iap->mb) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (bbinter_mbopen) */


static int bbinter_mbclose(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	iap->nmsgs = -1 ;
	iap->nmsgdels = 0 ;

	if (iap->f.mcinit) {
	    iap->f.mcinit = FALSE ;
	    rs1 = mbcache_finish(&iap->mc) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->f.mbopen) {
	    iap->f.mbopen = FALSE ;
	    rs1 = mailbox_close(&iap->mb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (bbinter_mbclose) */


static int bbinter_msgnum(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs ;


	if (iap->f.mcinit) {

	    rs = bbinter_info(iap,FALSE,"mb=%s msg=%u:%u\v",
	        iap->mbname,
	        MAX((iap->miscanpoint+1),0), iap->nmsgs) ;

	} else
	    rs = bbinter_info(iap,TRUE,"no current mailbox\v") ;

	return rs ;
}
/* end subroutine (bbinter_msgnum) */


static int bbinter_msgpoint(iap,sipointnext)
BBINTER		*iap ;
int		sipointnext ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	nmsgs = iap->nmsgs ;
	int	nscanlines = iap->scanlines ;
	int	nmargin = SCANMARGIN ;
	int	nabove = 0 ;
	int	nbelow = 0 ;
	int	nscroll = 0 ;
	int	ndiff, nadiff ;
	int	sicheck = 0 ;
	int	sipointcurr = iap->miscanpoint ;
	int	sitopcurr = iap->miscantop ;
	int	sitopnext = 0 ;
	int	sibottom ;
	int	nt ;
	int	f ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgpoint: sipointnext=%d\n",sipointnext) ;
#endif

	if (! iap->f.mcinit)
	    goto ret0 ;

	if ((sipointnext < 0) || (sipointnext >= nmsgs))
	    goto ret0 ;

	if (sipointnext == iap->miscanpoint)
	    goto ret0 ;

	sitopnext = sitopcurr ;
	sibottom = MIN((sitopcurr + nscanlines),(nmsgs - sitopcurr)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_msgpoint: sitopcurr=%u\n",sitopcurr) ;
	    debugprintf("bbinter_msgpoint: sipointcurr=%u\n",sipointcurr) ;
	    debugprintf("bbinter_msgpoint: sibottom=%u\n",sibottom) ;
	}
#endif

	ndiff = (sipointnext - iap->miscanpoint) ;
	nadiff = abs(ndiff) ;
	if (nadiff >= iap->scanlines) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_msgpoint: scan_full ndiff=%d\n",ndiff) ;
#endif

	    if (ndiff > 0) {
	        nt = (nmsgs > nscanlines) ? (nmsgs - nscanlines) : 0 ;
	        sitopnext = MIN((sipointnext - nmargin),nt) ;
	    } else {
	        sitopnext = 0 ;
	        if (sipointnext >= (nscanlines - nmargin - 1))
	            sitopnext = (sipointnext - (nscanlines - nmargin - 1)) ;
	    }

	    if (sitopnext < 0) sitopnext = 0 ;

	    sicheck = sitopnext ;
	    nscroll = nscanlines ;

	} else if (ndiff > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_msgpoint: scan_up ndiff=%d\n",ndiff) ;
#endif

	    nscroll = 0 ;
	    f = (nmsgs > (sitopcurr + nscanlines)) ;
	    nbelow = (f) ? (nmsgs - (sitopcurr + nscanlines)) : 0 ;
	    f = f && (sipointnext >= (sitopcurr + nscanlines - nmargin)) ;
	    if (f) {
	        nscroll = MIN(nbelow,nadiff) ;
	        sitopnext = (sitopcurr + nscroll) ;
	    }

	    if (sitopnext < 0) sitopnext = 0 ;

	    sicheck = (sitopcurr + nscanlines) ;

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_msgpoint: scan_down ndiff=%d\n",ndiff) ;
#endif

	    nscroll = 0 ;
	    f = (sitopcurr > 0) ;
	    nabove = (f) ? sitopcurr : 0 ;
	    f = f && (sipointnext < (sitopcurr + nmargin)) ;
	    if (f) {
	        nscroll = MIN(nabove,nadiff) ;
	        sitopnext = (sitopcurr - nscroll) ;
	    }

	    if (sitopnext < 0) sitopnext = 0 ;

	    sicheck = sitopnext ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgpoint: sitopnext=%u sicheck=%u nscroll=%u\n",
	        sitopnext,sicheck,nscroll) ;
#endif

	if (nscroll > 0)
	    rs = bbinter_scancheck(iap,sicheck,nscroll) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgpoint: bbinter_scancheck() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_scanpoint(&iap->di,sipointnext) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgpoint: display_scanpoint() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_scandisplay(&iap->di,sitopnext) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgpoint: display_scandisplay() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_midmsgs(&iap->di,iap->nmsgs,sipointnext) ;

	if (rs >= 0) {
	    iap->miscanpoint = sipointnext ;
	    iap->miscantop = sitopnext ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgpoint: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_msgpoint) */


static int bbinter_cmdwrite(iap,f_whole,argnum)
BBINTER		*iap ;
int		f_whole ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	offset_t	outoff ;

	int	rs = SR_OK ;
	int	rl ;
	int	cl ;
	int	mi ;
	int	outlen ;

	const char	*ccp ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	ccp = "file: " ;
	rs = bbinter_input(iap,response,LINEBUFLEN, ccp) ;
	rl = rs ;
	if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) rl -= 1 ;
	if (rs < 0)
	    goto ret0 ;

	if (rl > 0)
	    cl = sfshrink(response,rl,&cp) ;

	if (cl <= 0)
	    goto ret0 ;

	response[(cp-response)+cl] = '\0' ; /* should be optional */

/* do it */

	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;
	if (rs < 0)
	    goto ret0 ;

	if (f_whole) {
	    outoff = msp->moff ;
	    outlen = msp->mlen ;
	} else {
	    outoff = msp->boff ;
	    outlen = msp->blen ;
	}

	rs = bbinter_msgoutfile(iap,cp,cl,outoff,outlen) ;

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdwrite) */


static int bbinter_cmdpipe(iap,f_whole,argnum)
BBINTER		*iap ;
int		f_whole ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	offset_t	outoff ;

	int	rs = SR_OK ;
	int	cl ;
	int	mi ;
	int	outlen ;

	const char	*ccp ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	ccp = "cmd: " ;
	cp = response ;
	rs = bbinter_response(iap,response,LINEBUFLEN,ccp) ;
	cl = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_cmdpipe: cmd=»%t«\n",cp,cl) ;
#endif

	if (cl <= 0)
	    goto ret0 ;

/* do it */

	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;
	if (rs < 0)
	    goto ret0 ;

	if (f_whole) {
	    outoff = msp->moff ;
	    outlen = msp->mlen ;
	} else {
	    outoff = msp->boff ;
	    outlen = msp->blen ;
	}

	rs = bbinter_msgoutpipe(iap,response,outoff,outlen) ;

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdpipe) */


static int bbinter_cmdpage(iap,argnum)
BBINTER		*iap ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	mi ;
	int	mt ;
	int	nlines ;
	int	vlines = -1 ;

	const char	*cmd ;
	const char	*midp ;
	const char	*mfp ;

	char	msgid[MAILADDRLEN + 1] ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

/* do it */

	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;
	if (rs < 0)
	    goto ret0 ;

	mt = MAILMSGFILE_TTEMP ;
	midp = msp->vs[mbcachemf_mid] ;
	if ((midp == NULL) || (midp[0] == '\0')) {
	    mt = MAILMSGFILE_TTEMP ;
	    midp = msgid ;
	    snsdd(msgid,MAILADDRLEN,iap->mbname,iap->miscanpoint) ;
	}

	rs1 = mailmsgfile_get(&iap->msgfiles,midp,&mfp) ;
	nlines = rs1 ;
	if (rs1 == SR_NOTFOUND) {

	    if (iap->mfd < 0)
	        rs = bbinter_mbviewopen(iap) ;

	    if (rs >= 0) {

	        rs = mailmsgfile_new(&iap->msgfiles,mt,midp,
	            iap->mfd,msp->boff,msp->blen) ;
	        nlines = rs ;

	    } /* end if */

	    if (rs >= 0) {
	        rs = mailmsgfile_get(&iap->msgfiles,midp,&mfp) ;
		nlines = rs ;
	    }

	} /* end if */

	if ((rs >= 0) && (nlines >= 0)) {

	    vlines = msp->vlines ;
	    if (vlines < 0) vlines = msp->nlines ;

	    if ((rs >= 0) && (vlines != nlines)) {

	        rs = mbcache_msgsetlines(&iap->mc,mi,nlines) ;

		if (rs >= 0)
		    rs = display_scanloadlines(&iap->di,mi,nlines,FALSE) ;

	    } /* end if (setting lines) */

	} /* end if (lines) */

	if ((rs >= 0) && (mfp != NULL)) {

	    cmd = pip->prog_pager ;
	    rs = bbinter_msgoutview(iap,cmd,mfp) ;

#ifdef	COMMENT /* 2010-03-04 David A­D­ Morano -- why was this code here? */
	    if (rs >= 0) {
		DISPLAY_BOTINFO	bi ;

		memset(&bi,0,sizeof(DISPLAY_BOTINFO)) ;
	        strwcpy(bi.msgfrom,msp->vs[mbcachemf_from],DISPLAY_LMSGFROM) ;
		bi.msgnum = (mi+1) ;
		bi.msglines = nlines ;
		bi.msgline = 1 ;
		rs = display_botinfo(&iap->di,&bi) ;
	    } /* end if */
#endif /* COMMENT */

	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdpage) */


static int bbinter_cmdbody(iap,argnum)
BBINTER		*iap ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cl ;
	int	mi ;
	int	mt ;
	int	nlines, vlines ;

	const char	*ccp ;
	const char	*midp ;
	const char	*mfp ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;
	char	msgid[MAILADDRLEN + 1] ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	ccp = "file: " ;
	cp = response ;
	rs = bbinter_response(iap,response,LINEBUFLEN,ccp) ;
	cl = rs ;
	if (rs < 0)
	    goto ret0 ;

	if (cl <= 0)
	    goto ret0 ;

/* do it */

	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;
	if (rs < 0)
	    goto ret0 ;

	mt = MAILMSGFILE_TTEMP ;
	midp = msp->vs[mbcachemf_mid] ;
	if ((midp == NULL) || (midp[0] == '\0')) {
	    mt = MAILMSGFILE_TTEMP ;
	    midp = msgid ;
	    snsdd(msgid,MAILADDRLEN,iap->mbname,iap->miscanpoint) ;
	}

	rs1 = mailmsgfile_get(&iap->msgfiles,midp,&mfp) ;
	nlines = rs1 ;
	if (rs1 == SR_NOTFOUND) {

	    if (iap->mfd < 0)
	        rs = bbinter_mbviewopen(iap) ;

	    if (rs >= 0) {
		const offset_t	bo = msp->boff ;
		const int	bl = msp->blen ;
		rs = mailmsgfile_new(&iap->msgfiles,mt,midp,iap->mfd,bo,bl) ;
	        nlines = rs ;
	    } /* end if */

	    if (rs >= 0) {
	        rs = mailmsgfile_get(&iap->msgfiles,midp,&mfp) ;
		nlines = rs ;
	    }

	} /* end if */

	if ((rs >= 0) && (nlines >= 0)) {

		vlines = msp->vlines ;
		if (vlines < 0) vlines = msp->nlines ;

	        if ((rs >= 0) && (vlines != nlines)) {

	            rs = mbcache_msgsetlines(&iap->mc,mi,nlines) ;

	            if (rs >= 0)
	                rs = display_scanloadlines(&iap->di,mi,nlines,TRUE) ;

	        } /* end if (setting lines) */

	} /* end if (lines) */

	if ((rs >= 0) && (mfp != NULL)) {

	    rs = bbinter_filecopy(iap,mfp,cp) ;

	    if (rs >= 0) {
		DISPLAY_BOTINFO	bi ;

		memset(&bi,0,sizeof(DISPLAY_BOTINFO)) ;
	        strwcpy(bi.msgfrom,msp->vs[mbcachemf_from],DISPLAY_LMSGFROM) ;
		bi.msgnum = (mi+1) ;
		bi.msglines = nlines ;
		bi.msgline = 1 ;

		rs = display_botinfo(&iap->di,&bi) ;
	    } /* end if */

	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdbody) */


static int bbinter_cmdmove(iap,argnum)
BBINTER		*iap ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	offset_t	moff ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	rl ;
	int	cl ;
	int	mi ;
	int	mlen ;
	int	m ;
	int	mblen ;
	int	f_same = FALSE ;

	const char	*ccp ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;
	char	mbname[MAXNAMELEN + 1] ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	ccp = "mailbox: " ;
	rs = bbinter_input(iap,response,LINEBUFLEN, ccp) ;
	rl = rs ;
	if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) rl -= 1 ;
	if (rs < 0)
	    goto ret0 ;

	if (rl > 0)
	    cl = sfshrink(response,rl,&cp) ;

	if (cl <= 0)
	    goto ret0 ;

	response[(cp-response)+cl] = '\0' ; /* should be optional */

/* is the mailbox-name well formed? */

	if (hasprintbad(cp,cl)) {
	    rs = bbinter_info(iap,TRUE,"mailbox name is not well formed\v") ;
	    goto ret0 ;
	}

/* does the other mailbox exist? */

	if (iap->mbname != NULL) {
	    m = nleadstr(iap->mbname,cp,cl) ;
	    f_same = (iap->mbname[m] == '\0') && (m == cl) ;
	}

/* return if moving to the same mailbox? */

	if (f_same) {
	    rs = bbinter_info(iap,TRUE,"same mailbox specified\v") ;
	    goto ret0 ;
	}

	rs1 = snwcpy(mbname,MAXNAMELEN,cp,cl) ;
	mblen = rs1 ;
	if (rs1 < 0) {
	    rs = bbinter_info(iap,TRUE,"invalid mailbox specified\v") ;
	    goto ret0 ;
	}

/* does the new mailbox exist? */

	rs = bbinter_havemb(iap,mbname,mblen) ;

	if (rs == 0) {
	    char	disname[MAXNAMELEN + 1] ;

	    mkdisplayable(disname,MAXNAMELEN,cp,cl) ;

	    ccp = "create new mailbox=%s [yes] " ;
	    rs = bbinter_input(iap,response,LINEBUFLEN, ccp,disname) ;
	    rl = rs ;
	    if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) rl -= 1 ;

	    if ((rs >= 0) && (rl > 0)) {

#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("bbinter_cmdmove: response=>%t<\n",
			response,strlinelen(response,rl,60)) ;
#endif

	        cl = sfshrink(response,rl,&cp) ;
		if ((cl > 0) && (tolc(*cp) != 'y')) {
		    bbinter_info(iap,TRUE,"canceled\v") ;
	            goto ret0 ;
		}
	    }
	}

/* do it */

	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;
	if (rs < 0)
	    goto ret0 ;

	moff = msp->moff ;
	mlen = msp->mlen ;

	rs = bbinter_msgmove(iap,mbname,moff,mlen) ;

	if ((rs >= 0) && pip->f.nextmov)
	    rs = bbinter_msgdel(iap,mi,TRUE) ;

	if (rs >= 0)
	    rs = bbinter_info(iap,FALSE,"msg#%u -> %s\v",(mi+1),mbname) ;

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdmove) */


static int bbinter_cmddel(iap,f_del,argnum)
BBINTER		*iap ;
int		f_del ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	mi ;
	int	f_delprev = FALSE ;

	const char	*ccp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(5))
		debugprintf("bbinter_cmddel: f_del=%u argnum=%d\n",
		f_del,argnum) ;
#endif

	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	rs = bbinter_msgdel(iap,mi,f_del) ;
	f_delprev = (rs > 0) ;
	if (rs >= 0) {
	    if (! LEQUIV(f_del,f_delprev)) {
	        ccp = (f_del) ? "deletion scheduled" : "deletion canceled" ;
	    } else
		ccp = "no change" ;
	    rs = bbinter_info(iap,FALSE,"%s msg#%u\v",ccp,(mi+1)) ;
	}

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmddel) */


static int bbinter_cmddelnum(iap,f_del,argnum)
BBINTER		*iap ;
int		f_del ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	mi ;
	int	c ;

	const char	*ccp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("bbinter_cmddelnum: f_del=%u argnum=%d\n",
		f_del,argnum) ;
#endif

	if (argnum <= 0)
	     argnum = 1 ;

	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	rs = bbinter_msgdelnum(iap,mi,argnum,f_del) ;
	c = rs ;
	if (rs >= 0) {
	    ccp = "deletions %s=%u\v" ;
	    rs = bbinter_info(iap,FALSE,ccp,
		((f_del) ? "scheduled" : "canceled"),c) ;
	}

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmddelnum) */


static int bbinter_cmdsubject(iap,argnum)
BBINTER		*iap ;
int		argnum ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	const int	dislen = DISBUFLEN ;

	int	rs = SR_OK ;
	int	mi ;

	const char	*ccp ;

	char	disbuf[DISBUFLEN + 1] = { 0 } ;


	if (! iap->f.mcinit)
	    goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0)
	    mi = (argnum - 1) ;

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

/* do it */

	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;
	if (rs < 0)
	    goto ret0 ;

	ccp = msp->vs[mbcachemf_subject] ;
	if (ccp != NULL) {
	    rs = mkdisplayable(disbuf,dislen,ccp,-1) ;
	    if (rs >= 0) compactstr(disbuf,rs) ;
	}

	if (rs >= 0) {
	    const int	ml = MIN(pip->linelen,dislen) ;
	    disbuf[ml] = '\0' ;
	    rs = bbinter_info(iap,FALSE,"s> %s\v",disbuf) ;
	}

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdsubject) */


static int bbinter_msgoutfile(iap,cp,cl,moff,mlen)
BBINTER		*iap ;
const char	cp[] ;
int		cl ;
offset_t	moff ;
int		mlen ;
{
	struct proginfo	*pip = iap->pip ;

	const mode_t	operms = 0666 ;

	const int	oflags = (O_WRONLY | O_CREAT | O_TRUNC) ;

	int	rs = SR_OK ;
	int	rs1 ;

	const char	*ccp ;

	char	ofname[MAXPATHLEN + 1] ;


	if (ofname == NULL)
	    return SR_NOANODE ;

	if (iap->mfd < 0)
	    rs = bbinter_mbviewopen(iap) ;

	if (rs < 0)
	    goto ret0 ;

	if (cp[0] != '/') {

	    ccp = iap->pathprefix ;
	    if (ccp == NULL) {
		proginfo_pwd(pip) ;
		ccp = pip->pwd ;
	    }

	    rs1 = mkpath2w(ofname,ccp,cp,cl) ;

	} else
	    rs1 = mkpath1w(ofname,cp,cl) ;

	if (rs1 < 0) {
	    bbinter_info(iap,TRUE,"invalid file=%t\v",cp,cl) ;
	    goto ret0 ;
	}

	if ((rs1 = u_open(ofname,oflags,operms)) >= 0) {
	    int	ofd = rs1 ;

	    if ((rs = u_seek(iap->mfd,moff,SEEK_SET)) >= 0) {

	        if ((rs1 = uc_copy(iap->mfd,ofd,mlen)) >= 0) {
	            rs = bbinter_info(iap,FALSE, "size=%u file=%t\v",rs1,cp,cl) ;
	        } else
	            rs = bbinter_info(iap,TRUE, "file-copy-error (%d)\v",rs1) ;

	    } /* end if (copy) */

	    u_close(ofd) ;
	} else
	    rs = bbinter_info(iap,TRUE,"inaccessible (%d) file=%s\v",rs1,ofname) ;

ret0:
	return rs ;
}
/* end subroutine (bbinter_msgoutfile) */


static int bbinter_msgoutpipe(iap,cmd,outoff,outlen)
BBINTER		*iap ;
const char	cmd[] ;
offset_t		outoff ;
int		outlen ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1, rs2 ;
	int	ofd = -1 ;

	char	response[LINEBUFLEN + 1] ;


	if (cmd == NULL)
	    return SR_NOANODE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgoutpipe: outoff=%lu outlen=%u\n",
		outoff,outlen) ;
#endif

	if (iap->mfd < 0)
	    rs = bbinter_mbviewopen(iap) ;

	if (rs < 0)
	    goto ret0 ;

	rs = u_seek(iap->mfd,outoff,SEEK_SET) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_msgoutpipe: shell=>%s<\n",pip->prog_shell) ;
	    debugprintf("bbinter_msgoutpipe: cmd=»%s«\n",cmd) ;
	}
#endif

	rs = display_suspend(&iap->di) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgoutpipe: display_suspend() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if ((rs = uterm_suspend(&iap->ut)) >= 0) {
	    SPAWNPROC	ps ;
	    pid_t	pid ;
	    int		cs ;

	    memset(&ps,0,sizeof(SPAWNPROC)) ;

	    ps.disp[0] = SPAWNPROC_DCREATE ;
	    ps.disp[1] = SPAWNPROC_DINHERIT ;
	    ps.disp[2] = SPAWNPROC_DINHERIT ;
	    rs1 = spawncmdproc(&ps,pip->prog_shell,cmd) ;
	    pid = rs1 ;
	    ofd = ps.fd[0] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgoutpipe: spawncmdproc() rs=%d ofd=%d\n",
		rs1,ofd) ;
#endif

	    if (rs1 >= 0) {

	        rs1 = uc_copy(iap->mfd,ofd,outlen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgoutpipe: uc_copy() rs=%d\n",rs1) ;
#endif

	        if (ofd >= 0) {
	            u_close(ofd) ;
	            ofd = -1 ;
	        }

	        rs2 = u_waitpid(pid,&cs,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_msgoutpipe: u_waitpid() rs=%d\n",rs2) ;
	    if (WIFEXITED(cs)) {
	    debugprintf("bbinter_msgoutpipe: child ex=%u\n",WEXITSTATUS(cs)) ;
	    } else if (WIFSIGNALED(cs)) {
	    debugprintf("bbinter_msgoutpipe: child sig=%u\n",WTERMSIG(cs)) ;
	    }
	}
#endif /* CF_DEBUG */

	    } /* end if */

	    rs2 = uterm_resume(&iap->ut) ;
	} /* end block */

	if (rs >= 0) {
	    const char	*ccp ;

	    ccp = "resume> \v" ;
	    rs = bbinter_response(iap,response,LINEBUFLEN,ccp) ;

	    if (rs >= 0)
	        rs = bbinter_refresh(iap) ;

	    if (rs >= 0) {
	        if (rs1 >= 0) {
	            rs = bbinter_info(iap,FALSE, "bytes=%u\v",rs1) ;
	        } else {
		    if (rs1 == SR_PIPE) {
	                ccp = "not all data transfered (%d)\v" ;
		    } else
			ccp = "file-copy-error (%d)\v" ;
	            rs = bbinter_info(iap,TRUE,ccp,rs1) ;
	        }
	    }

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgoutpipe: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_msgoutpipe) */


static int bbinter_msgoutview(iap,cmd,vfname)
BBINTER		*iap ;
const char	cmd[] ;
const char	vfname[] ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1, rs2 ;

	const char	*cp ;


	if (cmd == NULL)
	    return SR_NOANODE ;

/* to the viewing */

#ifdef	COMMENT
	if (rs >= 0)
	    rs = display_allclear(&iap->di) ;
#endif

	if (rs >= 0)
	    rs = display_suspend(&iap->di) ;

	if (rs < 0)
	    goto ret0 ;

	if ((rs = uterm_suspend(&iap->ut)) >= 0) {
	    SPAWNPROC	ps ;
	    pid_t	pid ;
	    int		cs ;
	    const char	*av[5] ;

	    sfbasename(cmd,-1,&cp) ;

	    av[0] = cp ;
	    av[1] = vfname ;
	    av[2] = NULL ;

	    memset(&ps,0,sizeof(SPAWNPROC)) ;
	    ps.disp[0] = SPAWNPROC_DINHERIT ;
	    ps.disp[1] = SPAWNPROC_DINHERIT ;
	    ps.disp[2] = SPAWNPROC_DINHERIT ;

	    rs1 = spawnproc(&ps,cmd,av,NULL) ;
	    pid = rs1 ;
	    if (rs1 >= 0) {

	        rs2 = u_waitpid(pid,&cs,0) ;

	    } /* end if */

	    rs2 = uterm_resume(&iap->ut) ;
	} /* end block */

	if (rs >= 0) 
	    rs = bbinter_refresh(iap) ;

	if (rs >= 0)
	    rs = bbinter_info(iap,FALSE,"\v") ;

ret1:
ret0:
	return rs ;
}
/* end subroutine (bbinter_msgoutview) */


static int bbinter_filecopy(iap,srcfname,dstfname)
BBINTER		*iap ;
const char	srcfname[] ;
const char	dstfname[] ;
{
	const mode_t	operms = 0666 ;

	int	rs ;
	int	rs1 ;
	int	oflags ;
	int	wlen = 0 ;


	oflags = O_RDONLY ;
	if ((rs = u_open(srcfname,oflags,operms)) >= 0) {
	    int	sfd = rs ;

	    oflags = (O_WRONLY | O_CREAT | O_TRUNC) ;
	    if ((rs1 = u_open(dstfname,oflags,operms)) >= 0) {
		int	dfd = rs1 ;

		rs1 = uc_copy(sfd,dfd,-1) ;

	        u_close(dfd) ;
	    } /* end if (dst-file) */

	    if ((rs >= 0) && (rs1 < 0))
	        rs = bbinter_info(iap,TRUE, "file-copy-error (%d)\v",rs1) ;

	    u_close(sfd) ;
	} /* end if (src-file) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bbinter_filecopy) */


static int bbinter_msgmove(iap,mbname,moff,mlen)
BBINTER		*iap ;
const char	mbname[] ;
offset_t	moff ;
int		mlen ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 ;

	char	mbfname[MAXPATHLEN + 1] ;


	if (mbname == NULL)
	    return SR_NOANODE ;

	if (iap->mfd < 0) rs = bbinter_mbviewopen(iap) ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_seek(iap->mfd,(offset_t) moff,SEEK_SET) ;
	if (rs < 0)
	    goto ret0 ;

/* create target file-name */

	rs = mkpath2(mbfname,pip->folderdname,mbname) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = mailboxappend(mbfname,iap->mfd,mlen) ;
	if (rs1 < 0) {
	    rs = bbinter_info(iap,TRUE, "file-copy-error (%d)\v",rs1) ;
	}

ret1:
ret0:
	return rs ;
}
/* end subroutine (bbinter_msgmove) */


static int bbinter_msgdel(iap,mi,delcmd)
BBINTER		*iap ;
int		mi ;
int		delcmd ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	mark ;
	int	f_delprev = FALSE ;


	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	rs = mbcache_msgdel(&iap->mc,mi,delcmd) ;
	f_delprev = (rs > 0) ;
	if ((rs >= 0) && (delcmd >= 0)) {
	    if (! LEQUIV(f_delprev,delcmd)) {
		if (delcmd) {
			mark = iap->delmark ;
			iap->nmsgdels += 1 ;
		} else {
			mark = ' ' ;
			iap->nmsgdels -= 1 ;
		}
	        rs = display_scanmark(&iap->di,mi,mark) ;
	    }

	    if ((rs >= 0) && delcmd && pip->f.nextdel && ((mi+1) < iap->nmsgs))
	        rs = bbinter_msgpoint(iap,(mi+1)) ;

	} /* end if (delcmd >= 0) */

ret0:
	return (rs >= 0) ? f_delprev : rs ;
}
/* end subroutine (bbinter_msgdel) */


/* delete a number of messages rather than a message by-msg-number */
static int bbinter_msgdelnum(iap,mi,num,delcmd)
BBINTER		*iap ;
int		mi ;
int		num ;
int		delcmd ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	i ;
	int	mark ;
	int	c = 0 ;
	int	f_delprev = FALSE ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgdelnum: mi=%u n=%d delcmd=%d\n",
		mi,num,delcmd) ;
#endif

	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */
	    goto ret0 ;

	if (delcmd < 0)
	    goto ret0 ;

	for (i = 0 ; (rs >= 0) && (i < num) ; i += 1) {
	    if (mi >= iap->nmsgs) break ;

	    rs = mbcache_msgdel(&iap->mc,mi,delcmd) ;
	    f_delprev = (rs > 0) ;
	    if (rs < 0)
		break ;

	    if (! LEQUIV(f_delprev,delcmd)) {
		if (delcmd) {
			mark = iap->delmark ;
			iap->nmsgdels += 1 ;
		} else {
			mark = ' ' ;
			iap->nmsgdels -= 1 ;
		}
	        rs = display_scanmark(&iap->di,mi,mark) ;
		c += 1 ;
	    } /* end if */

	    mi += 1 ;

	} /* end for */

	if ((rs >= 0) && (delcmd > 0) && pip->f.nextdel) {

	    if (mi >= iap->nmsgs)
		mi = (iap->nmsgs - 1) ;

	    rs = bbinter_msgpoint(iap,mi) ;

	} /* end if */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_msgdelnum) */


static int bbinter_scancheck(iap,si,n)
BBINTER		*iap ;
int		si ;
int		n ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	DISPLAY_SDATA	dsd ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	c = 0 ;
	int	f ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_scancheck: si=%d n=%u\n",si,n) ;
#endif

	if (si < 0)
	    return SR_INVALID ;

	if (n <= 0)
	    goto ret0 ;

	for (i = 0 ; (i < n) && (si < iap->nmsgs) ; i += 1) {

	    rs = display_scancheck(&iap->di,si) ;
	    f = (rs > 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("bbinter_scancheck: "
	            "display_scancheck() rs=%d f=%u\n",
	            rs,f) ;
#endif

	    if ((rs >= 0) && (! f)) {

	        rs = mbcache_msgscan(&iap->mc,si,&msp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("bbinter_scancheck: "
	                "mbcache_msgscan() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {

		    int	lines ;


		    lines = msp->vlines ;
		    if (lines < 0) lines = msp->nlines ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("bbinter_scancheck: lines=%d\n",lines) ;
#endif

		    if (lines < 0) {
			MAILMSGFILE_MSGINFO	*mip ;
			const char *mid = msp->vs[mbcachemf_mid] ;
			rs1 = mailmsgfile_msginfo(&iap->msgfiles,mid,&mip) ;
			if (rs1 >= 0) {
				lines = mip->vlines ;
				if (lines < 0)
					lines = mip->nlines ;
			}

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("bbinter_scancheck: mailmsgfile_msginfo() "
			"lines=%d\n",lines) ;
#endif

		    } /* end if (lines) */

	            c += 1 ;
	            memset(&dsd,0,sizeof(DISPLAY_SDATA)) ;
	            dsd.from = msp->vs[mbcachemf_from] ;
	            dsd.subject = msp->vs[mbcachemf_subject] ;
	            dsd.date = msp->vs[mbcachemf_date] ;
	            dsd.lines = lines ;
	            dsd.msgi = si ;
		    dsd.mark = (msp->f.del) ? iap->delmark : ' ' ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
			const char	*cp ;
	                debugprintf("bbinter_scancheck: vs[from]=>%s<\n",
	                    msp->vs[mbcachemf_from]) ;
	                    cp = msp->vs[mbcachemf_subject] ;
	                debugprintf("bbinter_scancheck: vs[subject]=>%t<\n",
	                    cp,strlinelen(cp,-1,45)) ;
	                debugprintf("bbinter_scancheck: vs[date]=>%s<\n",
	                    msp->vs[mbcachemf_date]) ;
	            }
#endif /* CF_DEBUGS */

	            rs = display_scanload(&iap->di,si,&dsd) ;

	        } /* end if (mbcache_msgscan) */

	    } /* end if (display needs scan-load) */

	    if (rs < 0)
	        break ;

	    si += 1 ;
	    if (si >= iap->nmsgs)
	        break ;

	} /* end for */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_scancheck: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_scancheck) */


/* get input */
int bbinter_input(BBINTER *iap,char *linebuf,int linelen,const char *fmt,...)
{
	struct proginfo	*pip = iap->pip ;

	DISPLAY		*dip = &iap->di ;

	va_list	ap ;

	int	rs = SR_OK ;
	int	cmd ;
	int	len = 0 ;


	if (linebuf == NULL)
	    return SR_FAULT ;

	linebuf[0] = '\0' ;

	{
	    va_begin(ap,fmt) ;
	    rs = display_vinput(dip,fmt,ap) ;
	    va_end(ap) ;
	}

	if (rs >= 0)
	    rs = display_flush(&iap->di) ;

	if (rs >= 0) {
	    rs = uterm_read(&iap->ut,linebuf,linelen) ;
	    len = rs ;
	    if (rs >= 0) linebuf[rs] = '\0' ; /* for safety */
	}

	if (rs > 0) {
	    cmd = (linebuf[len-1] & 0xff) ;
	    if (iscmdstart(cmd)) {
		len -= 1 ;
		rs = bbinter_cmdinesc(iap,cmd) ;
		cmd = rs ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_input: uterm_read() rs=%d\n",rs) ;
	    debugprintf("bbinter_input: l=>%t<\n",
		linebuf,strlinelen(linebuf,len,40)) ;
	}
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bbinter_input) */


/* get input */
int bbinter_response(BBINTER *iap,char *linebuf,int linelen,const char *fmt,...)
{
	struct proginfo	*pip = iap->pip ;

	DISPLAY		*dip = &iap->di ;

	va_list	ap ;

	int	rs = SR_OK ;
	int	cl ;
	int	rl = 0 ;

	const char	*cp ;


	if (linebuf == NULL)
	    return SR_FAULT ;

	linebuf[0] = '\0' ;

	{
	    va_begin(ap,fmt) ;
	    rs = display_vinput(dip,fmt,ap) ;
	    va_end(ap) ;
	}

	if (rs < 0)
	    goto ret0 ;

	rs = display_flush(&iap->di) ;

	if (rs >= 0) {
	    rs = uterm_read(&iap->ut,linebuf,linelen) ;
	    if (rs >= 0) linebuf[rs] = '\0' ;
	    rl = rs ;
	}

	if (rs > 0) {
	    int	cmd = (linebuf[rl-1] & 0xff) ;
	    if (iscmdstart(cmd)) {
		rl -=1 ;
		rs = bbinter_cmdinesc(iap,cmd) ;
		cmd = rs ;
	    }
	}

	if ((rs >= 0) && (rl > 0)) {
		if (linebuf[rl-1] == '\n') rl -= 1 ;
		cl = sfshrink(linebuf,rl,&cp) ;
		if ((cl > 0) && (cp != linebuf)) {
		    memmove(linebuf,cp,cl) ;
		}
		linebuf[cl] = '\0' ;
		rl = cl ;
	}

ret0:
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (bbinter_response) */


static int bbinter_change(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rl ;
	int	m ;
	int	cl = 0 ;
	int	f_same = FALSE ;
	int	f_changed = FALSE ;

	const char	*ccp  ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;


	ccp = "change to mailbox: " ;
	rs = bbinter_input(iap,response,LINEBUFLEN, ccp) ;
	rl = rs ;
	if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) rl -= 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_change: bbinter_input() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (rl > 0)
	    cl = sfshrink(response,rl,&cp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_change: mb=>%t<\n",cp,cl) ;
#endif

	if (cl <= 0)
	    goto ret0 ;

	response[(cp-response)+cl] = '\0' ; /* should be optional */

/* are we changing to the same mailbox? */

	if (iap->f.mcinit && (iap->mbname != NULL)) {
	    m = nleadstr(iap->mbname,cp,cl) ;
	    f_same = (iap->mbname[m] == '\0') && (m == cl) ;
	}

	if (f_same && (strcmp(iap->mbname,"new") == 0))
	    f_same = FALSE ;

/* return if changing to the same mailbox? */

	if (f_same)
	    goto ret0 ;

/* does the new mailbox exist? */

	rs = bbinter_havemb(iap,cp,cl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_change: _havemb() rs=%d\n",rs) ;
#endif
	if (rs == 0) {
	    char	disname[MAXNAMELEN + 1] ;

	    mkdisplayable(disname,MAXNAMELEN,cp,cl) ;

	    bbinter_info(iap,TRUE,
	            "inaccessible mb=%s\v",disname) ;

	    goto ret0 ;
	}

/* OK, switch to the new mailbox */

	if (rs >= 0) {

	    f_changed = TRUE ;
	    rs = bbinter_mailstart(iap,cp,cl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_change: bbinter_mailstart() rs=%d\n",rs) ;
#endif

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_change: ret rs=%d f_changed=%u\n",rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (bbinter_change) */


/* do we have the specified mailbox? */
static int bbinter_havemb(iap,mbname,mblen)
BBINTER		*iap ;
const char	mbname[] ;
int		mblen ;
{
	struct proginfo	*pip = iap->pip ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f = FALSE ;

	char	mbfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_havemb: mbname=>%t<\n",mbname,mblen) ;
#endif
	if ((mbname == NULL) || (mbname[0] == '\0'))
	    goto ret0 ;

	if (pip->folderdname == NULL)
	    goto ret0 ;

	rs1 = mkpath2w(mbfname,pip->folderdname,mbname,mblen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_havemb: mkpath2w() rs=%d mbf=%s\n",rs1,mbfname) ;
#endif
	if (rs1 < 0)
	    goto ret0 ;

	rs1 = u_stat(mbfname,&sb) ;
	f = ((rs1 >= 0) && S_ISREG(sb.st_mode)) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_havemb: u_stat() rs=%d\n",rs1) ;
#endif

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_havemb: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bbinter_havemb) */


static int bbinter_mailempty(iap)
BBINTER		*iap ;
{
	int	rs = SR_OK ;

	char	mbname[3] ;


	if (iap->f.mcinit || iap->f.exit)
	    goto ret0 ;

	if ((rs >= 0) && iap->f.setmbname) {
		mbname[0] = '\0' ;
	        iap->f.setmbname = FALSE ;
	        rs = display_setmbname(&iap->di,mbname,0) ;
	} /* end if */

	if (rs >= 0)
	    rs = display_midmsgs(&iap->di,-1,-1) ;

	if (rs >= 0)
	    rs = display_scanclear(&iap->di) ;

ret0:
	return rs ;
}
/* end subroutine (bbinter_mailempty) */


static int bbinter_cmdpathprefix(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	const int	rlen = LINEBUFLEN ;

	int	rs ;
	int	rl ;
	int	cl = 0 ;
	int	f_ok = TRUE ;

	const char	*ps ;
	const char	*cp ;

	char	rbuf[LINEBUFLEN + 1] ;
	char	prefix[MAXNAMELEN + 1] ;


	ps = "change path-prefix: " ;
	rs = bbinter_input(iap,rbuf,rlen,ps) ;
	rl = rs ;
	if (rs >= 0) {
	    if ((rl > 0) && (rbuf[rl-1] == '\n')) rl -= 1 ;
	    rbuf[rl] = '\0' ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("bbinter_cmdpathprefix: bbinter_input() rs=%d\n",rs) ;
	    debugprintf("bbinter_cmdpathprefix: r=>%t<\n",rbuf,rl) ;
	}
#endif

	if ((rs >= 0) && (rl > 0)) {
	    const char	*px = iap->pathprefix ;
	    if ((cl = sfshrink(rbuf,rl,&cp)) > 0) {
		if ((rs = mkpath1w(prefix,cp,cl)) >= 0) {
	    	    if ((rs = bbinter_pathprefix(iap,prefix)) >= 0) {
			px = iap->pathprefix ;
	    	        f_ok = (rs > 0) ;
		    } /* end if (bbinter-pathprefix) */
		} /* end if (mkpath) */
	    } /* end if (non-zero string) */
	    	        if (f_ok) {
			    if (px != NULL)
		    		rs = bbinter_info(iap,FALSE,"dir=%s\v",px) ;
	    		} else {
				const char	*fmt = "inaccessible dir=%t\v" ;
				rs = bbinter_info(iap,TRUE,fmt,cp,cl) ;
			} /* end if */
	} /* end if */

	return (rs >= 0) ? f_ok : rs ;
}
/* end subroutine (bbinter_cmdpathprefix) */


static int bbinter_cmdshell(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	rs2, rs3 ;
	int	cl ;

	const char	*ccp ;
	const char	*cp ;

	char	response[LINEBUFLEN + 1] ;


	ccp = "cmd: " ;
	cp = response ;
	rs = bbinter_response(iap,response,LINEBUFLEN,ccp) ;
	cl = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("bbinter_cmdpipe: cmd=»%t«\n",cp,cl) ;
#endif

	if ((cl <= 0) || (cp[0] == '\0')) goto ret0 ;

	rs = display_suspend(&iap->di) ;
	if (rs < 0) goto ret0 ;

	if ((rs = uterm_suspend(&iap->ut)) >= 0) {
	    SPAWNPROC	ps ;
	    pid_t	pid ;
	    int		cs ;

	    memset(&ps,0,sizeof(SPAWNPROC)) ;
	    ps.disp[0] = SPAWNPROC_DINHERIT ;
	    ps.disp[1] = SPAWNPROC_DINHERIT ;
	    ps.disp[2] = SPAWNPROC_DINHERIT ;

	    rs1 = spawncmdproc(&ps,pip->prog_shell,response) ;
	    pid = rs1 ;
	    if (rs1 >= 0) {

	        rs2 = u_waitpid(pid,&cs,0) ;

	    } /* end if */

	    rs2 = uterm_resume(&iap->ut) ;
	} /* end block */

	if (rs >= 0) {
	    const char	*ccp ;

	    ccp = "resume> \v" ;
	    rs = bbinter_response(iap,response,LINEBUFLEN,ccp) ;

	    if (rs >= 0)
	        rs3 = bbinter_refresh(iap) ;

	    if (rs >= 0)
	        rs = rs3 ;

	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (bbinter_cmdshell) */


static int bbinter_pathprefix(iap,pathprefix)
BBINTER		*iap ;
const char	pathprefix[] ;
{
	struct proginfo	*pip = iap->pip ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 = SR_OK ;
	int	f_ok = FALSE ;

	const char	*ndp ;

	char	tmpdname[MAXPATHLEN + 1] ;
	char	newdname[MAXPATHLEN + 1] = { 0 } ;


	if (pathprefix == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_pathprefix: pf=%s\n",pathprefix) ;
#endif

	if (pathprefix[0] == '\0')
	    pathprefix = pip->pwd ;

	if (pathprefix[0] != '/') {

		ndp = newdname ;
		rs1 = mkpath2(tmpdname,iap->pathprefix,pathprefix) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_pathprefix: rs1=%d td=%s\n",rs1,tmpdname) ;
#endif

		if (rs1 >= 0)
			rs1 = pathclean(newdname,tmpdname,rs1) ;

	} else
		ndp = pathprefix ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_pathprefix: rs1=%d nd=%s\n",rs1,ndp) ;
#endif

	if ((rs >= 0) && (rs1 >= 0)) {
	    const char	*px = iap->pathprefix ;
	    if ((px == NULL) || (strcmp(px,ndp) != 0)) {

	        rs1 = u_stat(ndp,&sb) ;

	        if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode)))
	            rs1 = SR_NOTDIR ;

	        if (rs1 >= 0) {

	            f_ok = TRUE ;
	            if (iap->pathprefix != NULL) {
		        uc_free(iap->pathprefix) ;
		        iap->pathprefix = NULL ;
	            }

	            rs = uc_mallocstrw(ndp,-1,&iap->pathprefix) ;

	        } /* end if */

	    } else
		f_ok = TRUE ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_pathprefix: ret rs=%d f_ok=%u\n",rs,f_ok) ;
#endif

	return (rs >= 0) ? f_ok : rs ;
}
/* end subroutine (bbinter_pathprefix) */


static int bbinter_viewtop(iap,vln)
BBINTER		*iap ;
int		vln ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	f ;


	if (vln < 1)
	    vln = 1 ;

/* get out if no Mail-Cache (MC) */

	if (! iap->f.mcinit)
	    goto ret0 ;

/* free up Message-View (MV) if called for */

	if ((rs >= 0) && iap->f.mvinit) {
	    f = iap->f.viewchange ;
	    f = f || (iap->miscanpoint != iap->miviewpoint) ;
	    if (f) {
	        iap->f.viewchange = FALSE ;
	        rs = bbinter_msgviewclose(iap) ;
	    }
	}

/* open a MSGVIEW if not already open */

	if ((rs >= 0) && (! iap->f.mvinit)) {
	    rs = bbinter_msgviewopen(iap) ;
	} /* end if */

	if (rs >= 0) {
	    rs = bbinter_msgviewtop(iap,(vln-1)) ;
	    iap->nviewlines = rs ;
	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (bbinter_viewtop) */


static int bbinter_viewnext(iap,inc)
BBINTER		*iap ;
int		inc ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	f ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_viewnext: f_mcinit=%u inc=%d\n",iap->f.mcinit,inc) ;
#endif

/* get out if no Mail-Cache (MC) */

	if (! iap->f.mcinit)
	    goto ret0 ;

/* free up Message-View (MV) if called for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	debugprintf("bbinter_viewnext: f_mvinit=%u\n",iap->f.mvinit) ;
	debugprintf("bbinter_viewnext: f_viewchange=%u\n",iap->f.viewchange) ;
	debugprintf("bbinter_viewnext: miscanpoint=%d miviewpoint=%d\n",
		iap->miscanpoint,iap->miviewpoint) ;
	}
#endif

	if ((rs >= 0) && iap->f.mvinit) {
	    f = iap->f.viewchange ;
	    f = f || (iap->miscanpoint != iap->miviewpoint) ;
	    if (f) {
	        iap->f.viewchange = FALSE ;
	        rs = bbinter_msgviewclose(iap) ;
	    }
	}

/* open a MSGVIEW if not already open */

	if ((rs >= 0) && (! iap->f.mvinit)) {
	    inc = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_viewnext: _msgviewopen() \n") ;
#endif

	    rs = bbinter_msgviewopen(iap) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	debugprintf("bbinter_viewnext: _msgviewopen() rs=%d\n",rs) ;
	debugprintf("bbinter_viewnext: f_mvinit=%u\n",iap->f.mvinit) ;
	}
#endif

	} /* end if */

	if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_viewnext: _msgviewnext() \n") ;
#endif

	    rs = bbinter_msgviewnext(iap,inc) ;
	    iap->nviewlines = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_viewnext: _msgviewnext() rs=%d\n",rs) ;
#endif

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_viewnext: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bbinter_viewnext) */


static int bbinter_msgviewopen(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	MBCACHE_SCAN	*msp ;

	int	rs = SR_OK ;
	int	rs1 = SR_OK ;
	int	nlines ;
	int	vlines = -1 ;
	int	mt ;
	int	mi = iap->miscanpoint ;

	const char	*midp ;
	const char	*mfp = NULL ;

	char	msgid[MAILADDRLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_msgviewopen: f_mvinit=%u\n",iap->f.mvinit) ;
#endif

	iap->f.viewchange = FALSE ;
	if (iap->f.mvinit || (iap->miscanpoint < 0)) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	if ((rs >= 0) && (iap->mbname == NULL))
	    rs = SR_NOANODE ;

	if (rs < 0)
	    goto ret0 ;

	iap->miviewpoint = mi ;
	iap->lnviewtop = -1 ;
	rs = mbcache_msgscan(&iap->mc,mi,&msp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_msgviewopen: mbcache_msgscan() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	mt = MAILMSGFILE_TTEMP ;
	midp = msp->vs[mbcachemf_mid] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("bbinter_msgviewopen: mid=>%s<\n", midp) ;
	    debugprintf("bbinter_msgviewopen: mbname=>%s<\n",iap->mbname) ;
	    debugprintf("bbinter_msgviewopen: misscanpoint=%d\n",
		iap->miscanpoint) ;
	}
#endif

	if ((midp == NULL) || (midp[0] == '\0')) {
	    mt = MAILMSGFILE_TTEMP ;
	    midp = msgid ;
	    rs1 = snsdd(msgid,MAILADDRLEN,iap->mbname,iap->miscanpoint) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
		char	hexstr[100+1] ;
		mkhexstr(hexstr,100,msgid,-1) ;
	    debugprintf("bbinter_msgviewopen: 0 rs1=%d mid=>%s<\n",
		rs1,hexstr) ;
	}
#endif

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_msgviewopen: rs1=%d mid=>%t<\n",
		rs1,midp,rs1) ;
#endif

	rs1 = mailmsgfile_get(&iap->msgfiles,midp,&mfp) ;
	nlines = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("bbinter_msgviewopen: mailmsgfile_get() rs=%d\n",rs1) ;
	    if (rs1 >= 0)
	        debugprintf("bbinter_msgviewopen: mfp=%s\n",mfp) ;
	}
#endif

	if (rs1 == SR_NOTFOUND) {

	    if (iap->mfd < 0)
	        rs = bbinter_mbviewopen(iap) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("bbinter_msgviewopen: _mbviewopen() rs=%d fd=%d\n",
	            rs,iap->mfd) ;
#endif

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("bbinter_msgviewopen: boff=%lu blen=%u\n",
	                msp->boff,msp->blen) ;
#endif

	        rs = mailmsgfile_new(&iap->msgfiles,mt,midp,
	            iap->mfd,msp->boff,msp->blen) ;
	        nlines = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("bbinter_msgviewopen: mailmsgfile_new() rs=%d\n",
			rs) ;
	            debugprintf("bbinter_msgviewopen: msgscanlines=%d\n",
			msp->nlines) ;
	        }
#endif

	    } /* end if */

	    if (rs >= 0) {

	        rs = mailmsgfile_get(&iap->msgfiles,midp,&mfp) ;
		nlines = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("bbinter_msgviewopen: mailmsgfile_get() rs=%d\n",
			rs) ;
#endif

	    }

	} /* end if */

	if ((rs >= 0) && (nlines >= 0)) {

		vlines = msp->vlines ;
		if (vlines < 0) vlines = msp->nlines ;

	        if ((rs >= 0) && (vlines != nlines)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("bbinter_msgviewopen: new mi=%u lines=%u\n",
	                    mi,nlines) ;
#endif

	            rs = mbcache_msgsetlines(&iap->mc,mi,nlines) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("bbinter_msgviewopen: "
			"mbcache_msgsetlines() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {
	                rs = display_scanloadlines(&iap->di,mi,nlines,TRUE) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("bbinter_msgviewopen: "
			"display_scanloadlines() rs=%d\n",rs) ;
#endif

		    }

	        } /* end if (setting lines) */

	} /* end if (lines) */

	if ((rs >= 0) && (mfp != NULL)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("bbinter_msgviewopen: mfp=%s\n",mfp) ;
#endif

	    rs = mailmsgviewer_open(&iap->mv,mfp) ;
	    iap->f.mvinit = (rs >= 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("bbinter_msgviewopen: mailmsgviewer_open() rs=%d\n",
			rs) ;
#endif

	    if (rs >= 0) {
		DISPLAY_BOTINFO	bi ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("bbinter_msgviewopen: nlines=%d\n",nlines) ;
#endif

		memset(&bi,0,sizeof(DISPLAY_BOTINFO)) ;
	        strwcpy(bi.msgfrom,msp->vs[mbcachemf_from],DISPLAY_LMSGFROM) ;
		bi.msgnum = (mi+1) ;
		bi.msglines = nlines ;
		bi.msgline = 1 ;
		rs = display_botinfo(&iap->di,&bi) ;
	    } /* end if */

	} /* end if */

	if (rs < 0)
	    goto bad1 ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_msgviewopen: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad1:
	if (iap->f.mvinit) {
	    iap->f.mvinit = FALSE ;
	    mailmsgviewer_close(&iap->mv) ;
	}

	if (iap->mfd >= 0)
	    bbinter_mbviewclose(iap) ;

bad0:
	goto ret0 ;
}
/* end subroutine (bbinter_msgviewopen) */


static int bbinter_msgviewclose(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_msgviewclose: entered\n") ;
#endif

	if (! iap->f.mvinit) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	iap->lnviewtop = -1 ;
	iap->miviewpoint = -1 ;
	if (iap->f.mvinit) {
	    iap->f.mvinit = FALSE ;
	    rs = mailmsgviewer_close(&iap->mv) ;
	}

ret0:
	return rs ;
}
/* end subroutine (bbinter_msgviewclose) */


static int bbinter_msgviewsetlines(iap,nlines)
BBINTER		*iap ;
int		nlines ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	mi ;
	int	vlines ;
	int	f = FALSE ;


#ifdef	COMMENT
	if (! iap->f.mvinit) {
		rs = SR_NOANODE ;
		goto ret0 ;
	}
#endif /* COMMENT */

	mi = iap->miviewpoint ;
	if (mi < 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

	rs = mbcache_msglines(&iap->mc,mi,&vlines) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("bbinter_msgviewsetlines: "
	        "mbcache_msglines() mi=%u lines=%d\n",
	        mi,vlines) ;
#endif

	if (vlines < 0) {

	    f = TRUE ;
	    rs = mbcache_msgsetlines(&iap->mc,mi,nlines) ;

	    if (rs >= 0)
		rs = display_scanloadlines(&iap->di,mi,nlines,TRUE) ;

	} /* end if (setting lines) */

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bbinter_msgviewsetlines) */


static int bbinter_msgviewtop(iap,lntop)
BBINTER		*iap ;
int		lntop ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	c = 0 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_msgviewtop: entered lnviewtop=%d lntop=%d\n",
		iap->lnviewtop,lntop) ;
#endif

	if (lntop < 0)
	    return SR_INVALID ;

	if (! iap->f.mvinit) {
		rs = SR_NOANODE ;
		goto ret0 ;
	}

	if (iap->lnviewtop < 0) {

		lntop = 0 ;
		iap->lnviewtop = 0 ;
		rs = bbinter_msgviewrefresh(iap) ;
		c = rs ;

	} else {

		rs = bbinter_msgviewadj(iap,lntop) ;
		c = rs ;

	} /* end if */

	if ((rs >= 0) && (c > 0))
	    rs = display_botline(&iap->di,(lntop+1)) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_msgviewtop: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_msgviewtop) */


static int bbinter_msgviewadj(iap,lntop)
BBINTER		*iap ;
int		lntop ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	mi ;
	int	vi = 0 ;
	int	ln = 0 ;
	int	ndiff, nadiff ;
	int	lines ;
	int	n ;
	int	c = 0 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_msgviewadj: entered lnviewtop=%d lntop=%d\n",
		iap->lnviewtop,lntop) ;
#endif

	if (lntop < 0)
	    return SR_INVALID ;

	if (iap->lnviewtop < 0)
	    return SR_NOANODE ;

	if (! iap->f.mvinit) {
		rs = SR_NOANODE ;
		goto ret0 ;
	}

	mi = iap->miviewpoint ;
	rs = mbcache_msglines(&iap->mc,mi,&lines) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_msgviewadj: "
		"mbcache_msglines() mi=%u lines=%d\n",
		mi,lines) ;
#endif

	if (lines >= 0) {
	    int	f ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	debugprintf("bbinter_msgviewadj: viewlines=%u lines=%u\n",
		iap->viewlines,lines) ;
	debugprintf("bbinter_msgviewadj: viewtop=%u lntop=%u\n",
		iap->lnviewtop,lntop) ;
	}
#endif

	    f = (lntop >= lines) ;
	    if (f)
		goto ret0 ;

	} /* end if */

	ndiff = (lntop - iap->lnviewtop) ;
	nadiff = abs(ndiff) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("bbinter_msgviewadj: ndiff=%d\n",ndiff) ;
#endif

	if (nadiff >= iap->viewlines) {

	    iap->lnviewtop = lntop ;
	    rs = bbinter_msgviewrefresh(iap) ;
	    c = rs ;

	} else {

	    if (ndiff > 0) {
		vi = (iap->viewlines - ndiff) ;
		ln = (iap->lnviewtop + iap->viewlines) ;
	    } else if (ndiff < 0) {
		vi = 0 ;
		ln = MAX((iap->lnviewtop + ndiff),0) ;
	    }

	    if (ndiff != 0) {
		rs = display_viewscroll(&iap->di,ndiff) ;
		if (rs >= 0) {
		    rs = bbinter_msgviewload(iap,vi,nadiff,ln) ;
		    n = rs ;
		    c = (iap->viewlines - nadiff + n) ;
		}
	    }

	    if (rs >= 0)
		iap->lnviewtop = lntop ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_msgviewadj: ret rs=%d viewtop=%u c=%u\n",
		rs,iap->lnviewtop,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_msgviewadj) */


static int bbinter_msgviewnext(iap,inc)
BBINTER		*iap ;
int		inc ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	lntop ;
	int	c = 0 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_msgviewnext: inc=%d\n",inc) ;
#endif

	lntop = MAX((iap->lnviewtop + inc),0) ;
	rs = bbinter_msgviewtop(iap,lntop) ;
	c = rs ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_msgviewnext: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_msgviewnext) */


static int bbinter_msgviewrefresh(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs ;


	rs = bbinter_msgviewload(iap,0,iap->viewlines,iap->lnviewtop) ;

	return rs ;
}
/* end subroutine (bbinter_msgviewrefresh) */


static int bbinter_msgviewload(iap,vi,vn,ln)
BBINTER		*iap ;
int		vi ;
int		vn ;
int		ln ;
{
	struct proginfo	*pip = iap->pip ;

	int	rs = SR_OK ;
	int	ll = -1 ;
	int	i ;
	int	c = 0 ;

	const char	*lp = NULL ;

	char	dum[2] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("bbinter_msgviewload: vi=%u ln=%u\n",vi,ln) ;
#endif

	if ((vi < 0) || (ln < 0))
	    return SR_INVALID ;

	if (! iap->f.mvinit) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	if (vn > iap->viewlines)
	    rs = SR_INVALID ;

/* specified number of message lines (as we may have) */

	i = 0 ;
	while ((rs >= 0) && (i < vn) && (vi < iap->viewlines)) {

	    ll = mailmsgviewer_getline(&iap->mv,ln,&lp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_msgviewload: mailmsgviewer_getline() rs=%d\n",ll) ;
#endif

	    if (ll <= 0)
		break ;

	    rs = display_viewload(&iap->di,vi,lp,ll) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
		debugprintf("bbinter_msgviewload: display_viewload() rs=%d\n",
		rs) ;
#endif

	    i += 1 ;
	    c += 1 ;
	    ln += 1 ;
	    vi += 1 ;

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgviewload: while-end rs=%d\n",
		rs) ;
#endif

/* any needed blank lines */

	dum[0] = '\0' ;
	while ((rs >= 0) && (i < vn) && (vi < iap->viewlines)) {

	    rs = display_viewload(&iap->di,vi,dum,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgviewload: display_viewload() rs=%d\n",
		rs) ;
#endif

	    i += 1 ;
	    vi += 1 ;

	} /* end while */

/* extra */

	if ((rs >= 0) && (ll == 0)) {

	    rs = bbinter_msgviewsetlines(iap,ln) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("bbinter_msgviewload: _msgviewsetlines() rs=%d\n",
		rs) ;
#endif

	}

/* done */
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("bbinter_msgviewload: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbinter_msgviewload) */


static int bbinter_mbviewopen(iap)
BBINTER		*iap ;
{
	struct proginfo	*pip = iap->pip ;

	const mode_t	operms = 0666 ;

	const int	oflags = O_RDONLY ;

	int	rs = SR_OK ;

	char	mbfname[MAXPATHLEN + 1] ;


	if (iap->mfd >= 0)
	    goto ret0 ;

	if (iap->mbname == NULL) {
	    rs = SR_NOANODE ;
	}

	if (rs < 0)
	    goto ret0 ;

	rs = mkpath2(mbfname,pip->folderdname,iap->mbname) ;
	if (rs >= 0) {
	    rs = uc_open(mbfname,oflags,operms) ;
	    iap->mfd = rs ;
	}

ret0:
	return rs ;
}
/* end subroutine (bbinter_mbviewopen) */


static int bbinter_mbviewclose(iap)
BBINTER		*iap ;
{
	int	rs = SR_OK ;


	if (iap->mfd >= 0) {
	    rs = u_close(iap->mfd) ;
	    iap->mfd = -1 ;
	}

	return rs ;
}
/* end subroutine (bbinter_mbviewclose) */


/* catch bbinterrupts, terminate command, and return for new command */
static void sighand_int(sn)
int	sn ;
{
	int	olderrno = errno ;


#if	CF_DEBUGS
	debugprintf("sighand_int: sn=%d\n",sn) ;
#endif

	switch (sn) {

	case SIGWINCH:
	    if_win = TRUE ;
	    break ;

	case SIGTERM:
	    if_term = TRUE ;
	    break ;

	case SIGQUIT:
	    if_quit = TRUE ;
	    break ;

	case SIGINT:
	    if_int = TRUE ;
	    break ;

	default:
	    if_def = TRUE ;
	    break ;

	} /* end switch */

	errno = olderrno ;
}
/* end subroutine (sighand_int) */


/* bbinterrupt subroutine for window size changes */
static void sighand_win(sn)
int	sn ;
{
	int	olderrno = errno ;


	if_win = TRUE ;
	errno = olderrno ;
}
/* end subroutine (sighand_win) */



