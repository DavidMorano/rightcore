/* inter */

/* the user interface (command interpreter) for VMAIL */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_INTERPOLL	1		/* call 'inter_poll()' */
#define	CF_INITTEST	0		/* initialization test */
#define	CF_STARTCLEAR	1		/* clear screen at start */
#define	CF_WELCOME	1		/* issue welcome info */
#define	CF_KBDINFO	1		/* activate KBDINFO */
#define	CF_SIGPENDING	0		/* debug pending signals (hangs) */
#define	CF_SIGSTOP	1		/* use SIGSTOP rather than SIGTSTP */
#define	CF_SUSPEND	0		/* need |inter_suspend()| - NO! */
#define	CF_OPENPROG	0		/* use |uc_openprog(3uc)| */
#define	CF_MARKSPAMSYNC	0		/* mark spam synchronously */
#define	CF_TESTGO1	1		/* test-go-1 */
#define	CF_TRANSPROC	1		/* use |inter-trans()| */


/* revision history:

	= 2009-01-20, David A­D­ Morano
        This is a complete rewrite of the (utter) trash that performed
        this function previously.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the main interactive loop.

	Implimentation notes:

        Caching: We cache scanlines (scan-line data) in two places. This is
        probably needless but we are doing it anyway. It is first cached in the
        mailbox-cache (MBCACHE) object. It is secondarily cached in our own
        DISPLAY object. Mail-message viewing data is cached in the MAILMSGFILE
        object.


*******************************************************************************/


#define	INTER_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<sbuf.h>
#include	<spawnproc.h>
#include	<keysym.h>
#include	<toxc.h>
#include	<ansigr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"display.h"
#include	"help.h"
#include	"mailmsgfile.h"
#include	"mailmsgviewer.h"
#include	"readcmdkey.h"
#include	"termcmd.h"
#include	"kshlib.h"

#include	"inter.h"
#include	"inter_trans.h"


/* local defines */

#ifndef	DISBUFLEN
#define	DISBUFLEN	(COLUMNS*2)
#endif

#ifndef	RBUFLEN
#define	RBUFLEN		(COLUMNS*2)
#endif

#define	UTOPTS		(FM_NOFILTER | FM_NOECHO | FM_RAWIN | FM_TIMED)

#undef	CNTRL
#define CNTRL(x)	((x) & 037)

#ifndef	CH_BELL
#define CH_BELL		'\07'
#endif

#ifndef	CH_DELETE
#define CH_DELETE	'\177'
#endif

#ifndef	SCANMARGIN
#define	SCANMARGIN	2
#endif

#ifndef	TO_INFO
#define	TO_INFO		4
#endif

#ifndef	TO_LOCK
#define	TO_LOCK		4
#endif

#ifndef	INTER_FSTR
#define	INTER_FSTR	struct inter_fieldstr
#endif


/* external subroutines */

extern int	snsdd(char *,int,const char *,uint) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	termconseq(char *,int,int,int,int,int,int) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
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
extern int	pcsmailcheck(const char *,char *,int,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	spawncmdproc(SPAWNPROC *,const char *,const char *) ;
extern int	uterm_readcmd(UTERM *,TERMCMD *,int,int) ;
extern int	msleep(int) ;
extern int	hasallalnum(const char *,int) ;
extern int	hasprintbad(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	iscmdstart(int) ;
extern int	isNotAccess(int) ;
extern int	isNotPresent(int) ;

extern int	mailboxappend(const char *,int,int) ;
extern int	mkdisplayable(char *,int,const char *,int) ;
extern int	compactstr(char *,int) ;

extern int	progconf_check(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const void *,int) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	mkhexstr(char *,int,const void *,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */

INTER_FSTR {
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

static int	inter_starter(INTER *) ;

static int	inter_sigbegin(INTER *) ;
static int	inter_sigend(INTER *) ;

static int	inter_prefixbegin(INTER *) ;
static int	inter_prefixer(INTER *,cchar *) ;
static int	inter_prefixend(INTER *) ;

static int	inter_cmdbegin(INTER *) ;
static int	inter_cmdend(INTER *) ;
static int	inter_cmdkbd(INTER *,KEYSYMER *) ;

static int	inter_msgerbegin(INTER *) ;
static int	inter_msgerend(INTER *) ;

static int	inter_displaybegin(INTER *iap) ;
static int	inter_displayend(INTER *iap) ;

static int	inter_linescalc(INTER *) ;
static int	inter_loadcmdmap(INTER *) ;
static int	inter_loadcmdmapsc(INTER *,vecstr *) ;
static int	inter_loadcmdmapfile(INTER *,const char *) ;
static int	inter_loadcmdkey(INTER *,const char *,int) ;
static int	inter_loadcmdkeyone(INTER *,INTER_FSTR *) ;
static int	inter_startclear(INTER *) ;
static int	inter_startwelcome(INTER *) ;
static int	inter_refresh(INTER *) ;
static int	inter_cmdin(INTER *) ;
static int	inter_cmdinesc(INTER *,int) ;
static int	inter_cmddig(INTER *,int) ;
static int	inter_cmdhandle(INTER *,int) ;
#ifdef	COMMENT
static int	inter_charin(INTER *,const char *,...) ;
#endif /* COMMENT */
static int	inter_done(INTER *) ;
static int	inter_welcome(INTER *) ;
static int	inter_version(INTER *) ;
static int	inter_user(INTER *) ;
static int	inter_cmdhelp(INTER *) ;
static int	inter_testmsg(INTER *) ;
static int	inter_poll(INTER *) ;
static int	inter_checkconfig(INTER *) ;
static int	inter_checkwinsize(INTER *) ;
static int	inter_checksusp(INTER *) ;
static int	inter_checkclock(INTER *) ;
static int	inter_checkmail(INTER *) ;
static int	inter_checkmailinfo(INTER *,const char *) ;
static int	inter_checkchild(INTER *,int) ;
static int	inter_info(INTER *,int,const char *,...) ;
static int	inter_winfo(INTER *,int,const char *,int) ;

static int	inter_mailbeginer(INTER *,const char *,int) ;
static int	inter_mailscan(INTER *) ;
static int	inter_mailender(INTER *,int) ;

static int	inter_mbopen(INTER *,const char *,int) ;
static int	inter_mbclose(INTER *) ;

static int	inter_msgargvalid(INTER *,int) ;
static int	inter_msgnum(INTER *) ;
static int	inter_msgpoint(INTER *,int) ;
static int	inter_scancheck(INTER *,int,int) ;
static int	inter_change(INTER *) ;
static int	inter_input(INTER *,char *,int,const char *,...) ;
static int	inter_response(INTER *,char *,int,const char *,...) ;
static int	inter_havemb(INTER *,const char *,int) ;
static int	inter_mailempty(INTER *) ;
static int	inter_viewtop(INTER *,int) ;
static int	inter_viewnext(INTER *,int) ;

static int	inter_cmdpathprefix(INTER *) ;
static int	inter_cmdwrite(INTER *,int,int) ;
static int	inter_cmdpipe(INTER *,int,int) ;
static int	inter_cmdpage(INTER *,int) ;
static int	inter_cmdbody(INTER *,int) ;
static int	inter_cmdbodyer(INTER *,int,cchar *,int) ;
static int	inter_msgviewfile(INTER *,MBCACHE_SCAN *,cchar *,cchar **) ;
static int	inter_setscanlines(INTER *,int,MBCACHE_SCAN *,int,int) ;
static int	inter_cmdmove(INTER *,int) ;
static int	inter_cmdmsgtrash(INTER *,int) ;
static int	inter_cmdmsgdel(INTER *,int,int) ;
static int	inter_cmdmsgdelnum(INTER *,int,int) ;
static int	inter_cmdsubject(INTER *,int) ;
static int	inter_cmdshell(INTER *) ;
static int	inter_cmdmarkspam(INTER *,int) ;
static int	inter_cmdmarkspamer(INTER *,int,const char *,const char *) ;

static int	inter_msgoutfile(INTER *,const char *,int,offset_t,int) ;
static int	inter_msgoutpipe(INTER *,const char *,offset_t,int) ;
static int	inter_msgoutprog(INTER *,const char *,offset_t,int) ;
static int	inter_msgoutproger(INTER *,cchar *,offset_t,int) ;
static int	inter_msgoutview(INTER *,const char *,const char *) ;
static int	inter_msgappend(INTER *,int,const char *) ;
static int	inter_msgdel(INTER *,int,int) ;
static int	inter_msgdelnum(INTER *,int,int,int) ;
static int	inter_subproc(INTER *,pid_t) ;

static int	inter_msgviewopen(INTER *) ;
static int	inter_msgviewopener(INTER *,int,MBCACHE_SCAN *) ;
static int	inter_msgviewtop(INTER *,int) ;
static int	inter_msgviewadj(INTER *,int) ;
static int	inter_msgviewnext(INTER *,int) ;
static int	inter_msgviewclose(INTER *) ;
static int	inter_msgviewrefresh(INTER *) ;
static int	inter_msgviewload(INTER *,int,int,int) ;
static int	inter_msgviewsetlines(INTER *,int) ;

static int	inter_mbviewopen(INTER *) ;
static int	inter_mbviewclose(INTER *) ;
static int	inter_filecopy(INTER *,const char *,const char *) ;
#if	CF_SUSPEND
static int	inter_suspend(INTER *) ;
#endif /* CF_SUSPEND */
#if	CF_SIGPENDING
static int	inter_pending(INTER *) ;
#endif

#if	CF_INITTEST
static int	inter_test(INTER *) ;
#endif


/* local variables */

static cchar	*cmds[] = {
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
	"msgdeleter",
	"msgdelete",
	"msgsubject",
	"help",
	"searchnext",
	"testmsg",
	"markspam",
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
	cmd_msgtrash,
	cmd_msgsubject,
	cmd_help,
	cmd_searchnext,
	cmd_testmsg,
	cmd_markspam,
	cmd_overlast
} ;

static const CMDMAP_E	defcmds[] = {
	{ 'F',			cmd_scanfirst },
	{ 'L',			cmd_scanlast },
	{ 'P',			cmd_scanprevmult },
	{ 'N',			cmd_scannextmult },
	{ 'p',			cmd_scanprev },
	{ 'n',			cmd_scannext },
	{ 'D',			cmd_msgdelete },
	{ 'd',			cmd_msgtrash },
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
	{ 'T',			cmd_testmsg },
	{ 'Z',			cmd_markspam },
	{ 'q',			cmd_quit },
	{ 'z',			cmd_zero },
	{ 'g',			cmd_viewtop },
	{ '=',			cmd_msginfo },
	{ '.',			cmd_msginfo },
	{ '|',			cmd_msgpipe },
	{ 0xA6,			cmd_bodypipe },
	{ '!',			cmd_shell },
	{ 'H',			cmd_help },
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
	{ KEYSYM_Remove,	cmd_msgtrash },
	{ -1, -1 },
} ;

static cchar	*grterms[] = {
	"screen",
	"vt520",
	"vt540",
	"vt420",
	"vt440",
	"vt320",
	"vt340",
	NULL
} ;

static cchar	*syscmdmap[] = {
	"%p/etc/%s/%s.%f",
	"%p/etc/%s/%f",
	"%p/etc/%s.%f",
	NULL
} ;

static cchar	*usrcmdmap[] = {
	"%h/etc/%s/%s.%f",
	"%h/etc/%s/%f",
	"%h/etc/%s.%f",
	NULL
} ;


/* exported subroutines */


int inter_start(INTER *iap,PROGINFO *pip,UTERM *utp)
{
	int		rs ;

	if (iap == NULL) return SR_FAULT ;
	if (utp == NULL) return SR_FAULT ;

	memset(iap,0,sizeof(struct inter_head)) ;
	iap->pip = pip ;
	iap->utp = utp ;
	iap->ti_start = pip->daytime ;
	iap->termlines = pip->lines_term ;
	iap->displines = pip->lines_req ;
	iap->plen = strlen(INTER_IPROMPT) ;
	iap->mfd = -1 ;
	iap->miviewpoint = -1 ;
	iap->delmark = '*' ;

	if ((rs = inter_linescalc(iap)) >= 0) {
	    if ((rs = inter_sigbegin(iap)) >= 0) {
	    	if ((rs = inter_prefixbegin(iap)) >= 0) {
		    if ((rs = inter_cmdbegin(iap)) >= 0) {
			if ((rs = inter_msgerbegin(iap)) >= 0) {
			    if ((rs = inter_displaybegin(iap)) >= 0) {
				if ((rs = inter_transbegin(iap)) >= 0) {
				    rs = inter_starter(iap) ;
				    if (rs < 0)
					inter_transend(iap) ;
				} /* end if (inter-trans) */
				if (rs < 0)
				    inter_displayend(iap) ;
			    } /* end if (inter-display) */
			    if (rs < 0)
				inter_msgerend(iap) ;
			} /* end if (inter-mail) */
			if (rs < 0)
			    inter_cmdend(iap) ;
		    } /* end if (inter-cmd) */
		    if (rs < 0)
			inter_prefixend(iap) ;
		} /* end if (inter_prefixer) */
		if (rs < 0)
	    	    inter_sigend(iap) ;
	    } /* end if (inter-sig) */
	} /* end if (inter_linescalc) */

	return rs ;
}
/* end subroutine (inter_start) */


int inter_finish(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap == NULL) return SR_FAULT ;

	if (iap->magic != INTER_MAGIC) return SR_NOTOPEN ;

	if (iap->trans != NULL) {
	    rs1 = inter_transend(iap) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = inter_mbviewclose(iap) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = inter_mailender(iap,TRUE) ;
	if (rs >= 0) rs = rs1 ;

	if (iap->open.mv) {
	    iap->open.mv = FALSE ;
	    rs1 = mailmsgviewer_close(&iap->mv) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->open.mbcache || iap->open.mailbox) {
	    rs1 = inter_mbclose(iap) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->mbname != NULL) {
	    rs1 = uc_free(iap->mbname) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->mbname = NULL ;
	}

	rs1 = inter_displayend(iap) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = inter_msgerend(iap) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = inter_cmdend(iap) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = inter_prefixend(iap) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = inter_sigend(iap) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_finish: ret rs=%d\n",rs) ;
#endif

	iap->magic = 0 ;
	return rs ;
}
/* end subroutine (inter_finish) */


int inter_cmd(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rc = 1 ;

	if (iap == NULL) return SR_FAULT ;

	if (iap->magic != INTER_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_cmd: ent\n") ;
#endif

#if	CF_WELCOME && 0
	if (rs >= 0) {
	    time_t	ts ;
	    rs1 = display_infots(&iap->di,&ts) ;
	    f = (rs1 >= 0) ;
	    f = f && (pip->daytime > (ts + 7)) ;
	    f = f && (pip->daytime > (iap->ti_start + 3)) ;
	    if (f && (! iap->f.welcome)) {
	        iap->f.cmdprompt = FALSE ;
	        iap->f.welcome = TRUE ;
	        rs = inter_welcome(iap) ;
	    }
	}
#endif /* CF_WELCOME */

	if (rs >= 0) {
	    if ((rs = lib_issig(SIGINT)) > 0) {
	        iap->f.cmdprompt = FALSE ;
	        rs = inter_refresh(iap) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_cmd: f_cmdprompt=%u\n",iap->f.cmdprompt) ;
#endif

	if ((rs >= 0) && (! iap->f.cmdprompt)) {
	    const int	nl = iap->numlen ;
	    cchar	*fmt ;
	    cchar	*pp = INTER_IPROMPT ;
	    char	*np = iap->numbuf ;
	    iap->f.cmdprompt = TRUE ;
	    fmt = "%s %t\v" ;
	    rs = display_input(&iap->di,fmt,pp,np,nl) ;
	}

	if (rs >= 0) {
	    if ((rs = inter_cmdin(iap)) > 0) {
	        const int	cmd = rs ;
	        if (isdigitlatin(cmd) || (cmd == CH_DEL) || (cmd == CH_BS)) {
	            rs = inter_cmddig(iap,cmd) ;
	        } else {
	            iap->f.cmdprompt = FALSE ;
	            rs = inter_cmdhandle(iap,cmd) ;
	        }
	    }
	}

	if (rs >= 0) {
	    if ((rs = lib_issig(SIGTERM)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_cmd: SIGTERM\n") ;
#endif
	        iap->f.exit = TRUE ;
	    }
	}

#if	CF_INTERPOLL
	if ((rs >= 0) && (! iap->f.exit)) {
	    rs = inter_poll(iap) ;
	    if (rs > 0)
	        iap->f.cmdprompt = FALSE ;
	}
#endif

	if ((rs >= 0) && iap->f.exit) {
	    rs = inter_done(iap) ;
	}

	rc = (iap->f.exit) ? 0 : 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("inter_cmd: f_exit=%u\n",iap->f.exit) ;
	    debugprintf("inter_cmd: ret rs=%d rc=%d\n",rs,rc) ;
	}
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (inter_cmd) */


/* private subroutines */


static int inter_starter(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

#if	CF_INITTEST
	if (rs >= 0) {
	    rs = inter_test(iap) ;
	}
#endif

	if (rs >= 0) {
	    if ((rs = inter_mailbeginer(iap,pip->mbname_cur,-1)) >= 0) {

#if	CF_WELCOME 
	        rs = inter_startwelcome(iap) ;
#endif /* CF_WELCOME */

		if (rs >= 0) {
		    iap->magic = INTER_MAGIC ;
		}

	        if (rs < 0) {
	           inter_mailender(iap,TRUE) ;
	        }
	    } /* end if (inter_mailbeginer) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (inter_starter) */


static int inter_linescalc(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	double		percent = 0.0 ;
	int		rs = SR_OK ;
	int		f_percent = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_linescalc: displines=%u\n",iap->displines) ;
#endif

/* scan-lines */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_linescalc: svlines=%u\n",pip->svlines) ;
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
	    iap->scanlines = (int) ((iap->displines * percent) / 100.0) ;

	if (iap->scanlines < MINSCANLINES)
	    iap->scanlines = MINSCANLINES ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_linescalc: scanlines=%u%c\n",
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

	if (f_percent) {
	    iap->jumplines = (int) ((iap->scanlines * percent) / 100.0) ;
	}

	if (iap->jumplines < 1)
	    iap->jumplines = 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_linescalc: jumplines=%u%c\n",
	        iap->jumplines,((f_percent) ? '%' : ' ')) ;
#endif

/* view-lines */

	iap->viewlines = (iap->displines - iap->scanlines - FRAMELINES) ;

	return rs ;
}
/* end subroutine (inter_linescalc) */


static int inter_sigbegin(INTER	 *iap)
{
	PROGINFO	*pip = iap->pip ;
	UTERM		*utp = iap->utp ;
	const int	ucmd = utermcmd_getpgrp ;
	int		rs ;
	if ((rs = uterm_control(utp,ucmd,0)) >= 0) {
	    iap->f.ctty = TRUE ;
	    iap->pgrp = rs ;
	} else if (rs == SR_NOTTY) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: not on a controlling terminal (%d)\n" ;
	    if (! pip->f.quiet) {
	        bprintf(pip->efp,fmt,pn,rs) ;
	    }
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (inter_sigbegin) */


static int inter_sigend(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->f.subprocs) {
	    iap->f.subprocs = FALSE ;
	    rs1 = subprocs_finish(&iap->sp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (inter_sigend) */


static int inter_prefixbegin(INTER *iap)
{
	int		rs ;
	if ((rs = proginfo_pwd(iap->pip)) >= 0) {
	    rs = inter_prefixer(iap,"") ;
	}
	return rs ;
}
/* end subroutine (inter_prefixbegin) */


static int inter_prefixend(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->pathprefix != NULL) {
	    rs1 = uc_free(iap->pathprefix) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->pathprefix = NULL ;
	}

	return rs ;
}
/* end subroutine (inter_prefixend) */


static int inter_prefixer(INTER *iap,cchar *pathprefix)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		f_ok = FALSE ;
	const char	*ndp ;
	char		tmpdname[MAXPATHLEN + 1] ;
	char		newdname[MAXPATHLEN + 1] = { 0 } ;

	if (pathprefix == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_prefixer: pf=%s\n",pathprefix) ;
#endif

	if (pathprefix[0] == '\0')
	    pathprefix = pip->pwd ;

	if (pathprefix[0] != '/') {
	    ndp = newdname ;
	    rs1 = mkpath2(tmpdname,iap->pathprefix,pathprefix) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_prefixer: rs1=%d td=%s\n",rs1,tmpdname) ;
#endif
	    if (rs1 >= 0)
	        rs1 = pathclean(newdname,tmpdname,rs1) ;
	} else
	    ndp = pathprefix ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_prefixer: rs1=%d nd=%s\n",rs1,ndp) ;
#endif

	if ((rs >= 0) && (rs1 >= 0)) {
	    const char	*px = iap->pathprefix ;
	    if ((px == NULL) || (strcmp(px,ndp) != 0)) {
		struct ustat	sb ;

	        rs1 = u_stat(ndp,&sb) ;

	        if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode)))
	            rs1 = SR_NOTDIR ;

	        if (rs1 >= 0) {
		    const char	*cp ;

	            f_ok = TRUE ;
	            if (iap->pathprefix != NULL) {
	                uc_free(iap->pathprefix) ;
	                iap->pathprefix = NULL ;
	            }

	            if ((rs = uc_mallocstrw(ndp,-1,&cp)) >= 0) {
			iap->pathprefix = cp ;
		    }

	        } /* end if */

	    } else
	        f_ok = TRUE ;
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_prefixer: ret rs=%d f_ok=%u\n",rs,f_ok) ;
#endif

	return (rs >= 0) ? f_ok : rs ;
}
/* end subroutine (inter_prefixer) */


static int inter_cmdbegin(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	if ((rs = cmdmap_start(&iap->cm,defcmds)) >= 0) {
	    KEYSYMER	*ksp = &iap->ks ;
	    cchar	*pr = pip->pr ;
	    iap->open.cmdmap = TRUE ;
	    if ((rs = keysymer_open(ksp,pr)) >= 0) {
	        iap->open.keysymer = TRUE ;
		rs = inter_cmdkbd(iap,ksp) ;
		if (rs < 0) {
		    iap->open.keysymer = FALSE ;
	    	    keysymer_close(&iap->ks) ;
		}
	    } else {
		cchar	*pn = pip->progname ;
		cchar	*fmt = "%s: KEYSYMs not found (%d)\n" ;
		bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (keysymer_open) */
	    if (rs < 0) {
	    	iap->open.cmdmap = FALSE ;
		cmdmap_finish(&iap->cm) ;
	    }
	} /* end if (cmdmap_start) */
	return rs ;
}
/* end subroutine (inter_cmdbegin) */


static int inter_cmdend(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->open.kbdinfo) {
	    iap->open.kbdinfo = FALSE ;
	    rs1 = kbdinfo_close(&iap->ki) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->open.keysymer) {
	    iap->open.keysymer = FALSE ;
	    rs1 = keysymer_close(&iap->ks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (iap->open.cmdmap) {
	    iap->open.cmdmap = FALSE ;
	    rs1 = cmdmap_finish(&iap->cm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (inter_cmdend) */


static int inter_cmdkbd(INTER *iap,KEYSYMER *ksp)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	cchar		*kt ;
	kt = pip->kbdtype ;
	if ((kt != NULL) && (kt[0] != '\0')) {
	    cchar	*pr = pip->pr ;
	    cchar	*kd = KBDNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(tbuf,pr,kd,kt)) >= 0) {
		USTAT	sb ;
		if ((rs = u_stat(tbuf,&sb)) >= 0) {
		    const int	am = R_OK ;
		    if ((rs = sperm(&pip->id,&sb,am)) >= 0) {
			KBDINFO	*kip = &iap->ki ;
			if ((rs = kbdinfo_open(kip,ksp,tbuf)) >= 0) {
			    iap->open.kbdinfo = TRUE ;
			    rs = inter_loadcmdmap(iap) ;
			    if (rs < 0) {
				iap->open.kbdinfo = FALSE ;
				kbdinfo_close(kip) ;
			    }
			} else {
			    cchar	*pn = pip->progname ;
			    cchar	*fmt ;
			    fmt = "%s: KBD database not found (%d)\n" ;
			    bprintf(pip->efp,fmt,pn,rs) ;
			} /* end if (kbdinfo_open) */
		    } else if (isNotAccess(rs)) {
			rs = SR_OK ;
		    }
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
	    } /* end if (mkpath) */
	} /* end if (keyboard-type) */
	return rs ;
}
/* end subroutine (inter_cmdkbd) */


static int inter_msgerbegin(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,pip->vmdname,MSGDNAME)) >= 0) {
	    const mode_t	dm = VMDMODE ;
	    if ((rs = mkdirs(tbuf,dm)) >= 0) {
		MAILMSGFILE	*mmp = &iap->msgfiles ;
	        const int	cols = pip->linelen ;
	        if ((rs = mailmsgfile_start(mmp,tbuf,cols,-1)) >= 0) {
	    	    iap->open.msger = TRUE ;
		}
	    }
	}
	return rs ;
}
/* end subroutine (inter_msgerbegin) */


static int inter_msgerend(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (iap->open.msger) {
	    iap->open.msger = FALSE ;
	    rs1 = mailmsgfile_finish(&iap->msgfiles) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (inter_msgerend) */


/* initialize display manager */
static int inter_displaybegin(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	DISPLAY		*dip = &iap->di ;
	DISPLAY_ARGS	da ;
	int		rs ;

	memset(&da,0,sizeof(DISPLAY_ARGS)) ;
	da.termtype = pip->termtype ;
	da.tfd = pip->tfd ;
	da.displines = iap->displines ;
	da.scanlines = iap->scanlines ;
	if ((rs = display_start(dip,pip,&da)) >= 0) {
	    iap->open.display = TRUE ;

#if	CF_STARTCLEAR
	    rs = inter_startclear(iap) ;
#endif /* CF_STARTCLEAR */

	    if (rs >= 0) {
	        rs = display_rframe(dip) ;
	    }

	    if (rs < 0) {
	        iap->open.display = FALSE ;
		display_finish(dip) ;
	    }
	} /* end if (display_start) */

	return rs ;
}
/* end subroutine (inter_displaybegin) */


static int inter_displayend(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (iap->open.display) {
	    iap->open.display = FALSE ;
	    rs1 = display_finish(&iap->di) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (inter_displayend) */


static int inter_loadcmdmap(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	VECSTR		sc ;
	int		rs ;
	int		rs1 ;
	const char	*cfn = CMDMAPFNAME ;

	if ((rs = vecstr_start(&sc,3,0)) >= 0) {
	    if ((rs = inter_loadcmdmapsc(iap,&sc)) >= 0) {
		const int	am = R_OK ;
	        const int	flen = MAXPATHLEN ;
	        const char	**sa ;
	        char		fbuf[MAXPATHLEN + 1] ;

	        if (rs >= 0) {
	            sa = syscmdmap ; /* first: system (S) CMDMAP */
	            if ((rs = permsched(sa,&sc,fbuf,flen,cfn,am)) >= 0) {
	                rs = inter_loadcmdmapfile(iap,fbuf) ;
		    } else if (isNotAccess(rs)) {
			rs = SR_OK ;
		    }
	        }

	        if (rs >= 0) {
	            sa = usrcmdmap ; /* second: user (U) CMDMAP */
	            if ((rs = permsched(sa,&sc,fbuf,flen,cfn,am)) >= 0) {
	                rs = inter_loadcmdmapfile(iap,fbuf) ;
		    } else if (isNotAccess(rs)) {
			rs = SR_OK ;
		    }
	        }

	    } /* end if (inter_loadcmdmapsc) */
	    rs1 = vecstr_finish(&sc) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_loadcmdmap: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_loadcmdmap) */


static int inter_loadcmdmapsc(INTER *iap,vecstr *scp)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		i ;
	const char	*keys = "hps" ;
	const char	*vp ;
	char		kbuf[2] = { 0, 0 } ;

	for (i = 0 ; keys[i] != '\0' ; i += 1) {
	    kbuf[0] = (keys[i] & 0xff) ;
	    kbuf[1] = '\0' ;
	    vp = NULL ;
	    switch (i) {
	    case 0:
	        vp = pip->userhome ;
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
/* end subroutine (inter_loadcmdmapsc) */


static int inter_loadcmdmapfile(INTER *iap,const char *fname)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_loadcmdmapfile: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if (iap->open.kbdinfo) {
	    bfile	cfile, *cfp = &cfile ;
	    if ((rs = bopen(cfp,fname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        int		sl ;
	        cchar		*sp, *tp ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = breadline(cfp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len-1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((sl = sfshrink(lbuf,len,&sp)) > 0) {
		        if (sp[0] != '#') {
	         	    if ((tp = strnchr(sp,sl,'#')) != NULL) {
				sl = (tp-sp) ;
			    }
	                    rs = inter_loadcmdkey(iap,sp,sl) ;
		        }
		    }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(cfp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (CMDMAP-file) */
	} /* end if (enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_loadcmdmapfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_loadcmdmapfile) */


static int inter_loadcmdkey(INTER *iap,cchar *sp,int sl)
{
	INTER_FSTR	fs[4] ;
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		i ;
	int		fl ;
	int		f_loaded = FALSE ;
	const char	*fp ;

#ifdef	COMMENT
	for (i = 0 ; i < nelem(fs) ; i += 1) fs[i].fl = 0 ;
#else
	memset(fs,0,sizeof(INTER_FSTR)*4) ;
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_loadcmdkey: v=>%t<\n",sp,sl) ;
#endif

	if (sl < 0) sl = ((sp != NULL) ? strlen(sp) : 0) ;

	for (i = 0 ; (i < nelem(fs)) && sl && *sp &&
	    ((fl = sfnext(sp,sl,&fp)) >= 0) ; i += 1) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_loadcmdkey: f=>%t<\n",fp,fl) ;
#endif

	    fs[i].fp = fp ;
	    fs[i].fl = fl ;

	    sl -= ((fp+fl)-sp) ;
	    sp = (fp+fl) ;

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_loadcmdkey: mid rs=%d i=%u\n",rs,i) ;
#endif

	if (i >= 3) {
	    f_loaded = TRUE ;
	    rs = inter_loadcmdkeyone(iap,fs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_loadcmdkey: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? f_loaded : rs ;
}
/* end subroutine (inter_loadcmdkey) */


static int inter_loadcmdkeyone(INTER *iap,INTER_FSTR *fs)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		cl ;
	int		f ;
	const char	*cp ;

	cp = fs[1].fp ;
	cl = fs[1].fl ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_loadcmdkeyone: termtype=%t\n",cp,cl) ;
#endif

#ifdef	COMMENT
	m = nleadstr(pip->kbdtype,cp,cl) ;
	f = ((m > 0) && (cl == m) && (pip->kbdtype[m] == '\0')) ;
#else
	f = (strncmp(pip->kbdtype,cp,cl) == 0) && (pip->kbdtype[cl] == '\0') ;
#endif /* COMMENT */

	if (f) {
	    int		key = 0 ;
	    int		cmd = 0 ;

	if (fs[2].fl == 1) {

	    key = (fs[2].fp[0] & 0xff) ;

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_loadcmdkeyone: keyname=%t\n",
	            fs[2].fp,fs[2].fl) ;
#endif

	    rs1 = SR_NOTFOUND ;
	    if (iap->open.keysymer) {
	        rs1 = keysymer_lookup(&iap->ks,fs[2].fp,fs[2].fl) ;
	        key = rs1 ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_loadcmdkeyone: mid rs1=%d key=\\x%04X\n",
	        rs1,key) ;
#endif

	if (rs1 >= 0) {
	if ((cmd = matstr(cmds,fs[0].fp,fs[0].fl)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("inter_loadcmdkeyone: "
	            "cmdmap_load() key=\\x%04X cmd=%s(%u)\n",
	            key,cmds[cmd],cmd) ;
	    }
#endif

	    rs = cmdmap_load(&iap->cm,key,cmd) ;
	} /* end if (found command) */
	} /* end if (ok) */

	} /* end if (enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_loadcmdkeyone: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_loadcmdkeyone) */


#if	CF_INITTEST

static int inter_test(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	int		w = 0 ;

	if ((rs = display_input(&iap->di,"initializing ... \v")) >= 0) {
	    if ((display_flush(&iap->di)) >= 0) {
		sleep(1) ;
	        display_scanclear(&iap->di) ;
	        display_viewclear(&iap->di) ;
	    }
	}

	return rs ;
}
/* end subroutine (inter_test) */

#endif /* CF_INITTEST */


static int inter_startclear(INTER *iap)
{
	DISPLAY		*dop = &iap->di ;
	int		rs ;
	if ((rs = display_scanclear(dop)) >= 0) {
	    rs = display_viewclear(dop) ;
	}
	return rs ;
}
/* end subroutine (inter_startclear) */


static int inter_startwelcome(INTER *iap)
{
	int		rs = SR_OK ;
	if (! iap->f.welcome) {
	    const time_t	dt = iap->ti_start ;
	    time_t		ts ;
	    if ((rs = display_infots(&iap->di,&ts)) >= 0) {
	        if ((ts == 0) || (dt > (ts + 5))) {
	            iap->f.welcome = TRUE ;
	            rs = inter_welcome(iap) ;
		}
	    }
	} /* end if (welcome needed) */
	return rs ;
}
/* end subroutine (inter_startwelcome) */


static int inter_refresh(INTER *iap)
{
	int		rs ;

	if ((rs = display_refresh(&iap->di)) >= 0) {
	    if (iap->open.mv) {
	        rs = inter_msgviewrefresh(iap) ;
	    } else {
	        rs = display_viewclear(&iap->di) ;
	    }
	}

	return rs ;
}
/* end subroutine (inter_refresh) */


static int inter_cmdin(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	int		to ;
	int		cmd = 0 ;

	to = pip->to_read ;
	if ((rs = display_flush(&iap->di)) >= 0) {
	    const int	ropts = UTOPTS ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    lbuf[0] = '\0' ;
	    if ((rs = uterm_reade(iap->utp,lbuf,1,to,ropts,NULL,NULL)) > 0) {
	        cmd = MKCHAR(lbuf[0]) ;
	        if (iscmdstart(cmd)) {
	            rs = inter_cmdinesc(iap,cmd) ;
	            cmd = rs ;
	        }
	    }
	    pip->daytime = time(NULL) ;
	    pip->now.time = pip->daytime ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_cmdin: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (inter_cmdin) */


static int inter_cmdinesc(INTER *iap,int ch)
{
	PROGINFO	*pip = iap->pip ;
	TERMCMD		ck ;
	int		rs ;
	int		keynum = 0 ;
	int		to ;

	to = pip->to_read ;
	if ((rs = uterm_readcmd(iap->utp,&ck,to,ch)) >= 0) {
	    if (ck.type >= 0) {
		if (iap->open.kbdinfo) {
		    KBDINFO	*kip = &iap->ki ;
		    const int	rsn = SR_NOTFOUND ;
		    if ((rs = kbdinfo_lookup(kip,NULL,0,&ck)) >= 0) {
		        keynum = rs ;
		    } else if (rs == rsn) {
	    		rs = SR_OK ;
	    		keynum = 0 ;
		    }
	        } /* end if (iap->open.kbdinfo) */
	    } /* end if (ck-type) */
	} /* end if (uterm_readcmd) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_cmdinesc: ret rs=%d keynum=\\x%04x\n",
		rs,keynum) ;
#endif

	return (rs >= 0) ? keynum : rs ;
}
/* end subroutine (inter_cmdinesc) */


static int inter_cmddig(INTER *iap,int cmd)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		pl = (iap->plen + 1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int ch = (isprintlatin(cmd)) ? cmd : ' ' ;
	    debugprintf("inter_cmddig: numlen=%u dig=>%c< (%02X)\n",
	        iap->numlen,ch,cmd) ;
	}
#endif

	if (cmd == CH_BS)
	    cmd = CH_DEL ;

	if ((cmd == CH_DEL) && (iap->numlen > 0)) {
	    iap->numlen -= 1 ;
	    rs = display_cmddig(&iap->di,(iap->numlen+pl),cmd) ;
	    iap->numbuf[iap->numlen] = '\0' ;
	} else if (isdigitlatin(cmd) && (iap->numlen < INTER_NUMLEN)) {
	    iap->numbuf[iap->numlen] = cmd ;
	    rs = display_cmddig(&iap->di,(iap->numlen+pl),cmd) ;
	    iap->numlen += 1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (inter_cmddig) */


static int inter_cmdhandle(INTER *iap,int key)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argnum = -1 ;
	int		cmd ;
	int		mult = 1 ;
	int		nmsg = 0 ;
	int		f_nomailbox = FALSE ;
	int		f_errinfo = iap->f.info_err ;
	int		f = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int ch = (isprintlatin(key)) ? key : ' ' ;
	    debugprintf("inter_cmdhandle: key=»%c« (\\x%04X)\n",
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
	    debugprintf("inter_cmdhandle: cmdmap_lookup() cmd=%s (%d)\n",
	        ((cmd < cmd_overlast) ? cmds[cmd] : ""),cmd) ;
	}
#endif

	switch (cmd) {

	case cmd_refresh:
	    rs = inter_refresh(iap) ;
	    if (rs >= 0) {
	        rs = inter_info(iap,FALSE,"refreshed\v") ;
	    }
	    break ;

	case cmd_msginfo:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    rs = inter_msgnum(iap) ;
	    break ;

	case cmd_welcome:
	    rs = inter_welcome(iap) ;
	    break ;

	case cmd_version:
	    rs = inter_version(iap) ;
	    break ;

	case cmd_username:
	    rs = inter_user(iap) ;
	    break ;

	case cmd_help:
	    rs = inter_cmdhelp(iap) ;
	    break ;

	case cmd_testmsg:
	    rs = inter_testmsg(iap) ;
	    break ;

	case cmd_quitquick:
	case cmd_quit:
	    iap->f.exit = TRUE ;
	    if (iap->open.mbcache) {
	        int f = (cmd == cmd_quitquick) ;
	        rs = inter_mailender(iap,f) ;
	    }
	    if (rs >= 0) {
		rs = display_setdate(&iap->di,TRUE) ;
	    }
	    break ;

	case cmd_zero:
	    rs = display_scanclear(&iap->di) ;
	    if (rs >= 0) {
	        rs = display_viewclear(&iap->di) ;
	    }
	    break ;

	case cmd_scanfirst:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    nmsg = 0 ;
	    if (nmsg != iap->miscanpoint) {
	        rs = inter_msgpoint(iap,nmsg) ;
	    }
	    break ;

	case cmd_scanlast:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    nmsg = (iap->miscanpoint + 10000) ;
	    if (nmsg >= iap->nmsgs) {
	        nmsg = (iap->nmsgs - 1) ;
	    }
	    if (nmsg != iap->miscanpoint) {
	        rs = inter_msgpoint(iap,nmsg) ;
	    }
	    break ;

	case cmd_scannext:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg += (argnum > 0) ? argnum : 1 ;
	    if (nmsg < 0) {
	        nmsg = 0 ;
	    }
	    if ((nmsg != iap->miscanpoint) && (nmsg >= 0)) {
	        rs = inter_msgpoint(iap,nmsg) ;
	    }
	    break ;

	case cmd_scanprev:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg -= (argnum > 0) ? argnum : 1 ;
	    if (nmsg < 0) {
	        nmsg = 0 ;
	    }
	    if ((nmsg != iap->miscanpoint) && (nmsg >= 0)) {
	        rs = inter_msgpoint(iap,nmsg) ;
	    }
	    break ;

	case cmd_scannextmult:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    mult = iap->jumplines ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg += (argnum > 0) ? (argnum * mult) : mult ;
	    if (nmsg >= iap->nmsgs) {
	        nmsg = (iap->nmsgs - 1) ;
	    }
	    if ((nmsg != iap->miscanpoint) && (nmsg < iap->nmsgs)) {
	        rs = inter_msgpoint(iap,nmsg) ;
	    }
	    break ;

	case cmd_scanprevmult:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    mult = iap->jumplines ;
	    nmsg = (iap->miscanpoint >= 0) ? iap->miscanpoint : 0 ;
	    nmsg -= (argnum > 0) ? (argnum * mult) : mult ;
	    if (nmsg < 0) {
	        nmsg = 0 ;
	    }
	    if ((nmsg != iap->miscanpoint) && (nmsg >= 0)) {
	        rs = inter_msgpoint(iap,nmsg) ;
	    }
	    break ;

	case cmd_goto:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;

/* FALLTHROUGH */
	case cmd_return:
	    if (argnum > 0) {
	        nmsg = (argnum - 1) ;
	        if (nmsg >= iap->nmsgs) {
	            nmsg = (iap->nmsgs - 1) ;
		}
	        if (nmsg != iap->miscanpoint) {
	            iap->f.viewchange = TRUE ;
	            rs = inter_msgpoint(iap,nmsg) ;
	        }
	    }
	    f = (cmd == cmd_return) || (cmd == cmd_viewnextmult) ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0) && f) {
	        rs = inter_viewnext(iap,(iap->viewlines-1)) ;
	    }
	    break ;

	case cmd_space:
	    if ((rs >= 0) && (iap->miscanpoint >= 0)) {
	        rs = inter_viewnext(iap,(iap->viewlines-1)) ;
	    }
	    break ;

	case cmd_viewtop:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if ((rs >= 0) && (iap->miscanpoint >= 0)) {
	        rs = inter_viewtop(iap,argnum) ;
	    }
	    break ;

	case cmd_viewnextmult:
	    f = TRUE ;

/* FALLTHROUGH */
	case cmd_viewnext:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->open.mv) {
	        if (argnum < 1) argnum = 1 ;
	        if (f) {
	            argnum = argnum * (iap->viewlines - 1) ;
		}
	    } else {
	        argnum = 0 ;
	    }
	    if ((rs >= 0) && (iap->miscanpoint >= 0)) {
	        rs = inter_viewnext(iap,(+ argnum)) ;
	    }
	    break ;

	case cmd_viewprevmult:
	    f = TRUE ;

/* FALLTHROUGH */
	case cmd_viewprev:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->open.mv) {
	        if (argnum < 1) argnum = 1 ;
	        if (f) {
	            argnum = argnum * (iap->viewlines - 1) ;
		}
	    } else {
	        argnum = 0 ;
	    }
	    if ((rs >= 0) && (iap->miscanpoint >= 0)) {
	        rs = inter_viewnext(iap,(- argnum)) ;
	    }
	    break ;

	case cmd_change:
	    rs = inter_change(iap) ;
	    break ;

	case cmd_mbend:
	    rs = inter_mailender(iap,FALSE) ;
	    if (rs >= 0) {
	        rs = inter_mailempty(iap) ;
	    }
	    break ;

	case cmd_cwd:
	    rs = inter_cmdpathprefix(iap) ;
	    break ;

	case cmd_msgwrite:
	case cmd_bodywrite:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
		const int	f_wr = (cmd == cmd_msgwrite) ;
	        rs = inter_cmdwrite(iap,f_wr,argnum) ;
	    }
	    break ;

	case cmd_pagewrite:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdbody(iap,argnum) ;
	    }
	    break ;

	case cmd_msgpipe:
	case cmd_bodypipe:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdpipe(iap,(cmd == cmd_msgpipe),argnum) ;
	    }
	    break ;

	case cmd_markspam:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdmarkspam(iap,argnum) ;
	    }
	    break ;

	case cmd_pagebody:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdpage(iap,argnum) ;
	    }
	    break ;

	case cmd_shell:
	    rs = inter_cmdshell(iap) ;
	    break ;

	case cmd_msgmove:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdmove(iap,argnum) ;
	    }
	    break ;

	case cmd_msgtrash:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdmsgtrash(iap,argnum) ;
	    }
	    break ;

	case cmd_msgundelete:
	case cmd_msgdelete:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    f = (cmd == cmd_msgdelete) ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdmsgdel(iap,f,argnum) ;
	    }
	    break ;

	case cmd_msgundeletenum:
	case cmd_msgdeletenum:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    f = (cmd == cmd_msgdeletenum) ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdmsgdelnum(iap,f,argnum) ;
	    }
	    break ;

	case cmd_msgsubject:
	    f_nomailbox = (! iap->open.mbcache) ;
	    if (f_nomailbox) break ;
	    if (iap->miscanpoint >= 0) {
	        rs = inter_cmdsubject(iap,argnum) ;
	    }
	    break ;

	default:
	    rs = inter_info(iap,TRUE,"invalid command\v") ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        int ch = (isprintlatin(cmd)) ? key : ' ' ;
	        debugprintf("inter_cmdhandle: unknown key=»%c« (\\x%04X)\n",
	            ch,key) ;
	    }
#endif
	    break ;

	} /* end switch */

/* error-info message handling */

	if (f_nomailbox) {
	    inter_info(iap,TRUE,"no current mailbox\v") ;
	}

	if (f_errinfo && (! iap->f.info_msg)) {
	    display_info(&iap->di,"\v") ;
	}

	iap->f.info_msg = FALSE ;

/* done */

	iap->numlen = 0 ;
	iap->numbuf[0] = '\0' ;
	return rs ;
}
/* end subroutine (inter_cmdhandle) */


static int inter_info(INTER *iap,int f_err,cchar *fmt,...)
{
	int		rs ;

	iap->f.info_msg = TRUE ;
	iap->f.info_err = f_err ;

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = display_vinfo(&iap->di,fmt,ap) ;
	    va_end(ap) ;
	}

	return rs ;
}
/* end subroutine (inter_info) */


static int inter_winfo(INTER *iap,int f_err,const char *sp,int sl)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;

	iap->f.info_msg = TRUE ;
	iap->f.info_err = f_err ;
	rs = display_winfo(&iap->di,sp,sl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_winfo: display_winfo() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_winfo) */


static int inter_done(INTER *iap)
{
	int		rs ;

	rs = display_done(&iap->di) ;

	return rs ;
}
/* end subroutine (inter_done) */


static int inter_welcome(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

	iap->ti_info = pip->daytime ;
	pip->to_info = 3 ;
	if (iap->taginfo != infotag_welcome) {
	    const char	*fmt = "welcome %s - type ? for help\v" ;
	    iap->taginfo = infotag_welcome ;
	    rs = inter_info(iap,FALSE,fmt,pip->name) ;
	}

	return rs ;
}
/* end subroutine (inter_welcome) */


static int inter_version(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	const char	*pn = pip->progname ;
	const char	*org = pip->org ;
	const char	*fmt ;

	iap->taginfo = infotag_unspecified ;
	fmt = "%s PCS VMAIL(%s) v=%s\v" ;
	rs = inter_info(iap,FALSE,fmt,org,pn,VERSION) ;

	return rs ;
}
/* end subroutine (inter_version) */


static int inter_user(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	SBUF		b ;
	const int	dlen = DISBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	const char	*org = pip->org ;
	char		dbuf[DISBUFLEN + 1] ;

	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
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

	if ((rs >= 0) && (dbuf[0] != '\0')) {
	    iap->ti_info = pip->daytime ;
	    pip->to_info = 7 ;
	    iap->taginfo = infotag_unspecified ;
	    rs = inter_info(iap,FALSE,"%s\v",dbuf) ;
	    len = rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (inter_user) */


static int inter_cmdhelp(INTER *iap)
{
	int		rs ;
	int		len = 0 ;

	rs = inter_info(iap,FALSE,"hello there\v") ;
	len = rs ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (inter_cmdhelp) */


static int inter_testmsg(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	const char	*tm = pip->testmsg ;

	iap->taginfo = infotag_unspecified ;
	if (tm == NULL) tm = "" ;
	rs = inter_info(iap,FALSE,"testmsg> %s\v",tm) ;

	return rs ;
}
/* end subroutine (inter_testmsg) */


static int inter_poll(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if ((rs >= 0) && (pip->config != NULL)) {
	    rs = inter_checkconfig(iap) ;
	    c += rs ;
	}

	if ((rs >= 0) && pip->f.winadj) {
	    if ((rs = lib_issig(SIGWINCH)) > 0) {
	        rs = inter_checkwinsize(iap) ;
	        c += rs ;
	    }
	}

	if (rs >= 0) {
	    if ((rs = lib_issig(SIGTSTP)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("inter_poll: sig-tstop rs=%d\n",rs) ;
#endif
	        rs = inter_checksusp(iap) ;
	        c += rs ;
	    }
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("inter_poll: sig-tstop-out rs=%d\n",rs) ;
#endif
	}

	if ((rs >= 0) && pip->f.clock) {
	    rs = inter_checkclock(iap) ;
	    c += rs ;
	}

	if ((rs >= 0) && (pip->mailcheck > 0)) {
	    rs = inter_checkmail(iap) ;
	    c += rs ;
	}

	if ((rs >= 0) && iap->f.subprocs) {
	    if ((pip->daytime % 4) == 0) {
	        if ((rs = lib_issig(SIGCHLD)) > 0) {
	            rs = inter_checkchild(iap,TRUE) ;
	            c += rs ;
		}
	    }
	} /* end if (subprocs) */

/* what is this (next part) supposed to do? (don't know!) */

	if ((rs >= 0) && iap->f.setmbname && (iap->mbname == NULL)) {
	    char	zbuf[2] = { 0 } ;
	    iap->f.setmbname = FALSE ;
	    rs = display_setmbname(&iap->di,zbuf,-1) ;
	    c += 1 ;
	}

	if ((rs >= 0) && (! iap->open.mbcache) && (! iap->f.exit)) {
	    rs = inter_mailempty(iap) ;
	    c += rs ;
	}

/* tinmeout any outstanding "info" messages */

	if ((pip->to_info > 0) && 
	    (pip->daytime >= (iap->ti_info + pip->to_info))) {
	    pip->to_info = 0 ;
	    iap->taginfo = 0 ;
	}

/* get out */

	iap->ti_poll = pip->daytime ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_poll) */


static int inter_checkconfig(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	time_t		dt ;
	time_t		ct = iap->ti_config ;
	int		rs = SR_OK ;
	int		to ;
	int		f = FALSE ;

	dt = pip->daytime ;
	to = pip->to_config ;
	if ((dt-ct) >= to) {
	    iap->ti_config = dt ;
	    f = TRUE ;
	    rs = progconf_check(pip) ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_checkconfig) */


static int inter_checkwinsize(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	UTERM		*utp = iap->utp ;
	int		ucmd ;
	int		rs ;
	int		f = FALSE ;

	ucmd = utermcmd_getlines ;
	if ((rs = uterm_control(utp,ucmd,0)) >= 0) {
	    const int	nlines = rs ;
	    if (nlines != iap->displines) { /* lines changed */
	        iap->displines = nlines ;
	        f = TRUE ;
	        if ((rs = inter_linescalc(iap)) >= 0) {
	            const int	dlines = iap->displines ;
	            const int	slines = iap->scanlines ;
	            if ((rs = display_winadj(&iap->di,dlines,slines)) >= 0) {
	                if ((rs = inter_refresh(iap)) >= 0) {
	                    cchar	*fmt ;
	                    fmt = "window-size changed - %d\v" ;
	                    rs = inter_info(iap,FALSE,fmt,nlines) ;
	                    if (pip->debuglevel > 0) {
	                        cchar	*pn = pip->progname ;
	                        bfile	*efp = pip->efp ;
	                        fmt = "%s: window-size changed - %d\n",
	                        bprintf(efp,fmt,pn,nlines) ;
	                    }
	                }
	            }
	        }
	    } /* end if (lines changed) */
	} /* end if (uterm-status) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_checkwinsize) */


static int inter_checksusp(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	UTERM		*utp = iap->utp ;
	SBUF		b ;
	const int	dlen = DISBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		dl = 0 ;
	int		f = FALSE ;
	const char	*msg = "VMAIL suspended" ;
	const char	*pn ;
	const char	*fmt ;
	char		dbuf[DISBUFLEN+1] ;

	pn = pip->progname ;
	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
	    if (matstr(grterms,pip->termtype,-1) >= 0) {
	        char	tbuf[19+1] ;
	        termconseq(tbuf,19,'m',ANSIGR_BOLD,ANSIGR_BLINK,-1,-1) ;
	        sbuf_strw(&b,tbuf,-1) ;
	        sbuf_char(&b,CH_SS3) ;
	        sbuf_char(&b,0x40) ;
	        termconseq(tbuf,19,'m',ANSIGR_OFFALL,-1,-1,-1) ;
	        sbuf_strw(&b,tbuf,-1) ;
	        sbuf_char(&b,' ') ;
	    } /* end if (graphics terminal) */
	    sbuf_strw(&b,msg,-1) ;
	    sbuf_char(&b,'\v') ;
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	    dl = rs1 ;
	} /* end if (sbuf) */

	if (rs >= 0) {
	    if ((rs = inter_winfo(iap,FALSE,dbuf,dl)) >= 0) {
	        if ((rs = display_suspend(&iap->di)) >= 0) {
	            if ((rs = uterm_suspend(utp)) >= 0) {
	                bfile	*efp = pip->efp ;
	                f = TRUE ;

	                if (pip->debuglevel > 0) {
	                    fmt = "%s: program suspended\n",
	                    bprintf(efp,fmt,pn) ;
	                }
	                bflush(efp) ;

#if	CF_SIGSTOP
	                rs = uc_raise(SIGSTOP) ;
#else
	                rs = inter_suspend(iap) ;
#endif

	                if (pip->debuglevel > 0) {
	                    bfile	*efp = pip->efp ;
	                    fmt = "%s: program resumed (%d)\n",
	                    bprintf(efp,fmt,pn,rs) ;
	                }

	                rs1 = uterm_resume(utp) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (uterm-suspend) */
	    	    display_resume(&iap->di) ;
	        } /* end if (display-suspend) */
	    } /* end if (inter-info) */
	    if (rs >= 0) {
	        if ((rs = inter_refresh(iap)) >= 0) {
	            fmt = "VMAIL resumed\v" ;
	            rs = inter_info(iap,FALSE,fmt) ;
	        }
	    } /* end if (ok) */
	} /* end if (ok) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_checksusp) */


static int inter_checkclock(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	time_t		dt ;
	time_t		ct = iap->ti_clock ;
	int		rs = SR_OK ;
	int		to ;
	int		f = FALSE ;

	to = pip->to_clock ;
	dt = pip->daytime ;
	if ((dt-ct) >= to) {
	    iap->ti_clock = dt ;
	    f = TRUE ;
	    rs = display_setdate(&iap->di,FALSE) ;
	} /* end if (needed) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_checkclock) */


static int inter_checkmail(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		to ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if ((to = pip->mailcheck) > 0) {
	    time_t	dt = pip->daytime ;
	    time_t	ct = iap->ti_mailcheck ;
	    char	buf[BUFLEN + 1] ;

	    if ((dt-ct) >= to) {
		cchar	*un = pip->username ;

	        iap->ti_mailcheck = pip->daytime ;
	        rs1 = pcsmailcheck(pip->pr,buf,BUFLEN,un) ;
	        if (rs1 > 0) {
	            f = TRUE ;
	            iap->f.mailnew = TRUE ;
	            if ((rs = inter_checkmailinfo(iap,buf)) >= 0) {
	                rs = display_setnewmail(&iap->di,rs1) ;
		    }
	        } else {
	            if (iap->f.mailnew) {
	                iap->f.mailnew = FALSE ;
	                buf[0] = '\0' ;
	                if ((rs = inter_checkmailinfo(iap,buf)) >= 0) {
	                    rs = display_setnewmail(&iap->di,0) ;
			}
	            } /* end if ("mailnew" enabled) */
	        } /* end if (new mail) */

	    } /* end if (time-out) */
	} /* end if (mailcheck enabled) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_checkmail) */


static int inter_checkmailinfo(INTER *iap,cchar *buf)
{
	PROGINFO	*pip = iap->pip ;
	time_t		dt ;
	time_t		ct = iap->ti_info ;
	int		to ;
	int		rs = SR_OK ;
	int		f ;

	dt = pip->daytime ;
	to = pip->to_info ;
	f = ((dt-ct) >= to) ;
	if (f) {

	if (buf[0] != '\0') {
	    const int	to = iap->ti_mailcheck ;
	    ct = iap->ti_mailinfo ;
	    if ((dt-ct) >= to) {
	        iap->ti_mailinfo = pip->daytime ;
	        iap->f.mailinfo = TRUE ;
	        iap->taginfo = infotag_mailfrom ;
	        rs = inter_info(iap,FALSE,"mailfrom> %s\v",buf) ;
	    } /* end if (time-out) */

	} else if (iap->f.mailinfo) {

	    iap->f.mailinfo = FALSE ;
	    if (iap->taginfo == infotag_mailfrom) {
	        iap->taginfo = 0 ;
	        rs = inter_info(iap,FALSE,"\v") ;
	    }

	} /* end if */

	} /* end if (needed) */

	return rs ;
}
/* end subroutine (inter_checkmailinfo) */


/* ARGSUSED */ /* but really what do we do w/ 'f_check' */
static int inter_checkchild(INTER *iap,int f_check)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

	if (! iap->f.subprocs) {
	    rs = subprocs_start(&iap->sp) ;
	    iap->f.subprocs = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = subprocs_poll(&iap->sp) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("inter_checkchild: subprocs_poll() rs=%d\n",rs) ;
#endif
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_checkchild: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (inter_checkchild) */


static int inter_mailbeginer(INTER *iap,cchar *mbname,int mblen)
{
	PROGINFO	*pip = iap->pip ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_access = FALSE ;
	int		f_readonly = FALSE ;
	char		mbfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("inter_mailbeginer: mbname=%s\n",mbname) ;
	    debugprintf("inter_mailbeginer: folderdname=%s\n",
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

	if (iap->open.mbcache) {
	    rs = inter_mailender(iap,FALSE) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_mailbeginer: inter_mailender() rs=%d\n",rs) ;
#endif

	} /* end if */

	if (rs >= 0) {
	    DISPLAY	*dip = &iap->di ;
	    if ((rs = display_input(dip,"mb=%t\v",mbname,mblen)) >= 0) {
	        iap->f.setmbname = TRUE ;
	        rs = display_setmbname(dip,mbname,mblen) ;
	    }
	    if (rs >= 0)
	        display_flush(dip) ;
	} /* end if (ok) */

	if (rs >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_mailbeginer: inter_mbopen() \n") ;
#endif

	    if ((rs = inter_mbopen(iap,mbfname,f_readonly)) >= 0) {
		cchar	*cp ;
	        if ((rs = uc_mallocstrw(mbname,mblen,&cp)) >= 0) {
		    const int	n = iap->nmsgs ;
		    int		si ;
		    iap->mbname = cp ;
	            iap->miscanpoint = (iap->nmsgs > 0) ? 0 : -1 ;
		    si = iap->miscanpoint ;
	            rs = display_midmsgs(&iap->di,n,si) ;
	        }

	        if (rs >= 0) {
	            iap->miscantop = 0 ;
	            rs = inter_mailscan(iap) ;
	        }

	        if (rs >= 0) {
	            rs = display_viewclear(&iap->di) ;
		}

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
	if ((rs >= 0) && (! f_access)) {
	    rs = inter_info(iap,TRUE,"inaccessible mb=%s\v",mbname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_mailbeginer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_mailbeginer) */


static int inter_mailscan(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (iap->open.mbcache) {
	    int		si = iap->miscantop ;
	    int		n = iap->scanlines ;
	    if (si >= 0) {
	        rs = inter_scancheck(iap,si,n) ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_mailscan: inter_scancheck() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        DISPLAY	*dip = &iap->di ;
	        if ((rs = display_scanpoint(dip,iap->miscanpoint)) >= 0) {
	            rs = display_scandisplay(dip,iap->miscantop) ;
	        }
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_mailscan: display_scandisplay() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_mailscan) */


static int inter_mailender(INTER *iap,int f_quick)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rl, cl ;
	int		i ;
	int		f_yes = TRUE ;
	const char	*ccp ;
	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: ent mbcache=%u\n",iap->open.mbcache) ;
#endif

	if (iap->open.mbcache) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: f_quick=%u\n",f_quick) ;
#endif

	if (f_quick) f_yes = FALSE ;

	if ((rs >= 0) && (! f_quick) && (iap->nmsgdels > 0)) {
	    const int	rlen = RBUFLEN ;
	    char	rbuf[RBUFLEN+ 1] ;
	    f_yes = TRUE ;
	    ccp = "delete marked messages? [yes] \v" ;
	    rs = inter_input(iap,rbuf,rlen,ccp) ;
	    rl = rs ;
	    if ((rs >= 0) && (rl > 0) && (rbuf[rl-1] == '\n')) rl -= 1 ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("inter_mailend: delete? _input() rs=%d\n",rs) ;
#endif
	    if ((rs >= 0) && (rl > 0)) {
	        cl = sfshrink(rbuf,rl,&cp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("inter_mailend: sfshrink() cl=%d c=>%t<\n",
	                cl,cp,cl) ;
	            debugprinthex("inter_mailend: c=",40,cp,cl) ;
	        }
#endif
	        f_yes = ((cl == 0) || (tolc(cp[0]) == 'y')) ;
	    }
	    pip->daytime = time(NULL) ;
	} /* end if (user input) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: rs=%d ndels=%u f_yes=%u\n",
	        rs,iap->nmsgdels,f_yes) ;
#endif

	if ((rs >= 0) && (! f_yes) && (iap->nmsgdels > 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: msg-deleting\n") ;
#endif

	    for (i = 0 ; (rs >= 0) && (i < iap->nmsgs) ; i += 1) {
	        rs = mbcache_msgdel(&iap->mc,i,FALSE) ;
	    } /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: for-out rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = inter_info(iap,FALSE,"deletions canceled\v") ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: deletions rs=%d\n",rs) ;
#endif

	} /* end if */

/* "blank out" all scan lines in the DISPLAY object */

	if (! f_quick) {
	    DISPLAY	*dop = &iap->di ;
	    if (rs >= 0) {
		const int	n = iap->nmsgs ;
	        iap->miscantop = -1 ;
	        if ((rs = display_scanblanks(dop,n)) >= 0) {
	            rs = display_scanfull(dop) ; /* marks some condition */
		}
	    }
	    if (rs >= 0) {
	        iap->miscanpoint = -1 ;
	        rs = display_scanpoint(dop,-1) ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: not-quiet-out rs=%d\n",rs) ;
#endif
	} /* end if (not quick quit) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: 7 rs=%d\n",rs) ;
#endif

/* close out MB resources */

	if (rs >= 0) {
	    rs = inter_mbviewclose(iap) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: 8 rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = inter_mbclose(iap) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: 9 rs=%d\n",rs) ;
#endif

	if (iap->mbname != NULL) {
	    rs1 = uc_free(iap->mbname) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->mbname = NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mailend: 10 rs=%d\n",rs) ;
#endif

	} /* end if (mailbox is open) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_mailend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_mailender) */


#ifdef	COMMENT
int inter_charin(INTER *iap,const char *fmt,...)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;
	int		to ;
	int		ropts = UTOPTS ;
	int		cmd = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	to = pip->to_read ;
	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    rs = vbufprintf(lbuf,LINEBUFLEN,fmt,ap) ;
	    len += rs ;
	    va_end(ap) ;
	}

	if (rs >= 0)
	    rs = display_input(&iap->di,lbuf) ;

	if (rs >= 0) {
	    lbuf[0] = '\0' ;
	    rs = uterm_reade(iap->utp,lbuf,1,to,ropts,NULL,NULL) ;
	    if (rs > 0) cmd = (lbuf[0] & 0xff) ;
	}

	if (rs > 0) {
	    if (iscmdstart(cmd)) {
	        rs = inter_cmdinesc(iap,cmd) ;
	        cmd = rs ;
	    }
	}

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (inter_charin) */
#endif /* COMMENT */


static int inter_mbopen(INTER *iap,cchar *mbfname,int f_ro)
{
	MBCACHE_INFO	mcinfo ;
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		mflags ;

	if (iap->open.mailbox)
	    return SR_NOANODE ;

	mflags = 0 ;
	mflags |= (f_ro) ? MAILBOX_ORDONLY : MAILBOX_ORDWR ;
	mflags |= (! pip->f.useclen) ? MAILBOX_ONOCLEN : 0 ;
	mflags |= (pip->f.useclines) ? MAILBOX_OUSECLINES : 0 ;
	rs1 = mailbox_open(&iap->mb,mbfname,mflags) ;
	iap->open.mailbox = (rs1 >= 0) ;
	iap->f.mbreadonly = f_ro ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_mbopen: mailbox_open() rs1=%d\n",rs1) ;
#endif

	if (rs1 < 0)
	    goto ret0 ;

	rs = mbcache_start(&iap->mc,mbfname,mflags,&iap->mb) ;
	iap->open.mbcache = (rs >= 0) ;
	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_mbopen: mbcache_start() rs=%d\n",rs) ;
#endif

	rs = mbcache_mbinfo(&iap->mc,&mcinfo) ;
	if (rs < 0)
	    goto bad2 ;

	iap->nmsgs = mcinfo.nmsgs ;
	iap->miscanpoint = (mcinfo.nmsgs > 0) ? 0 : -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_mbopen: mbcache_mbinfo() rs=%d nmsgs=%u\n",
	        rs,iap->nmsgs) ;
#endif

ret0:
	return (rs >= 0) ? iap->open.mailbox : rs ;

/* bad stuff */
bad2:
	if (iap->open.mv) {
	    iap->open.mbcache = FALSE ;
	    mbcache_finish(&iap->mc) ;
	}

bad1:
	if (iap->open.mailbox) {
	    iap->open.mailbox = FALSE ;
	    mailbox_close(&iap->mb) ;
	}

	goto ret0 ;
}
/* end subroutine (inter_mbopen) */


static int inter_mbclose(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mbclose: ent f_deldup=%u\n",pip->f.deldup) ;
#endif

	if (iap->open.mbcache && pip->f.deldup) {
	    rs1 = mbcache_msgdeldup(&iap->mc) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mbclose: 9 rs=%d\n",rs) ;
#endif

	if (iap->open.mbcache) {
	    iap->open.mbcache = FALSE ;
	    rs1 = mbcache_finish(&iap->mc) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mbclose: 10 rs=%d\n",rs) ;
#endif

	if (iap->open.mailbox) {
	    iap->open.mailbox = FALSE ;
	    rs1 = mailbox_close(&iap->mb) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mbclose: ret rs=%d\n",rs) ;
#endif

	iap->nmsgs = -1 ;
	iap->nmsgdels = 0 ;
	return rs ;
}
/* end subroutine (inter_mbclose) */


static int inter_msgnum(INTER *iap)
{
	int		rs ;

	if (iap->open.mbcache) {
	    const char	*fmt = "mb=%s msg=%u:%u\v" ;
	    const int	mi = MAX(iap->miscanpoint,0) ;
	    rs = inter_info(iap,FALSE,fmt,iap->mbname,(mi+1),iap->nmsgs) ;
	} else {
	    rs = inter_info(iap,TRUE,"÷ no current mailbox\v") ;
	}

	return rs ;
}
/* end subroutine (inter_msgnum) */


static int inter_msgpoint(INTER *iap,int sipointnext)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		nmsgs = iap->nmsgs ;
	int		nscanlines = iap->scanlines ;
	int		nmargin = SCANMARGIN ;
	int		nabove = 0 ;
	int		nbelow = 0 ;
	int		nscroll = 0 ;
	int		ndiff, nadiff ;
	int		sicheck = 0 ;
	int		sitopcurr = iap->miscantop ;
	int		sitopnext = 0 ;
	int		nt ;
	int		f ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgpoint: sipointnext=%d\n",sipointnext) ;
#endif

	if (! iap->open.mbcache)
	    goto ret0 ;

	if ((sipointnext < 0) || (sipointnext >= nmsgs))
	    goto ret0 ;

	if (sipointnext == iap->miscanpoint)
	    goto ret0 ;

	sitopnext = sitopcurr ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int	sibottom = MIN((sitopcurr + nscanlines),(nmsgs - sitopcurr)) ;
	    int	sipointcurr = iap->miscanpoint ;
	    debugprintf("inter_msgpoint: sitopcurr=%u\n",sitopcurr) ;
	    debugprintf("inter_msgpoint: sipointcurr=%u\n",sipointcurr) ;
	    debugprintf("inter_msgpoint: sibottom=%u\n",sibottom) ;
	}
#endif

	ndiff = (sipointnext - iap->miscanpoint) ;
	nadiff = abs(ndiff) ;
	if (nadiff >= iap->scanlines) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_msgpoint: scan_full ndiff=%d\n",ndiff) ;
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
	        debugprintf("inter_msgpoint: scan_up ndiff=%d\n",ndiff) ;
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
	        debugprintf("inter_msgpoint: scan_down ndiff=%d\n",ndiff) ;
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
	    debugprintf("inter_msgpoint: sitopnext=%u sicheck=%u nscroll=%u\n",
	        sitopnext,sicheck,nscroll) ;
#endif

	if (nscroll > 0) {
	    rs = inter_scancheck(iap,sicheck,nscroll) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgpoint: inter_scancheck() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_scanpoint(&iap->di,sipointnext) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgpoint: display_scanpoint() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = display_scandisplay(&iap->di,sitopnext) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgpoint: display_scandisplay() rs=%d\n",rs) ;
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
	    debugprintf("inter_msgpoint: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_msgpoint) */


static int inter_cmdwrite(INTER *iap,int f_whole,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_cmdwrite: ent\n") ;
#endif

	if (iap->open.mbcache) {
	    int		mi = iap->miscanpoint ;
	    if (argnum >= 0) mi = (argnum - 1) ;
	    if ((mi >= 0) && (mi < iap->nmsgs)) {
		const int	rlen = RBUFLEN ;
	        cchar		*ccp = "file: " ;
	        char		rbuf[RBUFLEN+1] ;
	        if ((rs = inter_response(iap,rbuf,rlen,ccp)) > 0) {
		    MBCACHE_SCAN	*msp ;
		    rl = rs ;
	            if ((rs = mbcache_msgscan(&iap->mc,mi,&msp)) >= 0) {
		        offset_t	outoff ;
		        int		outlen ;
	                if (f_whole) {
	                    outoff = msp->moff ;
	                    outlen = msp->mlen ;
	                } else {
	                    outoff = msp->boff ;
	                    outlen = msp->blen ;
	                }
	                rs = inter_msgoutfile(iap,rbuf,rl,outoff,outlen) ;
	            } /* end if (mbcache-msgscan) */
		} /* end if (inter_response) */
	    } /* end if (valid) */
	} /* end if (mailbox cache open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_cmdwrite: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_cmdwrite) */


static int inter_cmdpipe(INTER *iap,int f_whole,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_cmdpipe: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ; /* lint */
	if (iap->open.mbcache) {
	    const int	rlen = RBUFLEN ;
	    int		mi = iap->miscanpoint ;
	    const char	*ccp = "cmd: " ;
	    char	rbuf[RBUFLEN + 1] ;
	    if (argnum >= 0) mi = (argnum - 1) ;
	    if ((mi >= 0) && (mi < iap->nmsgs))
	    if ((rs = inter_response(iap,rbuf,rlen,ccp)) > 0) {
		    MBCACHE_SCAN	*msp ;

		    if ((rs = mbcache_msgscan(&iap->mc,mi,&msp)) >= 0) {
	    	        offset_t	outoff ;
	    	        int		outlen ;
	    	        if (f_whole) {
	                    outoff = msp->moff ;
	                    outlen = msp->mlen ;
	                } else {
	                    outoff = msp->boff ;
	                    outlen = msp->blen ;
	                }
	                rs = inter_msgoutpipe(iap,rbuf,outoff,outlen) ;
	            } /* end if (scan) */

	    } /* end if (response) */
	} /* end if (valid) */

	return rs ;
}
/* end subroutine (inter_cmdpipe) */


static int inter_msgargvalid(INTER *iap,int argnum)
{
	int		rs = SR_NOMSG ;
	int		mi = 0 ;

	if (iap->open.mbcache) {
	    mi = iap->miscanpoint ;
	    if (argnum >= 0) mi = (argnum - 1) ;
	    if ((mi >= 0) && (mi < iap->nmsgs)) {
	        rs = SR_OK ;
	    }
	}

	if (rs == SR_NOMSG) {
	    const char	*fmt = "msg#%u ÷ %s\v" ;
	    const char	*ccp = "no such message in mailbox" ;
	    inter_info(iap,FALSE,fmt,(mi+1),ccp) ;
	}

	return (rs >= 0) ? mi : rs ;
}
/* end subroutine (inter_msgargvalid) */


static int inter_cmdmarkspam(INTER *iap,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;

	if ((rs = inter_msgargvalid(iap,argnum)) >= 0) {
	    MBCACHE	*mcp = &iap->mc ;
	    const int	mi = rs ;
	    const char	*cmd = pip->prog_postspam ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("inter_cmdmarkspam: cmd=%s\n",cmd) ;
#endif

	    if ((rs = mbcache_msgflags(mcp,mi)) >= 0) {
	        const int	mf = rs ;
	        if (! (mf & MBCACHE_MFMSPAM)) {
	            const int	w = MBCACHE_MFVSPAM ;
	            if ((rs = mbcache_msgsetflag(mcp,mi,w,TRUE)) >= 0) {
	        	const char	*mbname = pip->mbname_spam ;
	                rs = inter_cmdmarkspamer(iap,mi,mbname,cmd) ;
	            }
	        } else {
	            const char	*fmt = "msg#%u ÷ %s\v" ;
	            const char	*ccp = "already spam-processed" ;
	            rs = inter_info(iap,FALSE,fmt,(mi+1),ccp) ;
	        } /* end if (msg spammed or not) */
	    } /* end if (msg-flags) */

	} else if (rs == SR_NOMSG)
	    rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_cmdmarkspam: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (inter_cmdmarkspam) */


static int inter_cmdmarkspamer(INTER *iap,int mi,cchar *mbname,cchar *cmd)
{
	PROGINFO	*pip = iap->pip ;
	offset_t	moff ;
	int		rs ;

	if ((rs = mbcache_msgoff(&iap->mc,mi,&moff)) >= 0) {
	    const int	mlen = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("inter_cmdmarkspamer: moff=%lld mlen=%d\n",
		moff,mlen) ;
#endif
	    if (mbname != NULL) {
	        rs = inter_msgappend(iap,mi,mbname) ;
	    }
	    if ((rs >= 0) && (cmd != NULL)) {
	        if ((rs = inter_msgoutprog(iap,cmd,moff,mlen)) >= 0) {
	            if ((rs = inter_msgdel(iap,mi,TRUE)) >= 0) {
	                const char	*fmt = "msg#%u ÷ %s\v" ;
	                const char	*ccp = "processed as spam" ;
	                rs = inter_info(iap,FALSE,fmt,(mi+1),ccp) ;
	            }
	        } /* end if (msg-out-pipe) */
	    } /* end if */
	} /* end if (msg-offset) */

	return rs ;
}
/* end subroutine (inter_cmdmarkspamer) */


static int inter_cmdpage(INTER *iap,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;

	if ((rs = inter_msgargvalid(iap,argnum)) >= 0) {
	    MBCACHE_SCAN	*msp ;
	    const int		mi = rs ;
	    if ((rs = mbcache_msgscan(&iap->mc,mi,&msp)) >= 0) {
	        if (msp->boff >= 0) {
		    const int	ilen = MAILADDRLEN ;
		    const char	*midp = msp->vs[mbcachemf_hdrmid] ;
		    const char	*mfp = NULL ;
		    char	ibuf[MAILADDRLEN + 1] ;

	            if ((midp == NULL) || (midp[0] == '\0')) {
	                midp = ibuf ;
	                snsdd(ibuf,ilen,iap->mbname,iap->miscanpoint) ;
	            }

	            if ((rs = inter_msgviewfile(iap,msp,midp,&mfp)) >= 0) {
		        const int	f = FALSE ;
	                const int	nl = rs ;
		        if ((rs = inter_setscanlines(iap,mi,msp,nl,f)) >= 0) {
	        	    if (mfp != NULL) {
	            	        cchar	*cmd = pip->prog_pager ;
	            	        rs = inter_msgoutview(iap,cmd,mfp) ;
			    } /* end if (non-null msg-body file) */
		        } /* end if (inter_setscanlines) */
	            } /* end if (inter_msgviewfile) */

	        } else {
	            const char	*fmt = "msg#%u ÷ no msg-body\v" ;
	            rs = inter_info(iap,FALSE,fmt,(mi+1)) ;
	        }
	    } /* end if (mbcache_msgscan) */
	} else if (rs == SR_NOMSG)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (inter_cmdpage) */


static int inter_cmdbody(INTER *iap,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ; /* lint */
	if (iap->open.mbcache) {
	    int		mi = iap->miscanpoint ;
	    if (argnum >= 0) mi = (argnum - 1) ;
	    if ((mi >= 0) && (mi < iap->nmsgs)) {
		const int	rlen = RBUFLEN ;
		cchar		*ccp = "file: " ;
		char		rbuf[RBUFLEN + 1] ;
	        if ((rs = inter_response(iap,rbuf,rlen,ccp)) > 0) {
		    rs = inter_cmdbodyer(iap,mi,rbuf,rs) ;
		}
	   } /* end if (valid) */
	} /* end if (have MB-cache) */

	return rs ;
}
/* end subroutine (inter_cmdbody) */


static int inter_cmdbodyer(INTER *iap,int mi,cchar *rp,int rl)
{
	MBCACHE_SCAN	*msp ;
	int		rs ;
	if ((rs = mbcache_msgscan(&iap->mc,mi,&msp)) >= 0) {
	    if (msp->boff >= 0) {
		const int	ilen = MAILADDRLEN ;
		const char	*midp = msp->vs[mbcachemf_hdrmid] ;
		const char	*mfp ;
		char		ibuf[MAILADDRLEN + 1] ;

	        if ((midp == NULL) || (midp[0] == '\0')) {
	            midp = ibuf ;
	            snsdd(ibuf,ilen,iap->mbname,iap->miscanpoint) ;
	        }

	        if ((rs = inter_msgviewfile(iap,msp,midp,&mfp)) >= 0) {
	            const int	nl = rs ;
		    const int	f = TRUE ;
		    if ((rs = inter_setscanlines(iap,mi,msp,nl,f)) >= 0) {
	        	if (mfp != NULL) {
	            	    if ((rs = inter_filecopy(iap,mfp,rp)) >= 0) {
	  			DISPLAY_BOTINFO	bi ;
	                	cchar 	*vp = msp->vs[mbcachemf_scanfrom] ;
	                	memset(&bi,0,sizeof(DISPLAY_BOTINFO)) ;
	                	strwcpy(bi.msgfrom,vp,DISPLAY_LMSGFROM) ;
	                	bi.msgnum = (mi+1) ;
	                	bi.msglines = nl ;
	                	bi.msgline = 1 ;
	                	rs = display_botinfo(&iap->di,&bi) ;
	            	    } /* end if (inter_filecopy) */
			} /* end if (non-null msg-body file) */
		    } /* end if (inter_setscanlines) */
	        } /* end if (inter_msgviewfile) */

	    } else {
		cchar	*fmt = "msg#%u ÷ no msg-body\v" ;
		rs = inter_info(iap,FALSE,fmt,(mi+1)) ;
	    }
	} /* end if (mbcache_msgscan) */
	return rs ;
}
/* end subroutine (inter_cmdbodyer) */


static int inter_msgviewfile(INTER *iap,MBCACHE_SCAN *msp,cchar *mid,
		cchar **rpp)
{
	PROGINFO	*pip = iap->pip ;
	MAILMSGFILE	*mmfp = &iap->msgfiles ;
	const int	nrs = SR_NOTFOUND ;
	const int	mt = MAILMSGFILE_TTEMP ;
	int		rs ;
	int		nlines = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("inter_msgviewfile: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;
	if (msp == NULL) return SR_FAULT ;
	if (mid == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;
	if (mid[0] == '\0') return SR_INVALID ;

	if ((rs = mailmsgfile_get(mmfp,mid,rpp)) == nrs) {
	    rs = SR_OK ;
	    if (iap->mfd < 0) rs = inter_mbviewopen(iap) ;
	    if (rs >= 0) {
	  	const offset_t	boff = msp->boff ;
		const int	mfd = iap->mfd ;
		const int	blen = msp->blen ;
	 	if ((rs = mailmsgfile_new(mmfp,mt,mid,mfd,boff,blen)) >= 0) {
	    	    nlines = rs ;
		    rs = mailmsgfile_get(mmfp,mid,rpp) ;
		    nlines = rs ;
		}
	    }
	} else {
	    nlines = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("inter_msgviewfile: ret rs=%d nlines=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end if (inter_msgviewfile) */


static int inter_setscanlines(INTER *iap,int mi,MBCACHE_SCAN *msp,int nl,int f)
{
	int		rs = SR_OK ;
	int		vlines = msp->vlines ;
	if (vlines < 0) vlines = msp->nlines ;
	if (vlines != nl) {
	    if ((rs = mbcache_msgsetlines(&iap->mc,mi,nl)) >= 0) {
		rs = display_scanloadlines(&iap->di,mi,nl,f) ;
	    } /* end if (setting lines) */
	} /* end if (lines) */
	return rs ;
}
/* end subroutine (inter_setscanlines) */


static int inter_cmdmove(INTER *iap,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rl ;
	int		cl = 0 ; /* ¥ GCC false complaint */
	int		mi ;
	int		m ;
	int		mblen ;
	int		f_same = FALSE ;
	const char	*ccp ;
	const char	*cp ;
	char		response[LINEBUFLEN + 1] ;
	char		mbname[MAXNAMELEN + 1] ;

	if (! iap->open.mbcache) goto ret0 ;

	mi = iap->miscanpoint ;
	if (argnum >= 0) mi = (argnum - 1) ;
	if ((mi < 0) || (mi >= iap->nmsgs)) /* user error */ goto ret0 ;

	ccp = "mailbox: " ;
	rs = inter_input(iap,response,LINEBUFLEN, ccp) ;
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
	    rs = inter_info(iap,TRUE,"mailbox name is not well formed\v") ;
	    goto ret0 ;
	}

/* does the other mailbox exist? */

	if (iap->mbname != NULL) {
	    m = nleadstr(iap->mbname,cp,cl) ;
	    f_same = (iap->mbname[m] == '\0') && (m == cl) ;
	}

/* return if moving to the same mailbox? */

	if (f_same) {
	    rs = inter_info(iap,TRUE,"same mailbox specified\v") ;
	    goto ret0 ;
	}

	rs1 = snwcpy(mbname,MAXNAMELEN,cp,cl) ;
	mblen = rs1 ;
	if (rs1 < 0) {
	    rs = inter_info(iap,TRUE,"invalid mailbox specified\v") ;
	    goto ret0 ;
	}

/* does the new mailbox exist? */

	rs = inter_havemb(iap,mbname,mblen) ;

	if (rs == 0) {
	    char	disname[MAXNAMELEN + 1] ;

	    mkdisplayable(disname,MAXNAMELEN,cp,cl) ;

	    ccp = "create new mailbox=%s [yes] " ;
	    rs = inter_input(iap,response,LINEBUFLEN, ccp,disname) ;
	    rl = rs ;
	    if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) {
	        rl -= 1 ;
	    }

	    if ((rs >= 0) && (rl > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("inter_cmdmove: response=>%t<\n",
	                response,strlinelen(response,rl,60)) ;
#endif

	        cl = sfshrink(response,rl,&cp) ;
	        if ((cl > 0) && (tolc(*cp) != 'y')) {
	            inter_info(iap,TRUE,"canceled\v") ;
	            goto ret0 ;
	        }
	    }
	}

/* do it */

	if (rs >= 0) {
	    if ((rs = inter_msgappend(iap,mi,mbname)) >= 0) {
	        if (pip->f.nextmov) rs = inter_msgdel(iap,mi,TRUE) ;
	        if (rs >= 0) {
	            const char	*fmt = "msg#%u -> %s\v" ;
	            rs = inter_info(iap,FALSE,fmt,(mi+1),mbname) ;
	        }
	    } /* end if (msg-append) */
	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (inter_cmdmove) */


static int inter_cmdmsgtrash(INTER *iap,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	int		f_del = TRUE ;
	const char	*fmt ;
	const char	*mbname = pip->mbname_trash ;

	if ((mbname != NULL) && (mbname[0] != '\0')) {
	    if ((rs = inter_msgargvalid(iap,argnum)) >= 0) {
	        MBCACHE		*mcp = &iap->mc ;
	        const int	mi = rs ;
	        const int	w = MBCACHE_MFVTRASH ;
	        int		f_delprev = FALSE ;
	        const char	*ccp = NULL ;
	        fmt = "msg#%u ÷ %s\v" ;
	        if ((rs = mbcache_msgsetflag(mcp,mi,w,TRUE)) == 0) {
	            if ((rs = inter_msgappend(iap,mi,mbname)) >= 0) {
	                if ((rs = inter_msgdel(iap,mi,f_del)) >= 0) {
	                    f_delprev = (rs > 0) ;
	                    if (! LEQUIV(f_del,f_delprev)) {
	                        ccp = "deletion canceled" ;
	                        if (f_del) ccp = "deletion scheduled" ;
	                    } else
	                        ccp = "no change" ;
	                }
	            } /* end if (msg-append) */
	        } else if (rs > 0) {
	            ccp = "already trashed" ;
	            if (pip->f.nextdel && ((mi+1) < iap->nmsgs)) {
	                rs = inter_msgpoint(iap,(mi+1)) ;
		    }
	        } /* end if (msg-trash) */
	        if ((rs >= 0) && (ccp != NULL)) {
	            rs = inter_info(iap,FALSE,fmt,(mi+1),ccp) ;
		}
	    } else if (rs == SR_NOMSG)
	        rs = SR_OK ;
	} else {
	    fmt = "no trash mailbox configured" ;
	    rs = inter_info(iap,FALSE,fmt) ;
	}

	return rs ;
}
/* end subroutine (inter_cmdmsgtrash) */


static int inter_cmdmsgdel(INTER *iap,int f_del,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_cmdmsgdel: f_del=%u argnum=%d\n",
	        f_del,argnum) ;
#endif

	if ((rs = inter_msgargvalid(iap,argnum)) >= 0) {
	    const int	mi = rs ;
	    int		f_delprev = FALSE ;
	    if ((rs = inter_msgdel(iap,mi,f_del)) >= 0) {
	        const char	*fmt = "msg#%u ­ %s\v" ;
	        const char	*ccp ;
	        f_delprev = (rs > 0) ;
	        if (! LEQUIV(f_del,f_delprev)) {
	            ccp = (f_del) ? "deletion scheduled" : "deletion canceled" ;
	        } else
	            ccp = "no change" ;
	        rs = inter_info(iap,FALSE,fmt,(mi+1),ccp) ;
	    }
	} else if (rs == SR_NOMSG)
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (inter_cmdmsgdel) */


static int inter_cmdmsgdelnum(INTER *iap,int f_del,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("inter_cmdmsgdelnum: f_del=%u argnum=%d\n",
	        f_del,argnum) ;
	    debugprintf("inter_cmdmsgdelnum: nmsgs=%u\n",iap->nmsgs) ;
	}
#endif

	if (argnum <= 0) argnum = 1 ;
	if (iap->open.mbcache) {
	    const int	mi = iap->miscanpoint ;
	    if ((mi >= 0) && (mi < iap->nmsgs)) {
	        if ((rs = inter_msgdelnum(iap,mi,argnum,f_del)) >= 0) {
	            int		c = rs ;
	            cchar	*s = ((f_del) ? "scheduled" : "canceled") ;
	            cchar	*fmt = "deletions %s=%u\v" ;
	            rs = inter_info(iap,FALSE,fmt,s,c) ;
	        }
	    }
	}

	return rs ;
}
/* end subroutine (inter_cmdmsgdelnum) */


static int inter_cmdsubject(INTER *iap,int argnum)
{
	PROGINFO	*pip = iap->pip ;
	int		mi = iap->miscanpoint ;
	int		rs = SR_OK ;

	if (argnum >= 0) mi = (argnum - 1) ;
	if (iap->open.mbcache) {
	if ((mi >= 0) && (mi < iap->nmsgs)) {
	    MBCACHE_SCAN	*msp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("inter/cmdsubject: ent\n") ;
#endif

/* do it */

	if ((rs = mbcache_msgscan(&iap->mc,mi,&msp)) >= 0) {
	    const int	dislen = DISBUFLEN ;
	    const int	vl = msp->vl[mbcachemf_hdrsubject] ;
	    int		dl = 0 ;
	    cchar	*vp = msp->vs[mbcachemf_hdrsubject] ;
	    char	disbuf[DISBUFLEN+1] = { 0 } ;
#if	CF_DEBUG
		if (DEBUGLEVEL(5))
			debugprintf("inter/cmdsubject: b=>%t<\n",
			vp,strlinelen(vp,vl,40)) ;
#endif
	    if (vp != NULL) {
#if	CF_TRANSPROC
		{
		    const int	n = COLUMNS ;
		    rs = inter_transproc(iap,disbuf,dislen,vp,vl,n) ;
		    dl = rs ;
		}
#else /* CF_TRANSPROC */
	        dl = (strdcpycompact(disbuf,dislen,vp,vl) - disbuf) ;
#endif /* CF_TRANSPROC */
#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
		const int	n = COLUMNS ;
		debugprintf("inter/cmdsubject: dl=%d d=>%t<\n",
			dl,disbuf,strlinelen(disbuf,dl,40)) ;
		debugprinthexblock("inter_cmdsubject:",n,disbuf,dl) ;
	}
#endif
	    } /* end if (non-null) */
	    if (rs >= 0) {
	        const int	ml = MIN(pip->linelen,dl) ;
	        disbuf[ml] = '\0' ;
	        rs = inter_info(iap,FALSE,"s> %s\v",disbuf) ;
	    }
	} /* end if (mbcache_msgscan) */

	} /* end if (ok) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (inter_cmdsubject) */


static int inter_msgoutfile(INTER *iap,cchar *cp,int cl,offset_t moff,int mlen)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("inter_msgoutfile: ent moff=%lld\n",moff) ;
	    debugprintf("inter_msgoutfile: mlen=%d\n",mlen) ;
	}
#endif

	if (iap->mfd < 0) rs = inter_mbviewopen(iap) ;

	if (rs >= 0) {
	    const char	*fmt ;
	    char	ofname[MAXPATHLEN + 1] ;
	    if (cp[0] != '/') {
	        const char	*ccp = iap->pathprefix ;
	        if (ccp == NULL) {
	            proginfo_pwd(pip) ;
	            ccp = pip->pwd ;
	        }
	        rs1 = mkpath2w(ofname,ccp,cp,cl) ;
	    } else
	        rs1 = mkpath1w(ofname,cp,cl) ;
	    if (rs1 >= 0) {
		const mode_t	om = 0666 ;
		const int	of = (O_WRONLY | O_CREAT | O_TRUNC) ;
	        if ((rs1 = uc_open(ofname,of,om)) >= 0) {
	            const int	ofd = rs1 ;
		    if (moff >= 0) {
	                if ((rs = u_seek(iap->mfd,moff,SEEK_SET)) >= 0) {
	                    if ((rs1 = uc_copy(iap->mfd,ofd,mlen)) >= 0) {
	                        fmt = "size=%u file=%t\v" ;
	                        rs = inter_info(iap,FALSE,fmt,rs1,cp,cl) ;
	                    } else {
	                        fmt = "file-copy-error (%d)\v" ;
	                        rs = inter_info(iap,TRUE,fmt,rs1) ;
		            }
	                } /* end if (copy) */
		    } /* end if (have data) */
	            u_close(ofd) ;
	        } else {
	            fmt = "inaccessible (%d) file=%s\v" ;
	            rs = inter_info(iap,TRUE,fmt,rs1,ofname) ;
	        }
	    } else
	        inter_info(iap,TRUE,"invalid file=%t\v",cp,cl) ;
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgoutfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_msgoutfile) */


static int inter_msgoutpipe(INTER *iap,cchar *cmd,offset_t outoff,int outlen)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rs2 ;

	if (cmd == NULL) return SR_NOANODE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgoutpipe: ent outoff=%lu outlen=%u\n",
	        outoff,outlen) ;
#endif

	if (iap->mfd < 0) {
	    rs = inter_mbviewopen(iap) ;
	}

	if (rs < 0)
	    goto ret0 ;

	rs = u_seek(iap->mfd,outoff,SEEK_SET) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("inter_msgoutpipe: shell=>%s<\n",pip->prog_shell) ;
	    debugprintf("inter_msgoutpipe: cmd=»%s«\n",cmd) ;
	}
#endif

	if ((rs = display_suspend(&iap->di)) >= 0) {

	    if ((rs = uterm_suspend(iap->utp)) >= 0) {
	        SPAWNPROC	ps ;
	        int		cs ;

	        memset(&ps,0,sizeof(SPAWNPROC)) ;
	        ps.opts |= SPAWNPROC_OSETPGRP ;
	        ps.disp[0] = SPAWNPROC_DCREATE ;
	        ps.disp[1] = SPAWNPROC_DINHERIT ;
	        ps.disp[2] = SPAWNPROC_DINHERIT ;
	        if (iap->f.ctty) {
	            ps.opts |= SPAWNPROC_OSETCTTY ;
	            ps.fd_ctty = pip->tfd ;
	            ps.pgrp = iap->pgrp ; /* activates setting PGRP */
	        }

	        if ((rs1 = spawncmdproc(&ps,pip->prog_shell,cmd)) >= 0) {
	            const pid_t	pid = rs1 ;
	            const int	ofd = ps.fd[0] ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("inter_msgoutpipe: "
				"spawncmdproc() rs=%d ofd=%d\n",rs1,ofd) ;
#endif

	            rs1 = uc_copy(iap->mfd,ofd,outlen) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("inter_msgoutpipe: uc_copy() rs=%d\n",
			rs1) ;
#endif

	            u_close(ofd) ;	/* spawned program gets EOF */
	            rs2 = u_waitpid(pid,&cs,0) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("inter_msgoutpipe: u_waitpid() rs=%d\n",
				rs2) ;
	                if (WIFEXITED(cs)) {
	                    debugprintf("inter_msgoutpipe: child ex=%u\n",
				WEXITSTATUS(cs)) ;
	                } else if (WIFSIGNALED(cs)) {
	                    debugprintf("inter_msgoutpipe: child sig=%u\n",
				WTERMSIG(cs)) ;
	                }
	            }
#endif /* CF_DEBUG */

	            if (iap->f.ctty) {
			int	ucmd = utermcmd_setpgrp ;
	                uterm_control(iap->utp,ucmd,iap->pgrp) ;
	            }
	        } /* end if (spawncmdproc) */

	        rs2 = uterm_resume(iap->utp) ;
	    } /* end if (uterm) */
	    display_resume(&iap->di) ;

	    if (rs >= 0) {
	        const int	rlen = RBUFLEN ;
	        const char	*ccp ;
	        char		rbuf[RBUFLEN + 1] ;
	        ccp = "resume> \v" ;
	        if ((rs = inter_response(iap,rbuf,rlen,ccp)) >= 0) {
	            if ((rs = inter_refresh(iap)) >= 0) {
	                if (rs1 >= 0) {
	                    rs = inter_info(iap,FALSE,"bytes=%u\v",rs1) ;
	                } else {
	                    if (rs1 == SR_PIPE) {
	                        ccp = "not all data transfered (%d)\v" ;
	                    } else {
	                        ccp = "file-copy-error (%d)\v" ;
			    }
	                    rs = inter_info(iap,TRUE,ccp,rs1) ;
			}
	            } /* end if (inter_refresh) */
	        } /* end if (inter_response) */
	    } /* end if (ok) */

	} /* end if (display suspension) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgoutpipe: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_msgoutpipe) */


static int inter_msgoutprog(INTER *iap,cchar *cmd,offset_t ooff,int olen)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;

	if (cmd == NULL) return SR_NOANODE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("inter_msgoutprog: cmd=%s\n",cmd) ;
	    debugprintf("inter_msgoutprog: ooff=%llu olen=%u\n",
	        ooff,olen) ;
	}
#endif

	if ((rs = inter_mbviewopen(iap)) >= 0) {
	    if ((rs = u_seek(iap->mfd,ooff,SEEK_SET)) >= 0) {

#if	CF_OPENPROG
	    const int	of = O_WRONLY ;
	    const char	**ev = pip->envv ;
	    const char	*av[2] ;
	    av[0] = cmd ;
	    av[1] = NULL ;
	    if ((rs = uc_openprog(cmd,of,av,ev)) >= 0) {
	        const int	ofd = rs ;
	        rs1 = uc_copy(iap->mfd,ofd,olen) ;
	        u_close(ofd) ;	/* spawned program gets EOF */
	    }
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_msgoutprog: openprog-out rs=%d\n",rs) ;
#endif
#else /* CF_OPENPROG */
		rs = inter_msgoutproger(iap,cmd,ooff,olen) ;
#endif /* CF_OPENPROG */

	    } /* end if (uc_seek) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgoutprog: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_msgoutprog) */


/* ARGSUSED */
static int inter_msgoutproger(INTER *iap,cchar *cmd,offset_t off,int olen)
{
	PROGINFO	*pip = iap->pip ;
	int		rs ;
	int		rs1 ;
	cchar		*tmpdname = pip->tmpdname ;
	cchar		*tfn = "vmailXXXXXXXXX" ;
	char		template[MAXPATHLEN+1] ;

	tmpdname = pip->tmpdname ;
	if ((rs = mkpath2(template,tmpdname,tfn)) >= 0) {
	    const mode_t	om = 0660 ;
	    const int		of = (O_CREAT|O_RDWR) ;
	    char		tbuf[MAXPATHLEN+1] ;
	    if ((rs = opentmpfile(template,of,om,tbuf)) >= 0) {
		int	fd = rs ;
		if ((rs = uc_copy(iap->mfd,fd,olen)) >= 0) {
		    if ((rs = u_rewind(fd)) >= 0) {
		SPAWNPROC	ps ;

	        memset(&ps,0,sizeof(SPAWNPROC)) ;
	        ps.opts |= SPAWNPROC_OSETPGRP ;
	        ps.nice = 2 ;
	        ps.fd[0] = fd ;
	        ps.disp[0] = SPAWNPROC_DDUP ;
	        ps.disp[1] = SPAWNPROC_DINHERIT ;
	        ps.disp[2] = SPAWNPROC_DINHERIT ;

#if	CF_TESTGO1
	    if ((rs1 = spawnproc(&ps,cmd,NULL,NULL)) >= 0) {
	        pid_t	pid = rs1 ;

#if	CF_MARKSPAMSYNC
		{
	    	    int	cs ;
	            rs2 = u_waitpid(pid,&cs,0) ;
		}
#else /* CF_MARKSPAMSYNC */
	        rs = inter_subproc(iap,pid) ;
#endif /* CF_MARKSPAMSYNC */

	    } /* end if (spawnproc) */
#endif /* CF_TESTGO1 */

		    } /* end if (u_rewind) */
		} /* end if (uc_copy) */
	        u_close(fd) ;
		uc_unlink(tbuf) ;
	    } /* end if (opentmpfile) */
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (inter_msgoutproger) */


static int inter_subproc(INTER *iap,pid_t pid)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

	if (! iap->f.subprocs) {
	    rs = subprocs_start(&iap->sp) ;
	    iap->f.subprocs = (rs >= 0) ;
	}
	if (rs >= 0) {
	    rs = subprocs_add(&iap->sp,pid) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_subproc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_subproc) */


static int inter_msgoutview(INTER *iap,cchar *cmd,cchar *vfname)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		rs2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_msgoutview: ent cmd=%s\n",cmd) ;
#endif

	if (cmd == NULL) return SR_NOANODE ;

/* to the viewing */

#ifdef	COMMENT
	if (rs >= 0) {
	    rs = display_allclear(&iap->di) ;
	}
#endif

	if (rs >= 0) {
	    if ((rs = display_suspend(&iap->di)) >= 0) {

	        if ((rs = uterm_suspend(iap->utp)) >= 0) {
	            SPAWNPROC	ps ;
		    const char	*cp ;
	            const char	*av[5] ; /* watch size of this */

	            sfbasename(cmd,-1,&cp) ;

	            av[0] = cp ;
	            av[1] = vfname ;
	            av[2] = NULL ;

	            memset(&ps,0,sizeof(SPAWNPROC)) ;
	            ps.opts |= SPAWNPROC_OSETPGRP ;
	            ps.disp[0] = SPAWNPROC_DINHERIT ;
	            ps.disp[1] = SPAWNPROC_DINHERIT ;
	            ps.disp[2] = SPAWNPROC_DINHERIT ;
	            if (iap->f.ctty) {
	                ps.opts |= SPAWNPROC_OSETCTTY ;
	                ps.fd_ctty = pip->tfd ;
	                ps.pgrp = iap->pgrp ;
	            }

	            if ((rs1 = spawnproc(&ps,cmd,av,NULL)) >= 0) {
	                const pid_t	pid = rs1 ;
	                int		cs ;

	                if ((rs2 = u_waitpid(pid,&cs,0)) >= 0) {
			    rs2 = cs ;
		 	}

	                if (iap->f.ctty) {
	    		    int	ucmd = utermcmd_setpgrp ;
	                    uterm_control(iap->utp,ucmd,iap->pgrp) ;
	                }
	            } /* end if (spawnproc) */

	            rs = uterm_resume(iap->utp) ;
	        } /* end block */
	        display_resume(&iap->di) ;

	        if (rs >= 0) {
	            if ((rs = inter_refresh(iap)) >= 0) {
			if (rs1 >= 0) {
	                    rs = inter_info(iap,FALSE,"\v") ;
			} else {
	                    rs = inter_info(iap,FALSE,"failed (%d)\v",rs1) ;
			}
		    }
		}

	    } /* end if (display suspension) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_msgoutview: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_msgoutview) */


static int inter_filecopy(INTER *iap,cchar *srcfname,cchar *dstfname)
{
	const int	of = O_RDONLY ;
	const mode_t	om = 0666 ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = uc_open(srcfname,of,om)) >= 0) {
	    const int	aof = (O_WRONLY | O_CREAT | O_TRUNC) ;
	    int		sfd = rs ;

	    if ((rs1 = uc_open(dstfname,aof,om)) >= 0) {
	        int	dfd = rs1 ;
	        rs1 = uc_copy(sfd,dfd,-1) ;
	        u_close(dfd) ;
	    } /* end if (dst-file) */

	    if ((rs >= 0) && (rs1 < 0))
	        rs = inter_info(iap,TRUE, "file-copy-error (%d)\v",rs1) ;

	    u_close(sfd) ;
	} /* end if (src-file) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (inter_filecopy) */


static int inter_msgappend(INTER *iap,int mi,cchar *mbname)
{
	PROGINFO	*pip = iap->pip ;
	offset_t	moff ;
	int		rs ;
	int		rs1 ;
	int		mlen ;
	char		mbfname[MAXPATHLEN + 1] ;

	if (mi < 0) return SR_NOANODE ;
	if (mbname == NULL) return SR_NOANODE ;
	if (mbname[0] == '\0') return SR_NOANODE ;

	if ((rs = mbcache_msgoff(&iap->mc,mi,&moff)) >= 0) {
	    mlen = rs ;
	    if (iap->mfd < 0) rs = inter_mbviewopen(iap) ;
	    if (rs >= 0) {
	        if ((rs = u_seek(iap->mfd,moff,SEEK_SET)) >= 0) {
	            const char	*folder = pip->folderdname ;
	            if ((rs = mkpath2(mbfname,folder,mbname)) >= 0) {
	                rs1 = mailboxappend(mbfname,iap->mfd,mlen) ;
	                if (rs1 < 0) {
	                    const char	*fmt = "file-copy-error (%d)\v" ;
	                    rs = inter_info(iap,TRUE,fmt,rs1) ;
	                }
	            } /* end if (mkpath) */
	        } /* end if (seek) */
	    } /* end if */
	} /* end if (mbcache-msgoff) */

	return rs ;
}
/* end subroutine (inter_msgappend) */


static int inter_msgdel(INTER *iap,int mi,int delcmd)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		f_delprev = FALSE ;

	if ((mi >= 0) && (mi < iap->nmsgs)) {
	    int		mark ;
	    if ((rs = mbcache_msgdel(&iap->mc,mi,delcmd)) >= 0) {
		f_delprev = (rs > 0) ;

	        if (delcmd >= 0) {

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

	            if (rs >= 0) {
		        if (delcmd && pip->f.nextdel && ((mi+1) < iap->nmsgs)) {
	                    rs = inter_msgpoint(iap,(mi+1)) ;
	                }
	            }

	        } /* end if (delcmd >= 0) */

	    } /* end if (mbcache_msgdel) */
	} /* end if (ok) */

	return (rs >= 0) ? f_delprev : rs ;
}
/* end subroutine (inter_msgdel) */


/* delete a number of messages rather than a message by-msg-number */
static int inter_msgdelnum(INTER *iap,int mi,int num,int delcmd)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgdelnum: mi=%u n=%d delcmd=%d\n",
	        mi,num,delcmd) ;
#endif

	if ((mi >= 0) && (mi < iap->nmsgs) && (delcmd >= 0)) {
	    int		mark ;
	    int		i ;
	    int		f_delprev = FALSE ;

	for (i = 0 ; (rs >= 0) && (i < num) ; i += 1) {
	    if (mi >= iap->nmsgs) break ;

	    rs = mbcache_msgdel(&iap->mc,mi,delcmd) ;
	    f_delprev = (rs > 0) ;
	    if (rs < 0) break ;

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
	    if (mi >= iap->nmsgs) mi = (iap->nmsgs - 1) ;
	    rs = inter_msgpoint(iap,mi) ;
	} /* end if */

	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_msgdelnum) */


static int inter_scancheck(INTER *iap,int si,int n)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_scancheck: si=%d n=%u\n",si,n) ;
#endif

	if (si < 0)
	    return SR_INVALID ;

	if (n > 0) {
	    MAILMSGFILE	*mlp = &iap->msgfiles ;
	    int		i ;
	    for (i = 0 ; (i < n) && (si < iap->nmsgs) ; i += 1) {
		int	f ;

	        rs = display_scancheck(&iap->di,si) ;
	        f = (rs > 0) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("inter_scancheck: "
	                "display_scancheck() rs=%d f=%u\n",
	                rs,f) ;
#endif

	        if ((rs >= 0) && (! f)) {
		    MBCACHE_SCAN	*msp ;
	            if ((rs = mbcache_msgscan(&iap->mc,si,&msp)) >= 0) {
			DISPLAY_SDATA	dsd ;
	                int		lines = msp->vlines ;

	                if (lines < 0) lines = msp->nlines ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("inter_scancheck: lines=%d\n",lines) ;
#endif

	                if (lines < 0) {
	                    MAILMSGFILE_MI	*mip ;
	                    const char *mid = msp->vs[mbcachemf_hdrmid] ;
	                    rs1 = mailmsgfile_msginfo(mlp,&mip,mid) ;
	                    if (rs1 >= 0) {
	                        lines = mip->vlines ;
	                        if (lines < 0)
	                            lines = mip->nlines ;
	                    }

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("inter_scancheck: "
				    "mailmsgfile_msginfo() "
	                            "lines=%d\n",lines) ;
#endif

	                } /* end if (lines) */

	                c += 1 ;
	                scandata_init(&dsd) ;
	                dsd.fbuf = msp->vs[mbcachemf_scanfrom] ;
	                dsd.flen = msp->vl[mbcachemf_scanfrom] ;
	                dsd.sbuf = msp->vs[mbcachemf_hdrsubject] ;
	                dsd.slen = msp->vl[mbcachemf_hdrsubject] ;
	                dsd.date = msp->vs[mbcachemf_scandate] ;
	                dsd.lines = lines ;
	                dsd.msgi = si ;
	                dsd.mark = (msp->f.del) ? iap->delmark : ' ' ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3)) {
	                    const char	*cp ;
	                    debugprintf("inter_scancheck: vs[from]=>%s<\n",
	                        msp->vs[mbcachemf_scanfrom]) ;
	                    cp = msp->vs[mbcachemf_hdrsubject] ;
	                    debugprintf("inter_scancheck: vs[subject]=>%t<\n",
	                        cp,strlinelen(cp,-1,45)) ;
	                    debugprintf("inter_scancheck: vs[date]=>%s<\n",
	                        msp->vs[mbcachemf_scandate]) ;
	                }
#endif /* CF_DEBUGS */

	                rs = display_scanload(&iap->di,si,&dsd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_scancheck: display_scanload() rs=%d\n",rs) ;
#endif

	            } /* end if (mbcache_msgscan) */
	        } /* end if (display needs scan-load) */

	        si += 1 ;
	        if (si >= iap->nmsgs) break ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (greater-than-zero) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_scancheck: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_scancheck) */


/* get input */
int inter_input(INTER *iap,char *lbuf,int llen,cchar *fmt,...)
{
	PROGINFO	*pip = iap->pip ;
	DISPLAY		*dip = &iap->di ;
	int		rs = SR_OK ;
	int		cmd ;
	int		len = 0 ;

	if (lbuf == NULL)
	    return SR_FAULT ;

	lbuf[0] = '\0' ;

/* output prompt (on the "input" line) and flush to device */

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    if ((rs = display_vinput(dip,fmt,ap)) >= 0) {
	        rs = display_flush(&iap->di) ;
	    }
	    va_end(ap) ;
	}

/* read input (in "cooked" mode) */

	if (rs >= 0) {
	    rs = uterm_read(iap->utp,lbuf,llen) ;
	    len = rs ;
	    if (rs >= 0) lbuf[rs] = '\0' ; /* for safety */
	}

	if (rs > 0) {
	    cmd = (lbuf[len-1] & 0xff) ;
	    if (iscmdstart(cmd)) {
	        len -= 1 ;
	        rs = inter_cmdinesc(iap,cmd) ;
	        cmd = rs ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("inter_input: uterm_read() rs=%d\n",rs) ;
	    debugprintf("inter_input: l=>%t<\n",
	        lbuf,strlinelen(lbuf,len,40)) ;
	}
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (inter_input) */


/* get input */
int inter_response(INTER *iap,char *lbuf,int llen,cchar *fmt,...)
{
	DISPLAY		*dip = &iap->di ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (lbuf == NULL) return SR_FAULT ;

	lbuf[0] = '\0' ;

/* output prompt (on the "input" line) and flush to device */

	{
	    va_list	ap ;
	    va_begin(ap,fmt) ;
	    if ((rs = display_vinput(dip,fmt,ap)) >= 0) {
	        rs = display_flush(&iap->di) ;
	    }
	    va_end(ap) ;
	}

/* read input (in "cooked" mode) */

	if (rs >= 0) {
	    if ((rs = uterm_read(iap->utp,lbuf,llen)) >= 0) {
	        const int	cmd = MKCHAR(lbuf[rl-1]) ;
	        rl = rs ;
	        lbuf[rs] = '\0' ;
	        if (iscmdstart(cmd)) {
	            rl -= 1 ;
	            rs = inter_cmdinesc(iap,cmd) ;
	        }
	        if ((rs >= 0) && (rl > 0)) {
		    int		cl ;
		    const char	*cp ;
	            if (lbuf[rl-1] == '\n') rl -= 1 ;
	            if ((cl = sfshrink(lbuf,rl,&cp)) > 0) {
	                if (cp != lbuf) {
	                    memmove(lbuf,cp,cl) ;
		        }
	            }
	            lbuf[cl] = '\0' ;
	            rl = cl ;
	        } /* end if (positive) */
	    } /* end if (uterm_read) */
	} /* end if (ok) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (inter_response) */


static int inter_change(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		rl ;
	int		m ;
	int		cl = 0 ;
	int		f_same = FALSE ;
	int		f_changed = FALSE ;
	const char	*ccp  ;
	const char	*cp ;
	char		response[LINEBUFLEN + 1] ;

	ccp = "change to mailbox: " ;
	rs = inter_input(iap,response,llen, ccp) ;
	rl = rs ;
	if ((rs >= 0) && (rl > 0) && (response[rl-1] == '\n')) rl -= 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_change: inter_input() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	if (rl > 0)
	    cl = sfshrink(response,rl,&cp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_change: mb=>%t<\n",cp,cl) ;
#endif

	if (cl <= 0)
	    goto ret0 ;

	response[(cp-response)+cl] = '\0' ; /* should be optional */

/* are we changing to the same mailbox? */

	if (iap->open.mbcache && (iap->mbname != NULL)) {
	    m = nleadstr(iap->mbname,cp,cl) ;
	    f_same = (iap->mbname[m] == '\0') && (m == cl) ;
	}

	if (f_same && (strcmp(iap->mbname,"new") == 0))
	    f_same = FALSE ;

/* does the new mailbox exist? */

	if (! f_same) {
	    if ((rs = inter_havemb(iap,cp,cl)) > 0) {
	        f_changed = TRUE ;
	        rs = inter_mailbeginer(iap,cp,cl) ;
	    } else if (rs == 0) {
	        char	disname[MAXNAMELEN + 1] ;
	        mkdisplayable(disname,MAXNAMELEN,cp,cl) ;
	        inter_info(iap,TRUE,
	            "inaccessible mb=%s\v",disname) ;
	    }
	} /* end if (not the same MBOX) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_change: ret rs=%d f_changed=%u\n",rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (inter_change) */


/* do we have the specified mailbox? */
static int inter_havemb(INTER *iap,cchar *mbname,int mblen)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	char		mbfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_havemb: mbname=>%t<\n",mbname,mblen) ;
#endif
	if ((mbname != NULL) && (mbname[0] != '\0')) {
	    if (pip->folderdname != NULL) {
	        const char	*fdn = pip->folderdname ;
	        if ((rs1 = mkpath2w(mbfname,fdn,mbname,mblen)) >= 0) {
	            struct ustat	sb ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("inter_havemb: mkpath2w() rs=%d mbf=%s\n",
	                    rs1,mbfname) ;
#endif
	            rs1 = u_stat(mbfname,&sb) ;
	            f = ((rs1 >= 0) && S_ISREG(sb.st_mode)) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("inter_havemb: u_stat() rs=%d\n",rs1) ;
#endif
	        } /* end if (mkpath) */
	    } /* end if (folder-directory-name) */
	} /* end if (have non-nul MBNAME) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_havemb: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_havemb) */


static int inter_mailempty(INTER *iap)
{
	int		rs = SR_OK ;
	int		f = ((! iap->open.mbcache) && (! iap->f.exit)) ;

	if (f) {

	    if ((rs >= 0) && iap->f.setmbname) {
	        char	mbname[3] ;
	        mbname[0] = '\0' ;
	        iap->f.setmbname = FALSE ;
	        rs = display_setmbname(&iap->di,mbname,0) ;
	    } /* end if */

	    if (rs >= 0)
	        rs = display_midmsgs(&iap->di,-1,-1) ;

	    if (rs >= 0)
	        rs = display_scanclear(&iap->di) ;

	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_mailempty) */


static int inter_cmdpathprefix(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	const int	rlen = LINEBUFLEN ;
	int		rs ;
	int		f_ok = TRUE ;
	const char	*ps = "change path-prefix: " ;
	char		rbuf[LINEBUFLEN + 1] ;

	if ((rs = inter_input(iap,rbuf,rlen,ps)) >= 0) {
	    int	rl = rs ;
	    if ((rl > 0) && (rbuf[rl-1] == '\n')) rl -= 1 ;
	    rbuf[rl] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("inter_cmdpathprefix: inter_input() rs=%d\n",rs) ;
	        debugprintf("inter_cmdpathprefix: r=>%t<\n",rbuf,rl) ;
	    }
#endif

	    if (rl > 0) {
	        int		cl = 0 ;
	        const char	*cp ;
	        const char	*px = iap->pathprefix ;
	        if ((cl = sfshrink(rbuf,rl,&cp)) > 0) {
		    char	prefix[MAXNAMELEN + 1] ;
	            if ((rs = mkpath1w(prefix,cp,cl)) >= 0) {
	                if ((rs = inter_prefixer(iap,prefix)) >= 0) {
	                    px = iap->pathprefix ;
	                    f_ok = (rs > 0) ;
	                } /* end if (inter-pathprefix) */
	            } /* end if (mkpath) */
	        } /* end if (non-zero string) */
	        if (f_ok) {
	            if (px != NULL) {
	                rs = inter_info(iap,FALSE,"dir=%s\v",px) ;
		    }
	        } else {
	            cchar	*fmt = "inaccessible dir=%t\v" ;
	            rs = inter_info(iap,TRUE,fmt,cp,cl) ;
	        } /* end if */
	    } /* end if (positive response) */
	} /* end if (inter_input) */

	return (rs >= 0) ? f_ok : rs ;
}
/* end subroutine (inter_cmdpathprefix) */


static int inter_cmdshell(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	const int	rlen = RBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		rs2 ;
	int		rs3 ;
	cchar		*ccp = "cmd: " ;
	char		rbuf[RBUFLEN + 1] ;

	if ((rs = inter_response(iap,rbuf,rlen,ccp)) > 0) {
	    if ((rs = display_suspend(&iap->di)) >= 0) {
		rs2 = SR_OK ;
		rs3 = SR_OK ;

	        if ((rs = uterm_suspend(iap->utp)) >= 0) {
	            const char	*cmdshell = pip->prog_shell ;
	            SPAWNPROC	ps ;

	            memset(&ps,0,sizeof(SPAWNPROC)) ;
	            ps.opts |= SPAWNPROC_OSETPGRP ;
	            ps.disp[0] = SPAWNPROC_DINHERIT ;
	            ps.disp[1] = SPAWNPROC_DINHERIT ;
	            ps.disp[2] = SPAWNPROC_DINHERIT ;
	            if (iap->f.ctty) {
	                ps.opts |= SPAWNPROC_OSETCTTY ;
	                ps.fd_ctty = pip->tfd ;
	                ps.pgrp = iap->pgrp ;
	            }

	            if ((rs2 = spawncmdproc(&ps,cmdshell,rbuf)) >= 0) {
	                const pid_t	pid = rs2 ;
	            	int		cs ;

	                rs3 = u_waitpid(pid,&cs,0) ;

	                if (iap->f.ctty) {
	    		    int	ucmd = utermcmd_setpgrp ;
	                    uterm_control(iap->utp,ucmd,iap->pgrp) ;
	                }
	            } /* end if (spawncmdproc) */

	            rs1 = uterm_resume(iap->utp) ;
	        } /* end if (uterm) */
	        display_resume(&iap->di) ;

	        if (rs >= 0) {
	            ccp = "resume> \v" ;
	            if ((rs = inter_response(iap,rbuf,rlen,ccp)) >= 0) {
	                if ((rs = inter_refresh(iap)) >= 0) {
			    if (rs1 >= 0) rs1 = rs2 ;
			    if (rs1 >= 0) rs1 = rs3 ;
			    if (rs1 < 0) {
	            	        cchar	*fmt = "command error (%d)\v" ;
	            		rs = inter_info(iap,TRUE,fmt,rs1) ;
			    }
			}
		    }
	        } /* end if */

	    } /* end if (display suspension) */
	} /* end if (positive) */

	return rs ;
}
/* end subroutine (inter_cmdshell) */


static int inter_viewtop(INTER *iap,int vln)
{
	int		rs = SR_OK ;

	if (vln < 1) vln = 1 ;

/* get out if no Mail-Cache (MC) */

	if (iap->open.mbcache) {

/* free up Message-View (MV) if called for */

	if (iap->open.mv) {
	    int	f = iap->f.viewchange ;
	    f = f || (iap->miscanpoint != iap->miviewpoint) ;
	    if (f) {
	        iap->f.viewchange = FALSE ;
	        rs = inter_msgviewclose(iap) ;
	    }
	}

/* open a MSGVIEW if not already open */

	if ((rs >= 0) && (! iap->open.mv)) {
	    rs = inter_msgviewopen(iap) ;
	} /* end if */

	if (rs >= 0) {
	    rs = inter_msgviewtop(iap,(vln-1)) ;
	    iap->nviewlines = rs ;
	} /* end if */

	} /* end if (mbcache) */

	return rs ;
}
/* end subroutine (inter_viewtop) */


static int inter_viewnext(INTER *iap,int inc)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		f ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("inter_viewnext: open.mbcache=%u inc=%d\n",
		iap->open.mbcache,inc) ;
#endif

/* get out if no Mail-Cache (MC) */

	if (! iap->open.mbcache)
	    goto ret0 ;

/* free up Message-View (MV) if called for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("inter_viewnext: f_mvinit=%u\n",iap->open.mv) ;
	    debugprintf("inter_viewnext: f_viewchange=%u\n",iap->f.viewchange) ;
	    debugprintf("inter_viewnext: miscanpoint=%d miviewpoint=%d\n",
	        iap->miscanpoint,iap->miviewpoint) ;
	}
#endif

	if ((rs >= 0) && iap->open.mv) {
	    f = iap->f.viewchange ;
	    f = f || (iap->miscanpoint != iap->miviewpoint) ;
	    if (f) {
	        iap->f.viewchange = FALSE ;
	        rs = inter_msgviewclose(iap) ;
	    }
	}

/* open a MSGVIEW if not already open */

	if ((rs >= 0) && (! iap->open.mv)) {
	    inc = 0 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("inter_viewnext: _msgviewopen()\n") ;
#endif

	    rs = inter_msgviewopen(iap) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("inter_viewnext: _msgviewopen() rs=%d\n",rs) ;
	        debugprintf("inter_viewnext: f_mvinit=%u\n",iap->open.mv) ;
	    }
#endif

	} /* end if */

	if ((rs >= 0) && iap->open.mv) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("inter_viewnext: _msgviewnext() \n") ;
#endif

	    rs = inter_msgviewnext(iap,inc) ;
	    iap->nviewlines = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("inter_viewnext: _msgviewnext() rs=%d\n",rs) ;
#endif

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("inter_viewnext: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_viewnext) */


static int inter_msgviewopen(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	MBCACHE_SCAN	*msp ;
	const int	mi = iap->miscanpoint ;
	int		rs = SR_OK ;
	int		nlines = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("inter_msgviewopen: f_mvinit=%u\n",iap->open.mv) ;
	    debugprintf("inter_msgviewopen: miscanpoint=%d\n",
		iap->miscanpoint) ;
	    debugprintf("inter_msgviewopen: mvname=%s\n",iap->mbname) ;
	}
#endif

	iap->f.viewchange = FALSE ;
	if ((! iap->open.mv) && (iap->miscanpoint >= 0)) {
	    if (iap->mbname != NULL) {
	        iap->miviewpoint = mi ;
	        iap->lnviewtop = -1 ;
	        if ((rs = mbcache_msgscan(&iap->mc,mi,&msp)) >= 0) {
	            if (msp->boff >= 0) {
		        rs = inter_msgviewopener(iap,mi,msp) ;
			nlines = rs ;
	            }
	        } /* end if (mbcache_msgscan) */
	    } else
	        rs = SR_NOANODE ;
	} else
	    rs = SR_NOANODE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_msgviewopen: ret rs=%d nl=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (inter_msgviewopen) */


static int inter_msgviewopener(INTER *iap,int mi,MBCACHE_SCAN *msp)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		nl = 0 ;
	const char	*midp = msp->vs[mbcachemf_hdrmid] ;
	char		msgid[MAILADDRLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("inter_msgviewopener: ent mid=>%s<\n", midp) ;
	    debugprintf("inter_msgviewopener: mbname=>%s<\n",iap->mbname) ;
	    debugprintf("inter_msgviewopener: misscanpoint=%d\n",
	        iap->miscanpoint) ;
	}
#endif

	if ((midp == NULL) || (midp[0] == '\0')) {
	    midp = msgid ;
	    rs = snsdd(msgid,MAILADDRLEN,iap->mbname,iap->miscanpoint) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_msgviewopen: mid=>%s<\n",midp) ;
#endif

	if (rs >= 0) {
	    cchar	*mfp = NULL ;
	    if ((rs = inter_msgviewfile(iap,msp,midp,&mfp)) >= 0) {
	        const int	f = TRUE ;
	        nl = rs ;
	        if ((rs = inter_setscanlines(iap,mi,msp,nl,f)) >= 0) {
	            if (mfp != NULL) {
	                if ((rs = mailmsgviewer_open(&iap->mv,mfp)) >= 0) {
	                    DISPLAY_BOTINFO	bi ;
		            const int	lmsgfrom = DISPLAY_LMSGFROM ;
	                    const char	*mfrom = msp->vs[mbcachemf_scanfrom] ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(5)) {
	                    debugprintf("inter_msgviewopen: nl=%d\n",nl) ;
	                    debugprintf("inter_msgviewopen: mfrom=%s\n",
				mfrom) ;
	                }
#endif

	                iap->open.mv = TRUE ;
	                memset(&bi,0,sizeof(DISPLAY_BOTINFO)) ;
	                if (mfrom != NULL) strwcpy(bi.msgfrom,mfrom,lmsgfrom) ;
	                bi.msgnum = (mi+1) ;
	                bi.msglines = nl ;
	                bi.msgline = 1 ;
	                rs = display_botinfo(&iap->di,&bi) ;
		        if (rs < 0) {
	        	    iap->open.mv = FALSE ;
	        	    mailmsgviewer_close(&iap->mv) ;
		        }
	            } /* end if (mailmsgviewer_open) */
	        } /* end if (non-null mail-file-name) */
	    } /* end if (inter_setscanlines) */
	} /* end if (inter_msgviewfile) */
	if (rs < 0) {
	    if (iap->mfd >= 0) {
	        inter_mbviewclose(iap) ;
	    }
	} /* end if (error) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("inter_msgviewopener: ret rs=%d nl=%u\n",rs,nl) ;
#endif

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (inter_msgviewopener) */


static int inter_msgviewclose(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("inter_msgviewclose: ent\n") ;
#endif

	if (iap->open.mv) {
	    iap->lnviewtop = -1 ;
	    iap->miviewpoint = -1 ;
	    if (iap->open.mv) {
	        iap->open.mv = FALSE ;
	        rs = mailmsgviewer_close(&iap->mv) ;
	    }
	} else
	    rs = SR_NOANODE ;

	return rs ;
}
/* end subroutine (inter_msgviewclose) */


static int inter_msgviewsetlines(INTER *iap,int nlines)
{
	PROGINFO	*pip = iap->pip ;
	const int	mi = iap->miviewpoint ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (pip == NULL) return SR_FAULT ;

	if (mi >= 0) {
	    int	vlines ;
	    if ((rs = mbcache_msglines(&iap->mc,mi,&vlines)) >= 0) {
	        if (vlines < 0) {
	            f = TRUE ;
	            if ((rs = mbcache_msgsetlines(&iap->mc,mi,nlines)) >= 0) {
	                rs = display_scanloadlines(&iap->di,mi,nlines,TRUE) ;
		    }
	        } /* end if (setting lines) */
	    } /* end if (mbcache_msglines) */
	} else
	    rs = SR_INVALID ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (inter_msgviewsetlines) */


static int inter_msgviewtop(INTER *iap,int lntop)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("inter_msgviewtop: ent lnviewtop=%d lntop=%d\n",
	        iap->lnviewtop,lntop) ;
#endif

	if (lntop < 0)
	    return SR_INVALID ;

	if (! iap->open.mv) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	if (iap->lnviewtop < 0) {
	    lntop = 0 ;
	    iap->lnviewtop = 0 ;
	    rs = inter_msgviewrefresh(iap) ;
	    c = rs ;
	} else {
	    rs = inter_msgviewadj(iap,lntop) ;
	    c = rs ;
	} /* end if */

	if ((rs >= 0) && (c > 0))
	    rs = display_botline(&iap->di,(lntop+1)) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgviewtop: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_msgviewtop) */


static int inter_msgviewadj(INTER *iap,int lntop)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		mi ;
	int		lines ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("inter_msgviewadj: ent lnviewtop=%d lntop=%d\n",
	        iap->lnviewtop,lntop) ;
#endif

	if (lntop < 0)
	    return SR_INVALID ;

	if (iap->lnviewtop < 0)
	    return SR_NOANODE ;

	if (! iap->open.mv) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	mi = iap->miviewpoint ;
	if ((rs = mbcache_msglines(&iap->mc,mi,&lines)) >= 0) {
	    int	vi = 0 ;
	    int	ln = 0 ;
	    int	ndiff, nadiff ;
	    int	n ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("inter_msgviewadj: "
	        "mbcache_msglines() mi=%u lines=%d\n",mi,lines) ;
	}
#endif

	if (lines >= 0) {
	    int	f ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("inter_msgviewadj: viewlines=%u lines=%u\n",
	            iap->viewlines,lines) ;
	        debugprintf("inter_msgviewadj: viewtop=%u lntop=%u\n",
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
	    debugprintf("inter_msgviewadj: ndiff=%d\n",ndiff) ;
#endif

	if (nadiff >= iap->viewlines) {

	    iap->lnviewtop = lntop ;
	    rs = inter_msgviewrefresh(iap) ;
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
	        if ((rs = display_viewscroll(&iap->di,ndiff)) >= 0) {
	            rs = inter_msgviewload(iap,vi,nadiff,ln) ;
	            n = rs ;
	            c = (iap->viewlines - nadiff + n) ;
	        }
	    }

	    if (rs >= 0)
	        iap->lnviewtop = lntop ;

	} /* end if */

	} /* end if (mbcache_msglines) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgviewadj: ret rs=%d viewtop=%u c=%u\n",
	        rs,iap->lnviewtop,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_msgviewadj) */


static int inter_msgviewnext(INTER *iap,int inc)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgviewnext: inc=%d\n",inc) ;
#endif

	if (iap->open.mv) {
	    const int	lntop = MAX((iap->lnviewtop + inc),0) ;
	    rs = inter_msgviewtop(iap,lntop) ;
	    c = rs ;
	} /* end if (msg-viewer open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgviewnext: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_msgviewnext) */


static int inter_msgviewrefresh(INTER *iap)
{
	int		rs ;

	rs = inter_msgviewload(iap,0,iap->viewlines,iap->lnviewtop) ;

	return rs ;
}
/* end subroutine (inter_msgviewrefresh) */


static int inter_msgviewload(INTER *iap,int vi,int vn,int ln)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		ll = -1 ;
	int		i = 0 ;
	int		c = 0 ;
	const char	*lp = NULL ;
	char		dum[2] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("inter_msgviewload: vi=%u ln=%u\n",vi,ln) ;
#endif

	if ((vi < 0) || (ln < 0))
	    return SR_INVALID ;

	if (! iap->open.mv) {
	    rs = SR_NOANODE ;
	    goto ret0 ;
	}

	if (vn > iap->viewlines)
	    rs = SR_INVALID ;

/* specified number of message lines (as we may have) */

	while ((rs >= 0) && (i < vn) && (vi < iap->viewlines)) {
	    ll = mailmsgviewer_getline(&iap->mv,ln,&lp) ;
	    if (ll <= 0) break ;

	    rs = display_viewload(&iap->di,vi,lp,ll) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("inter_msgviewload: display_viewload() rs=%d\n",
	            rs) ;
#endif

	    i += 1 ;
	    c += 1 ;
	    ln += 1 ;
	    vi += 1 ;

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgviewload: while-end rs=%d\n",
	        rs) ;
#endif

/* any needed blank lines */

	dum[0] = '\0' ;
	while ((rs >= 0) && (i < vn) && (vi < iap->viewlines)) {
	    rs = display_viewload(&iap->di,vi,dum,0) ;
	    i += 1 ;
	    vi += 1 ;
	} /* end while */

/* extra */

	if ((rs >= 0) && (ll == 0)) {
	    rs = inter_msgviewsetlines(iap,ln) ;
	}

/* done */
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("inter_msgviewload: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (inter_msgviewload) */


static int inter_mbviewopen(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;
	int		fd = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("inter_mbviewopen: ent mfd=%d\n",iap->mfd) ;
#endif

	if (iap->mfd < 0) {
	    if (iap->mbname != NULL) {
		const mode_t	om = 0666 ;
		const int	of = O_RDONLY ;
	        const char	*folder = pip->folderdname ;
	        char		mbfname[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(mbfname,folder,iap->mbname)) >= 0) {
	            rs = uc_open(mbfname,of,om) ;
	            iap->mfd = rs ;
	            fd = rs ;
	        }
	    } else
	        rs = SR_NOANODE ;
	} else
	    fd = iap->mfd ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_mbviewopen: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (inter_mbviewopen) */


static int inter_mbviewclose(INTER *iap)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iap->mfd >= 0) {
	    rs1 = u_close(iap->mfd) ;
	    if (rs >= 0) rs = rs1 ;
	    iap->mfd = -1 ;
	}

	return rs ;
}
/* end subroutine (inter_mbviewclose) */


#if	CF_SUSPEND
static int inter_suspend(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	sigset_t	nsm, osm ;
	const int	sig = SIGTSTP ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_suspend: ent\n") ;
#endif

	uc_sigsetempty(&nsm) ;
	if ((rs = uc_sigsetadd(&nsm,sig)) >= 0) {
	    if ((rs = pt_sigmask(SIG_UNBLOCK,&nsm,&osm)) >= 0) {
	        struct sigaction	sao, san ;

	        uc_sigsetfill(&nsm) ;
	        memset(&san,0,sizeof(struct sigaction)) ;
	        san.sa_handler = SIG_DFL ;
	        san.sa_mask = nsm ;
	        san.sa_flags = 0 ;

	        if ((rs = u_sigaction(sig,&san,&sao)) >= 0) {
	            rs = uc_raise(SIGTSTP) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("inter_suspend: uc_raise() rs=%d\n",rs) ;
#endif

#if	CF_SIGPENDING
	            msleep(10) ;
	            inter_pending(iap) ;
#endif

	            u_sigaction(sig,&sao,NULL) ;
	        } /* end if (sigaction) */

	        pt_sigmask(SIG_SETMASK,&osm,NULL) ;
	    } /* end if (sigmask) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("inter_suspend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (inter_suspend) */
#endif /* CF_SUSPEND */


#if	CF_SIGPENDING
static int inter_pending(INTER *iap)
{
	PROGINFO	*pip = iap->pip ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    sigset_t	psm ;
	    debugprintf("inter_pending: ¬\n") ;
	    uc_sigsetempty(&psm) ;
	    if ((rs = u_sigpending(&psm)) >= 0) {
	        int	i ;
	        for (i = 0 ; i < SIGRTMIN ; i += 1) {
	            if ((rs = uc_sigsetismem(&psm,i)) > 0) {
	                debugprintf("inter_pending: sig=%d\n",i) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	        debugprintf("inter_pending: for-out rs=%d i=%d\n",rs,i) ;
	    } /* end if (sigpending) */
	}
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (inter_pending) */
#endif /* CF_SIGPENDING */


