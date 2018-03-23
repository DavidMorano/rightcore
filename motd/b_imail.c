/* b_imail */

/* SHELL built-in to return load averages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGOFF	0		/* debug file offsets */
#define	CF_DEBUGDEF	0		/* debug default (level==5) */
#define	CC_DEBUGSRCH	0		/* debug searching */
#define	CF_DEBUGN	0		/* special */
#define	CF_MSGENV	0		/* message environment */
#define	CF_TESTIN	0		/* test std-input */
#define	CF_OUTENTRY	1		/* use 'outema_ent(3imail)' */
#define	CF_XMAILER	0		/* compliled-in x-mailer */
#define	CF_PRTMPDIR	1		/* use a PR tmp-dir */
#define	CF_CSPATH	1		/* use CSPATH */
#define	CF_MSGOPTSNONE	0		/* |msgopts_none()| */
#define	CF_PROCSAVE	1		/* |procsave()| */
#define	CF_PROCUSERBOX	1		/* |procuserbox()| */
#define	CF_PROCPREPARE	1		/* |procprepare()| */
#define	CF_PROCMSGSTAGE	1		/* |procmailmsgstage¤| */
#define	CF_PROCARGS	1		/* |procargs()| */
#define	CF_PROGUSERLIST	1		/* |proguserlist¤| */
#define	CF_PROGLOGER	1		/* |progloger¤| */
#define	CF_PROCOURCONF	1		/* |procourconf¤| */
#define	CF_PROCEXTID	0		/* |procextid()| */
#define	CF_VSENDERS	0		/* alternative version */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Whew!  This was bigger (much bigger) than I expected.  This
	program (command) reads an RFC-822 mail message (actually an UNIX®
	mailbox-formatted set of mail messages) on STDIN and (as specified)
	generally:
		1. deliver it to a MTA
		2. saves a copy to the caller's "copy" mailbox

	Basic mail-message parsing is done with the MAILMSGSTAGE object.  That
	object is optimized to efficiently allow for repeated access to the
	individual component messages of the input mailbox data.

	This command is a huge, self contained, high-feature rich mail
	processing engine.  Google mail processing is probably not nearly this
	careful and fast!

	Synopsis:

	$ imail <recip(s)> [-t[=<b>]]

	Implementation note:

	Choices for what code sequences should be subroutines (seperate from
	other subroutines) were made so as to achieve the maximum
	code-reusability.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/wait.h>		/* for 'WIFEXITED(3c)' */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>
#include	<stdlib.h>
#include	<string.h>
#include	<utmpx.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<estrings.h>
#include	<ctdec.h>
#include	<fsdir.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<osetstr.h>
#include	<field.h>
#include	<paramfile.h>
#include	<expcook.h>
#include	<mailmsghdrfold.h>
#include	<mailmsghdrs.h>
#include	<ema.h>
#include	<filebuf.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<outstore.h>
#include	<nulstr.h>
#include	<ascii.h>
#include	<spawnproc.h>
#include	<dater.h>
#include	<tmtime.h>
#include	<upt.h>
#include	<grmems.h>
#include	<sysrealname.h>
#include	<sysusernames.h>
#include	<linefold.h>
#include	<mkuuid.h>
#include	<pcsns.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_imail.h"
#include	"defs.h"
#include	"proglog.h"
#include	"mailmsgstage.h"
#include	"outema.h"


/* local defines */

#define	MSGOPTS		struct msgopts

#define	MSGDATA		struct msgdata
#define	MSGDATA_FL	struct msgdata_flags

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 2),2048)
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	ZNAMELEN
#define	ZNAMELEN	8
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	8
#endif

#ifndef	BUFLEN
#define	BUFLEN		(8 * 1024)
#endif

#ifndef	PATHBUFLEN
#define	PATHBUFLEN	(8 * MAXPATHLEN)
#endif

#ifndef	PBUFLEN
#define	PBUFLEN		(6 * MAXPATHLEN)
#endif

#undef	JOBCLEN
#define	JOBCLEN		14

#define	CVTBUFLEN	100

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif

#ifndef	MSGCOLS
#define	MSGCOLS		76		/* message-columns (RFC-?) */
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8		/* number of columns for a TAB */
#endif

#undef	DMODE
#define	DMODE		0777

#ifndef	TO_TMPFILES
#define	TO_TMPFILES	(1*3600)	/* temporary file time-out */
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#define	LOCINFO_GMCUR	struct locinfo_gmcur
#define	LOCINFO_RNCUR	struct locinfo_rncur

#define	CONFIG		struct config
#define	CONFIG_FL	struct config_flags

#define	MAILFILE	struct mailfile
#define	MAILFILE_FL	struct mailfile_flags

#define	NDF		"imail.ndeb"


/* local types */

typedef	int	(*vrecipsch_t)(const void **,const void **) ;


/* external subroutines */

extern int	snmkuuid(char *,int,MKUUID *) ;
extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	nleadstr(cchar *,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	pathclean(char *,cchar *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	mailboxappend(cchar *,int,int,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	rmdirfiles(cchar *,cchar *,int) ;
extern int	findxfile(IDS *,char *,cchar *) ;
extern int	getprogpath(IDS *,vecstr *,char *,cchar *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;
extern int	mkplogid(char *,int,cchar *,int) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	mkbestaddr(char *,int,cchar *,int) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	mktmpuserdir(char *,cchar *,cchar *,mode_t) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	prsetfname(cchar *,char *,cchar *,int,int,
			cchar *,cchar *,cchar *) ;
extern int	localgetorg(cchar *,char *,int,cchar *) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	hdrextid(char *,int,cchar *,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	vecstr_addcspath(vecstr *) ;
extern int	ema_first(EMA *,cchar **) ;
extern int	filebuf_writehdrkey(FILEBUF *,cchar *) ;
extern int	filebuf_printcont(FILEBUF *,int,cchar *,int) ;
extern int	stremacmp(cchar *,cchar *) ;
extern int	ncolstr(int,int,cchar *,int) ;
extern int	hasalluc(cchar *,int) ;
extern int	hasuc(cchar *,int) ;
extern int	isfnamespecial(cchar *,int) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;
extern int	hasMeAlone(cchar *,int) ;
extern int	isOneOf(const int *,int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isNotValid(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;
extern int	proginfo_rootname(PROGINFO *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	progdebugout(PROGINFO *,cchar *,cchar *) ;
extern int	proglogout(PROGINFO *,cchar *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strwset(char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures (and some variables) */

struct msgopts {
	uint		mkenv:1 ;
	uint		mkclen:1 ;
	uint		mkclines:1 ;
	uint		mkfrom:1 ;
	uint		mkretpath:1 ;
	uint		mkbcc:1 ;
	uint		mkcrnl:1 ;
	uint		mksubj:1 ;
	uint		mkxuuid:1 ;	/* xUUID */
	uint		mkxnote:1 ;	/* Apple-Note */
	uint		mkxmcdate:1 ;	/* created-date (yes, what a crock!) */
	uint		reqsubj:1 ;
	uint		nosubj:1 ;
	uint		norecip:1 ;
} ;

struct mailfile_flags {
	uint		tmpfile:1 ;
} ;

struct mailfile {
	PROGINFO	*pip ;
	cchar		*mailfname ;
	MAILFILE_FL	f ;
	SHIO		infile ;
	int		mfd ;
} ;

struct msgenvelem {
	cchar		*ep ;
	int		el ;
} ;

struct msgenv {
	struct msgenvelem	a, d, r ;
} ;

/* the following -> three <- data-sets should be order-synchronized */

/* name-strings for logging purposes */
static cchar	*msgloghdrs[] = {
	"mid",
	"from",
	"sender",
	"returnpath",
	"replyto",
	"errorsto",
	"to",
	"cc",
	"bcc",
	"inreplyto",
	"xforwardedto",
	NULL
} ;

enum msgloghdrs {
	msgloghdr_messageid,
	msgloghdr_from,
	msgloghdr_sender,
	msgloghdr_returnpath,
	msgloghdr_replyto,
	msgloghdr_errorsto,
	msgloghdr_to,
	msgloghdr_cc,
	msgloghdr_bcc,
	msgloghdr_inreplyto,
	msgloghdr_xforwardedto,
	msgloghdr_overlast
} ;

/* indexes for getting header-key names */
static const int	msgloghi[] = {
	HI_MESSAGEID,
	HI_FROM,
	HI_SENDER,
	HI_RETURNPATH,
	HI_ERRORSTO,
	HI_REPLYTO,
	HI_TO,
	HI_CC,
	HI_BCC,
	HI_INREPLYTO,
	HI_XFORWARDEDTO,
	-1
} ;

struct msgdata_flags {
	uint		plaintext:1 ;
	uint		subject:1 ;	/* has a subject */
	uint		senders:1 ;	/* 'senders' initialized */
	uint		disallowsender:1 ;	/* do not add SENDER */
} ;

struct msgdata {
	cchar		*env ;
	cchar		*envfrom ;
	cchar		*envdate ;
	cchar		*retpath ;
	cchar		*hdrmid ;
	MSGDATA_FL	f ;
	BITS		senders ;
	EMA		addrs[msgloghdr_overlast] ;
	int		hlen ;		/* header-length */
	int		mn ;		/* message number */
	int		nrecips ;	/* final number of recipients */
	int		nvsenders ;
	int		naddrs[msgloghdr_overlast] ;
} ;

struct locinfo_gmcur {
	GRMEMS_CUR	gmcur ;
} ;

struct locinfo_rncur {
	SYSREALNAME_CUR	rncur ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		init:1 ;
	uint		nocache:1 ;
	uint		take:1 ;	/* "take" recipient addresses */
	uint		clustername:1 ;
	uint		postmail:1 ;
	uint		sendmail:1 ;
	uint		folder:1 ;
	uint		cmbname:1 ;	/* "copy" mailbox name */
	uint		xmailer:1 ;
	uint		useclen:1 ;
	uint		useclines:1 ;
	uint		to:1 ;		/* time-out */
	uint		mailhost:1 ;
	uint		deliver:1 ;
	uint		org:1 ;		/* add our organization */
	uint		sender:1 ;	/* SENDER address */
	uint		addenv:1 ;	/* add mail-envelope */
	uint		addsender:1 ;	/* add us as sender */
	uint		addfrom:1 ;	/* add any given FROMs */
	uint		addsubj:1 ;	/* add subject (if we have one) */
	uint		addxuuid:1 ;	/* add a UUID */
	uint		addnote:1 ;	/* add Apple-Note */
	uint		addxmcdate:1 ;	/* add Mail-Created-Date */
	uint		onesender :1 ;	/* allow only one "sender" header */
	uint		onefrom:1 ;	/* allow only one "from" header*/
	uint		envfrom:1 ;
	uint		hdrmid:1 ;
	uint		hdrsubject:1 ;
	uint		hdrdate:1 ;
	uint		hdrclen:1 ;
	uint		hdrclines:1 ;
	uint		hdrfroms:1 ;
	uint		hdruuid:1 ;
	uint		uuid:1 ;
	uint		def_from:1 ;
	uint		epath:1 ;
	uint		usingsendmail:1 ;
	uint		dater:1 ;
	uint		tmpuserdir:1 ;
	uint		msgpriority:1 ;
	uint		reqsubj:1 ;
	uint		notsubj:1 ;
	uint		gm:1 ;
	uint		rn:1 ;
	uint		tmpmaint:1 ;
	uint		maint:1 ;
	uint		all:1 ;
	uint		allok:1 ;
	uint		ns:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	PROGINFO	*pip ;
	cchar		*clustername ;
	cchar		*jobdname ;	/* job-tmpdname */
	cchar		*folder ;
	cchar		*cmbname ;	/* mailbox-name (for copy) */
	cchar		*postmail ;	/* MTA program */
	cchar		*sendmail ;
	cchar		*folderdname ;
	cchar		*mailprogfname ; /* path for MTA program */
	cchar		*xmailer ;	/* our mailer name ("i-mail") */
	cchar		*mailhost ;
	cchar		*hdraddr_sender ;
	cchar		*hdrname_from ;
	cchar		*hdraddr_from ;
	cchar		*hdraddr_replyto ;
	cchar		*hdraddr_errorsto ;
	cchar		*env ;		/* new addition */
	cchar		*envfrom ;	/* new addition */
	cchar		*envdate ;	/* new addition */
	cchar		*hdrdate ;	/* new addition */
	cchar		*hdrsubject ;	/* new addition */
	cchar		*hdrpriority ;	/* new addition */
	cchar		*hdruuid ;	/* new addition */
	cchar		*pr_pcs ;	/* PCS program-root */
	MSGDATA		*md ;
	vecstr		stores ;	/* allocation maintenance */
	vecstr		epath ;		/* execution path (broken out) */
	osetstr		recips ;	/* recipient addresses */
	EMA		hdrfroms ;	/* FROM addresses */
	MAILMSGSTAGE	ms ;
	DATER		dc ;		/* date converter */
	PARAMOPT	hdradds ;
	GRMEMS		gm ;
	SYSREALNAME	rn ;
	MKUUID		uuid ;
	PCSNS		ns ;
	pthread_t	tid ;
	time_t		msgtime ;
	uid_t		uid_pr ;
	gid_t		gid_pr ;
	int		pagesize ;
	int		msgcols ;
	int		to ;
	int		msgpriority ;
	int		nmsgs ;
	int		tmpsize ;	/* ¤unused¤ */
	int		kserial ;
	int		serial ;	/* for MSGIDs */
	int		maxpostargs ;
	char		gnbuf[GROUPNAMELEN+1] ;
} ;

struct config_flags {
	uint		p:1 ;
} ;

struct config {
	PROGINFO	*pip ;
	PARAMFILE	params ;
	EXPCOOK		cooks ;
	CONFIG_FL	f ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;
static int	procmklogid(PROGINFO *) ;
static int	procourconf_begin(PROGINFO *,cchar *) ;
static int	procourconf_end(PROGINFO *) ;
static int	procourconf_tmpdname(PROGINFO *) ;
static int	procnames(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procisexit(PROGINFO *) ;
static int	procspecs(PROGINFO *,cchar *,int) ;
static int	procspec(PROGINFO *,cchar *,int) ;
static int	procloadname(PROGINFO *,cchar *,int) ;
static int	procall(PROGINFO *) ;
static int	processin(PROGINFO *,cchar *) ;
static int	procprepare(PROGINFO *) ;
static int	procdeliver(PROGINFO *) ;

static int	procdelivery(PROGINFO *,int,int,MSGOPTS *) ;
static int	procdelivermsg(PROGINFO *,int,int,MSGOPTS *) ;

static int	procdelivermsgalls(PROGINFO *,int,int,MSGOPTS *,vechand *) ;
static int	procdelivermsgones(PROGINFO *,int,int,MSGOPTS *,vechand *) ;
static int	procdeliverentero(PROGINFO *,int,int,cchar **,
			cchar *,int,VECHAND *) ;
static int	procdeliverenter(PROGINFO *,int,cchar **,int,VECHAND *) ;
static int	procwaitok(PROGINFO *,pid_t,int) ;
static int	procwaitbad(PROGINFO *,pid_t,int) ;

static int	procsave(PROGINFO *) ;
static int	procuserbox(PROGINFO *,int,MSGOPTS *) ;
static int	procextract(PROGINFO *,int,MSGOPTS *) ;
static int	procextracter(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procdisposefile(PROGINFO *,char *,int to,MSGOPTS *) ;

static int	procmsg(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsgenv(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdrs(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_specials(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsgbody(PROGINFO *,FILEBUF *,int) ;

static int	procmsghdr_path(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_recv(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_org(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_mid(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_xpri(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_clines(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_clen(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_singles(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_rto(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_eto(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_sender(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_vsenders(PROGINFO *,FILEBUF *,MSGDATA *,MSGOPTS *) ;
static int	procmsghdr_from(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_addrs(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_addrsbcc(PROGINFO *,FILEBUF *,int) ;
static int	procmsghdr_date(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_subj(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_xuuid(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_xmcdate(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_xuti(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_xm(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr_all(PROGINFO *,FILEBUF *,int,MSGOPTS *) ;
static int	procmsghdr(PROGINFO *,FILEBUF *,int,cchar *,int) ;

static int	procmsginsthdr(PROGINFO *,FILEBUF *,int,cchar *,int,int) ;

static int procprinthdr(PROGINFO *,FILEBUF *,cchar *,cchar *,int) ;
static int procprinthdr_mid(PROGINFO *,FILEBUF *,cchar *,int) ;
static int procprinthdr_addrsome(PROGINFO *,FILEBUF *,cchar *,cchar *,int,int) ;
static int procprinthdr_line(PROGINFO *,FILEBUF *,int,int,int,cchar *,int) ;
static int procprinthdr_addrs(PROGINFO *,FILEBUF *,MAILMSGSTAGE *,int,cchar *) ;
static int procprinthdr_ema(PROGINFO *,FILEBUF *,cchar *,EMA *,int) ;
static int procprinthdr_bcc(PROGINFO *,FILEBUF *,vechand *) ;
static int procprinthdr_xpri(PROGINFO *,FILEBUF *) ;
static int procprinthdr_mailnote(PROGINFO *,FILEBUF *,cchar *,cchar *,int) ;

#if	CF_VSENDERS
static int	procprinthdr_emaone(PROGINFO *,FILEBUF *,cchar *,EMA_ENT *) ;
#endif

static int	procreciploads(PROGINFO *,vechand *,int,const int *) ;
static int	procrecipload(PROGINFO *,vechand *,MSGDATA *,int) ;
static int	procreciploademas(PROGINFO *,vechand *,EMA *) ;
static int	procreciploadema(PROGINFO *,vechand *,EMA_ENT *) ;

static int	procmsgfilebcc(PROGINFO *,int,int,int,cchar *) ;
static int	procgetns(PROGINFO *,char *,int,cchar *,int) ;

#if	CF_PROCEXTID
static int	procextid(PROGINFO *,char *,cchar *,int) ;
#endif /* CF_PROCEXTID */

static int	proclognmsgs(PROGINFO *) ;
static int	proclogrecips(PROGINFO *,VECHAND *) ;
static int	proclogmsg(PROGINFO *,int) ;
static int	proclogmsg_er(PROGINFO *,int) ;
static int	proclogmsg_mailer(PROGINFO *,int) ;
static int	proclogmsg_priority(PROGINFO *,int) ;
static int	proclogmsg_org(PROGINFO *,int) ;
static int	proclogmsg_addrs(PROGINFO *,int) ;
static int	proclogmsg_addr(PROGINFO *,int,int) ;
static int	proclogmsg_addremas(PROGINFO *,int,cchar *,EMA *,int) ;
static int	proclogmsg_addrema(PROGINFO *,int,cchar *,EMA_ENT *,int) ;
static int	proclogmsg_midhelp(PROGINFO *,MSGDATA *) ;
static int	proclogmsg_bcc(PROGINFO *,int) ;
static int	proclogmsg_sender(PROGINFO *,int) ;
static int	proclogmsg_from(PROGINFO *,int) ;
static int	proclogmsg_date(PROGINFO *,int) ;
static int	proclogmsg_subj(PROGINFO *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_hdrfrom(LOCINFO *,cchar *,int) ;
static int	locinfo_jobdname(LOCINFO *) ;
static int	locinfo_mailerprog(LOCINFO *) ;
static int	locinfo_defopts(LOCINFO *) ;
static int	locinfo_setfolder(LOCINFO *,cchar *,int) ;
static int	locinfo_setcmb(LOCINFO *,cchar *,int) ;
static int	locinfo_msgdatabegin(LOCINFO *) ;
static int	locinfo_msgdataget(LOCINFO *,int,MSGDATA **) ;
static int	locinfo_msgdataend(LOCINFO *) ;
static int	locinfo_cmbfname(LOCINFO *,char *) ;
static int	locinfo_mkmid(LOCINFO *,char *,int) ;
static int	locinfo_mkenvfrom(LOCINFO *) ;
static int	locinfo_mkenvdate(LOCINFO *) ;
static int	locinfo_mkenv(LOCINFO *) ;
static int	locinfo_mkhdrsender(LOCINFO *) ;
static int	locinfo_mkhdrfrom(LOCINFO *) ;
static int	locinfo_mkhdrname_from(LOCINFO *) ;
static int	locinfo_mkhdraddrfrom(LOCINFO *) ;
static int	locinfo_mkhdrdate(LOCINFO *) ;
static int	locinfo_mkhdrxuuid(LOCINFO *) ;
static int	locinfo_uuid(LOCINFO *) ;
static int	locinfo_loadrecip(LOCINFO *,cchar *,int) ;
static int	locinfo_opentmpfile(LOCINFO *,char *,int,cchar *) ;
static int	locinfo_tmpcheck(LOCINFO *) ;
static int	locinfo_tmpmaint(LOCINFO *) ;
static int	locinfo_tmpdone(LOCINFO *) ;
static int	locinfo_fchmodown(LOCINFO *,int,struct ustat *,mode_t) ;
static int	locinfo_cvtdate(LOCINFO *,char *,cchar *,int) ;
static int	locinfo_msgpriority(LOCINFO *,cchar *,int) ;
static int	locinfo_xmailer(LOCINFO *) ;
static int	locinfo_loadprids(LOCINFO *) ;
static int	locinfo_gm(LOCINFO *,cchar *,int) ;
static int	locinfo_gmcurbegin(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmcurend(LOCINFO *,LOCINFO_GMCUR *) ;
static int	locinfo_gmlook(LOCINFO *,LOCINFO_GMCUR *,cchar *,int) ;
static int	locinfo_gmread(LOCINFO *,LOCINFO_GMCUR *,char *,int) ;
static int	locinfo_rncurbegin(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rncurend(LOCINFO *,LOCINFO_RNCUR *) ;
static int	locinfo_rnlook(LOCINFO *,LOCINFO_RNCUR *,cchar *,int) ;
static int	locinfo_rnread(LOCINFO *,LOCINFO_RNCUR *,char *,int) ;
static int	locinfo_folderdname(LOCINFO *) ;
static int	locinfo_groupname(LOCINFO *) ;
static int	locinfo_prpcs(LOCINFO *) ;
static int	locinfo_pcsns(LOCINFO *) ;
static int	locinfo_pcsnsget(LOCINFO *,char *,int,cchar *,int) ;
static int	locinfo_isnotdisabled(LOCINFO *,int) ;

#ifdef	COMMENT
static int	locinfo_loaduids(LOCINFO *) ;
#endif /* COMMENT */

static int	mailfile_open(MAILFILE *,PROGINFO *,cchar *) ;
static int	mailfile_close(MAILFILE *) ;
static int	mailfile_altsetup(MAILFILE *,cchar *) ;

#ifdef	COMMENT
static int	mailfile_getfd(MAILFILE *) ;
static int	mailfile_read(MAILFILE *,char *,int) ;
#endif

static int	config_start(CONFIG *,PROGINFO *,cchar *) ;
static int	config_starter(CONFIG *) ;
static int	config_read(CONFIG *) ;
static int	config_reader(CONFIG *) ;
static int	config_finish(CONFIG *) ;
static int	config_files(CONFIG *,PARAMFILE *) ;
static int	config_loadcooks(CONFIG *) ;

#ifdef	COMMENT
static int	config_check(struct config *) ;
#endif

static int	msgdata_start(MSGDATA *,int) ;
static int	msgdata_setmid(MSGDATA *,cchar *,int) ;
static int	msgdata_finish(MSGDATA *) ;
static int	msgdata_loadrecips(MSGDATA *,MAILMSGSTAGE *,int) ;
static int	msgdata_loadrecip(MSGDATA *,int,MAILMSGSTAGE *,int,cchar *) ;
static int	msgdata_loadrecipadd(MSGDATA *,int,cchar *,int) ;
static int	msgdata_checksubject(MSGDATA *,MAILMSGSTAGE *,int) ;
static int	msgdata_loadbodytype(MSGDATA *,MAILMSGSTAGE *,int) ;
static int	msgdata_procsender(MSGDATA *,LOCINFO *) ;
static int	msgdata_getnvsenders(MSGDATA *) ;
static int	msgdata_getnsenders(MSGDATA *) ;
static int	msgdata_setvsender(MSGDATA *,int) ;
static int	msgdata_isvsender(MSGDATA *,int) ;
static int	msgdata_sethlen(MSGDATA *,int) ;
static int	msgdata_setsubject(MSGDATA *) ;
static int	msgdata_setnrecips(MSGDATA *,int) ;
static int	msgdata_getnrecips(MSGDATA *) ;
static int	msgdata_getsubject(MSGDATA *,cchar **) ;

#ifdef	COMMENT
static int	msgdata_setfrom(MSGDATA	 *,cchar *,int) ;
#endif

#if	CF_MSGOPTSNONE
static int	msgopts_none(MSGOPTS *) ;
#endif

static int	msgopts_all(MSGOPTS *) ;
static int	msgopts_dead(MSGOPTS *) ;

static int	loadorg(PROGINFO *) ;
static int	loadpath(PROGINFO *) ;
static int	loadpathpr(PROGINFO *) ;
static int	loadpathprbin(PROGINFO *,cchar *) ;
static int	loadpathspec(PROGINFO *,cchar *) ;
static int	loadpathcomp(PROGINFO *,cchar *,int) ;

#if	CF_CSPATH
static int	loadpathcs(PROGINFO *) ;
#endif /* CF_CSPATH */

static int	vrecipsch(const void **,const void **) ;
static int	ishigh(int) ;
static int	hasMailNote(cchar *,int,cchar *) ;
static int	isBadDate(int) ;
static int	isHdrEmpty(int) ;
static int	isHup(int) ;

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
static int	debugfilebuf_printoff(FILEBUF *,cchar *,cchar *,int) ;
#endif


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"cf",
	"nocache",
	"mh",
	"mailer",
	"hdr",
	"priority",
	"note",
	"onefrom",
	"sender",
	"replyto",
	"errorsto",
	"eto",
	"all",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_cf,
	argopt_nocache,
	argopt_mh,
	argopt_mailer,
	argopt_hdr,
	argopt_priority,
	argopt_note,
	argopt_onefrom,
	argopt_sender,
	argopt_replyto,
	argopt_errorsto,
	argopt_eto,
	argopt_all,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const MAPEX	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_NOMEM, EX_OSERR },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*progmodes[] = {
	"imail",
	"rmailnote",
	NULL
} ;

enum progmodes {
	progmode_imail,
	progmode_rmailnote,
	progmode_overlast
} ;

static const char	*akonames[] = {
	"log",
	"folder",
	"deliver",
	"copy",
	"take",
	"useclen",
	"useclines",
	"to",
	"mailer",
	"org",
	"sender",
	"addenv",
	"addsender",
	"addfrom",
	"addsubj",
	"addxuuid",
	"ignore",			/* ignore exit-interrupt */
	"priority",
	"subjrequired",
	"maint",
	"note",
	"onefrom",
	"onesender",
	"replyto",
	"errorsto",
	"eto",
	NULL
} ;

enum akonames {
	akoname_log,
	akoname_folder,
	akoname_deliver,
	akoname_copy,
	akoname_take,
	akoname_useclen,
	akoname_useclines,
	akoname_to,
	akoname_mailer,
	akoname_org,
	akoname_sender,
	akoname_addenv,
	akoname_addsender,
	akoname_addfrom,
	akoname_addsubj,
	akoname_addxuuid,
	akoname_ignore,			/* ignore exit-interrupt */
	akoname_priority,
	akoname_subjrequired,
	akoname_maint,
	akoname_note,
	akoname_onefrom,
	akoname_onesender,
	akoname_replyto,
	akoname_errorsto,
	akoname_eto,
	akoname_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*schedconf[] = {
	"%r/etc/%n/%n.%f",
	"%r/etc/%n/%f",
	"%r/etc/%n.%f",
	"%r/%n.%f",
	NULL
} ;

static const char	*params[] = {
	"postmail",
	"sendmail",
	"folder",
	"copy",
	"logfile",
	"logsize",
	"useclen",
	"useclines",
	"subjrequired",
	"to",
	"mailhost",
	"mailer",
	"org",
	"allok",
	"addsender",
	NULL
} ;

enum params {
	param_postmail,
	param_sendmail,
	param_folder,
	param_copy,
	param_logfile,
	param_logsize,
	param_useclen,
	param_useclines,
	param_subjrequired,
	param_to,
	param_mailhost,
	param_mailer,
	param_org,
	param_allok,
	param_addsender,
	param_overlast
} ;

static const char	*skiphdrs[] = {
	"message-id",
	"return-path",
	"received",
	"content-length",
	"content-lines",
	"content-type",			/* singlet */
	"content-transfer-encoding",	/* singlet */
	"from",
	"sender",
	"reply-to",
	"errors-to",
	"delivered-to",			/* re-energize for gateway */
	"to",
	"cc",
	"bcc",
	"date",
	"subject",
	"status",			/* should not be injected */
	"x-mailer",
	"x-delivered-to",		/* all trash from here on */
	"x-original-to",
	"x-forwarded-for",
	"x-sender",
	"x-from",
	"x-reply-to",
	"x-errors-to",
	"x-to",
	"x-cc",
	"x-bcc",
	"x-date",
	"x-subject",
	"x-references",
	"x-in-reply-to",
	"x-content-type",
	"x-content-transfer-encoding",
	"x-expires",
	"x-universally-unique-identifier",
	"x-uniform-type-identifier",
	"x-mail-created-date",
	NULL
} ;

static const char	*chewhdrs[] = {
	"references",
	"in-reply-to",
	"content-type",
	"content-transfer-encoding",
	NULL
} ;

/* these are mail-msg-header indexes that should be singlets only */
static const int	msghdrsingles[] = {
	HI_CTYPE,
	HI_CENCODING,
	-1
} ;

static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static const char	*ematypes[] = {
	"",
	"­£p",
	"­£a",
	"­£s",
	"­£g",
	NULL
} ;

enum msgpris {
	msgpri_none,
	msgpri_high,
	mggpri_moderate,
	msgpri_normal,
	msgpri_bulk,
	msgpri_low,
	msgpri_overlast
} ;

static const char	*msgpris[] = {
	"none",
	"high",
	"moderate",
	"normal",
	"bulk",
	"low",
	NULL
} ;

enum userboxes {
	userbox_copy,
	userbox_dead,
	userbox_overlast
} ;

static cchar		*userboxes[] = {
	"copy",
	"dead",
	NULL
} ;

static const int	rsdatebad[] = {
	SR_DOM,
	SR_INVALID,
	0
} ;

static const int	rshup[] = {
	SR_HANGUP,
	SR_OK,
	SR_NOTCONN,
	0
} ;


/* exported subroutines */


int b_imail(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_imail) */


int p_imail(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_imail) */


int p_rmailnote(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_rmailnote) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pm = NULL ;
	cchar		*sn = NULL ;
	cchar		*pr = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*ifname = NULL ;
	cchar		*cfname = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_imail: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
	debugprintf("b_imail: initial mallout=%u\n",mo_start) ;
#endif

#if	CF_TESTIN
	u_close(0) ;
	u_open(NULLDEV,O_WRONLY,0660) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;

#if	CF_DEBUGDEF
	pip->debuglevel = 5 ;
#endif

	pip->f.logprog = TRUE ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("b_imail: ak=%s(%d)\n",argopts[kwi],kwi) ;
#endif

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pm = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* intput file */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_cf:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            pip->have.cfname = TRUE ;
	                            pip->final.cfname = TRUE ;
	                            cfname = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* mailhost */
	                case argopt_mh:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            lip->have.mailhost = TRUE ;
	                            lip->final.mailhost = TRUE ;
	                            lip->mailhost = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* mailer */
	                case argopt_mailer:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            lip->have.xmailer = TRUE ;
	                            lip->final.xmailer = TRUE ;
	                            lip->f.xmailer = TRUE ;
	                            lip->xmailer = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* msg-priority */
	                case argopt_priority:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = locinfo_msgpriority(lip,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* add an Apple-Note */
	                case argopt_note:
	                    lip->final.addnote = TRUE ;
	                    lip->have.addnote = TRUE ;
	                    lip->f.addnote = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.addnote = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_sender:
	                    if (argr > 0) {
	                        lip->have.sender = TRUE ;
	                        lip->final.sender = TRUE ;
	                        lip->f.sender = TRUE ;
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            lip->hdraddr_sender = argp ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* allow only one "from" */
	                case argopt_onefrom:
	                    lip->final.onefrom = TRUE ;
	                    lip->have.onefrom = TRUE ;
	                    lip->f.onefrom = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.onefrom = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_replyto:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            if (sfskipwhite(argp,argl,&cp) > 0) {
	                                lip->hdraddr_replyto = cp ;
	                            }
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_errorsto:
	                case argopt_eto:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            if (sfskipwhite(argp,argl,&cp) > 0) {
	                                lip->hdraddr_errorsto = cp ;
	                            }
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_all:
	                    lip->final.all = TRUE ;
	                    lip->have.all = TRUE ;
	                    lip->f.all = TRUE ;
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optbool(argp,argl) ;
	                            lip->f.all = (rs > 0) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.cfname = TRUE ;
	                                pip->final.cfname = TRUE ;
	                                cfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.quiet = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.hdrfroms = TRUE ;
	                                lip->final.hdrfroms = TRUE ;
	                                if (argp[0] == '+') {
	                                    lip->f.def_from = TRUE ;
	                                } else if (argp[0] != '-') {
	                                    const int	al = argl ;
	                                    cchar	*ap = argp ;
	                                    rs = locinfo_hdrfrom(lip,ap,al) ;
	                                }
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* mailbox-name for copy */
	                    case 'm':
	                        lip->have.cmbname = TRUE ;
	                        lip->final.cmbname = TRUE ;
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                cchar	*mb = argp ;
	                                lip->cmbname = argp ;
	                                lip->f.cmbname = 
	                                    (mb[0] != '\0') && (mb[0] != '-') ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* subject (when there isn't already one) */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->have.hdrsubject = TRUE ;
	                                lip->final.hdrsubject = TRUE ;
	                                lip->hdrsubject = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 't':
	                        lip->have.take = TRUE ;
	                        lip->final.take = TRUE ;
	                        lip->f.take = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.take = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_imail: args rs=%d debuglevel=%u\n",
	        rs,pip->debuglevel) ;
#endif

#if	CF_DEBUGS
	debugprintf("b_imail: args rs=%d debuglevel=%u\n",
	    rs,pip->debuglevel) ;
#endif

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_imail: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    cchar	*pn = pip->progname ;
	    shio_printf(pip->efp,"%s: version %s\n",pn,pip->version) ;
	}

/* get our program mode */

	if (pm == NULL) pm = pip->progname ;

	if ((pip->progmode = matstr(progmodes,pm,-1)) >= 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pm=%s (%u)\n" ;
	        shio_printf(pip->efp,fmt,pn,pm,pip->progmode) ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid program-mode (%s)\n" ;
	    shio_printf(pip->efp,fmt,pn,pm) ;
	    ex = EX_USAGE ;
	    rs = SR_INVALID ;
	}

/* get the program root */

	if (rs >= 0) {
	    PIVARS	vars = initvars ;
	    cchar	*varsn = VARSEARCHNAME ;
	    switch (pip->progmode) {
	    case progmode_rmailnote:
	        vars.vpr1 = "RMAILNOTE_PROGRAMROOT" ;
	        vars.vpr2 = VARPRPCS ;
	        vars.vprname = VARPRPCS ;
	        varsn = "RMAILNOTE_NAME" ;
	        break ;
	    } /* end switch */
	    if ((rs = proginfo_setpiv(pip,pr,&vars)) >= 0) {
	        rs = proginfo_setsearchname(pip,varsn,sn) ;
	    }
	} /* end block */

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	if (ofname != NULL) rs = 1 ; /* lint */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    if ((rs = proginfo_pwd(pip)) >= 0) {
	        if ((rs = proginfo_rootname(pip)) >= 0) {
	            rs = procopts(pip,&akopts) ;
	        }
	    }
	}

	if ((! pip->final.ignore) && hasalluc(pip->progname,-1)) {
	    pip->f.ignore = TRUE ;
	}

	if (afname == NULL) afname = getourenv(pip->envv,VARAFNAME) ;

	if (ifname == NULL) ifname = getourenv(pip->envv,VARIFNAME) ;

	if (rs >= 0) {
	    switch (pip->progmode) {
	    case progmode_rmailnote:
	        {
	            uid_t	ruid = getuid() ;
	            uid_t	euid = geteuid() ;
	            if (! lip->final.addnote) {
	                lip->have.addnote = TRUE ;
	                lip->f.addnote = TRUE ;
	            }
	            if (! lip->final.addsender) {
	                lip->have.addsender = TRUE ;
	                lip->f.addsender = TRUE ;
	            }
	            if (! lip->final.take) {
	                lip->have.take = TRUE ;
	                lip->f.take = FALSE ;
	            }
	            if (! lip->final.cmbname) {
	                cchar	*cmb = "mailnotes" ;
	                lip->final.cmbname = TRUE ;
	                lip->f.cmbname = TRUE ;
	                rs = locinfo_setcmb(lip,cmb,-1) ;
	            }
	            if (lip->hdraddr_errorsto == NULL) {
	                lip->hdraddr_errorsto = "+" ;
	            }
	            if ((rs >= 0) && (ruid != euid)) {
	                rs = u_setreuid(euid,-1) ;
	            }
	        }
	        break ;
	    } /* end witch */
	} /* end if (ok) */

	if (rs < 0) goto badarg ;

/* mail folder name */

	if (lip->folder == NULL) {
	    if ((cp = getourenv(envv,VARFOLDER)) != NULL) {
	        lip->folder = cp ;
	        lip->have.folder = TRUE ;
	        lip->final.folder = TRUE ;
	    }
	}

	if (lip->folder == NULL)
	    lip->folder = FOLDERDNAME ;

/* mail-host name */

	if (lip->mailhost == NULL) {
	    lip->mailhost = getourenv(envv,VARMAILHOST) ;
	    if (lip->mailhost != NULL) {
	        lip->have.mailhost = TRUE ;
	        lip->final.mailhost = TRUE ;
	    }
	}

	if (lip->mailhost == NULL)
	    lip->mailhost = MAILHOST ;

/* configuration file processing */

	if (cfname == NULL) cfname = getourenv(envv,VARCFNAME) ;
	if (cfname == NULL) cfname = getourenv(envv,VARCONFIG) ;

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    if (cfname != NULL) {
	        shio_printf(pip->efp,"%s: conf=%s\n",pn,cfname) ;
	    }
	}

/* go through the loops */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
#if	CF_PROCOURCONF
	            if ((rs = procourconf_begin(pip,cfname)) >= 0) {
#if	CF_PROGLOGER
	                if ((rs = proglog_begin(pip,&u)) >= 0) {
#if	CF_PROGUSERLIST
	                    if ((rs = proguserlist_begin(pip)) >= 0) {
	                        ARGINFO	*aip = &ainfo ;
	                        BITS	*bop = &pargs ;
	                        cchar	*afn = afname ;
	                        if ((rs = procnames(pip,aip,bop,afn)) >= 0) {
	                            rs = processin(pip,ifname) ;
	                        }
	                        rs1 = proguserlist_end(pip) ;
	                        if (rs >= 0) rs = rs1 ;
	                    } /* end if (proguserlist) */
#endif /* CF_PROGUSERLIST */
	                    rs1 = proglog_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (proglog) */
#endif /* CF_PROGLOGER */
	                rs1 = procourconf_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (procourconf) */
#endif /* CF_PROCOURCONF */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        ex = EX_NOUSER ;
	        fmt = "%s: userinfo failure (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if */
#ifdef	COMMENT
	    if ((rs >= 0) && lip->f.reqsubj && lip->f.notsubj) {
	        rs = SR_AGAIN ;
	    }
#endif /* COMMENT */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* finished */
	if ((rs < 0) && (ex == EX_OK)) {
	    cchar	*pn = pip->progname ;
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            cchar	*fmt = "%s: invalid operation (%d)\n" ;
	            shio_printf(pip->efp,fmt,pn,rs) ;
	        }
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        shio_printf(pip->efp,"%s: unknown error (%d)\n",pn,rs) ;
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_imail: exiting ex=%d (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
	        UCMALLREG_CUR	cur ;
	        UCMALLREG_REG	reg ;
	        const int	size = (10*sizeof(uint)) ;
	        cchar		*ids = "main" ;
	        uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
	            mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
	        ucmallreg_curbegin(&cur) ;
	        while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
	            debugprinthexblock(ids,80,reg.addr,reg.size) ;
	        } /* end while */
	        ucmallreg_curend(&cur) ;
	    } /* end if (positive) */
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<recip(s)> ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-t[=<b>]] [-f <fromaddr>] [-s <subject>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procisexit(PROGINFO *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return lib_sigterm() ;
}
/* end subroutine (procisexit) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while (rs >= 0) {
	            kl = keyopt_enumkeys(kop,&kcur,&kp) ;
	            if (kl == SR_NOTFOUND) break ;
	            rs = kl ;
	            if (rs < 0) break ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	                int	v ;

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_ignore:
	                    if (! pip->final.ignore) {
	                        pip->have.ignore = TRUE ;
	                        pip->final.ignore = TRUE ;
	                        pip->f.ignore = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.ignore = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_folder:
	                    if ((! lip->final.folder) && (vl > 0)) {
	                        rs = locinfo_setfolder(lip,vp,vl) ;
	                        lip->have.folder = TRUE ;
	                        lip->final.folder = TRUE ;
	                    }
	                    break ;
	                case akoname_copy:
	                    if (! lip->final.cmbname) {
	                        if (vl > 0) {
	                            if ((rs1 = optbool(vp,vl)) >= 0) {
	                                lip->f.cmbname = (rs1 > 0) ;
	                            } else {
	                                lip->f.cmbname = TRUE ;
	                                rs = locinfo_setcmb(lip,vp,vl) ;
	                            }
	                        }
	                        lip->final.cmbname = TRUE ;
	                        lip->have.cmbname = TRUE ;
	                    }
	                    break ;
	                case akoname_log:
	                    if (! pip->final.logprog) {
	                        pip->have.logprog = TRUE ;
	                        pip->final.logprog = TRUE ;
	                        pip->f.logprog = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.logprog = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_deliver:
	                    if (! lip->final.deliver) {
	                        lip->have.deliver = TRUE ;
	                        lip->final.deliver = TRUE ;
	                        lip->f.deliver = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.deliver = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_take:
	                    if (! lip->final.take) {
	                        lip->have.take = TRUE ;
	                        lip->final.take = TRUE ;
	                        lip->f.take = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.take = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_useclen:
	                    if (! lip->final.useclen) {
	                        lip->have.useclen = TRUE ;
	                        lip->final.useclen = TRUE ;
	                        lip->f.useclen = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.useclen = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_useclines:
	                    if (! lip->final.useclines) {
	                        lip->have.useclines = TRUE ;
	                        lip->final.useclines = TRUE ;
	                        lip->f.useclines = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.useclines = (rs > 0) ;
	                        }
	                    }
	                    break ;
/* time-out */
	                case akoname_to:
	                    if (! lip->final.to) {
	                        lip->have.to = TRUE ;
	                        lip->final.to = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->to = v ;
	                        }
	                    }
	                    break ;
	                case akoname_mailer:
	                    if ((! lip->final.xmailer) && (vl > 0)) {
	                        lip->final.xmailer = TRUE ;
	                        lip->have.xmailer = TRUE ;
	                        lip->f.xmailer = TRUE ;
	                        if ((rs1 = optbool(vp,vl)) >= 0) {
	                            lip->f.xmailer = (rs1 > 0) ;
	                        } else {
	                            cchar	**vpp = &lip->xmailer ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_org:
	                    if (! lip->final.org) {
	                        lip->final.org = TRUE ;
	                        lip->have.org = TRUE ;
	                        lip->f.org = TRUE ;
	                        if (vl > 0) {
	                            if ((rs1 = optbool(vp,vl)) >= 0) {
	                                lip->f.org = (rs1 > 0) ;
	                            } else if (pip->org == NULL) {
	                                cchar	**vpp = &pip->org ;
	                                rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                            }
	                        }
	                    }
	                    break ;
	                case akoname_sender:
	                    if (! lip->final.sender) {
	                        lip->final.sender = TRUE ;
	                        lip->have.sender = TRUE ;
	                        lip->f.sender = TRUE ;
	                        if (vl > 0) {
	                            if ((rs1 = optbool(vp,vl)) >= 0) {
	                                lip->f.sender = (rs1 > 0) ;
	                            } else {
	                                cchar **vpp = &lip->hdraddr_sender ;
	                                rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                            }
	                        }
	                    }
	                    break ;
	                case akoname_addenv:
	                    if (! lip->final.addenv) {
	                        lip->have.addenv = TRUE ;
	                        lip->final.addenv = TRUE ;
	                        lip->f.addenv = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.addenv = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_addsender:
	                    if (! lip->final.addsender) {
	                        lip->have.addsender = TRUE ;
	                        lip->final.addsender = TRUE ;
	                        lip->f.addsender = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.addsender = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_addfrom:
	                    if (! lip->final.addfrom) {
	                        lip->have.addfrom = TRUE ;
	                        lip->final.addfrom = TRUE ;
	                        lip->f.addfrom = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.addfrom = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_addsubj:
	                    if (! lip->final.addsubj) {
	                        lip->have.addsubj = TRUE ;
	                        lip->final.addsubj = TRUE ;
	                        lip->f.addsubj = TRUE ;
	                        if ((vl > 0) && (lip->hdrsubject == NULL)) {
	                            cchar	**vpp = &lip->hdrsubject ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_addxuuid:
	                    if (! lip->final.addxuuid) {
	                        lip->have.addxuuid = TRUE ;
	                        lip->final.addxuuid = TRUE ;
	                        lip->f.addxuuid = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.addxuuid = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_priority:
	                    if (! lip->final.msgpriority) {
	                        lip->final.msgpriority = TRUE ;
	                        lip->have.msgpriority = TRUE ;
	                        if (vl > 0) {
	                            rs = locinfo_msgpriority(lip,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_subjrequired:
	                    if (! lip->final.reqsubj) {
	                        lip->have.reqsubj = TRUE ;
	                        lip->final.reqsubj = TRUE ;
	                        lip->f.reqsubj = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.reqsubj = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_maint:
	                    if (! lip->final.maint) {
	                        lip->have.maint = TRUE ;
	                        lip->final.maint = TRUE ;
	                        lip->f.maint = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.maint = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_note:
	                    if (! lip->final.addnote) {
	                        lip->have.addnote = TRUE ;
	                        lip->final.addnote = TRUE ;
	                        lip->f.addnote = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.addnote = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_onefrom:
	                    if (! lip->final.onefrom) {
	                        lip->have.onefrom = TRUE ;
	                        lip->final.onefrom = TRUE ;
	                        lip->f.onefrom = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.onefrom = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_onesender:
	                    if (! lip->final.onesender) {
	                        lip->have.onesender = TRUE ;
	                        lip->final.onesender = TRUE ;
	                        lip->f.onesender = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.onesender = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_replyto:
	                    if (lip->hdraddr_replyto == NULL) {
	                        if (vl > 0) {
	                            cchar **vpp = &lip->hdraddr_replyto ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                case akoname_errorsto:
	                case akoname_eto:
	                    if (lip->hdraddr_errorsto == NULL) {
	                        if (vl > 0) {
	                            cchar **vpp = &lip->hdraddr_errorsto ;
	                            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	                        }
	                    }
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
	                rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->org = uip->organization ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->homedname = uip->homedname ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    rs = procmklogid(pip) ;
	} /* end if (ok) */

	if (rs >= 0) {
	    rs = ids_load(&pip->id) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	rs1 = ids_release(&pip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int procmklogid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
	    if (rs & KSHLIB_RMKSH) {
	        if ((rs = lib_serial()) >= 0) {
	            LOCINFO	*lip = pip->lip ;
	            const int	plen = LOGIDLEN ;
	            const int	pv = pip->pid ;
	            cchar	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            lip->kserial = rs ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                const int	s = lip->kserial ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    cchar	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procmklogid) */


static int procourconf_begin(PROGINFO *pip,cchar *cfname)
{
	LOCINFO		*lip = pip->lip ;
	const int	size = sizeof(CONFIG) ;
	int		rs ;
	cchar		*pn = pip->progname ;
	void		*p ;

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    cchar	*pn = pip->progname ;
	    pip->config = p ;
	    if ((rs = config_start(pip->config,pip,cfname)) >= 0) {
	        pip->open.config = TRUE ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_imail: config_start() rs=%d\n",rs) ;
#endif

	        if (pip->debuglevel > 0) {
	            shio_printf(pip->efp,"%s: mailhost=%s\n",
	                pn,lip->mailhost) ;
	            shio_printf(pip->efp,"%s: folder=%s\n",
	                pn,lip->folder) ;
	        }

/* residual initialization */

	        if ((rs = procourconf_tmpdname(pip)) >= 0) {

	            if (pip->logsize < 0) pip->logsize = LOGSIZE ;

	            if ((rs = locinfo_jobdname(lip)) >= 0) {
	                if ((rs = locinfo_tmpcheck(lip)) >= 0) {
	                    if ((rs = locinfo_mailerprog(lip)) >= 0) {
	                        rs = locinfo_defopts(lip) ;
	                    }
	                } /* end if (locinfo_tmpcheck) */
	            } /* end if (locinfo_jobdname) */

	            if (! lip->have.take) lip->f.take = TRUE ;

#if	CF_XMAILER
	            if (lip->xmailer == NULL) {
	                lip->xmailer = MAILERNAME ;
	            }
#endif /* CF_XMAILER */

	        } /* end if (procourconfig_tmpdname) */

	        if (rs < 0) {
	            pip->open.config = FALSE ;
	            config_finish(pip->config) ;
	        }
	    } /* end if (config_start) */
	    if (rs < 0) {
	        uc_free(pip->config) ;
	        pip->config = NULL ;
	    }
	} /* end if (m-a) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: take=%u\n",pn,lip->f.take) ;
	    shio_printf(pip->efp,"%s: addsender=%u\n",pn,lip->f.addsender) ;
	}

	return rs ;
}
/* end subroutine (procourconf_begin) */


static int procourconf_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->config != NULL) {
	    if (pip->open.config) {
	        pip->open.config = FALSE ;
	        rs1 = config_finish(pip->config) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(pip->config) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->config = NULL ;
	}
	return rs ;
}
/* end subroutine (procourconf_end) */


static int procourconf_tmpdname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		**envv = pip->envv ;
	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
	return rs ;
}
/* end subroutine (procourconf_tmpdname) */


static int procnames(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	if (lip->f.all && lip->f.allok) {
	    rs = procall(pip) ;
	} else {
	    rs = procargs(pip,aip,bop,afn) ;
	}
	return rs ;
}
/* end subroutine (procnames) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procargs: ent\n") ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: procargs argc=%u\n",pn,aip->argc) ;
	}

	if (rs >= 0) {
	    int	ai ;
	    int	f ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	        if (f) {
	            if ((cl = sfshrink(aip->argv[ai],-1,&cp)) > 0) {
	                pan += 1 ;
	                rs = procspec(pip,cp,cl) ;
	                c += rs ;
	            }
	        }

	        if (rs >= 0) rs = procisexit(pip) ;
	        if (rs < 0) break ;
	    } /* end for (handling positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0)
	        afn = STDINFNAME ;

	    if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    rs = procspecs(pip,cp,cl) ;
	                    c += rs ;
	                }
	            }

	            if (rs >= 0) rs = procisexit(pip) ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = shio_close(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*fmt ;
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	        shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (processing file argument file list) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: procargs c=%u (%d)\n",pn,c,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procargs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procspecs(PROGINFO *pip,cchar lbuf[],int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


/* process a specification name */
static int procspec(PROGINFO *pip,cchar np[],int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		size ;
	int		c = 0 ;
	char		*bestaddr = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_imail/procspec: name=%t\n",np,nl) ;
#endif

	if (nl < 0)
	    nl = strlen(np) ;

	size = (nl + 1) ;
	if ((rs = uc_malloc(size,&bestaddr)) >= 0) {

	    if ((rs = mkbestaddr(bestaddr,nl,np,nl)) >= 0) {
	        const int	bal = rs ;
	        if ((bal > 1) && (strnpbrk(bestaddr,bal,"@!") != NULL)) {
	            rs = locinfo_loadrecip(lip,bestaddr,bal) ;
	            c += rs ;
	        } else {
	            rs = procloadname(pip,bestaddr,bal) ;
	            c += rs ;
	        }
	    } /* end if (mkbestname) */

	    rs1 = uc_free(bestaddr) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_imail/procspec: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspec) */


static int procloadname(PROGINFO *pip,cchar np[],int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;

	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadrecip: ent rn=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if ((np[0] == '\0') || hasMeAlone(np,nl)) {
	    np = pip->username ;
	    nl = -1 ;
	    rs = locinfo_loadrecip(lip,np,nl) ;
	    c += rs ;
	} else {
	    const int	nch = MKCHAR(np[0]) ;
	    cchar	*tp ;
	    if ((tp = strnchr(np,nl,'+')) != NULL) {
	        nl = (tp-np) ;
	    }
	    if (strnchr(np,nl,'.') != NULL) {
	        LOCINFO_RNCUR	rnc ;
	        if ((rs = locinfo_rncurbegin(lip,&rnc)) >= 0) {
	            if ((rs = locinfo_rnlook(lip,&rnc,np,nl)) > 0) {
	                const int	ul = USERNAMELEN ;
	                char		ub[USERNAMELEN+1] ;
	                while ((rs = locinfo_rnread(lip,&rnc,ub,ul)) > 0) {
	                    rs = locinfo_loadrecip(lip,ub,rs) ;
	                    c += rs ;
	                    if (rs < 0) break ;
	                } /* end while (reading entries) */
	            } /* end if (locinfo_rnlook) */
	            rs1 = locinfo_rncurend(lip,&rnc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (srncursor) */
	    } else if (nch == MKCHAR('¡')) {
	        cchar		*gnp = (np+1) ;
	        int		gnl = (nl-1) ;
	        if (gnl == 0) {
	            rs = locinfo_groupname(lip) ;
	            gnl = rs ;
	            gnp = lip->gnbuf ;
	        }
	        if (rs >= 0) {
	            rs = locinfo_gm(lip,gnp,gnl) ;
	            c += rs ;
	        } /* end if (ok) */
	    } else {
	        if (nch == '!') {
	            np += 1 ;
	            nl -= 1 ;
	        }
	        if (nl > 0) {
	            rs = locinfo_loadrecip(lip,np,nl) ;
	            c += rs ;
	        }
	    } /* end if */
	} /* end if */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: procloadname c=%u (%d)\n",pn,c,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/procloadname: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procloadname) */


static int procall(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	if (lip->f.all && lip->f.allok) {
	    SYSUSERNAMES	uns ;
	    if ((rs = sysusernames_open(&uns,NULL)) >= 0) {
	        const int	unlen = USERNAMELEN ;
	        char		unbuf[USERNAMELEN+1] ;
	        while ((rs = sysusernames_readent(&uns,unbuf,unlen)) > 0) {
	            rs = locinfo_loadrecip(lip,unbuf,rs) ;
	            c += rs ;
	            if (rs < 0) break ;
	        } /* end while */
	        rs1 = sysusernames_close(&uns) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sysusernames) */
	} /* end if (all-users) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procall) */


static int processin(PROGINFO *pip,cchar *ifname)
{
	LOCINFO		*lip = pip->lip ;
	MAILFILE	mf ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	cchar		*pn = pip->progname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/process: ent ifname=%s\n",ifname) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: process ifn=%s\n",pn,ifname) ;
	}

	if ((rs = mailfile_open(&mf,pip,ifname)) >= 0) {
	    const int	mfd = rs ;
	    int		opts = 0 ;

#if	CF_PROCMSGSTAGE
	    opts |= ((lip->f.useclen) ? MAILMSGSTAGE_OUSECLEN : 0) ;
	    opts |= ((lip->f.useclines) ? MAILMSGSTAGE_OUSECLINES : 0) ;
	    if ((rs = mailmsgstage_start(&lip->ms,mfd,lip->to,opts)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("b_imail/process: "
	                "mailmsgstage_start() rs=%d\n",rs) ;
#endif

#if	CF_PROCPREPARE
	        if ((rs = locinfo_msgdatabegin(lip)) >= 0) {
	            n = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_imail/process: "
	                    "locinfo_msgdatabegin() rs=%d\n",rs) ;
#endif

	            rs = proclognmsgs(pip) ;

	            if ((rs >= 0) && (n > 0)) {

	                rs = procprepare(pip) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("b_imail/process: "
	                        "procprepare() rs=%d f_deliver=%u\n",
	                        rs,lip->f.deliver) ;
#endif

	                if ((rs >= 0) && lip->f.deliver) {
	                    rs = procdeliver(pip) ;
	                }

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("b_imail/process: "
	                        "procdeliver() rs=%d\n",rs) ;
#endif

#if	CF_PROCSAVE
	                if ((rs >= 0) && lip->f.cmbname) {
	                    rs = procsave(pip) ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(2))
	                        debugprintf("b_imail/process: "
	                            "procsave() rs=%d\n",rs) ;
#endif
	                }
#endif /* CF_PROCSAVE */

#if	CF_PROCUSERBOX
	                if (rs >= 0) {
	                    MSGOPTS	mo ;
	                    msgopts_dead(&mo) ;
	                    mo.nosubj = TRUE ;
	                    mo.norecip = TRUE ;
	                    rs = procuserbox(pip,userbox_dead,&mo) ;
	                }
#endif /* CF_PROCUSERBOX */

	                if ((rs >= 0) && pip->open.logprog) {
	                    int		mi ;
	                    int		nm = lip->nmsgs ;
	                    for (mi = 0 ; (rs >= 0) && (mi < nm) ; mi += 1) {
	                        rs = proclogmsg(pip,mi) ;
	                    } /* end for */
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("b_imail/process: "
	                            "proclogmsg() rs=%d\n",rs) ;
#endif
	                } /* end if */

	            } /* end if (had some messages) */

	            rs1 = locinfo_msgdataend(lip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (msgdataend) */
#endif /* CF_PROCPREPARE */

	        rs1 = mailmsgstage_finish(&lip->ms) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mailmsgstage) */
#endif /* CF_PROCMSGSTAGE */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_imail/process: mailmsgstage rs=%d\n",rs) ;
#endif

	    rs1 = mailfile_close(&mf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mailfile) */

	if (pip->open.logprog) {
	    if (lip->f.reqsubj && lip->f.notsubj) {
	        cchar	*fmt = "subjects missing (when required)" ;
	        proglog_printf(pip,fmt) ;
	    }
	    proglog_printf(pip,"done (%d)",rs) ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: process n=%u (%d)\n",pn,n,rs) ;
	}

#ifdef	COMMENT
	if ((rs >= 0) && lip->f.reqsubj && lip->f.notsubj) {
	    if (! pip->f.quiet) {
	        cchar	*fmt = "%s: no subject (subject required)\n" ;
	        shio_printf(pip->efp,fmt,pip->progname) ;
	    }
	}
#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/process: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (processin) */


static int procprepare(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs = SR_OK ;
	int		mi ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procprepare: ent\n") ;
#endif

	msp = &lip->ms ;
	for (mi = 0 ; (rs >= 0) && (mi < lip->nmsgs) ; mi += 1) {
	    MSGDATA	*mdp ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/procprepare: loop mi=%u\n",mi) ;
#endif
	    if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/procprepare: loop mid1 rs=%d\n",rs) ;
#endif
	        if ((rs = msgdata_loadrecips(mdp,msp,mi)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("b_imail/procprepare: loop mid2 rs=%d\n",rs) ;
#endif
	            if ((rs = msgdata_checksubject(mdp,msp,mi)) >= 0) {
#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("b_imail/procprepare: loop mid3 rs=%d\n",rs) ;
#endif
	                if ((rs = msgdata_loadbodytype(mdp,msp,mi)) >= 0) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("b_imail/procprepare: loop mid4 rs=%d\n",
	                            rs) ;
#endif
	                    rs = msgdata_procsender(mdp,lip) ;
	                }
	            }
	        } /* end if */
	    } /* end if */
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procprepare: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (procprepare) */


static int procdeliver(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	MSGOPTS		opts ;
	const int	of = (O_CREAT | O_RDWR) ;
	int		rs ;
	int		c = 0 ;
	cchar		*pre = "mbtmp" ;
	char		tbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/procdeliver: ent\n") ;
#endif

	memset(&opts,0,sizeof(MSGOPTS)) ;
	opts.mkclines = TRUE ;
	opts.mkcrnl = TRUE ;			/* possible CRNL enforcement */
	opts.mkenv = lip->f.addenv ;
	opts.mkfrom = lip->f.addfrom ;
	opts.mksubj = lip->f.addsubj ;
	opts.mkxuuid = lip->f.addxuuid ;	/* UUID */
	opts.mkxnote = lip->f.addnote ;		/* Apple-Note */
	opts.mkxmcdate = lip->f.addxmcdate ;	/* Mail-Created-Date */
	opts.reqsubj = lip->f.reqsubj ;

	if ((rs = locinfo_opentmpfile(lip,tbuf,of,pre)) >= 0) {
	    const int	nmsgs = lip->nmsgs ;
	    const int	sfd = rs ;
	    int		mi ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_imail/procdeliver: nmsgs=%u\n",nmsgs) ;
#endif

	    u_unlink(tbuf) ;
	    for (mi = 0 ; mi < nmsgs ; mi += 1) {
	        rs = procdelivery(pip,mi,sfd,&opts) ;
	        c += rs ;
	        if (rs < 0) break ;
	    } /* end for */

	    u_close(sfd) ;
	} /* end if (locinfo_opentmpfile) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/procdeliver: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdeliver) */


/* deliver one message (#=<mi>) */
static int procdelivery(PROGINFO *pip,int mi,int sfd,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/procdelivery: mi=%u\n",mi) ;
#endif

	if (mi > 0) {
	    rs = u_rewind(sfd) ;
	}

	if (rs >= 0) {
	    FILEBUF	fb ;
	    offset_t	fboff = 0 ;
	    const int	bsize = lip->pagesize ;

	    if ((rs = filebuf_start(&fb,sfd,fboff,bsize,0)) >= 0) {

	        rs = procmsg(pip,&fb,mi,optp) ;
	        wlen += rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_imail/procdelivery: "
	                "procmsg() rs=%d wlen=%u\n",
	                rs,wlen) ;
#endif

	        rs1 = filebuf_finish(&fb) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */

	    if ((rs >= 0) && (mi > 0)) {
	        rs = uc_ftruncate(sfd,wlen) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("b_imail/procdelivery: mid rs=%d wlen=%u\n",
	            rs,wlen) ;
	        debugprintf("b_imail/procdelivery: f_deliver=%u\n",
	            lip->f.deliver) ;
	    }
#endif /* CF_DEBUG */

	    if ((rs >= 0) && lip->f.deliver) {
	        MSGDATA		*mdp ;
	        if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	            if ((rs = msgdata_getsubject(mdp,NULL)) >= 0) {
	                const int	f_subj = rs ;
	                cchar		*pn = pip->progname ;
	                cchar		*fmt ;
	                if (f_subj || (! lip->f.reqsubj)) {
	                    if ((rs = procdelivermsg(pip,mi,sfd,optp)) >= 0) {
	                        const int	nr = rs ;
	                        if ((rs = msgdata_setnrecips(mdp,nr)) >= 0) {
	                            if ((nr == 0) && (! pip->f.quiet)) {
	                                fmt = "%s: msg=%u has no recipients\n" ;
	                                shio_printf(pip->efp,fmt,pn,mi) ;
	                            } else if (pip->debuglevel > 0) {
	                                fmt = "%s: msg=%u recipients=%u\n" ;
	                                shio_printf(pip->efp,fmt,pn,mi,nr) ;
	                            }
	                        } /* end if */
	                    } /* end if (procdelivermsg) */
	                } else if (lip->f.reqsubj && (! f_subj)) {
	                    wlen = 0 ;
	                    if (! pip->f.quiet) {
	                        fmt = "%s: msg=%u no-subject (required)\n" ;
	                        shio_printf(pip->efp,fmt,pn,mi) ;
	                    }
	                    if (pip->f.logprog) {
	                        fmt = "msg=%u no-subject (required)" ;
	                        proglog_printf(pip,fmt,mi) ;
	                    }
	                } /* end if (deliverability -- subject) */
	            } /* end if (msgdata_getsubject) */
	        } /* end if (locingo_msgdataget) */
	    } /* end if (general delivery specified) */

	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/procdelivery: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdelivery) */


static int procdelivermsg(PROGINFO *pip,int mi,int sfd,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	vechand		aa ;
	const int	atypes[] = { msgloghdr_to, msgloghdr_cc, -1 } ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/procdelivermsg: f_take=%u mi=%u\n",
	        lip->f.take,mi) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: procdelivermsg mi=%u\n",pn,mi) ;
	}

	if ((rs = vechand_start(&aa,0,0)) >= 0) {
	    if ((rs = procreciploads(pip,&aa,mi,atypes)) >= 0) {

	        if (lip->f.take) {
	            rs = procdelivermsgalls(pip,mi,sfd,optp,&aa) ;
	            c += rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_imail/procdelivermsg: ~alls() rs=%d\n",
	                    rs) ;
#endif

	        }

	        if (rs >= 0) {
	            rs = procdelivermsgones(pip,mi,sfd,optp,&aa) ;
	            c += rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_imail/procdelivermsg: ~ones() rs=%d\n",
	                    rs) ;
#endif

	        }

	    } /* end if (procreciploads) */
	    rs1 = vechand_finish(&aa) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vechand) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: procdelivermsg c=%u (%d)\n",pn,c,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/procdelivermsg: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdelivermsg) */


static int procdelivermsgalls(pip,mi,sfd,optp,tlp)
PROGINFO	*pip ;
int		mi ;
int		sfd ;
MSGOPTS		*optp ;
vechand		*tlp ;
{
	LOCINFO		*lip = pip->lip ;
	VECHAND		loglist, *llp = &loglist ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = vechand_start(llp,4,0)) >= 0) {
	    const int	mpargs = lip->maxpostargs ;
	    const int	nbargs = 3 ; /* number of base-arguments */
	    int		size ;
	    cchar	**args = NULL ;

	    size = (1 + nbargs + mpargs + 1) * sizeof(cchar *) ;
	    if ((rs = uc_malloc(size,&args)) >= 0) {
	        const int	n = vechand_count(tlp) ;
	        int		i = 0 ;
	        int		ai = 0 ;
	        int		f = lip->f.usingsendmail ;
	        char		*ap ;

	        args[ai++] = (f) ? lip->sendmail : lip->postmail ;
	        if (lip->f.usingsendmail) { /* extra crap needed for SENDMAIL */
	            args[ai++] = "-oi" ;
	            args[ai++] = "-B" ;
	            args[ai++] = "8BITMIME" ;
	        }
	        args[ai] = NULL ;

	        while ((rs >= 0) && (i < n)) {
	            int	j ;

	            for (j = 0 ; (rs >= 0) && (j < mpargs) ; j += 1) {

	                while ((rs1 = vechand_get(tlp,i++,&ap)) >= 0) {
	                    if (ap != NULL) break ;
	                } /* end while */

	                if (rs1 < 0) break ;

	                args[ai + j] = ap ;
	                rs = vechand_add(llp,ap) ;

	            } /* end for */

	            if (rs >= 0) {

	                args[ai + j] = NULL ;
	                if (j == 0) break ;

	                c += j ;
	                rs = procdeliverenter(pip,sfd,args,j,llp) ;

	            } /* end if */

	            vechand_delall(llp) ;
	        } /* end while */

	        rs1 = uc_free(args) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (memory allocation) */

	    rs1 = vechand_finish(llp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vechand) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdelivermsgalls) */


static int procdelivermsgones(pip,mi,sfd,optp,tlp)
PROGINFO	*pip ;
int		mi ;
int		sfd ;
MSGOPTS		*optp ;
vechand		*tlp ;
{
	LOCINFO		*lip = pip->lip ;
	vechand		baddrs, *blp = &baddrs ;
	const int	nbargs = 3 ; /* number of base-arguments */
	int		rs ;
	int		rs1 ;
	int		atypes[] = { msgloghdr_bcc, -1 } ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    int		i ;
	    cchar	*ap ;
	    debugprintf("b_imail/procdelivermsgones: ent mi=%u\n",mi) ;
	    debugprintf("b_imail/procdelivermsgones: takes:\n") ;
	    for (i = 0 ; vechand_get(tlp,i,&ap) >= 0 ; i += 1) {
	        if (ap == NULL) continue ;
	        debugprintf("b_imail/procdelivermsgones: t=%s\n",ap) ;
	    }
	}
#endif /* CF_DEBUG */

	if ((rs = vechand_start(blp,1,0)) >= 0) {
	    vechand	loglist, *llp = &loglist ;
	    const int	rsn = SR_NOTFOUND ;
	    int		size ;
	    int		ai ;
	    int		i ;
	    cchar	*ap ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/procdelivermsgones: mid1 rs=%d\n",rs) ;
#endif

/* add any BCC "takes" */

	    if (lip->f.take) {
	        if ((rs = procreciploads(pip,blp,mi,atypes)) >= 0) {
	            for (i = 0 ; vechand_get(blp,i,&ap) >= 0 ; i += 1) {
	                if (ap != NULL) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(5))
	                        debugprintf("b_imail/procdelivermsgones: "
				"ap=%s\n",ap) ;
#endif
	                    rs = vechand_search(tlp,ap,vrecipsch,NULL) ;
	                    if (rs >= 0) { /* if found */
	                        vechand_del(blp,i--) ;
	                    } else if (rs == rsn) {
	                        rs = SR_OK ;
	                    }
	                }
	                if (rs < 0) break ;
	            } /* end for */
	        } /* end if */
	    } /* end if (take) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/procdelivermsgones: mid2 rs=%d\n",rs) ;
#endif

/* add any specified recipients (watching out for duplicates) */

	    if (rs >= 0) {
	        osetstr_cur	rcur ;
	        osetstr		*rlp = &lip->recips ;

	        if ((rs = osetstr_curbegin(rlp,&rcur)) >= 0) {
	            while ((rs1 = osetstr_enum(rlp,&rcur,&ap)) >= 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("b_imail/procdelivermsgones: "
	                        "R recip=%s\n", ap) ;
#endif

	                rs1 = rsn ;
	                if (lip->f.take) {
	                    vrecipsch_t	v = vrecipsch ;
	                    if ((rs1 = vechand_search(blp,ap,v,NULL)) == rsn) {
	                        rs1 = vechand_search(tlp,ap,v,NULL) ;
	                    }
	                } /* end if */

	                if (rs1 == rsn) {
	                    rs = vechand_add(blp,ap) ;
	                }

	                if (rs < 0) break ;
	            } /* end while */
	            if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	            rs1 = osetstr_curend(rlp,&rcur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (osetstr-cur) */

/* now: submit a separate message f/e of remaining BCC recipients */

	        if (rs >= 0) {
	            if ((rs = vechand_start(llp,4,0)) >= 0) {
	                cchar	**args = NULL ;
	                size = ((1 + nbargs + 1 + 1) * sizeof(cchar *)) ;
	                if ((rs = uc_malloc(size,&args)) >= 0) {
	                    int	f ;

	                    ai = 0 ;
	                    f = lip->f.usingsendmail ;
	                    args[ai++] = (f) ? lip->sendmail : lip->postmail ;
	                    if (lip->f.usingsendmail) { /* for SENDMAIL */
	                        args[ai++] = "-oi" ;
	                        args[ai++] = "-B" ;
	                        args[ai++] = "8BITMIME" ;
	                    }
	                    args[ai] = NULL ;

	                    for (i = 0 ; vechand_get(blp,i,&ap) >= 0 ; i += 1) {
	                        if ((ap == NULL) || (ap[0] == '\0')) continue ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(5)) {
	                            debugprintf("b_imail/procdelivermsgones: "
	                                "B recip=%s\n",ap) ;
	                        }
#endif

	                        args[ai] = ap ;
	                        args[ai+1] = NULL ;
	                        if ((rs = vechand_add(llp,ap)) >= 0) {
	                            cchar	**rp = NULL ;
	                            c += 1 ;
	                            rs1 = vechand_search(tlp,ap,vrecipsch,rp) ;
	                            if (rs1 >= 0) {
	                                rs = procdeliverenter(pip,sfd,args,1,
	                                    llp) ;
	                            } else {
	                                rs = procdeliverentero(pip,sfd,mi,args,
	                                    ap,1,llp) ;
	                            }
	                            vechand_delall(llp) ;
	                        } /* end if (vechand_add) */

	                        if (rs < 0) break ;
	                    } /* end for */

	                    uc_free(args) ;
	                } /* end if (memory allocation) */
	                vechand_finish(llp) ;
	            } /* end if (vechand) */
	        } /* end if (ok) */

	    } /* end if (ok) */

	    rs1 = vechand_finish(blp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vechand) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procdelivermsgones: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdelivermsgones) */


static int procdeliverentero(pip,sfd,mi,args,ap,n,llp)
PROGINFO	*pip ;
int		sfd ;
int		mi ;
cchar		*args[] ;
cchar		*ap ;
int		n ;
vechand		*llp ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procdeliverentero: ent mi=%u\n",mi) ;
#endif

	if ((rs = u_rewind(sfd)) >= 0) {
	    const int	of = (O_CREAT | O_RDWR) ;
	    char	tfname[MAXPATHLEN + 1] = { 0 } ;

	    if ((rs = locinfo_opentmpfile(lip,tfname,of,"tf")) >= 0) {
	        MSGDATA		*mdp ;
	        const int	tfd = rs ;

	        u_unlink(tfname) ;
	        if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	            const int	hlen = mdp->hlen ;
	            if ((rs = procmsgfilebcc(pip,tfd,sfd,hlen,ap)) >= 0) {
	                wlen += rs ;
	                u_rewind(tfd) ;
	                rs = procdeliverenter(pip,tfd,args,n,llp) ;
	            }
	        }

	        u_close(tfd) ;
	    } /* end if (locinfo_opentmpfile) */

	} /* end if (rewind) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procdeliverentero: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return rs ;
}
/* end subroutine (procdeliverentero) */


static int procdeliverenter(pip,sfd,args,n,llp)
PROGINFO	*pip ;
int		sfd ;
cchar		*args[] ;
int		n ;
VECHAND		*llp ;
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    int		i = 0 ;
	    cchar	*progfname = lip->mailprogfname ;
	    debugprintf("b_imail/procdeliverenter: progfname=%s\n",progfname) ;
	    for (i = 0 ; args[i] != NULL ; i += 1) {
	        debugprintf("b_imail/procdeliverenter: a%u=%s\n",i,args[i]) ;
	    }
	}
#endif /* CF_DEBUG */

	if ((n > 0) && ((rs = u_rewind(sfd)) >= 0)) {
	    const int	of = (O_WRONLY | O_CREAT) ;
	    cchar	*progfname = lip->mailprogfname ;
	    char	ofname[MAXPATHLEN+1] = { 0 } ;
	    if ((rs = locinfo_opentmpfile(lip,ofname,of,"of")) >= 0) {
	        const int	ofd = rs ;
	        char		efname[MAXPATHLEN+1] = { 0 } ;

	        if ((rs = locinfo_opentmpfile(lip,efname,of,"ef")) >= 0) {
	            SPAWNPROC	ps ;
	            const int	efd = rs ;
	            cchar	**envv = pip->envv ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("b_imail/procdeliverenter: "
	                    "spawnproc()\n") ;
#endif

	            memset(&ps,0,sizeof(SPAWNPROC)) ;
	            ps.disp[0] = SPAWNPROC_DDUP ;
	            ps.disp[1] = SPAWNPROC_DDUP ;
	            ps.disp[2] = SPAWNPROC_DDUP ;
	            ps.fd[0] = sfd ;
	            ps.fd[1] = ofd ;
	            ps.fd[2] = efd ;

	            if ((rs = spawnproc(&ps,progfname,args,envv)) >= 0) {
	                const pid_t	pid = rs ;
	                int		cs ;
	                cchar	*strout = "stdout" ;
	                cchar	*strerr = "stderr" ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("b_imail/procdeliverenter: "
	                        "spawnproc() rs=%d\n",rs) ;
#endif

	                if ((rs1 = u_waitpid(pid,&cs,0)) >= 0) {
	                    rs = procwaitok(pip,pid,cs) ;
	                } else {
	                    rs = procwaitbad(pip,pid,rs1) ;
	                } /* end if */

	                if (rs >= 0)
	                    rs = proclogrecips(pip,llp) ;

	                if (rs >= 0)
	                    rs = progdebugout(pip,strout,ofname) ;

	                if (rs >= 0)
	                    rs = progdebugout(pip,strerr,efname) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("b_imail/procdeliverenter: "
	                        "progdebugout() rs=%d\n",rs) ;
#endif

	                if (rs >= 0)
	                    rs = proglogout(pip,strout,ofname) ;

	                if (rs >= 0)
	                    rs = proglogout(pip,strerr,efname) ;

	            } /* end if (spawnproc) */

	            u_close(efd) ;
	            u_unlink(efname) ;
	        } /* end if (locinfo_opentmpfile) */

	        u_close(ofd) ;
	        u_unlink(ofname) ;
	    } /* end if (locinfo_opentmpfile) */
	} /* end if (u_rename) */

	return rs ;
}
/* end subroutine (procdeliverenter) */


static int procwaitok(PROGINFO *pip,pid_t pid,int cs)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if (WIFEXITED(cs)) {

	    if (pip->debuglevel > 0) {
	        fmt = "%s: mailer(%u) exited normally ex=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,
	            pid,WEXITSTATUS(cs)) ;
	    }

	    if (pip->open.logprog) {
	        fmt = "mailer(%u) exited normally ex=%u" ;
	        proglog_printf(pip,fmt,
	            pid,WEXITSTATUS(cs)) ;
	    }

	} else if (WIFSIGNALED(cs)) {

	    if (! pip->f.quiet) {
	        fmt = "%s: mailer(%u) was signalled sig=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,
	            pid,WTERMSIG(cs)) ;
	    }

	    if (pip->open.logprog) {
	        fmt = "mailer(%u) was signalled sig=%u" ;
	        proglog_printf(pip,fmt,pn,
	            pid,WTERMSIG(cs)) ;
	    }

	} else {

	    if (! pip->f.quiet) {
	        fmt = "%s: mailer(%u) exited "
	            "abnormally cs=%u\n" ;
	        shio_printf(pip->efp,fmt,pn,
	            pid,cs) ;
	    }

	    if (pip->open.logprog) {
	        fmt = "mailer(%u) exited abnormally cs=%u" ;
	        proglog_printf(pip,fmt,pid,cs) ;
	    }

	} /* end if */
	return rs ;
}
/* end subroutine (procwaitok) */


static int procwaitbad(PROGINFO *pip,pid_t pid,int rrs)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (! pip->f.quiet) {
	    fmt = "%s: mailer(%u) exited "
	        "w/ unknown disposition (%d)\n",
	        shio_printf(pip->efp,fmt,pn,pid,rrs) ;
	}

	if (pip->open.logprog) {
	    fmt = "mailer(%u) exited "
	        "w/ unknown disposition (%d)",
	        proglog_printf(pip,fmt,pid,rrs) ;
	}

	return rs ;
}
/* end subroutine (procwaitbad) */


static int procsave(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	const int	to = -1 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procsave: ent\n") ;
#endif

	if (lip->f.cmbname) {
	    char	mbfname[MAXPATHLEN + 1] ;
	    if ((rs = locinfo_cmbfname(lip,mbfname)) >= 0) {
	        if (mbfname[0] != '\0') {
	            cchar	*x = "mbtmpXXXXXXXXX" ;
	            char	template[MAXPATHLEN + 1] ;
	            if ((rs = mkpath2(template,pip->tmpdname,x)) >= 0) {
	                const mode_t	om = 0660 ;
	                const int	of = O_RDWR ;
	                char		tbuf[MAXPATHLEN + 1] ;
	                if ((rs = opentmpfile(template,of,om,tbuf)) >= 0) {
	                    MSGOPTS	mo ;
	                    const int	sfd = rs ;

	                    uc_unlink(tbuf) ;

	                    msgopts_all(&mo) ;
	                    mo.reqsubj = lip->f.reqsubj ;
	                    mo.mkxuuid = lip->f.addxuuid ;
	                    mo.mkxnote = lip->f.addnote ;
	                    mo.mkxmcdate = lip->f.addxmcdate ;
	                    if ((rs = procextract(pip,sfd,&mo)) > 0) {
	                        wlen = rs ;
	                        u_rewind(sfd) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("b_imail/procsave: "
	                                "mbfname=%s\n", mbfname) ;
#endif

	                        rs = mailboxappend(mbfname,sfd,-1,to) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("b_imail/procsave: "
	                                "mailboxappend() rs=%d\n", rs) ;
#endif

	                    } /* end if */

	                    rs1 = u_close(sfd) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (opentmpfile) */
	            } /* end if (mkpath) */
	        } /* end if (mvfname) */
	    } /* end if (locinfo_cmbfname) */
	} /* end if (cmbname) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procsave: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsave) */


/* ARGSUSED */
static int procuserbox(PROGINFO *pip,int bn,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = locinfo_folderdname(lip)) >= 0) {
	    cchar	*dname = lip->folderdname ;
	    char	mbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(mbuf,dname,userboxes[bn])) >= 0) {
	        rs = procdisposefile(pip,mbuf,-1,mop) ;
	    } /* end if (mkpath) */
	} /* end if (locinfo_folderdname) */
	return rs ;
}
/* end subroutine (procuserbox) */


static int procdisposefile(PROGINFO *pip,char *mbuf,int to,MSGOPTS *mop)
{
	int		rs ;
	int		wlen = 0 ;
	if ((rs = opentmp(NULL,0,0)) >= 0) {
	    const int	sfd = rs ;
	    if ((rs = procextract(pip,sfd,mop)) > 0) {
	        wlen = rs ;
	        if ((rs = u_rewind(sfd)) >= 0) {
	            rs = mailboxappend(mbuf,sfd,-1,to) ;
	        } /* end if */
	    } /* end if (procextract) */
	    u_close(sfd) ;
	} /* end if (opentmp) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdisposefile) */


static int procextract(PROGINFO *pip,int sfd,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	FILEBUF		fb ;
	int		rs ;
	int		rs1 ;
	int		ps ;
	int		wlen = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procextract: ent\n") ;
#endif
	ps = lip->pagesize ;
	if ((rs = filebuf_start(&fb,sfd,0,ps,0)) >= 0) {
	    const int	n = lip->nmsgs ;
	    int		i ;
	    for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	        rs = procextracter(pip,&fb,i,mop) ;
	        wlen += rs ;
	    } /* end for */
	    rs1 = filebuf_finish(&fb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procextract: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procextract) */


static int procextracter(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	MSGDATA		*mdp ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	    if ((rs = msgdata_getsubject(mdp,NULL)) >= 0) {
	        const int	f_subj = rs ;
	        if ((rs = msgdata_getnrecips(mdp)) >= 0) {
	            const int	nr = rs ;
	            if (mop->nosubj || mop->norecip) {
	                int	f = FALSE ;
	                f = f || ((! f_subj) && mop->nosubj) ;
	                f = f || ((nr == 0) && mop->norecip) ;
	                if (f) {
	                    rs = procmsg(pip,fbp,mi,mop) ;
	                    wlen += rs ;
	                }
	            } else {
	                int	f = TRUE ;
	                f = f && (f_subj || (! mop->reqsubj)) ;
	                f = f && (nr > 0) ;
	                if (f) {
	                    rs = procmsg(pip,fbp,mi,mop) ;
	                    wlen += rs ;
	                }
	            }
	        } /* end if (msgdata_getnrecips) */
	    } /* end if (msgdata_getsubject) */
	} /* end if (locinfo_msgdataget) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procextracter) */


static int procmsg(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	cchar		*n = "procmsg" ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsg: ent mi=%u\n",mi) ;
#endif

/* process the envelope (if any) */

	if (rs >= 0) {
	    rs = procmsgenv(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	if (DEBUGLEVEL(4))
	    debugfilebuf_printoff(fbp,n,"env",wlen) ;
#endif

/* process the headers (if any) */

	if (rs >= 0) {
	    rs = procmsghdrs(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsg: procmsghdrs() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	if (DEBUGLEVEL(4))
	    debugfilebuf_printoff(fbp,n,"hdrs",wlen) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsg: hlen=%lu\n",wlen) ;
#endif

/* continue w/ message body */

	if (rs >= 0) {
	    MSGDATA	*mdp ;
	    if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	        if ((rs = msgdata_sethlen(mdp,wlen)) >= 0) {
	            if ((rs = filebuf_print(fbp,NULL,0)) >= 0) { /* EOH */
	                wlen += rs ;
	                rs = procmsgbody(pip,fbp,mi) ;
	                wlen += rs ;
	            }
	        }
	    }
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_imail/procmsg: procmsgbody() rs=%d\n",rs) ;
	    debugprintf("b_imail/procmsg: ret rs=%d wlen=%u\n",rs,wlen) ;
	}
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsg) */


#if	CF_MSGENV

/* process MSG environment information */
static int procmsgenv(pip,fbp,mi,optp)
PROGINFO	*pip ;
MSGOPTS		*optp ;
FILEBUF		*fbp ;
int		mi ;
{
	LOCINFO		*lip = pip->lip ;
	struct msgenv	me, *mep = &me ;
	MAILMSGSTAGE	*msp ;
	DATER		*edp = &mip->edate ;
	int		rs = SR_OK ;
	int		i, sl ;
	int		cl ;
	int		ml ;
	int		al = 0 ;
	int		abl = addrlen, sabl ;
	int		sal = 0 ;
	int		wlen = 0 ;
	int		f_remote ;
	cchar		*cp ;
	char		*ap = addrbuf ;
	char		*sap = addrbuf ;
	char		*addr = NULL ;

	mip->e_from[0] = '\0' ;

	sal = 0 ;
	for (i = 0 ; mailmsg_envget(msgp,i,mep) >= 0 ; i += 1) {
	    EMAINFO	ai ;
	    int		atype ;
	    int		froml = -1 ;
	    char	datebuf[DATEBUFLEN + 1] ;
	    cchar	*fromp = NULL ;

	    if (pip->open.logprogenv) {

	        cp = mep->a.ep ;
	        cl = mep->a.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/procmsgenv: env a=>%t<\n",cp,cl) ;
#endif

	        logfile_printf(&pip->envsum,"%4d:%2d F %t",
	            pip->msgn,i,
	            ((cp != NULL) ? cp : "*NA*"),cl) ;

	        cp = mep->d.ep ;
	        cl = mep->d.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/procmsgenv: env d=>%t<\n",cp,cl) ;
#endif

	        logfile_printf(&pip->envsum,"%4d:%2d D %t",
	            pip->msgn,i,
	            cp,cl) ;

	        cp = mep->r.ep ;
	        cl = mep->r.el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/procmsgenv: env r=>%t<\n",cp,cl) ;
#endif

	        if ((cp != NULL) && (cp[0] != '\0')) {
	            logfile_printf(&pip->envsum,"%4d:%2d R %t",
	                pip->msgn,i,
	                cp,cl) ;
		}

	    } /* end if (special environment logging) */

	    fromp = mep->a.ep ;
	    froml = mep->a.el ;
	    if ((fromp == NULL) || (fromp[0] == '\0')) {
	        fromp = "mailer-daemon" ;
	        froml = -1 ;
	    }

	    if ((rs = dater_setstd(edp,mep->d.ep,mep->d.el)) >= 0) {
	        datebuf[0] = '\0' ;
	        rs = dater_mkstrdig(edp,datebuf,DATEBUFLEN) ;
	    }

	    if ((i > 0) && (abl < addrlen)) {
	        *ap++ = '!' ;
	        sal += 1 ;
	        abl -= 1 ;
	    }

	    addr = ap ;
	    al = 0 ;

	    sap = ap ;
	    sabl = abl ;

	    f_remote = ((mep->r.ep != NULL) && (mep->r.ep[0] != '\0')) ;
	    if (f_remote) {

	        ml = MIN(mep->r.el,(abl-1)) ;
	        sl = strwcpy(ap,mep->r.ep,ml) - ap ;

	        ap += sl ;
	        al += sl ;
	        sal += sl ;
	        abl -= sl ;

	        sap = ap ;
	        sabl = abl ;

	        *ap++ = '!' ;
	        al += 1 ;
	        abl -= 1 ;

	    } /* end if (remote node name) */

	    atype = emainfo(&ai,fromp,froml) ;

	    sl = emainfo_mktype(&ai,EMAINFO_TUUCP,ap,abl) ;

	    if (sl > 0) {
	        ap += sl ;
	        al += sl ;
	        abl -= sl ;
	    }

	    if (pip->open.logprog && pip->f.logprogmsg) {
	        proglog_printf(pip,"  | %-25s", datebuf) ;
	        proglog_printf(pip,"  |   %c %t",
	            atypes[atype],addr,MIN(al,(LOGLINELEN - TABLEN))) ;
	    }

	    ap = sap ;
	    abl = sabl ;

	    if (rs < 0) break ;
	} /* end for (looping through envelopes) */

	if ((rs >= 0) && (i > 0)) {

	    sal += (sl + ((f_remote) ? 1 : 0)) ;

	    addrbuf[sal] = '\0' ;
	    strwcpy(mip->e_from,addrbuf,MAILADDRLEN) ;

	    if (pip->open.logprog && pip->f.logprogmsg) {
	        proglog_printf(pip,"  > %t",
	            addrbuf,MIN(sal,(LOGLINELEN - 4))) ;
	    }

	    dater_gettime(&mip->edate,&mip->etime) ;

	} /* end if */

	return (i > 0) ? wlen : 0 ;
}
/* end subroutine (procmsgenv) */

#else /* CF_MSGENV */

static int procmsgenv(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		len = -1 ;
	int		wlen = 0 ;

	if (optp->mkenv) {
	    if ((rs = locinfo_mkenv(lip)) >= 0) {
	        len = rs ;
		if (lip->env != NULL) {
	    	    rs = filebuf_write(fbp,lip->env,len) ;
	    	    wlen += rs ;
		}
	    }
	} /* end if (option) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsgenv) */

#endif /* CF_MSGENV */


static int procmsghdrs(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	cchar		*n = "procmsghdrs" ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_path(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	if (DEBUGLEVEL(4))
	    debugfilebuf_printoff(fbp,n,"rpath",wlen) ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_recv(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	if (DEBUGLEVEL(4))
	    debugfilebuf_printoff(fbp,n,"recv",wlen) ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_org(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}
	if (rs >= 0) {
	    rs = procmsghdr_mid(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}
	if (rs >= 0) {
	    rs = procmsghdr_xpri(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
	if (DEBUGLEVEL(4))
	    debugfilebuf_printoff(fbp,n,"mid",wlen) ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_xm(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = procmsghdr_all(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = procmsghdr_clines(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = procmsghdr_clen(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = procmsghdr_singles(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = procmsghdr_rto(pip,fbp,mi,optp) ; /* reply-to */
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = procmsghdr_eto(pip,fbp,mi,optp) ; /* errors-to */
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: procmsghdr_eto()\n") ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_sender(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: procmsghdr_sender()\n") ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_from(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: procmsghdr_from()\n") ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_addrs(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: procmsghdr_addrs() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_date(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: procmsghdr_date() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_subj(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: procmsghdr_subj() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = procmsghdr_specials(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdrs: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdrs) */


static int procmsghdr_specials(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	if (rs >= 0) {
	    rs = procmsghdr_xuuid(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}
	if (rs >= 0) {
	    rs = procmsghdr_xuti(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}
	if (rs >= 0) {
	    rs = procmsghdr_xmcdate(pip,fbp,mi,optp) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_specials) */


static int procmsgbody(PROGINFO *pip,FILEBUF *fbp,int mi)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	offset_t	boff = 0 ;
	const int	xlen = 0x10000 ;
	int		rs = SR_OK ;
	int		bl ;
	int		mlen ;
	int		wlen = 0 ;
	cchar		*bp = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsgbody: ent mi=%u\n",mi) ;
#endif

	msp = &lip->ms ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsgbody: filebuf_print() rs=%d\n",rs) ;
#endif

	while ((rs = mailmsgstage_bodyget(msp,mi,boff,&bp)) > 0) {
	    bl = rs ;

	    mlen = MIN(bl,xlen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("b_imail/procmsgbody: "
	            "mailmsgstage_bodyget() bl=%u\n", bl) ;
	        debugprintf("b_imail/procmsgbody: mlen=%u\n",mlen) ;
	    }
#endif

	    rs = filebuf_write(fbp,bp,mlen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_imail/procmsgbody: filebuf_write() rs=%d\n",
	            rs) ;
#endif

	    if (rs < 0)
	        break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        cchar	*tp ;
	        int	tlen = mlen ;
	        if ((tp = strnchr(bp,mlen,'\n')) != NULL) {
	            tlen = ((tp + 1) - bp) ;
	        }
	        debugprintf("b_imail/procmsgbody: b=>%t<\n",
	            bp,strlinelen(bp,tlen,50)) ;
	    }
#endif /* CF_DEBUG */

	    boff += mlen ;
	    wlen += mlen ;

	} /* end while (body-get) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsgbody: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsgbody) */


/* we always add a MSGID if there is not one already */
static int procmsghdr_mid(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MSGDATA		*mdp = NULL ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    const int		ilen = MAILADDRLEN ;
	    int			sl ;
	    cchar		*sp ;
	    char		ibuf[MAILADDRLEN+1] = { 0 } ;

	    if (! lip->f.addnote) {
	        int		hl = 0 ;
	        cchar		*hp ;
	        cchar		*kn = HN_MESSAGEID ;
	        if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	            hl = rs ;
	            sp = ibuf ;
	            if ((rs = hdrextid(ibuf,ilen,hp,hl)) > 0) {
	                sl = rs ;
	                if ((rs = procmsghdr(pip,fbp,mi,kn,FALSE)) >= 0) {
	                    wlen += rs ;
	                    if (mdp->hdrmid == NULL) {
	                        rs = msgdata_setmid(mdp,sp,sl) ;
	                    }
	                }
	            } /* end if (hdrextid) */
	        } else if (isHdrEmpty(rs)) {
	            rs = SR_OK ;
	        } /* end if (had it) */
	    } /* end if (not in "node" mode) */

/* get the stored one if we do not already have it */

	    if ((rs >= 0) && (ibuf[0] == '\0')) {

	        sp = mdp->hdrmid ;
	        sl = -1 ;
	        if (sp == NULL) { /* make it and store it */
	            sp = ibuf ;
	            if ((rs = locinfo_mkmid(lip,ibuf,ilen)) >= 0) {
	                sl = rs ;
	                if (ibuf[0] != '\0') {
	                    rs = msgdata_setmid(mdp,sp,sl) ;
	                }
	            } /* end if (locinfo_mkmid) */
	        } /* end if (created) */

	        if (rs >= 0) {
	            rs = procprinthdr_mid(pip,fbp,sp,sl) ;
	            wlen += rs ;
	        }

	    } /* end if */

	} /* end if (locinfo_msgdataget) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_mid) */


static int procmsghdr_xpri(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*kn = HN_XPRIORITY ;
	cchar		*hp ;

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    rs = 1 ;
	} else if (isHdrEmpty(rs)) {
	    rs = procprinthdr_xpri(pip,fbp) ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_xpri) */


/* only a final delivery agent adds the return-path */
/* ARGSUSED */
static int procmsghdr_path(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#ifdef	COMMENT
	cchar		*kn = HN_RETURNPATH ;
#endif

	if (fbp == NULL) return SR_FAULT ;

	msp = &lip->ms ;
	if (optp->mkretpath) {
	    rs = 1 ;
	    if (msp == NULL) rs = SR_FAULT ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_path) */


static int procmsghdr_recv(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	int		rs ;
	int		wlen = 0 ;
	cchar		*kn = HN_RECEIVED ;

	rs = procmsghdr(pip,fbp,mi,kn,FALSE) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_recv) */


static int procmsghdr_org(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("procmsghdr_org: ent\n") ;
#endif

	if (lip->f.org) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    cchar		*kn = HN_ORGANIZATION ;
	    cchar		*hp ;

	    if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	        rs = 1 ;
	    } else if (isHdrEmpty(rs)) {
	        if ((rs = loadorg(pip)) > 0) {
	            int		ol = rs ;
	            cchar	*op = pip->org ;
	            if (pip->org != NULL) {
	                while (ol && ishigh(op[ol-1])) ol -= 1 ;
	                if (ol > 0) {
	                    rs = procprinthdr(pip,fbp,kn,pip->org,ol) ;
	                    wlen += rs ;
	                }
	            }
	        }
	    } /* end if (not present) */
	} /* end if (wanted) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_org) */


static int procmsghdr_all(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	NULSTR		ns ;
	int		rs = SR_OK ;
	int		i ;
	int		kl ;
	int		wlen = 0 ;
	cchar		*kn = NULL ;
	cchar		*kp ;

	msp = &lip->ms ;
	for (i = 0 ; (kl = mailmsgstage_hdrikey(msp,mi,i,&kp)) > 0 ; i += 1) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_imail/procmsghdr_all: hdr=%t\n",kp,kl) ;
#endif

	    if (matcasestr(skiphdrs,kp,kl) < 0) {
	        if ((rs = nulstr_start(&ns,kp,kl,&kn)) >= 0) {
	            const int	f_nz = (matcasestr(chewhdrs,kp,kl) >= 0) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_imail/procmsghdr_all: hdr=%t f_nz=%u\n",
	                    kp,kl,f_nz) ;
#endif

	            rs = procmsghdr(pip,fbp,mi,kn,f_nz) ;
	            wlen += rs ;

	            nulstr_finish(&ns) ;
	        } /* end if (nulstr) */
	    } /* end if (header not being ignored) */

	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/procmsghdr_all: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_all) */


static int procmsghdr_clines(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (optp->mkclines) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    cchar		*kn = HN_CLINES ;
	    cchar		*hp ;

	    if ((rs = mailmsgstage_flags(msp,mi)) >= 0) {
	        int	mflags = rs ;
	        int	v ;
	        int	f = FALSE ;
	        f = f || (mflags & MAILMSGSTAGE_MCLINES) ;
	        f = f || (mflags & MAILMSGSTAGE_MCPLAIN) ;
	        if (f) {
	            if ((v = mailmsgstage_clines(msp,mi)) >= 0) {
	                const int	diglen = DIGBUFLEN ;
	                char		digbuf[DIGBUFLEN + 1] ;
	                hp = digbuf ;
	                if ((rs = ctdeci(digbuf,diglen,v)) >= 0) {
	                    rs = procprinthdr(pip,fbp,kn,hp,rs) ;
	                    wlen += rs ;
	                }
	            }
	        } /* end if (do it) */
	    } /* end if (get-flags) */

	} /* end if (wanted) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_clines) */


static int procmsghdr_clen(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (optp->mkclen) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    const int		diglen = DIGBUFLEN ;
	    int			v ;
	    char		digbuf[DIGBUFLEN + 1] ;

	    if ((v = mailmsgstage_clen(msp,mi)) >= 0) {
	        cchar	*kn = HN_CLEN ;
	        cchar	*hp = digbuf ;
	        if ((rs = ctdeci(digbuf,diglen,v)) >= 0) {
	            rs = procprinthdr(pip,fbp,kn,hp,rs) ;
	            wlen += rs ;
	        }
	    } /* end if (mailmsgstage_clen) */

	} /* end if (needed) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_clen) */


static int procmsghdr_singles(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs = SR_OK ;
	int		hi ;
	int		i, j ;
	int		hl ;
	int		wlen = 0 ;
	cchar		*kn ;
	cchar		*hp ;

	msp = &lip->ms ;

	for (j = 0 ; msghdrsingles[j] >= 0 ; j += 1) {
	    hi = msghdrsingles[j] ;
	    kn = mailmsghdrs_names[hi] ;

/* grab only one header (the first one that is non-zero) */

	    hl = 0 ;
	    for (i = 0 ; rs >= 0 ; i += 1) {
	        hl = mailmsgstage_hdrival(msp,mi,kn,i,&hp) ;
	        if (hl == SR_NOTFOUND) break ;
	        if (hl > 0) break ;
	        rs = hl ;
	    } /* end for */

	    if ((rs >= 0) && (hl > 0)) {
	        rs = procmsginsthdr(pip,fbp,mi,kn,i,TRUE) ;
	        wlen += rs ;
	    } /* end if (had it) */

	    if (rs < 0) break ;
	} /* end for (looping through msg-hdr-singles array) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_singles) */


/* reply-to */
static int procmsghdr_rto(PROGINFO *pip,FILEBUF *ofp,int mi,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = locinfo_isnotdisabled(lip,0)) > 0) {
	    int		hl = -1 ;
	    cchar	*kn = HN_REPLYTO ;
	    cchar	*var1 = VARMAILREPLYTO ;
	    cchar	*var2 = VARMAILREPLY ;
	    cchar	*hp ;
	    if (lip->hdraddr_replyto != NULL) {
	        hp = lip->hdraddr_replyto ;
	        if (hp[0] == '+') {
	            if ((rs = locinfo_mkhdraddrfrom(lip)) >= 0) {
	                hl = rs ;
	                hp = lip->hdraddr_from ;
	            }
	        }
	        if ((rs >= 0) && (hp[0] != '\0')) {
	            rs = procprinthdr_addrsome(pip,ofp,kn,hp,hl,-1) ;
	            wlen += rs ;
	        }
	    } else {
	        MAILMSGSTAGE	*msp = &lip->ms ;
	        if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	            rs = procprinthdr_addrsome(pip,ofp,kn,hp,rs,-1) ;
	            wlen += rs ;
	        } else if (isHdrEmpty(rs)) {
	            rs = SR_OK ;
	            hp = NULL ;
	            if (hp == NULL) hp = getourenv(pip->envv,var1) ;
	            if (hp == NULL) hp = getourenv(pip->envv,var2) ;
	            if (hp != NULL) {
	                hl = -1 ;
	                rs = procprinthdr_addrsome(pip,ofp,kn,hp,hl,-1) ;
	                wlen += rs ;
	            }
	        } /* end if (no existing header) */
	    }
	} /* end if (locinfo_isnotdisabled) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_rto) */


/* errors-to */
static int procmsghdr_eto(PROGINFO *pip,FILEBUF *ofp,int mi,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = locinfo_isnotdisabled(lip,1)) > 0) {
	    int		hl = -1 ;
	    cchar	*kn = HN_ERRORSTO ;
	    cchar	*var = VARMAILERRORSTO ;
	    cchar	*hp ;
	    if (lip->hdraddr_errorsto != NULL) {
	        hp = lip->hdraddr_errorsto ;
	        if (hp[0] == '+') {
	            if ((rs = locinfo_mkhdraddrfrom(lip)) >= 0) {
	                hl = rs ;
	                hp = lip->hdraddr_from ;
	            }
	        }
	        if ((rs >= 0) && (hp[0] != '\0')) {
	            rs = procprinthdr_addrsome(pip,ofp,kn,hp,hl,-1) ;
	            wlen += rs ;
	        }
	    } else {
	        MAILMSGSTAGE	*msp = &lip->ms ;
	        if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	            rs = procprinthdr_addrsome(pip,ofp,kn,hp,rs,-1) ;
	            wlen += rs ;
	        } else if (isHdrEmpty(rs)) {
	            rs = SR_OK ;
	            if ((hp = getourenv(pip->envv,var)) != NULL) {
	                hl = -1 ;
	                rs = procprinthdr_addrsome(pip,ofp,kn,hp,hl,-1) ;
	                wlen += rs ;
	            }
	        } /* end if (no existing header) */
	    }
	} /* end if (locinfo_isnotdisabled) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_eto) */


static int procmsghdr_sender(PROGINFO *pip,FILEBUF *ofp,int mi,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	MSGDATA		*mdp ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	    if ((rs = msgdata_getnsenders(mdp)) > 0) {
	        if ((rs = msgdata_getnvsenders(mdp)) > 0) {
	            rs = procmsghdr_vsenders(pip,ofp,mdp,mop) ;
	            wlen += rs ;
	        }
	    } else if (rs == 0) {
	        if (lip->f.addsender && (! mdp->f.disallowsender)) {
	            if ((rs = locinfo_mkhdrsender(lip)) >= 0) {
	                const int	hl = rs ;
	                cchar		*kn = HN_SENDER ;
	                cchar		*hp = lip->hdraddr_sender ;
	                if (hp != NULL) {
	                    rs = procprinthdr_addrsome(pip,ofp,kn,hp,hl,1) ;
	                    wlen += rs ;
	                }
	            } /* end if (locinfo_mkhdrsender) */
	        } /* end if (add-sender) */
	    } /* end if (msgdata_getnsenders) */
	} /* end if (locinfo_msgdataget) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_sender) */


#if	CF_VSENDERS
static int procmsghdr_vsenders(PROGINFO *pip,FILEBUF *ofp,MSGDATA *mdp,
	MSGOPTS *mop)
{
	int		rs ;
	int		wlen = 0 ;
	if ((rs = msgdata_getnvsenders(mdp)) > 0) {
	    EMA		*slp = (mdp->addrs + msgloghdr_sender) ;
	    EMA_ENT	*ep ;
	    int		i ;
	    cchar	*kn = HN_SENDER ;
	    for (i = 0 ; rs >= 0 ; i += 1) {
	        if ((rs = msgdata_isvsender(mdp,i)) > 0) {
	            if ((rs = ema_get(slp,i,&ep)) >= 0) {
	                rs = procprinthdr_emaone(pip,ofp,kn,ep) ;
	                wlen += rs ;
	            }
	        } /* end if (msgdara_isvsender) */
	        if (lip->f.onesender && (wlen > 0)) break ;
	    } /* end for */
	} /* end if (msgdara_getnvsenders) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_vsenders) */
static int procprinthdr_emaone(PROGINFO *pip,FILEBUF *ofp,cchar *kn,EMA_ENT *ep)
{
	LOCINFO		*pip = lip->pip ;
	OUTEMA		ld ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = outema_start(&ld,ofp,lip->msgcols)) >= 0) {
	    cchar	*kn = HN_SENDER ;
	    if ((rs = outema_hdrkey(&ld,kn)) >= 0) {
	        rs = outema_ent(&ld,ep) ;
	    }
	    rs1 = outema_finish(&ld) ;
	    if (rs >= 0) rs = rs1 ;
	    wlen += rs1 ;
	} /* end if (outema) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_emaone) */
#else /* CF_VSENDERS */
static int procmsghdr_vsenders(PROGINFO *pip,FILEBUF *ofp,MSGDATA *mdp,
	MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = msgdata_getnsenders(mdp)) > 0) {
	    EMA		*slp = (mdp->addrs + msgloghdr_sender) ;
	    EMA_ENT	*ep ;
	    if ((rs = ema_count(slp)) > 0) {
	        OUTEMA		ld ;
	        const int	msgcols = lip->msgcols ;
	        if ((rs = outema_start(&ld,ofp,msgcols)) >= 0) {
	            cchar	*kn = HN_SENDER ;
	            if ((rs = outema_hdrkey(&ld,kn)) >= 0) {
	                int	i ;
	                int	c = 0 ;
	                for (i = 0 ; ema_get(slp,i,&ep) >= 0 ; i += 1) {
	                    if ((ep != NULL) && (ep->ol > 0)) {
	                        if ((rs = msgdata_isvsender(mdp,i)) > 0) {
	                            if ((rs = outema_ent(&ld,ep)) > 0) {
	                                c += 1 ;
	                            }
	                        }
	                    }
	                    if (lip->f.onesender && (c > 0)) break ;
	                    if (rs < 0) break ;
	                } /* end for */
	            } /* end if (hdr-key) */
	            rs1 = outema_finish(&ld) ;
	            if (rs >= 0) rs = rs1 ;
	            wlen += rs1 ;
	        } /* end if (outema) */
	    } /* end if (non-zero entries) */
	} /* end if (msgdara_getnsenders) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_vsenders) */
#endif /* CF_VSENDERS */


static int procmsghdr_from(PROGINFO *pip,FILEBUF *ofp,int mi,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*kn = HN_FROM ;
	cchar		*hp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_from: ent mi=%u\n",mi) ;
#endif

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    if (lip->f.onefrom) {
	        rs = procprinthdr_addrsome(pip,ofp,kn,hp,rs,1) ;
	        wlen += rs ;
	    } else {
	        rs = procprinthdr_addrs(pip,ofp,msp,mi,kn) ;
	        wlen += rs ;
	    }
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	    if (mop->mkfrom) {
	        if ((rs = locinfo_mkhdrfrom(lip)) > 0) {
	            EMA		*elp = &lip->hdrfroms ;
	            const int	m = (lip->f.onefrom) ? 1 : -1 ;
	            rs = procprinthdr_ema(pip,ofp,kn,elp,m) ;
	            wlen += rs ;
	        }
	    }
	} /* end if (had it) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_from) */


static int procmsghdr_addrs(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	const int	hdraddridx[] = { HI_TO, HI_CC, -1 } ;
	int		rs = SR_OK ;
	int		i, hi ;
	int		wlen = 0 ;
	cchar		*kn ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_addrs: ent\n") ;
#endif /* CF_DEBUG */

	msp = &lip->ms ;

	for (i = 0 ; (hi = hdraddridx[i]) >= 0 ; i += 1) {
	    kn = mailmsghdrs_names[hi] ;
	    rs = procprinthdr_addrs(pip,fbp,msp,mi,kn) ;
	    wlen += rs ;
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_addrs: mid rs=%d \n",rs) ;
#endif /* CF_DEBUG */

	if ((rs >= 0) && optp->mkbcc) {
	    rs = procmsghdr_addrsbcc(pip,fbp,mi) ;
	    wlen += rs ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_addrs: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif /* CF_DEBUG */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_addrs) */


static int procmsghdr_addrsbcc(PROGINFO *pip,FILEBUF *fbp,int mi)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	const int	atypes[] = {
	            msgloghdr_to, msgloghdr_cc, msgloghdr_bcc, -1 } ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*kn = HN_BCC ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_addrsbcc: ent\n") ;
#endif

	msp = &lip->ms ;
	if ((rs = procprinthdr_addrs(pip,fbp,msp,mi,kn)) >= 0) {
	    vechand	aa ;
	    wlen += rs ;

	    if ((rs = vechand_start(&aa,0,0)) >= 0) {

	        if ((rs = procreciploads(pip,&aa,mi,atypes)) >= 0) {
	            rs = procprinthdr_bcc(pip,fbp,&aa) ;
	            wlen += rs ;
	        }

	        rs1 = vechand_finish(&aa) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (aa) */

	} /* end if (procprinthdr_addrs) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_addrsbcc: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_addrsbcc) */


static int procreciploads(PROGINFO *pip,vechand *ehp,int mi,const int *atypes)
{
	LOCINFO		*lip = pip->lip ;
	MSGDATA		*mdp = NULL ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procreciploads: ent\n") ;
#endif

	if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	    int	i ;
	    for (i = 0 ; atypes[i] >= 0 ; i += 1) {
	        const int	ai = atypes[i] ;
	        rs = procrecipload(pip,ehp,mdp,ai) ;
	        c += rs ;
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procreciploads: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procreciploads) */


static int procrecipload(PROGINFO *pip,vechand *ehp,MSGDATA *mdp,int ai)
{
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procrecipload: ent ai=%u\n",ai) ;
#endif

	rs = procreciploademas(pip,ehp,(mdp->addrs + ai)) ;
	c += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procrecipload: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procrecipload) */


/* this subroutine is called recursively */
static int procreciploademas(PROGINFO *pip,vechand *ehp,EMA *elp)
{
	EMA_ENT		*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	for (i = 0 ; ema_get(elp,i,&ep) >= 0 ; i += 1) {
	    rs = procreciploadema(pip,ehp,ep) ;
	    c += rs ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procreciploademas) */


/* this subroutine is called recursively */
static int procreciploadema(PROGINFO *pip,vechand *ehp,EMA_ENT *ep)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*ap ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procreciploadema: ent\n") ;
#endif

	ap = ep->rp ;
	if (ap == NULL) ap = ep->ap ;

	if (ap != NULL) {
	    const int	rsn = SR_NOTFOUND ;
	    if ((rs = vechand_search(ehp,ap,vrecipsch,NULL)) == rsn) {
	        c += 1 ;
	        rs = vechand_add(ehp,ap) ;
	    }
	}

	if ((rs >= 0) && (ep->listp != NULL)) {
	    rs = procreciploademas(pip,ehp,ep->listp) ;
	    c += rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procreciploadema: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procreciploadema) */


static int procmsghdr_date(pip,fbp,mi,optp)
PROGINFO	*pip ;
MSGOPTS		*optp ;
FILEBUF		*fbp ;
int		mi ;
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs = SR_OK ;
	int		hl = 0 ;
	int		i ;
	int		wlen = 0 ;
	cchar		*kn = HN_DATE ;
	cchar		*hp ;

	msp = &lip->ms ;

/* grab only one DATE header (the first one that is non-zero) */

	for (i = 0 ; (rs >= 0) && (hl == 0) ; i += 1) {
	    hl = mailmsgstage_hdrival(msp,mi,kn,i,&hp) ;
	    if (hl == SR_NOTFOUND) break ;
	    rs = hl ;
	} /* end for */

	if ((rs >= 0) && (hl > 0)) {

#ifdef	COMMENT
	    rs = procmsghdr(pip,fbp,mi,kn,FALSE) ;
	    wlen += rs ;
#else
	    rs = procprinthdr(pip,fbp,kn,hp,hl) ;
	    wlen += rs ;
#endif /* COMMENT */

	} /* end if (had it) */

/* add one if we did not have it */

	if ((rs >= 0) && (hl <= 0)) {
	    if ((rs = locinfo_mkhdrdate(lip)) >= 0) {
	        hl = rs ;
	        hp = lip->hdrdate ;
	        rs = procprinthdr(pip,fbp,kn,hp,hl) ;
	        wlen += rs ;
	    }
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_date) */


static int procmsghdr_subj(pip,ofp,mi,mop)
PROGINFO	*pip ;
MSGOPTS		*mop ;
FILEBUF		*ofp ;
int		mi ;
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		hl = 0 ;
	int		wlen = 0 ;
	int		f_extra = (mop->mksubj || mop->mkxnote) ;
	cchar		*kn = HN_SUBJECT ;
	cchar		*hp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_subj: ent f_mksubj=%u\n",
	        mop->mksubj) ;
#endif

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    hl = rs ;
	    if (mop->mkxnote) {
	        cchar	*mn = SUBJ_MAILNOTE ;
	        if (hasMailNote(hp,hl,mn)) {
	            rs = procprinthdr(pip,ofp,kn,hp,hl) ;
	            wlen += rs ;
	        } else {
	            rs = procprinthdr_mailnote(pip,ofp,kn,hp,hl) ;
	            wlen += rs ;
	        }
	    } else {
	        rs = procmsghdr(pip,ofp,mi,kn,FALSE) ;
	        wlen += rs ;
	    }
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	    if (f_extra) {
	        hp = kn ;
	        hl = 0 ;
	        if (lip->hdrsubject != NULL) {
	            if ((hl = sfshrink(lip->hdrsubject,-1,&hp)) > 0) {
	                hp = lip->hdrsubject ;
	            }
	        }
	        if (mop->mkxnote) {
	            rs = procprinthdr_mailnote(pip,ofp,kn,hp,hl) ;
	            wlen += rs ;
	        } else if (hl > 0) {
	            rs = procprinthdr(pip,ofp,kn,hp,hl) ;
	            wlen += rs ;
	        }
	        if ((rs >= 0) && (hl > 0)) {
	            MSGDATA	*mdp ;
	            if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	                rs = msgdata_setsubject(mdp) ;
	            }
	        } /* end if (procprinthdr) */
	    } /* end if (extra) */
	} /* end if (alternate subject given) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_subj: mid2 rs=%d hl=%d\n",rs,hl) ;
#endif

	if ((rs >= 0) && (hl <= 0) && lip->f.reqsubj) {
	    lip->f.notsubj = TRUE ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/procmsghdr_subj: DELIVER=0\n") ;
#endif
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr_subj: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_subj) */


static int procmsghdr_xuuid(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs = SR_OK ;
	int		hl ;
	int		wlen = 0 ;
	cchar		*kn = HN_XUUID ;
	cchar		*hp ;

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    rs = procmsghdr(pip,fbp,mi,kn,FALSE) ;
	    wlen += rs ;
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	    if (optp->mkxuuid) {
	        if ((rs = locinfo_mkhdrxuuid(lip)) >= 0) {
	            hp = lip->hdruuid ;
	            hl = rs ;
	            rs = procprinthdr(pip,fbp,kn,hp,hl) ;
	            wlen += rs ;
	        }
	    }
	} /* end if (alternate subject given) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_xuuid) */


static int procmsghdr_xmcdate(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		hl ;
	int		wlen = 0 ;
	cchar		*kn = HN_XMCDATE ;
	cchar		*hp ;

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    rs = procmsghdr(pip,fbp,mi,kn,FALSE) ;
	    wlen += rs ;
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	    if (optp->mkxmcdate) {
	        if ((rs = locinfo_mkhdrdate(lip)) >= 0) {
	            hp = lip->hdrdate ;
	            hl = rs ;
	            rs = procprinthdr(pip,fbp,kn,hp,hl) ;
	            wlen += rs ;
	        }
	    }
	} /* end if (alternate subject given) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_xmcdate) */


static int procmsghdr_xuti(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *optp)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*kn = HN_XUTI ;
	cchar		*hp ;

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    rs = procmsghdr(pip,fbp,mi,kn,FALSE) ;
	    wlen += rs ;
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	    if (optp->mkxnote) {
	        hp = UTI_APPLENOTE ;
	        rs = procprinthdr(pip,fbp,kn,hp,-1) ;
	        wlen += rs ;
	    }
	} /* end if (alternate subject given) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_xuti) */


/* "x-mailer" */
/* ARGSUSED */
static int procmsghdr_xm(PROGINFO *pip,FILEBUF *fbp,int mi,MSGOPTS *mop)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		hl ;
	int		wlen = 0 ;
	cchar		*kn = HN_XMAILER ;
	cchar		*hp ;

	msp = &lip->ms ;
	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    rs = procmsghdr(pip,fbp,mi,kn,FALSE) ;
	    wlen += rs ;
	} else if (isHdrEmpty(rs)) {
	    if ((rs = locinfo_xmailer(lip)) >= 0) {
	        hp = lip->xmailer ;
	        if (lip->f.xmailer) {
	            hl = strlen(hp) ;
	            rs = procprinthdr(pip,fbp,kn,hp,hl) ;
	            wlen += rs ;
	        } /* end if */
	    } /* end if (locinfo-xmailer) */
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr_xm) */


static int procmsghdr(PROGINFO *pip,FILEBUF *fbp,int mi,cchar kn[],int f_nz)
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		rs = SR_OK ;
	int		n ;
	int		wlen = 0 ;

	if (kn == NULL) return SR_FAULT ;
	if (kn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr: hdr=%s\n",kn) ;
#endif

	msp = &lip->ms ;
	if ((n = mailmsgstage_hdrcount(msp,mi,kn)) > 0) {
	    const int	mcols = lip->msgcols ;
	    int		ind = 0 ;
	    int		c ;
	    int		ln ;
	    int		sl, cl ;
	    int		i, j ;
	    cchar	*sp ;
	    cchar	*cp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/procmsghdr: n=%d\n",n) ;
#endif

	    for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	        ln = 0 ;
	        c = 0 ;

	        if (! f_nz) {
	            rs = filebuf_writehdrkey(fbp,kn) ;
	            wlen += rs ;
	            ind = rs ;
	        }

	        for (j = 0 ; rs >= 0 ; j += 1) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("b_imail/procmsghdr: loop=%u\n",j) ;
#endif

	            sl = mailmsgstage_hdriline(msp,mi,kn,i,j,&sp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("b_imail/procmsghdr: "
	                    "mailmsgstage_hdriline() rs=%d\n",sl) ;
#endif
	            if (sl == SR_NOTFOUND) break ;
	            if ((sl == 0) || (sp == NULL)) continue ;

/* shrink and skip empty lines (some new RFC says to skip empty lines!) */

	            rs = sl ;
	            if (rs >= 0) {
	                if ((cl = sfshrink(sp,sl,&cp)) > 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(5)) {
	                        debugprintf("b_imail/procmsghdr: "
	                            "hi=%d hj=%d hc=%d\n",i,j,c) ;
	                        debugprintf("b_imail/procmsghdr_: l=>%t<\n",
	                            cp,strlinelen(cp,cl,50)) ;
	                    }
#endif

	                    if (f_nz && (c == 0)) {
	                        rs = filebuf_writehdrkey(fbp,kn) ;
	                        wlen += rs ;
	                        ind = rs ;
	                    }

	                    if (rs >= 0) {
	                        const int	in = ind ;
	                        const int	mc = mcols ;
	                        c += 1 ;
	                        rs = procprinthdr_line(pip,fbp,mc,ln,in,cp,cl) ;
	                        wlen += rs ;
	                        ln += 1 ;
	                        ind = 0 ;
	                    }

	                } /* end if (non-empty hdr) */
	            } /* end if (ok) */

	        } /* end for (lines within a header instance) */

	        if ((rs >= 0) && (! f_nz) && (c == 0)) {
	            rs = filebuf_print(fbp,kn,0) ;
	            wlen += rs ;
	            ln += 1 ;
	        }

	    } /* end for (header instances) */
	} /* end if (non-zero count) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procmsghdr: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsghdr) */


static int procmsginsthdr(pip,fbp,mi,kn,hi,f_nz)
PROGINFO	*pip ;
FILEBUF		*fbp ;
int		mi ;
cchar		kn[] ;
int		hi ;
int		f_nz ;
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGSTAGE	*msp ;
	int		mcols ;
	int		rs = SR_OK ;
	int		ind = 0 ;
	int		c = 0 ;
	int		ln = 0 ;
	int		sl, cl ;
	int		j ;
	int		wlen = 0 ;
	cchar		*sp ;
	cchar		*cp ;

	if (kn == NULL) return SR_FAULT ;
	if (kn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procmsginsthdr: hdr=%s hi=%d\n",kn,hi) ;
#endif

	msp = &lip->ms ;
	mcols = lip->msgcols ;

	if (! f_nz) {
	    rs = filebuf_writehdrkey(fbp,kn) ;
	    wlen += rs ;
	    ind = rs ;
	}

	for (j = 0 ; rs >= 0 ; j += 1) {

	    sl = mailmsgstage_hdriline(msp,mi,kn,hi,j,&sp) ;
	    if (sl == SR_NOTFOUND) break ;
	    if ((sl == 0) || (sp == NULL)) continue ;

/* shrink and skip empty lines (some new RFC says to skip empty lines!) */

	    rs = sl ;
	    if (rs >= 0) {
	        if ((cl = sfshrink(sp,sl,&cp)) > 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5)) {
	                debugprintf("procmsginsthdr: hi=%d hj=%d hc=%d\n",
	                    hi,j,c) ;
	                debugprintf("procmsginsthdr: l=>%t<\n",
	                    cp,strlinelen(cp,cl,50)) ;
	            }
#endif

	            if (f_nz && (c == 0)) {
	                rs = filebuf_writehdrkey(fbp,kn) ;
	                wlen += rs ;
	                ind = rs ;
	            }

	            if (rs >= 0) {
	                const int	in = ind ;
	                const int	mc = mcols ;
	                c += 1 ;
	                rs = procprinthdr_line(pip,fbp,mc,ln,in,cp,cl) ;
	                wlen += rs ;
	                ln += 1 ;
	                ind = 0 ;
	            }

	        } /* end if (non-empty hdr) */
	    } /* end if (ok) */

	} /* end for (lines within a header instance) */

	if ((rs >= 0) && (! f_nz) && (c == 0)) {
	    rs = filebuf_print(fbp,kn,0) ;
	    wlen += rs ;
	    ln += 1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procmsginsthdr: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsginsthdr) */


/* "indent" is initial indent only! */
static int procprinthdr_line(pip,fbp,mcols,ln,ind,lp,ll)
PROGINFO	*pip ;
FILEBUF		*fbp ;
int		mcols ;
int		ln ;
int		ind ;
cchar		*lp ;
int		ll ;
{
	LOCINFO		*lip = pip->lip ;
	MAILMSGHDRFOLD	mf ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procprinthdr_line: ent l=>%t<\n",
	        lp,strlinelen(lp,ll,50)) ;
#endif

	if ((rs = mailmsghdrfold_start(&mf,mcols,ln,lp,ll)) >= 0) {
	    const int	ntab = NTABCOLS ;
	    const int	mcols = lip->msgcols ;
	    int		i = 0 ;
	    int		leader = 0 ;
	    int		n ;
	    int		sl ;
	    cchar	*sp ;

	    if (ind == 0) {
	        leader = ' ' ;
	        ind = 1 ;
	    }

	    while ((sl = mailmsghdrfold_get(&mf,ind,&sp)) > 0) {
	        if ((ind > 1) && (leader == '\t')) {
	            const int	acols = (mcols-ind) ;
	            if ((n = ncolstr(ntab,ind,sp,sl)) > acols) {
	                rs = n ; /* use (for LINT) */
	                ind = 1 ;
	                leader = ' ' ;
	            }
	        }
	        rs = filebuf_printcont(fbp,leader,sp,sl) ;
	        wlen += rs ;
	        i += 1 ;
	        ind = ntab ;
	        leader = '\t' ;
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (i == 0)) {
	        rs = filebuf_print(fbp,lp,0) ;
	        wlen += rs ;
	    }

	    rs1 = mailmsghdrfold_finish(&mf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mailmsghdrfold) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_line) */


#if	CF_PROCEXTID
/* extract an ID out of an ID-type header (a lot of work for a simple task) */
static int procextid(PROGINFO *pip,char id[],cchar hp[],int hl)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (hp == NULL) return SR_FAULT ;

	id[0] = '\0' ;
	if (hp[0] != '\0') {
	    EMA		aid ;
	    if ((rs = ema_start(&aid)) >= 0) {
	        if ((rs = ema_parse(&aid,hp,hl)) > 0) {
	            EMA_ENT	*ep ;
	            int		i ;
	            for (i = 0 ; ema_get(&aid,i,&ep) >= 0 ; i += 1) {
	                if (ep != NULL) {
	                    if ((! ep->f.error) && (ep->rl > 0)) {
	                        const int	malen = MAILADDRLEN ;
	                        char		*bp ;
	                        bp = strwcpy(id,ep->rp,MIN(malen,ep->rl)) ;
	                        len = (bp - id) ;
	                    }
	                }
	                if (len > 0) break ;
	            } /* end for */
	        } /* end if */
	        rs1 = ema_finish(&aid) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (ema) */
	} /* end if (non-nul) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procextid) */
#endif /* CF_PROCEXTID */


static int procprinthdr(PROGINFO *pip,FILEBUF *fbp,cchar *kn,cchar *hp,int hl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		ln = 0 ;
	int		wlen = 0 ;

	if (hl < 0)
	    hl = strlen(hp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procprinthdr: h=>%t<\n",
	        hp,strlinelen(hp,hl,50)) ;
#endif

	if ((rs = filebuf_writehdrkey(fbp,kn)) >= 0) {
	    const int	mcols = lip->msgcols ;
	    int		ind = rs ;
	    wlen += rs ;
	    rs = procprinthdr_line(pip,fbp,mcols,ln,ind,hp,hl) ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr) */


static int procprinthdr_mid(PROGINFO *pip,FILEBUF *fbp,cchar *mid,int ml)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (mid != NULL) {
	    const int	mcols = lip->msgcols ;
	    const int	ln = 0 ;
	    cchar	*kn = HN_MESSAGEID ;

	    if (ml < 0)
	        ml = strlen(mid) ;

	    if ((rs = filebuf_writehdrkey(fbp,kn)) >= 0) {
	        int	ind = rs ;
	        int	hdrlen = (2 + ml + 2) ;
	        int	size ;
	        char	*hdrbuf ;
	        wlen += rs ;

	        size = (hdrlen + 1) ;
	        if ((rs = uc_malloc(size,&hdrbuf)) >= 0) {
	            int		hl ;
	            char	*hp = hdrbuf ;

	            rs = sncpy3(hdrbuf,hdrlen," <",mid,">") ;
	            hl = rs ;
	            if (rs >= 0) {
	                rs = procprinthdr_line(pip,fbp,mcols,ln,ind,hp,hl) ;
	                wlen += rs ;
	            }

	            uc_free(hdrbuf) ;
	        } /* end if (m-a-f) */

	    } /* end if (filebuf-hdrkey) */

	} /* end if (non-null) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_mid) */


static int procprinthdr_addrsome(PROGINFO *pip,FILEBUF *ofp,cchar *kn,
cchar *hp,int hl,int n)
{
	LOCINFO		*lip = pip->lip ;
	EMA		a, *ap = &a ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = ema_start(ap)) >= 0) {
	    if ((rs = ema_parse(ap,hp,hl)) >= 0) {
	        if ((rs = ema_count(ap)) > 1) {
	            OUTEMA	ld ;
	            const int	msgcols = lip->msgcols ;
	            if ((rs = outema_start(&ld,ofp,msgcols)) >= 0) {
	                if ((rs = outema_hdrkey(&ld,kn)) >= 0) {
	                    EMA_ENT	*ep ;
	                    int		c = 0 ;
	                    int		i ;
	                    for (i = 0 ; ema_get(ap,i,&ep) >= 0 ; i += 1) {
	                        if ((ep != NULL) && (ep->ol > 0)) {
	                            if ((rs = outema_ent(&ld,ep)) > 0) {
	                                c += 1 ;
	                            }
	                        }
	                        if ((n >= 0) && (c >= n)) break ;
	                        if (rs < 0) break ;
	                    } /* end for */
	                } /* end if (hdr-key) */
	                rs1 = outema_finish(&ld) ;
	                if (rs >= 0) rs = rs1 ;
	                wlen += rs1 ;
	            } /* end if (outema) */
	        } else if (rs == 1) {
	            rs = procprinthdr(pip,ofp,kn,hp,hl) ;
	            wlen += rs ;
	        } /* end if (non-zero entries) */
	    } /* end if (ema_parse) */
	    rs1 = ema_finish(ap) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ema) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_addrsome) */


/* output all EMA-type header instances of a given keyname */
static int procprinthdr_addrs(pip,ofp,mp,mi,kn)
PROGINFO	*pip ;
FILEBUF		*ofp ;
MAILMSGSTAGE	*mp ;
int		mi ;
cchar		kn[] ;
{
	EMA		a ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i = 0 ;
	int		hl ;
	int		wlen = 0 ;
	cchar		*hp ;

	if (ofp == NULL) return SR_FAULT ;
	if (mp == NULL) return SR_FAULT ;
	if (kn == NULL) return SR_FAULT ;

	if (kn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procprinthdr_addrs: ent hdr=%s\n",kn) ;
#endif

	while ((hl = mailmsgstage_hdrival(mp,mi,kn,i,&hp)) >= 0) {
	    if ((hl > 0) && (hp != NULL)) {
	        if ((rs = ema_start(&a)) >= 0) {
	            if ((rs = ema_parse(&a,hp,hl)) >= 0) {
	                rs = procprinthdr_ema(pip,ofp,kn,&a,-1) ;
	                wlen += rs ;
	            } /* end if (ema_parse) */
	            rs1 = ema_finish(&a) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (ema) */
	    } /* end if (non-zero) */
	    if (rs < 0) break ;
	    i += 1 ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procprinthdr_addrs: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_addrs) */


/* output a header that comtains one or more EMAs */
static int procprinthdr_ema(PROGINFO *pip,FILEBUF *ofp,cchar *kn,EMA *ap,
int m)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (ofp == NULL) return SR_FAULT ;
	if (kn == NULL) return SR_FAULT ;

	if (kn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procprinthdr_ema: hdr=%s\n",kn) ;
#endif

	if (ap != NULL) {
	    if ((rs = ema_count(ap)) > 0) {
	        OUTEMA		ld ;
	        const int	msgcols = lip->msgcols ;
	        const int	n = rs ;
	        if ((rs = outema_start(&ld,ofp,msgcols)) >= 0) {
	            if ((rs = outema_hdrkey(&ld,kn)) >= 0) {
	                EMA_ENT	*ep ;
	                int	i = n ; /* use (for LINT) */
	                int	c = 0 ;

	                for (i = 0 ; ema_get(ap,i,&ep) >= 0 ; i += 1) {
	                    if ((ep != NULL) && (ep->ol > 0)) {
	                        if ((rs = outema_ent(&ld,ep)) > 0) {
	                            c += 1 ;
	                        }
	                    }
	                    if ((m >= 0) && (c >= m)) break ;
	                    if (rs < 0) break ;
	                } /* end for */

	            } /* end if (hdr-key) */
	            rs1 = outema_finish(&ld) ;
	            if (rs >= 0) rs = rs1 ;
	            wlen += rs1 ;
	        } /* end if (outema) */
	    } /* end if (non-zero entries) */
	} /* end if (non-null) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/procprinthdr_ema: ret rs=%d wlen=%u\n",
	        rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_ema) */


static int procprinthdr_bcc(PROGINFO *pip,FILEBUF *fbp,vechand *tlp)
{
	LOCINFO		*lip = pip->lip ;
	OUTEMA		out ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = outema_start(&out,fbp,lip->msgcols)) >= 0) {
	    osetstr	*rlp = &lip->recips ;
	    osetstr_cur	rcur ;
	    int		f_hdr = FALSE ;
	    cchar	*kn = HN_BCC ;

	    if ((rs = osetstr_curbegin(rlp,&rcur)) >= 0) {
	        const int	rsn = SR_NOTFOUND ;
	        cchar		*ap ;
	        while ((rs1 = osetstr_enum(rlp,&rcur,&ap)) >= 0) {
	            if ((rs = vechand_search(tlp,ap,vrecipsch,NULL)) == rsn) {
	                rs = SR_OK ;
	                if (! f_hdr) {
	                    f_hdr = TRUE ;
	                    rs = outema_hdrkey(&out,kn) ;
	                }
	                if (rs >= 0) {
	                    rs = outema_item(&out,ap,-1) ;
	                }
	            }
	            if (rs < 0) break ;
	        } /* end while */
	        if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	        rs1 = osetstr_curend(rlp,&rcur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (osetstr-cur) */

	    rs1 = outema_finish(&out) ;
	    if (rs >= 0) rs = rs1 ;
	    wlen += rs1 ;
	} /* end if (outema) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_bcc) */


static int procprinthdr_xpri(PROGINFO *pip,FILEBUF *fbp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (lip->msgpriority > 0) {
	    cchar	*kn = HN_XPRIORITY ;
	    if ((rs = filebuf_writehdrkey(fbp,kn)) >= 0) {
	        int	pi = lip->msgpriority ;
	        wlen += rs ;
	        rs = filebuf_printf(fbp," %u (%s)\n",pi,msgpris[pi]) ;
	        wlen += rs ;
	    } /* end if */
	} /* end if (had a msg-priority) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_xpri) */


static int procprinthdr_mailnote(PROGINFO *pip,FILEBUF *ofp,cchar *kn,
cchar *hp,int hl)
{
	OUTSTORE	m ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = outstore_start(&m)) >= 0) {
	    cchar	*sp ;
	    if (rs >= 0) rs = outstore_strw(&m,SUBJ_MAILNOTE,-1) ;
	    if (rs >= 0) rs = outstore_strw(&m," ",1) ;
	    if (rs >= 0) rs = outstore_strw(&m,hp,hl) ;
	    if ((rs = outstore_get(&m,&sp)) >= 0) {
	        rs = procprinthdr(pip,ofp,kn,sp,rs) ;
	        wlen += rs ;
	    }
	    rs1 = outstore_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (outstore) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprinthdr_mailnote) */


static int procmsgfilebcc(PROGINFO *pip,int tfd,int sfd,int hlen,cchar *ap)
{
	LOCINFO		*lip = pip->lip ;
	FILEBUF		mfile ;
	int		rs ;
	int		rs1 ;
	int		bsize ;
	int		wlen = 0 ;

	bsize = lip->pagesize ;
	if ((rs = filebuf_start(&mfile,tfd,0L,bsize,0)) >= 0) {
	    int		rlen = hlen ;
	    int		mlen ;
	    int		len ;
	    cchar	*kn = HN_BCC ;
	    char	buf[BUFLEN + 1] ;

/* write out all message headers (already formatted) */

	    while ((rs >= 0) && (rlen > 0)) {
	        mlen = MIN(BUFLEN,rlen) ;
	        rs = u_read(sfd,buf,mlen) ;
	        len = rs ;
	        if (rs <= 0) break ;

	        rs = filebuf_write(&mfile,buf,len) ;
	        wlen += rs ;
	        rlen -= rs ;

	    } /* end while */

/* write out the additional BCC header */

	    if ((rs >= 0) && (ap != NULL) && (ap[0] != '\0')) {

#ifdef	COMMENT /* use enhancement below? */
	        hl = bufprintf(buf,BUFLEN,"%s: %s\n",kn,ap) ;
	        if (hl > 0) {
	            rs = filebuf_print(&mfile,buf,hl) ;
	            wlen += rs ;
	        }
#else
	        rs = procprinthdr(pip,&mfile,kn,ap,-1) ;
	        wlen += rs ;
#endif /* COMMENT */

	    } /* end if */

/* write out the body of the mssage */

	    while (rs >= 0) {
	        rs = u_read(sfd,buf,BUFLEN) ;
	        len = rs ;
	        if (rs <= 0) break ;

	        rs = filebuf_write(&mfile,buf,len) ;
	        wlen += rs ;

	    } /* end while */

	    rs1 = filebuf_finish(&mfile) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (mfile) */

#ifdef	COMMENT
	if (rs >= 0)
	    u_rewind(tfd) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procmsgfilebcc) */


static int proclognmsgs(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    proglog_printf(pip,"nmsgs=%u",lip->nmsgs) ;
	    if (! lip->f.take)
	        proglog_printf(pip,"take=OFF") ;
	}

	return rs ;
}
/* end subroutine (proclognmsgs) */


static int proclogmsg(PROGINFO *pip,int mi)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("imail/proclogmsg: mi=%u f_openlog=%u\n",mi,
	        pip->open.logprog) ;
#endif

	if (pip->open.logprog) {
	    if ((rs = proglog_printf(pip,"msg=%u",mi)) >= 0) {
	        if (rs >= 0) rs = proclogmsg_mailer(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_priority(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_org(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_addrs(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_bcc(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_sender(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_from(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_date(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_subj(pip,mi) ;
	        if (rs >= 0) rs = proclogmsg_er(pip,mi) ;
	    } else if (isHup(rs)) {
	        rs = SR_OK ;
	    } /* end if (proglog_printf) */
	} /* end if (log-prog) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("imail/proclogmsg: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proclogmsg) */


static int proclogmsg_er(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    int			v ;
	    v = mailmsgstage_clen(msp,mi) ;
	    if (v >= 0) proglog_printf(pip,"  clen=%u",v) ;
	    v = mailmsgstage_clines(msp,mi) ;
	    if (v >= 0) proglog_printf(pip,"  clines=%u",v) ;
	    {
	        MSGDATA		*mdp ;
	        int		nrecips ;
	        if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	            rs = msgdata_getnrecips(mdp) ;
	            nrecips = rs ;
	            if (rs >= 0)
	                proglog_printf(pip,"  nrecips=%u",nrecips) ;
	        }
	    } /* end block */
	} /* end if (log-prog) */
	return rs ;
}
/* end subroutine (proclogmsg_er) */


static int proclogmsg_mailer(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    cchar		*kn = HN_XMAILER ;
	    int			hl ;
	    cchar		*hp ;
	    if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	        cchar	*pre = "  xmailer=" ;
	        hl = rs ;
	        if (hp != NULL) {
	            const int	pl = strlen(pre) ;
	            int		n ;
	            n = (LOGFILE_FMTLEN-pl-2) ;
	            if (hl <= n) {
	                cchar	*fmt = "  xmailer=>%t<" ;
	                rs = proglog_printf(pip,fmt,hp,hl) ;
	            } else {
	                rs = proglog_printfold(pip,pre,hp,hl) ;
	            } /* end if (type of display) */
	        } /* end if (non-null) */
	    } else if (isHdrEmpty(rs)) {
	        rs = SR_OK ;
	    } /* end if (positive) */
	} /* end if (log-prog) */
	return rs ;
}
/* end subroutine (proclogmsg_mailer) */


static int proclogmsg_priority(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (pip->open.logprog) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    cchar		*kn = HN_XPRIORITY ;
	    int			hl ;
	    cchar		*hp ;
	    if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	        hl = rs ;
	        if (hp != NULL) {
	            const int	ml = (LOGFILE_FMTLEN-8) ;
	            int		rs1 ;
	            cchar	*fmt = "  xpriority=%t" ;
	            rs1 = proglog_printf(pip,fmt,hp,MIN(ml,hl)) ;
	            if (rs1 >= 0) rs = rs1 ;
	        }
	    } else if (isHdrEmpty(rs)) {
	        rs = SR_OK ;
	    } /* end if */
	} /* end if (log-prog) */
	return rs ;
}
/* end subroutine (proclogmsg_priority) */


static int proclogmsg_org(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    int			ml ;
	    int			hl = 0 ;
	    cchar		*kn = HN_ORGANIZATION ;
	    cchar		*hp ;

	    if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	        hl = rs ;
	    } else if (isHdrEmpty(rs)) {
	        rs = SR_OK ;
	        if (lip->f.org && (pip->org != NULL)) {
	            hp = pip->org ;
	            hl = strlen(hp) ;
	            while (hl && ishigh(hp[hl-1])) hl -= 1 ;
	        }
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("imail/proclogmsg_org: hl=%d h=>%t<\n",
	            hl,hp,strlinelen(hp,hl,50)) ;
#endif

	    if ((rs >= 0) && (hp != NULL) && (hl > 0)) {
	        int	rs1 ;
	        ml = (LOGFILE_FMTLEN - 8) ;
	        rs1 = proglog_printf(pip,"  org=>%t<",hp,MIN(ml,hl)) ;
	        if (rs1 >= 0) rs = rs1 ;
	    }

	} /* end if (log-prog) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("imail/proclogmsg_org: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proclogmsg_org) */


static int proclogmsg_addrs(PROGINFO *pip,int mi)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("imail/proclogmsg_addrs: ent\n") ;
#endif

	if (pip->open.logprog) {
	    const int	at_ol = msgloghdr_overlast ;
	    int		at ;
	    for (at = 0 ; (rs >= 0) && (at < at_ol) ; at += 1) {
	        rs = proclogmsg_addr(pip,mi,at) ;
	    } /* end for */
	} /* end if (logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("imail/proclogmsg_addrs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proclogmsg_addrs) */


static int proclogmsg_addr(PROGINFO *pip,int mi,int at)
{
	LOCINFO		*lip = pip->lip ;
	MSGDATA		*mdp = NULL ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/proclogmsg_addr: ent at=%u\n",at) ;
#endif

	if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	    EMA		*alp = (mdp->addrs + at) ;
	    int		m = -1 ;
	    cchar	*kn = msgloghdrs[at] ;
	    switch (at) {
	    case msgloghdr_sender:
	        m = 1 ;
	        break ;
	    case msgloghdr_from:
	        if (lip->f.onefrom) m = 1 ;
	        break ;
	    }
	    if ((rs = proclogmsg_addremas(pip,mi,kn,alp,m)) >= 0) {
	        c += rs ;
	        if ((at == msgloghdr_messageid) && (c == 0)) {
	            rs = proclogmsg_midhelp(pip,mdp) ;
	        } /* end if (message-id) */
	    } /* end if (proclogmsg_addremas) */
	} /* end if (locinfo_msgdataget) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/proclogmsg_addr: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclogmsg_addr) */


static int proclogmsg_addremas(PROGINFO *pip,int mi,cchar *kn,EMA *alp,int m)
{
	const int	kl = strlen(kn) ;
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*kp = kn ;

	if (hasuc(kn,kl)) {
	    rs = uc_mallocstrw(kn,kl,&kp) ;
	}

	if (rs >= 0) {
	    EMA_ENT	*ep ;
	    int		i ;
	    for (i = 0 ; ema_get(alp,i,&ep) >= 0 ; i += 1) {
	        rs = proclogmsg_addrema(pip,mi,kp,ep,m) ;
	        c += rs ;
	        if ((m >= 0) && (c >= m)) break ;
	        if (rs < 0) break ;
	    } /* end for */
	    if (kp != kn)
	        uc_free(kp) ;
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclogmsg_addremas) */


static int proclogmsg_addrema(PROGINFO *pip,int mi,cchar *kn,EMA_ENT *ep,int m)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*ap ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/proclogmsg_addrema: ent k=%s\n",kn) ;
#endif

	ap = ep->rp ;
	if (ap == NULL) ap = ep->ap ;

	if (ap != NULL) {
	    const int	al = strlen(ap) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        int		dl = al ;
	        cchar		*dp = ap ;
	        debugprintf("b_imail/proclogmsg_addrema: al=%u\n",al) ;
	        debugprintf("b_imail/proclogmsg_addrema: a0=>%t<\n",
	            dp,strlinelen(dp,dl,40)) ;
	        dp += 40 ;
	        dl -= 40 ;
	        if (dl > 0) {
	            debugprintf("b_imail/proclogmsg_addrema: a1=>%t<\n",
	                dp,strlinelen(dp,dl,40)) ;
	        }
	    }
#endif

	    c += 1 ;
	    {
	        const int	plen = 60 ;
	        cchar		*et = ematypes[ep->type] ;
	        char		pbuf[60+1] ;
	        if ((rs = bufprintf(pbuf,plen,"  %s%s=",kn,et)) >= 0) {
	            const int	pl = rs ;
	            int		n ;
	            n = (LOGFILE_FMTLEN-pl-2) ;
	            if (al <= n) {
	                rs1 = proglog_printf(pip,"%s%t",pbuf,ap,al) ;
	            } else {
	                rs1 = proglog_printfold(pip,pbuf,ap,al) ;
	            }
	        } /* end if (bufprintf) */
	    } /* end block */

	    if ((rs >= 0) && (ep->cp != NULL) && (rs1 >= 0)) {
	        const int	maxnamelen = (80-16-6) ;
	        cchar		*fmt ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_imail/proclogmsg_addrema: c=>%t<\n",
	                ep->cp,ep->cl) ;
#endif

	        fmt = (ep->cl > maxnamelen) ? "    (%t¬)" : "    (%s)" ;
	        rs1 = proglog_printf(pip,fmt,ep->cp,(maxnamelen-1)) ;

	    } /* end if */

	    if ((rs >= 0) && (ep->listp != NULL) && (rs1 >= 0)) {
	        rs = proclogmsg_addremas(pip,mi,kn,ep->listp,m) ;
	        c += rs ;
	    }

	} /* end if (non-null) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclogmsg_addrema) */


static int proclogmsg_midhelp(PROGINFO *pip,MSGDATA *mdp)
{
	int		rs = SR_OK ;
	cchar	*pre = "  mid=" ;
	if (mdp->hdrmid != NULL) {
	    const int	pl = strlen(pre) ;
	    const int	vl = strlen(mdp->hdrmid) ;
	    int		n ;
	    cchar		*vp = mdp->hdrmid ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/proclogmsg_addr: "
	            "mid vl=%u\n",vl) ;
#endif
	    n = (LOGFILE_FMTLEN-pl-2) ;
	    if (vl <= n) {
	        cchar	*fmt = "  mid=%t" ;
	        proglog_printf(pip,fmt,vp,vl) ;
	    } else {
	        proglog_printfold(pip,pre,vp,vl) ;
	    }
	} /* end if (non-null) */
	return rs ;
}
/* end subroutine (proclogmsg_mid) */


static int proclogmsg_bcc(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.logprog) {
	    vechand	aa ;
	    int		atypes[] = {
	                    msgloghdr_to, msgloghdr_cc, msgloghdr_bcc, -1 } ;
	    cchar	*kn = HN_BCC ;
	    if ((rs = vechand_start(&aa,0,0)) >= 0) {
	        if ((rs = procreciploads(pip,&aa,mi,atypes)) >= 0) {
	            osetstr	*rlp = &lip->recips ;
	            osetstr_cur	rcur ;
	            if ((rs = osetstr_curbegin(rlp,&rcur)) >= 0) {
	                const int	rsn = SR_NOTFOUND ;
	                cchar		*ap ;
	                while ((rs1 = osetstr_enum(rlp,&rcur,&ap)) >= 0) {
	                    vrecipsch_t	vs = vrecipsch ;
	                    if ((rs = vechand_search(&aa,ap,vs,NULL)) == rsn) {
	                        cchar	*fmt = "  %s=%s" ;
	                        rs = SR_OK ;
	                        rs1 = proglog_printf(pip,fmt,kn,ap) ;
	                        if (rs1 < 0) break ;
	                    } /* end if */
	                    if (rs < 0) break ;
	                } /* end while */
	                if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	                rs1 = osetstr_curend(rlp,&rcur) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (osetstr-cur) */
	        } /* end if (procreciploads) */
	        rs1 = vechand_finish(&aa) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vechand) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (proclogmsg_bcc) */


static int proclogmsg_sender(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.logprog && lip->f.addsender) {
	    if (lip->hdraddr_sender != NULL) {
	        MSGDATA		*mdp = NULL ;
	        if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	            const int	at = msgloghdr_sender ;
	            if (mdp->naddrs[at] <= 0) {
	                EMA	a, *alp = &a ;
	                if ((rs = ema_start(alp)) >= 0) {
	                    cchar	*as = lip->hdraddr_sender ;
	                    if ((rs = ema_parse(alp,as,-1)) >= 0) {
	                        cchar	*kn = HN_SENDER ;
	                        rs = proclogmsg_addremas(pip,mi,kn,alp,1) ;
	                    }
	                    rs1 = ema_finish(alp) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (ema) */
	            } /* end if */
	        } /* end if (locinfo_msgdataget) */
	    } /* end if (non-null) */
	} /* end if (logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/proclogmsg_sender: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proclogmsg_sender) */


static int proclogmsg_from(PROGINFO *pip,int mi)
{
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    LOCINFO	*lip = pip->lip ;
	    MSGDATA	*mdp = NULL ;
	    if ((rs = locinfo_msgdataget(lip,mi,&mdp)) >= 0) {
	        const int	at = msgloghdr_from ;
	        int		f ;
	        f = (mdp->naddrs[at] <= 0) || lip->f.addfrom ;
	        if (f && lip->f.hdrfroms) {
	            EMA		*alp = &lip->hdrfroms ;
	            const int	m = (lip->f.onefrom) ? 1 : -1 ;
	            cchar	*kn = HN_FROM ;
	            rs = proclogmsg_addremas(pip,mi,kn,alp,m) ;
	        }
	    } /* end if (locinfo_msgdataget) */
	} /* end if (log-prog) */

	return rs ;
}
/* end subroutine (proclogmsg_from) */


static int proclogmsg_date(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (pip->open.logprog) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    int			hl ;
	    int			i ;
	    cchar		*kn = HN_DATE ;
	    cchar		*hp ;
	    char		timebuf[TIMEBUFLEN + 3] ;

	    for (i = 0 ; rs >= 0 ; i += 1) {

	        hl = mailmsgstage_hdrival(msp,mi,kn,i,&hp) ;
	        if (hl == SR_NOTFOUND) break ;
	        rs = hl ;

	        if ((rs >= 0) && (hl >= 0) && (hp != NULL)) {
	            if ((rs = locinfo_cvtdate(lip,timebuf,hp,hl)) > 0) {
	                c += 1 ;
	                proglog_printf(pip,"  date=%s",timebuf) ;
	            }
	        }

	    } /* end for */

	    if ((rs >= 0) && (c == 0) && (lip->hdrdate != NULL)) {
	        hp = lip->hdrdate ;
	        hl = -1 ;
	        if ((rs = locinfo_cvtdate(lip,timebuf,hp,hl)) > 0) {
	            c += 1 ;
	            proglog_printf(pip,"  date»%s",timebuf) ;
	        }
	    }

	} /* end if (logging) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/proclogmsg_date: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclogmsg_date) */


static int proclogmsg_subj(PROGINFO *pip,int mi)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;

	if (pip->open.logprog) {
	    MAILMSGSTAGE	*msp = &lip->ms ;
	    int			ml ;
	    int			hl = 0 ;
	    cchar		*kn = HN_SUBJECT ;
	    cchar		*hp ;

	    if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	        hl = rs ;
	    } else if (isHdrEmpty(rs)) {
	        cchar	*hdrsubj = lip->hdrsubject ;
	        rs = SR_OK ;
	        if (hdrsubj != NULL) {
	            hp = hdrsubj ;
	            hl = strlen(hdrsubj) ;
	        }
	    }

	    if ((rs >= 0) && (hl >= 0) && (hp != NULL)) {
	        ml = (LOGFILE_FMTLEN - 12) ;
	        proglog_printf(pip,"  subj=>%t<",hp,MIN(ml,hl)) ;
	    }

	} /* end if (logging) */

	return rs ;
}
/* end subroutine (proclogmsg_subj) */


static int proclogrecips(PROGINFO *pip,VECHAND *llp)
{
	int		rs = SR_OK ;
	int		n = 0 ;

	if (pip->open.logprog) {
	    int		i ;
	    cchar	*rp ;
	    for (i = 0 ; vechand_get(llp,i,&rp) >= 0 ; i += 1) {
	        if (rp != NULL) {
	            n += 1 ;
	            proglog_printf(pip,"  %s",rp) ;
	        }
	    } /* end for */
	} /* end if (logging) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (proclogrecips) */


static int procgetns(PROGINFO *pip,char *nbuf,int nlen,cchar *un,int w)
{
	LOCINFO		*lip = pip->lip ;
	return locinfo_pcsnsget(lip,nbuf,nlen,un,w) ;
}
/* end subroutine (procgetns) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->uid_pr = -1 ;
	lip->gid_pr = -1 ;
	lip->msgcols = MSGCOLS ;
	lip->to = -1 ;
	lip->maxpostargs = MAXPOSTARGS ;
	lip->sendmail = PROG_SENDMAIL ;
	lip->postmail = PROG_POSTMAIL ;
	lip->pagesize = getpagesize() ;

	lip->f.deliver = OPT_DELIVER ;		/* default is to deliver */
	lip->f.cmbname = OPT_CMBNAME ;		/* save MSG copy */
	lip->f.xmailer = OPT_MAILER ;		/* include "x-mailer" header */
	lip->f.org = OPT_ORG ;
	lip->f.sender = OPT_SENDER ;
	lip->f.reqsubj = OPT_REQSUBJ ;
	lip->f.addsubj = OPT_ADDSUBJ ;
	lip->f.addsender = OPT_ADDSENDER ;
	lip->f.addfrom = OPT_ADDFROM ;
	lip->f.onefrom = OPT_ONEFROM ;
	lip->f.onesender = OPT_ONESENDER ;
	lip->f.take = OPT_TAKE ;
	lip->f.useclen = OPT_USECLEN ;
	lip->f.useclines = OPT_USECLINES ;

	if ((rs = vecstr_start(&lip->stores,4,0)) >= 0) {
	    lip->open.stores = TRUE ;
	    if ((rs = vecstr_start(&lip->epath,4,0)) >= 0) {
	        rs = osetstr_start(&lip->recips,4) ;
	        if (rs < 0)
	            vecstr_finish(&lip->epath) ;
	    }
	    if (rs < 0)
	        vecstr_finish(&lip->stores) ;
	} /* end if (vecstr_start) */

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.ns) {
	    lip->open.ns = FALSE ;
	    rs1 = pcsns_close(&lip->ns) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.rn) {
	    lip->open.rn = FALSE ;
	    rs1 = sysrealname_close(&lip->rn) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.gm) {
	    lip->open.gm = FALSE ;
	    rs1 = grmems_finish(&lip->gm) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->f.dater) {
	    lip->f.dater = FALSE ;
	    rs1 = dater_finish(&lip->dc) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = locinfo_msgdataend(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->open.hdrfroms) {
	    lip->open.hdrfroms = FALSE ;
	    rs1 = ema_finish(&lip->hdrfroms) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = osetstr_finish(&lip->recips) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&lip->epath) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = locinfo_tmpdone(lip) ;
	if (rs >= 0) rs = rs1 ;

	if (lip->jobdname != NULL) {
	    rs1 = uc_free(lip->jobdname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->jobdname = NULL ;
	}

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


static int locinfo_hdrfrom(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if ((sp[0] != '\0') && (sl != 0)) {
	    EMA		*emap = &lip->hdrfroms ;
	    if (! lip->open.hdrfroms) {
	        if ((rs = ema_start(emap)) >= 0) {
	            lip->open.hdrfroms = TRUE ;
	        }
	    } /* end if (ema_start) */
	    if (rs >= 0) {
	        if ((rs = ema_parse(emap,sp,sl)) >= 0) {
	            c += rs ;
	        }
	    } /* end if (ok) */
	} /* end if (needed) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_hdrfrom) */


static int locinfo_jobdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->jobdname == NULL) {
	    const mode_t	dm = DMODE ;
	    cchar		*sn = pip->searchname ;
	    char		rbuf[MAXPATHLEN + 1] ;

#if	CF_PRTMPDIR
#ifdef	COMMENT
	    {
	        cchar	*rn = pip->rootname ;
	        if ((rs = mkpath3(rbuf,pip->tmpdname,rn,sn)) >= 0) {
	            rl = rs ;
	            rs = mkdirs(rbuf,dm) ;
	        }
	    }
#else /* COMMENT */
	    {
	        cchar	*pr = pip->pr ;
	        rs = prmktmpdir(pr,rbuf,pip->tmpdname,sn,dm) ;
	        rl = rs ;
	    }
#endif /* COMMENT */
#else /* CF_PRTMPDIR */
	    {
	        rs = mktmpuserdir(rbuf,pip->username,sn,dm) ;
	        rl = rs ;
	    }
#endif /* CF_PRTMPDIR */

	    if (rs >= 0) {
	        cchar	**vpp = &lip->jobdname ;
	        rs = locinfo_setentry(lip,vpp,rbuf,rl) ;
	    }

	} else {
	    rl = strlen(lip->jobdname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/locinfo_jobdname: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (locinfo_jobdname) */


static int locinfo_mailerprog(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		plen = 0 ;

	if (lip == NULL)
	    return SR_FAULT ;

	if (! lip->f.epath) {
	    lip->f.epath = TRUE ;
	    rs = loadpath(pip) ;
	}

	if (rs >= 0) {
	    vecstr	*plp = &lip->epath ;
	    cchar	*prog = PROG_POSTMAIL ;
	    char	progfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG && 0
	    if (DEBUGLEVEL(3)) {
	        int	i ;
	        cchar	*cp ;
	        debugprintf("b_imail/locinfo_mailerprog: path¬\n") ;
	        for (i = 0 ; vecstr_get(plp,i,&cp) >= 0 ; i += 1) {
	            debugprintf("b_imail/locinfo_mailerprog: p%u=%t\n",
	                i,cp,strlinelen(cp,-1,50)) ;
	        }
	    }
#endif /* CF_DEBUG */

	    rs = getprogpath(&pip->id,plp,progfname,prog,-1) ;
	    plen = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_imail/locinfo_mailerprog: getprogpath() rs=%d\n",
	            rs) ;
#endif

	    if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	        prog = PROG_RMAIL ;
	        rs = getprogpath(&pip->id,plp,progfname,prog,-1) ;
	        plen = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("b_imail/locinfo_mailerprog: "
	                "getprogpath() rs=%d\n", rs) ;
#endif
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("b_imail/locinfo_mailerprog: mailerprog=%s\n",
	            progfname) ;
#endif

	    if (rs >= 0) {
	        cchar	**vpp = &lip->mailprogfname ;
	        rs = locinfo_setentry(lip,vpp,progfname,plen) ;
	    } else {
	        shio_printf(pip->efp,"%s: MTA unavailable\n",pip->progname) ;
	    }

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (locinfo_mailerprog) */


static int locinfo_defopts(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->f.addnote) {
	    lip->f.addxmcdate = TRUE ;
	    lip->f.addxuuid = TRUE ;
	}
	return rs ;
}
/* end subroutine (locinfo_defopts) */


static int locinfo_setfolder(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	char		tmpdname[MAXPATHLEN + 1] ;

	if (lip == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	pip = lip->pip ;
	lip->have.folder = TRUE ;
	if (vl < 0) vl = strlen(vp) ;

	if (vl > 0) {

	    if (vp[0] != '/') {
	        if (vl == 0) {
	            vp = FOLDERDNAME ;
	            vl = -1 ;
	        }
	        vl = mkpath2w(tmpdname,pip->homedname,vp,vl) ;
	        vp = tmpdname ;
	    }

	    if (vl > 0) {
	        int	f = TRUE ;
	        cchar	*ep = lip->folder ;
	        if (ep != NULL) {
	            int	m = nleadstr(ep,vp,vl) ;
	            f = (m != vl) || (ep[m] != '\0') ;
	        }
	        if (f) {
	            cchar	**vpp = &lip->folder ;
	            rs = locinfo_setentry(lip,vpp,vp,vl) ;
	        }
	    } /* end if */

	} /* end if (positive) */

	return rs ;
}
/* end subroutine (locinfo_setfolder) */


/* set Currenet-Mail-Box */
static int locinfo_setcmb(LOCINFO *lip,cchar *vp,int vl)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	pip = lip->pip ;
	if (pip == NULL) return SR_FAULT ; /* lint */

	lip->have.cmbname = TRUE ;
	if (vl < 0)
	    vl = strlen(vp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/locinfo_setcmb: v=%t\n",vp,vl) ;
#endif

	if (vl > 0) {
	    int		f = TRUE ;
	    cchar	*ep = lip->cmbname ;
	    if (ep != NULL) {
	        int	m = nleadstr(ep,vp,vl) ;
	        f = (m != vl) || (ep[m] != '\0') ;
	    }
	    if (f) {
	        cchar	**vpp = &lip->cmbname ;
	        cchar	*mb ;
	        lip->changed.cmbname = TRUE ;
	        rs = locinfo_setentry(lip,vpp,vp,vl) ;
	        if (rs >= 0) {
	            mb = lip->cmbname ;
	            lip->f.cmbname = (mb[0] != '\0') && (mb[0] != '-') ;
	        }
	    }
	} /* end if (positive) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("locinfo_setcmb: ret rs=%d\n",rs) ;
	    debugprintf("locinfo_setcmb: cmbname=%s\n",lip->cmbname) ;
	}
#endif

	return rs ;
}
/* end subroutine (locinfo_setcmb) */


static int locinfo_msgdatabegin(LOCINFO *lip)
{
	PROGINFO	*pip ;
	MAILMSGSTAGE	*msp ;
	int		rs ;
	int		n = 0 ;

	if (lip == NULL) return SR_FAULT ;

	pip = lip->pip ;
	if (pip == NULL) return SR_FAULT ;

	msp = &lip->ms ;
	if ((rs = mailmsgstage_count(msp)) > 0) {
	    MSGDATA	*mdp ;
	    int		size ;
	    n = rs ;
	    lip->nmsgs = n ;
	    size = (n * sizeof(MSGDATA)) ;
	    if ((rs = uc_malloc(size,&mdp)) >= 0) {
	        int	i ;
	        lip->md = mdp ;
	        for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	            rs = msgdata_start((mdp + i),i) ;
	        } /* end for */
	        if (rs < 0) {
	            int	j ;
	            for (j = 0 ; j < i ; j += 1) {
	                msgdata_finish(lip->md + j) ;
	            }
	            uc_free(lip->md) ;
	            lip->md = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (positive) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (locinfo_msgdatabegin) */


static int locinfo_msgdataget(LOCINFO *lip,int mi,MSGDATA **rpp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	pip = lip->pip ;
	if (pip == NULL) return SR_FAULT ; /* LINT */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("locinfo_msgdataget: ent mi=%d\n",mi) ;
	    debugprintf("locinfo_msgdataget: md{%p}\n",lip->md) ;
	}
#endif
	if (lip->md == NULL) return SR_NOTFOUND ;

	if ((mi < 0) || (mi >= lip->nmsgs)) rs = SR_NOTFOUND ;

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? (lip->md + mi) : NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("locinfo_msgdataget: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_msgdataget) */


static int locinfo_msgdataend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL)
	    return SR_FAULT ;

	if (lip->md != NULL) {
	    int	i ;
	    for (i = 0 ; i < lip->nmsgs ; i += 1) {
	        rs1 = msgdata_finish(lip->md + i) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    {
	        rs1 = uc_free(lip->md) ;
	        if (rs >= 0) rs = rs1 ;
	        lip->md = NULL ;
	    }
	    lip->nmsgs = 0 ;
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (locinfo_msgdataend) */


static int locinfo_cmbfname(LOCINFO *lip,char mbfname[])
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;
	cchar		*mf = lip->folder ;
	cchar		*mb = lip->cmbname ;

	if (mbfname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_imail/locinfo_cmbfname: f_cmbname=%u\n",
	        lip->f.cmbname) ;
	    debugprintf("b_imail/locinfo_cmbfname: mf=%s\n",mf) ;
	    debugprintf("b_imail/locinfo_cmbfname: mb=%s\n",mb) ;
	}
#endif

	mbfname[0] = '\0' ;
	if (lip->f.cmbname && ((mb == NULL) || (mb[0] != '-'))) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_imail/locinfo_cmbfname: pre mb=%s\n",mb) ;
#endif

	    if ((mb == NULL) || (mb[0] == '+')) {
	        if ((mb != NULL) && (mb[1] != '\0')) {
	            mb += 1 ;
	        } else
	            mb = MB_COPY ;
	    }

	    if (mf != NULL) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("b_imail/locinfo_cmbfname: mf=%s\n",mf) ;
	            debugprintf("b_imail/locinfo_cmbfname: mb=%s\n",mb) ;
	        }
#endif

	        if (mb[0] == '/') {
	            rs = mkpath1(mbfname,mb) ;
	            len = rs ;
	        } else {
	            struct ustat	sb ;
	            char		mfdname[MAXPATHLEN+1] ;

	            if (mf[0] == '/') {
	                rs = mkpath1(mfdname,mf) ;
	            } else
	                rs = mkpath2(mfdname,pip->homedname,mf) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_imail/locinfo_cmbfname: "
	                    "mid rs=%d mfdname=%s\n",
	                    rs,mfdname) ;
#endif

	            if (rs >= 0) {

	                rs = u_stat(mfdname,&sb) ;
	                if ((rs >= 0) && (! S_ISDIR(sb.st_mode))) {
	                    rs = SR_NOTDIR ;
	                }

	                if (rs == SR_NOENT) {
	                    rs = mkdirs(mfdname,0775) ;
	                }

	                if (rs >= 0) {
	                    rs = mkpath2(mbfname,mfdname,mb) ;
	                    len = rs ;
	                } /* end if (folder directory exists) */

	            } /* end if */

	        } /* end if */

	    } /* end if (none-NULL folder-directory mame) */

	} /* end if (requested) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_imail/locinfo_cmbfname: ret rs=%d len=%u\n",
	        rs,len) ;
	    debugprintf("b_imail/locinfo_cmbfname: mbfname=%s\n",
	        mbfname) ;
	}
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_cmbfname) */


static int locinfo_mkenvfrom(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		ml = 0 ;

	if (lip->envfrom == NULL) {
	    int		malen = 1 ;	/* for the '!' character */
	    cchar	*nn = pip->nodename ;
	    cchar	*un = pip->username ;
	    char	*mabuf ;
	    malen += (strlen(nn)+1) ;
	    malen += (strlen(un)+1) ;
	    if ((rs = uc_malloc((malen+1),&mabuf)) >= 0) {
		if ((rs = sncpy3(mabuf,malen,nn,"!",un)) >= 0) {
		    cchar	**vpp = &lip->envfrom ;
		    ml = rs ;
		    rs = locinfo_setentry(lip,vpp,mabuf,ml) ;
		}
		uc_free(mabuf) ;
	    } /* end if (m-a-f) */
	} else {
	    ml = strlen(lip->envfrom) ;
	}

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (locinfo_mkenvfrom) */


static int locinfo_mkenvdate(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip->msgtime == 0)
	    lip->msgtime = pip->daytime ;

	if (lip->envdate == NULL) {
	    cchar	**vpp = &lip->envdate ;
	    char	tbuf[TIMEBUFLEN + 1] ;
	    timestr_edate(lip->msgtime,tbuf) ;
	    len = strlen(tbuf) ;
	    rs = locinfo_setentry(lip,vpp,tbuf,len) ;
	} else {
	    len = strlen(lip->envdate) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mkenvdate) */


static int locinfo_mkenv(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		rl = 0 ;
	if (lip->env == NULL) {
	    if ((rs = locinfo_mkenvfrom(lip)) >= 0) {
	        if ((rs = locinfo_mkenvdate(lip)) >= 0) {
	            BUFFER	b ;
		    const int	flen = strlen(lip->envfrom) ;
		    const int	dlen = strlen(lip->envdate) ;
	            if ((rs = buffer_start(&b,(flen+dlen+10))) >= 0) {
		        cchar	*rbuf ;
	                buffer_strw(&b,"From ",5) ;
	                buffer_strw(&b,lip->envfrom,flen) ;
	                buffer_char(&b,' ') ;
	                buffer_strw(&b,lip->envdate,dlen) ;
	                buffer_char(&b,'\n') ;
	                if ((rs = buffer_get(&b,&rbuf)) >= 0) {
	                    cchar	**vpp = &lip->env ;
	                    rl = rs ;
	                    rs = locinfo_setentry(lip,vpp,rbuf,rl) ;
	                }
	                rs1 = buffer_finish(&b) ;
		        if (rs >= 0) rs = rs1 ;
	            } /* end if (buffer) */
	        }
	    }
	} else {
	    rl = strlen(lip->env) ;
	} /* end if (non-NULL) */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (locinfo_mkenv) */


/* we make a new one each time since they are unique for each message */
static int locinfo_mkmid(LOCINFO *lip,char *mbuf,int mlen)
{
	PROGINFO	*pip = lip->pip ;
	SBUF		mb ;
	int		rs ;
	int		rs1 ;

	mbuf[0] = '\0' ;
	if ((rs = sbuf_start(&mb,mbuf,mlen)) >= 0) {
	    uint	uv = (uint) pip->pid ;
	    const int	serial = lip->serial++ ;
	    const int	nl = strlen(pip->nodename) ;
	    cchar	*dn = pip->domainname ;
	    cchar	*nn = pip->nodename ;

	    if (nl > USERNAMELEN) {
	        rs1 = (int) gethostid() ;
	        sbuf_hexi(&mb,rs1) ;
	        sbuf_char(&mb,'-') ;
	    } else {
	        sbuf_strw(&mb,nn,nl) ;
	    }

	    sbuf_decui(&mb,uv) ;

	    sbuf_char(&mb,'.') ;

	    {
	        uv = (uint) pip->daytime ;
	        sbuf_hexui(&mb,uv) ;
	    }

	    sbuf_char(&mb,'.') ;
	    sbuf_deci(&mb,lip->kserial) ;
	    sbuf_char(&mb,'.') ;
	    sbuf_deci(&mb,serial) ;

	    sbuf_char(&mb,'@') ;

	    sbuf_strw(&mb,dn,-1) ;

	    rs1 = sbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (locinfo_mkmid) */


static int locinfo_mkhdrsender(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 = 0 ;
	int		bl = 0 ;

	if (lip->hdraddr_sender == NULL) {
	    BUFFER	b ;
	    if ((rs = buffer_start(&b,MAILADDRLEN)) >= 0) {
	        cchar	*cn = pip->domainname ;
	        cchar	*bp ;

	        if (cn == NULL) {
	            if (lip->clustername == NULL) {
	                lip->clustername = getourenv(pip->envv,VARCLUSTER) ;
	            }
	            cn = lip->clustername ;
	        }
	        if (cn == NULL) {
	            cn = pip->nodename ;
	        }

	        buffer_strw(&b,pip->username,-1) ;
	        buffer_char(&b,'@') ;
	        buffer_strw(&b,cn,-1) ;

/* add a name if we can find one */

	        if (lip->hdrname_from == NULL) {
	            rs1 = locinfo_mkhdrname_from(lip) ;
	        }

	        if ((rs1 >= 0) && (lip->hdrname_from != NULL)) {
	            buffer_char(&b,' ') ;
	            buffer_char(&b,CH_LPAREN) ;
	            buffer_strw(&b,lip->hdrname_from,-1) ;
	            buffer_char(&b,CH_RPAREN) ;
	        } /* end if (adding name) */

	        if (rs >= 0) {
	            if ((rs = buffer_get(&b,&bp)) >= 0) {
	                cchar	**vpp = &lip->hdraddr_sender ;
	                bl = rs ;
	                rs = locinfo_setentry(lip,vpp,bp,bl) ;
	            }
	        }

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} else {
	    bl = strlen(lip->hdraddr_sender) ;
	}

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (locinfo_mkhdrsender) */


static int locinfo_mkhdrfrom(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if ((! lip->f.hdrfroms) || lip->f.def_from) {
	    int	al = -1 ;

	    if (lip->hdraddr_from == NULL) {
	        lip->hdraddr_from = getourenv(pip->envv,VARIMAILFROM) ;
	    }
	    if (lip->hdraddr_from == NULL) {
	        lip->hdraddr_from = getourenv(pip->envv,VARMAILFROM) ;
	    }
	    if (lip->hdraddr_from == NULL) {
	        rs = locinfo_mkhdraddrfrom(lip) ;
	        al = rs ;
	    }

	    if ((rs >= 0) && (lip->hdraddr_from != NULL)) {
	        cchar	*ap = lip->hdraddr_from ;
	        rs = locinfo_hdrfrom(lip,ap,al) ;
	        c += rs ;
	    } /* end if */

	} /* end if (needed) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/locinfo_mkhdrfrom: ret rs=%d c=%u\n",
	        rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_mkhdrfrom) */


static int locinfo_mkhdrdate(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip->msgtime == 0)
	    lip->msgtime = pip->daytime ;

	if (lip->hdrdate == NULL) {
	    cchar	**vpp = &lip->hdrdate ;
	    char	tbuf[TIMEBUFLEN + 1] ;
	    timestr_hdate(lip->msgtime,tbuf) ;
	    len = strlen(tbuf) ;
	    rs = locinfo_setentry(lip,vpp,tbuf,len) ;
	} else {
	    len = strlen(lip->hdrdate) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mkhdrdate) */


static int locinfo_mkhdrxuuid(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->have.hdruuid) {
	    if ((rs = locinfo_uuid(lip)) >= 0) {
	        MKUUID		*up = &lip->uuid ;
	        const int	ulen = 100 ;
	        char		ubuf[100+1] ;
	        if ((rs = snmkuuid(ubuf,ulen,up)) >= 0) {
	            cchar	**vpp = &lip->hdruuid ;
	            lip->have.hdruuid = TRUE ;
	            rs = locinfo_setentry(lip,vpp,ubuf,rs) ;
	        }
	    }
	} else {
	    rs = strlen(lip->hdruuid) ;
	}
	return rs ;
}
/* end subroutine (locinfo_mkhdrxuuid) */


static int locinfo_uuid(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (! lip->have.uuid) {
	    MKUUID	*up = &lip->uuid ;
	    if ((rs = mkuuid(up,0)) >= 0) {
	        lip->have.uuid = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_uuid) */


static int locinfo_loadrecip(LOCINFO *lip,cchar *np,int nl)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	int		c = 0 ;

	if (nl < 0) nl = strlen(np) ;

	if ((nl == 0) || ((nl == 1) && (np[0] == '-'))) {
	    np = pip->username ;
	    nl = -1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/locinfo_loadrecip: recip=%t\n",np,nl) ;
#endif

	rs = osetstr_add(&lip->recips,np,nl) ;
	c += rs ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_loadrecip) */


static int locinfo_mkhdrname_from(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		nbl = 0 ;
	int		len = 0 ;

	if (lip->hdrname_from == NULL) {
	    if ((rs = locinfo_prpcs(lip)) >= 0) {
	        const int	nlen = REALNAMELEN ;
	        const int	w = pcsnsreq_fullname ;
	        cchar		*un = pip->username ;
	        cchar		*nbp = NULL ;
	        char		nbuf[REALNAMELEN+1] = { 0 } ;

/* try PCS first */

	        nbp = nbuf ;
	        if ((rs = procgetns(pip,nbuf,nlen,un,w)) >= 0) {
	            nbl = rs ;

/* try USERINFO-derived possibilities */

	            if ((nbp == NULL) || (nbp[0] == '\0')) {
	                nbp = pip->fullname ;
	                nbl = -1 ;
	            }

	            if ((nbp == NULL) || (nbp[0] == '\0')) {
	                nbp = pip->name ;
	                nbl = -1 ;
	            }

/* store any result */

	            if ((nbp != NULL) && (nbp[0] != '\0')) {
	                cchar	**vpp = &lip->hdrname_from ;
	                rs = locinfo_setentry(lip,vpp,nbp,nbl) ;
	                len = rs ;
	            }

	        } /* end if (ok) */

	    } /* end if (locinfo_prpcs) */
	} else {
	    len = strlen(lip->hdrname_from) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/locinfo_mkhdrname_from: ret rs=%d len=%d\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mkhdrname_from) */


static int locinfo_prpcs(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->pr_pcs == NULL) {
	    const int	plen = MAXPATHLEN ;
	    cchar	*dn = pip->domainname ;
	    char	pbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpr(pbuf,plen,VARPRPCS,dn)) >= 0) {
	        cchar	**vpp = &lip->pr_pcs ;
	        rs = locinfo_setentry(lip,vpp,pbuf,rs) ;
	    } /* end if (mkpr) */
	} else {
	    rs = strlen(lip->pr_pcs) ;
	} /* end if */

	return rs ;
}
/* end subroutine (locinfo_mkprpcs) */


static int locinfo_mkhdraddrfrom(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		bl = 0 ;

	if (lip->hdraddr_from == NULL) {
	    BUFFER	b ;

/* cache the clustername if necessary */

	    if (lip->clustername == NULL) {
	        lip->clustername = getourenv(pip->envv,VARCLUSTER) ;
	    }

/* put an address together */

	    if ((rs = buffer_start(&b,MAILADDRLEN)) >= 0) {
	        cchar	*cn = lip->clustername ;
	        cchar	*nn = pip->nodename ;
	        cchar	*cp ;

	        buffer_strw(&b,pip->username,-1) ;

	        buffer_char(&b,'@') ;

	        cp = (cn != NULL) ? cn : nn ;
	        buffer_strw(&b,cp,-1) ;

/* add a name if we can find one */

	        if (lip->hdrname_from == NULL)
	            rs = locinfo_mkhdrname_from(lip) ;

	        if ((rs >= 0) && (lip->hdrname_from != NULL)) {
	            buffer_char(&b,' ') ;
	            buffer_char(&b,CH_LPAREN) ;
	            buffer_strw(&b,lip->hdrname_from,-1) ;
	            buffer_char(&b,CH_RPAREN) ;
	        } /* end if (adding name) */

	        if (rs >= 0) {
	            cchar	*bp ;
	            if ((rs = buffer_get(&b,&bp)) >= 0) {
	                cchar	**vpp = &lip->hdraddr_from ;
	                bl = rs ;
	                rs = locinfo_setentry(lip,vpp,bp,bl) ;
	            }
	        }

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */
	} else {
	    bl = strlen(lip->hdraddr_from) ;
	}

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (locinfo_mkhdraddrfrom) */


static int locinfo_opentmpfile(LOCINFO *lip,char *tbuf,int of,cchar *prefix)
{
	int		rs = SR_OK ;
	int		fd = -1 ;

	if ((rs = locinfo_jobdname(lip)) >= 0) {
	    const mode_t	om = 0660 ;
	    int			fl ;
	    char		cname[JOBCLEN + 1] ;
	    char		template[MAXPATHLEN + 1] ;
	    char		*bp ;

	bp = strwcpy(cname,prefix,JOBCLEN) ;
	fl = (bp - cname) ;
	if (fl < JOBCLEN) {
	    strwset(bp,'X',(JOBCLEN - fl)) ;
	}

	if ((rs = mkpath2(template,lip->jobdname,cname)) >= 0) {
	    rs = opentmpfile(template,of,om,tbuf) ;
	    fd = rs ;
	}

	} /* end if (locinfo_jobdname) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (locinfo_opentmpfile) */


static int locinfo_tmpcheck(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (lip->jobdname != NULL) {
	    TMTIME	t ;
	    if ((rs = tmtime_localtime(&t,pip->daytime)) >= 0) {
	        if ((t.hour >= HOUR_MAINT) && lip->f.maint) {
		    uptsub_t	thr = (uptsub_t) locinfo_tmpmaint ;
	            pthread_t	tid ;
	            if ((rs = uptcreate(&tid,NULL,thr,lip)) >= 0) {
	                rs = 1 ;
	                lip->tid = tid ;
	                lip->f.tmpmaint = TRUE ;
	            } /* end if (uptcreate) */
	        } /* end if (after hours) */
	    } /* end if (tmtime_localtime) */
	} /* end if (job-dname) */

	return rs ;
}
/* end subroutine (locinfo_tmpcheck) */


/* this runs as an independent thread */
static int locinfo_tmpmaint(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	const int	to = TO_TMPFILES ;
	int		rs ;
	int		c = 0 ;
	int		f_need = lip->f.maint ;
	cchar		*dname = lip->jobdname ;
	char		tsfname[MAXPATHLEN+1] ;

	if ((rs = mkpath2(tsfname,dname,TSFNAME)) >= 0) {
	    const mode_t	om = 0666 ;
	    const int		of = (O_WRONLY|O_CREAT) ;
	    if ((rs = u_open(tsfname,of,om)) >= 0) {
	        struct ustat	usb ;
	        const int	fd = rs ;
	        if ((rs = u_fstat(fd,&usb)) >= 0) {
	            time_t	dt = pip->daytime ;
	            if ((rs = locinfo_fchmodown(lip,fd,&usb,om)) >= 0) {
	                int	maintlapse = (dt - usb.st_mtime) ;
	                f_need = f_need || (usb.st_size == 0) ;
	                f_need = f_need || (maintlapse >= to) ;
	                if (f_need) {
	                    int		tl ;
	                    char	timebuf[TIMEBUFLEN + 3] ;
	                    timestr_log(dt,timebuf) ;
	                    tl = strlen(timebuf) ;
	                    timebuf[tl++] = '\n' ;
	                    rs = u_write(fd,timebuf,tl) ;
	                } /* end if (timed-out) */
	            } /* end if (locinfo_fchmodown) */
	        } /* end if (stat) */
	        u_close(fd) ;
	    } /* end if (open file) */
	} /* end if (mkpath timestamp) */

	if ((rs >= 0) && f_need) {
	    rs = rmdirfiles(dname,NULL,to) ;
	    c = rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_tmpmaint) */


static int locinfo_tmpdone(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->f.tmpmaint) {
	    int	trs ;
	    rs1 = uptjoin(lip->tid,&trs) ;
	    if (rs >= 0) rs = rs1 ;
	    if (rs >= 0) rs = trs ;
	}
	return rs ;
}
/* end subroutine (locinfo_tmpdone) */


static int locinfo_fchmodown(LOCINFO *lip,int fd,struct ustat *sbp,mode_t mm)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if ((sbp->st_size == 0) && (pip->euid == sbp->st_uid)) {
	    if ((sbp->st_mode & S_IAMB) != mm) {
	        if ((rs = locinfo_loadprids(lip)) >= 0) {
	            if ((rs = uc_fminmod(fd,mm)) >= 0) {
	                const uid_t	uid_pr = lip->uid_pr ;
	                const gid_t	gid_pr = lip->gid_pr ;
	                const int	n = _PC_CHOWN_RESTRICTED ;
	                if ((rs = u_fpathconf(fd,n,NULL)) == 0) {
	                    f = TRUE ;
	                    u_fchown(fd,uid_pr,gid_pr) ; /* may fail */
	                } else if (rs == SR_NOSYS) {
			    rs = SR_OK ;
	                }
	            }
	        } /* end if (locinfo_loadprids) */
	    } /* end if (need change) */
	} /* end if (zero-file) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_fchmodown) */


static int locinfo_cvtdate(LOCINFO *lip,char tbuf[],cchar *hp,int hl)
{
	PROGINFO	*pip = lip->pip ;
	DATER		*dp = &lip->dc ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;
	tbuf[0] = '\0' ;
	if ((hp != NULL) && (hl != 0)) {

	    if (hl < 0) hl = strlen(hp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("b_imail/locinfo_cvtdate: ds=>%t<\n",
	            hp,strlinelen(hp,hl,50)) ;
#endif

	    if (! lip->f.dater) {
	        struct timeb	now ;
	        const int	zlen = ZNAMELEN ;
	        char		zbuf[ZNAMELEN + 1] ;
	        if ((rs = initnow(&now,zbuf,zlen)) >= 0) {
	            rs = dater_start(dp,&now,zbuf,rs) ;
	            lip->f.dater = (rs >= 0) ;
	        }
#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/locinfo_cvtdate: init rs=%d\n",rs) ;
#endif
	    } /* end if */

	    if ((rs >= 0) && (hl > 0)) {
	        if ((rs = dater_setmsg(dp,hp,hl)) >= 0) {
	            rs = dater_mkstrdig(dp,tbuf,TIMEBUFLEN) ;
	            len = rs ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("b_imail/locinfo_cvtdate: "
	                    "dater_mkstrdif() rs=%d\n",
	                    rs) ;
#endif
	        } else if (isBadDate(rs)) {
	            rs = SR_OK ;
		}

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("b_imail/locinfo_cvtdate: dater_setmsg rs=%d\n",
	                rs) ;
#endif
	    } /* end if */

	} /* end if (non-empty) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/locinfo_cvtdate: ret rs=%d len=%d\n",
	        rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_cvtdate) */


static int locinfo_msgpriority(LOCINFO *lip,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (sl < 0) sl = strlen(sp) ;
	if (sl > 0) {
	    int	ch = MKCHAR(sp[0]) ;
	    lip->final.msgpriority = TRUE ;
	    lip->have.msgpriority = TRUE ;
	    if (isalphalatin(ch)) {
	        int	i ;
	        if ((i = matostr(msgpris,1,sp,sl)) >= 0) {
	            lip->msgpriority = i ;
	        } else {
	            rs = SR_INVALID ;
		}
	    } else {
	        rs = optvalue(sp,sl) ;
	        lip->msgpriority = rs ;
	        if (rs > msgpri_overlast) rs = SR_DOM ;
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_msgpriority) */


static int locinfo_xmailer(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (lip->xmailer == NULL) {
	    PROGINFO	*pip = lip->pip ;
	    const int	rlen = MAXNAMELEN ;
	    int		ol = -1 ;
	    cchar	*sn = pip->searchname ;
	    cchar	*pv = pip->version ;
	    cchar	*op = pip->org ;
	    cchar	*fmt ;
	    char	rbuf[MAXNAMELEN+1] ;
	    if (op == NULL) op = pip->domainname ;
	    ol = strlen(op) ;
	    while (ol && ishigh(op[ol-1])) ol -= 1 ;
	    fmt = (ol) ? "%s-%s (%t)" : "%s-%s" ;
	    if ((rl = bufprintf(rbuf,rlen,fmt,sn,pv,op,ol)) >= 0) {
	        cchar	**vpp = &lip->xmailer ;
	        rs = locinfo_setentry(lip,vpp,rbuf,rl) ;
	    } else {
	        rl = 0 ;
	    }
	} else {
	    rl = strlen(lip->xmailer) ;
	}

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (locinfo_xmailer) */


#ifdef	COMMENT
static int locinfo_loaduids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	lip->uid = pip->uid ;
	lip->euid = pip->euid ;
	return SR_OK ;
}
/* end subroutine (locinfo_loaduids) */
#endif /* COMMENT */


static int locinfo_loadprids(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->uid_pr < 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(pip->pr,&sb)) >= 0) {
	        lip->uid_pr = sb.st_uid ;
	        lip->gid_pr = sb.st_gid ;
	    } /* end if (u_stat) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (locinfo_loadprids) */


static int locinfo_gm(LOCINFO *lip,cchar *gnp,int gnl)
{
	LOCINFO_GMCUR	gc ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = locinfo_gmcurbegin(lip,&gc)) >= 0) {
	    if ((rs = locinfo_gmlook(lip,&gc,gnp,gnl)) > 0) {
	        const int	ul = USERNAMELEN ;
	        char		ub[USERNAMELEN+1] ;
	        while ((rs = locinfo_gmread(lip,&gc,ub,ul)) > 0) {
	            rs = locinfo_loadrecip(lip,ub,rs) ;
	            c += rs ;
	            if (rs < 0) break ;
	        } /* end while */
	    } /* end if */
	    rs1 = locinfo_gmcurend(lip,&gc) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (gmcursor) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (locinfo_gm) */


int locinfo_gmcurbegin(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;

	if (! lip->open.gm) {
	    const int	max = 20 ;
	    const int	ttl = (12*3600) ;
	    rs = grmems_start(&lip->gm,max,ttl) ;
	    lip->open.gm = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = grmems_curbegin(&lip->gm,&curp->gmcur) ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmcurbegin) */


int locinfo_gmcurend(LOCINFO *lip,LOCINFO_GMCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = grmems_curend(&lip->gm,&curp->gmcur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_gmcurend) */


int locinfo_gmlook(LOCINFO *lip,LOCINFO_GMCUR *curp,cchar *gnp,int gnl)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;

	if ((rs = grmems_lookup(&lip->gm,&curp->gmcur,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmlook) */


int locinfo_gmread(LOCINFO *lip,LOCINFO_GMCUR *curp,char *ubuf,int ulen)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;

	if ((rs = grmems_lookread(&lip->gm,&curp->gmcur,ubuf,ulen)) == rsn) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (locinfo_gmread) */


int locinfo_rncurbegin(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;

	if (curp == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rncurbegin: ent\n") ;
#endif
	if (! lip->open.rn) {
	    rs = sysrealname_open(&lip->rn,NULL) ;
	    lip->open.rn = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = sysrealname_curbegin(&lip->rn,&curp->rncur) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rncurbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_rncurbegin) */


int locinfo_rncurend(LOCINFO *lip,LOCINFO_RNCUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp == NULL) return SR_FAULT ;

	rs1 = sysrealname_curend(&lip->rn,&curp->rncur) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_rncurend) */


int locinfo_rnlook(LOCINFO *lip,LOCINFO_RNCUR *curp,cchar *gnp,int gnl)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	const int	fo = 0 ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (gnp == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

	if ((rs = sysrealname_look(&lip->rn,&curp->rncur,fo,gnp,gnl)) >= 0) {
	    rs = 1 ;
	} else if (rs == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnlook: sysrealname_look() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnlook) */


int locinfo_rnread(LOCINFO *lip,LOCINFO_RNCUR *curp,char *ubuf,int ulen)
{
	PROGINFO	*pip = lip->pip ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (pip == NULL) return SR_FAULT ;

	if ((ulen >= 0) && (ulen < USERNAMELEN)) return SR_OVERFLOW ;

	if ((rs = sysrealname_lookread(&lip->rn,&curp->rncur,ubuf)) == rsn) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/locinfo_rnread: sysrealname_lookread() rs=%d\n",
	        rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_rnread) */


static int locinfo_folderdname(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->folderdname == NULL) {
	    cchar	*hdname = pip->homedname ;
	    cchar	*folder = lip->folder ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if (folder[0] == '/') {
	        rs = mkpath1(tbuf,folder) ;
	    } else {
	        rs = mkpath2(tbuf,hdname,folder) ;
	    }
	    if (rs >= 0) {
	        const int	tl = rs ;
	        const int	rsn = SR_NOENT ;
	        struct ustat	sb ;
	        if ((rs = uc_stat(tbuf,&sb)) == rsn) {
	            if ((rs = mkdirs(tbuf,0775)) >= 0) {
	                if (pip->euid != pip->uid) {
	                    rs = u_chown(tbuf,pip->uid,-1) ;
	                }
	            }
	        } else if (! S_ISDIR(sb.st_mode))
	            rs = SR_NOTDIR ;
	        if (rs >= 0) {
	            cchar	**vpp = &lip->folderdname ;
	            rs = locinfo_setentry(lip,vpp,tbuf,tl) ;
	        } /* end if (ok) */
	    } /* end if (mkpath) */
	}
	return rs ;
}
/* end subroutine (locinfo_folderdname) */


static int locinfo_groupname(LOCINFO *lip)
{
	int		rs ;
	if (lip == NULL) return SR_FAULT ;
	if (lip->gnbuf[0] == '\0') {
	    const int	gnlen = GROUPNAMELEN ;
	    rs = getgroupname(lip->gnbuf,gnlen,-1) ;
	} else {
	    rs = strlen(lip->gnbuf) ;
	}
	return rs ;
}
/* end subroutine (locinfo_groupname) */


static int locinfo_pcsns(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (! lip->open.ns) {
	    if ((rs = locinfo_prpcs(lip)) >= 0) {
	        cchar	*pr_pcs = lip->pr_pcs ;
	        if ((rs = pcsns_open(&lip->ns,pr_pcs)) >= 0) {
	            lip->open.ns = TRUE ;
	        }
	    }
	} /* end if (needed initialization) */
	return rs ;
}
/* end subroutine (locinfo_pcsns) */


static int locinfo_pcsnsget(LOCINFO *lip,char *rbuf,int rlen,cchar *un,int w)
{
	PROGINFO	*pip = lip->pip ;
	int		rs ;
	if (pip == NULL) return SR_FAULT ;
	if ((rs = locinfo_pcsns(lip)) >= 0) {
	    rs = pcsns_get(&lip->ns,rbuf,rlen,un,w) ;
	}
	return rs ;
}
/* end subroutine (locinfo_pcsnsget) */


static int locinfo_isnotdisabled(LOCINFO *lip,int w)
{
	int		rs = SR_OK ;
	int		f = TRUE ;
	cchar		*hdraddr = NULL ;
	switch (w) {
	case 0:
	    hdraddr = lip->hdraddr_replyto ;
	    break ;
	case 1:
	    hdraddr = lip->hdraddr_errorsto ;
	    break ;
	} /* end switch */
	if ((hdraddr != NULL) && (hdraddr[0] != '\0')) {
	    if ((rs = optbool(hdraddr,-1)) == 0) {
	        f = FALSE ;
	    } else if (isNotValid(rs)) {
	        f = TRUE ;
	        rs = SR_OK ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (locinfo_isnotdisabled) */


static int mailfile_open(MAILFILE *mfp,PROGINFO *pip,cchar *ifname)
{
	int		rs = SR_OK ;
	int		mfd = -1 ;

	if ((ifname == NULL) || (ifname[0] == '\0') || (ifname[0] == '-'))
	    ifname = STDINFNAME ;

	memset(mfp,0,sizeof(MAILFILE)) ;
	mfp->pip = pip ;
	mfp->mfd = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mailfile_open: fn=%s\n",ifname) ;
#endif

	if (isfnamespecial(ifname,-1)) {
	    if ((rs = shio_open(&mfp->infile,ifname,"r",0666)) >= 0) {
	        LOCINFO	*lip = pip->lip ;
	        if ((rs = locinfo_jobdname(lip)) >= 0) {
	            rs = mailfile_altsetup(mfp,lip->jobdname) ;
	            mfd = rs ;
	        }
	        shio_close(&mfp->infile) ;
	    } /* end if (open-file) */
	} else {
	    const mode_t	om = 0660 ;
	    const int		of = O_RDWR ;
	    rs = uc_open(ifname,of,om) ;
	    mfd = rs ;
	} /* end if (open-file) */

	if (rs >= 0) mfp->mfd = mfd ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mailfile_open: ret rs=%d mfd=%u\n",rs,mfd) ;
#endif

	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (mailfile_open) */


static int mailfile_altsetup(MAILFILE *mfp,cchar *tmpdname)
{
	PROGINFO	*pip = mfp->pip ;
	int		rs ;
	int		mfd = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/mailfile_altsetup: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ; /* LINT */

	if ((rs = opentmp(tmpdname,0,0)) >= 0) {
	    const int	tlen = 2048 ;
	    char	*tbuf ;
	    mfd = rs ;
	    if ((rs = uc_valloc(tlen,&tbuf)) >= 0) {
	        SHIO	*ifp = &mfp->infile ;
	        int	wlen = 0 ;

	        while ((rs = shio_read(ifp,tbuf,tlen)) > 0) {
	            rs = u_write(mfd,tbuf,rs) ;
	            wlen += rs ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (wlen > 0)) {
	            rs = u_rewind(mfd) ;
	        }

	        uc_free(tbuf) ;
	    } /* end if (m-a-f) */
	    if (rs < 0) {
	        u_close(mfd) ;
	        mfd = -1 ;
	    }
	} /* end if (opentmp) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("b_imail/mailfile_altsetup: ret rs=%d mfd=%u\n",
	        rs,mfd) ;
#endif

	return (rs >= 0) ? mfd : rs ;
}
/* end subroutine (mailfile_altsetup) */


static int mailfile_close(MAILFILE *mfp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mfp == NULL) return SR_FAULT ;

/* only close out 'mfp->mfd' if we opned it ourselves */

	if (mfp->mfd >= 0) {
	    rs1 = u_close(mfp->mfd) ;
	    if (rs >= 0) rs = rs1 ;
	    mfp->mfd = -1 ;
	}

	if (mfp->mailfname != NULL) {
	    if (mfp->mailfname[0] != '\0') {
	        u_unlink(mfp->mailfname) ;
	    }
	    rs1 = uc_free(mfp->mailfname) ;
	    if (rs >= 0) rs = rs1 ;
	    mfp->mailfname = NULL ;
	}

	return rs ;
}
/* end subroutine (mailfile_close) */


#ifdef	COMMENT

static int mailfile_getfd(MAILFILE *mfp)
{


	if (mfp == NULL)
	    return SR_FAULT ;

	return mfp->mfd ;
}
/* end subroutine (mailfile_getfd) */

static int mailfile_read(MAILFILE *mfp,char rbuf[],int rlen)
{
	int		rs = SR_OK ;

	if (mfp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (mfp->mfd >= 0) {
	    rs = u_read(mfp->mfd,rbuf,rlen) ;
	} else {
	    rs = shio_read(&mfp->infile,rbuf,rlen) ;
	}

	return rs ;
}
/* end subroutine (mailfile_read) */

#endif /* COMMENT */


/* configuration maintenance */
static int config_start(CONFIG *cfp,PROGINFO *pip,cchar *cfname)
{
	PARAMFILE	*pfp ;
	int		rs ;
	int		c = 0 ;

	if (cfp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/config_start: ent cf=%s\n",cfname) ;
#endif

	memset(cfp,0,sizeof(struct config)) ;
	cfp->pip = pip ;

	pfp = &cfp->params ;
	if ((rs = paramfile_open(pfp,pip->envv,NULL)) >= 0) {

	    if ((cfname != NULL) && (cfname[0] != '\0')) {

	        if ((rs = perm(cfname ,-1,-1,NULL,R_OK)) >= 0) {
	            c += 1 ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_imail/config_start: cf=%s\n",cfname) ;
#endif

	            rs = paramfile_fileadd(pfp,cfname) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_imail/config_start: "
	                    "1 paramfile_fileadd() rs=%d \n", rs) ;
#endif

	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }

	    } else {
	        rs = config_files(cfp,pfp) ;
	        c += rs ;
	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("b_imail/config_start: mid rs=%d c=%u\n",rs,c) ;
#endif

	    if ((rs >= 0) && (c > 0)) {
	        rs = config_starter(cfp) ;
	    }

	    if (rs < 0)
	        paramfile_close(&cfp->params) ;
	} /* end if (paramfile) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/config_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_start) */


static int config_starter(CONFIG *cfp)
{
	int		rs ;
	EXPCOOK		*ckp = &cfp->cooks ;
	if ((rs = expcook_start(ckp)) >= 0) {
	    if ((rs = config_loadcooks(cfp)) >= 0) {
	        cfp->f.p = TRUE ;
	        rs = config_read(cfp) ;
	    }
	    if (rs < 0)
	        expcook_finish(ckp) ;
	}
	return rs ;
}
/* end subroutine (config_starter) */


static int config_files(CONFIG *cfp,PARAMFILE *pfp)
{
	PROGINFO	*pip = cfp->pip ;
	LOCINFO		*lip ;
	vecstr		sv ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*cfn = CONFIGFNAME ;

	lip = pip->lip ;
	if (lip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/config_files: ent\n") ;
#endif

	if ((rs = vecstr_start(&sv,4,0)) >= 0) {
	    const int	am = R_OK ;
	    const int	rlen = MAXPATHLEN ;
	    char	rbuf[MAXPATHLEN + 1] ;

	    vecstr_envadd(&sv,"r",pip->pr,-1) ;
	    vecstr_envadd(&sv,"e","etc",-1) ;
	    vecstr_envadd(&sv,"n",pip->searchname,-1) ;

	    if (rs >= 0) {
	        if ((rs = permsched(schedconf,&sv,rbuf,rlen,cfn,am)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_imail/config_files: cf=%s\n",rbuf) ;
#endif
	            c += 1 ;
	            rs = paramfile_fileadd(pfp,rbuf) ;
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    }

	    if (rs >= 0) {
	        vecstr_envset(&sv,"r",pip->homedname,-1) ;
	    }

	    if (rs >= 0) {
	        if ((rs = permsched(schedconf,&sv,rbuf,rlen,cfn,am)) >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_imail/config_files: cf=%s\n",rbuf) ;
#endif
	            c += 1 ;
	            rs = paramfile_fileadd(pfp,rbuf) ;
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    }

	    rs1 = vecstr_finish(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/config_files: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (config_files) */


static int config_loadcooks(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	EXPCOOK		*ckp = &cfp->cooks ;
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		vl ;
	cchar		*ks = "PSNDHRU" ;
	cchar		*vp ;
	char		hbuf[MAXHOSTNAMELEN + 1] ;
	char		kbuf[2] ;

	kbuf[1] = '\0' ;
	for (i = 0 ; (rs >= 0) && (ks[i] != '\0') ; i += 1) {
	    int	kch = MKCHAR(ks[i]) ;
	    vp = NULL ;
	    vl = -1 ;
	    switch (kch) {
	    case 'P':
	        vp = pip->progname ;
	        break ;
	    case 'S':
	        vp = pip->searchname ;
	        break ;
	    case 'N':
	        vp = pip->nodename ;
	        break ;
	    case 'D':
	        vp = pip->domainname ;
	        break ;
	    case 'H':
	        {
	            cchar	*nn = pip->nodename ;
	            cchar	*dn = pip->domainname ;
	            rs = snsds(hbuf,hlen,nn,dn) ;
	            vl = rs ;
	            vp = hbuf ;
	        }
	        break ;
	    case 'R':
	        vp = pip->pr ;
	        break ;
	    case 'U':
	        vp = pip->username ;
	        break ;
	    } /* end switch */
	    if ((rs >= 0) && (vp != NULL)) {
	        kbuf[0] = kch ;
	        rs = expcook_add(ckp,kbuf,vp,vl) ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (config_loadcooks) */


#ifdef	COMMENT
static int config_check(cfp)
struct config	*cfp ;
{
	PROGINFO	*pip = cfp->pip ;
	int		rs = SR_NOTOPEN ;

	if (cfp == NULL)
	    return SR_FAULT ;

	if (cfp->f.p) {
	    if ((rs = paramfile_check(&cfp->params,pip->daytime)) > 0) {
	        rs = config_read(cfp) ;
	    }
	}

	return rs ;
}
/* end subroutine (config_check) */
#endif /* COMMENT */


static int config_finish(CONFIG *cfp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (cfp == NULL) return SR_FAULT ;
	pip = cfp->pip ;
	if (pip == NULL) return SR_FAULT ;

	if (cfp->f.p) {
	    rs1 = expcook_finish(&cfp->cooks) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = paramfile_close(&cfp->params) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_imail/config_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_finish) */


static int config_read(CONFIG *cfp)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;

	if (cfp == NULL) return SR_FAULT ;

	pip = cfp->pip ;
	if (pip == NULL) return SR_FAULT ; /* lint */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/config_read: f_p=%u\n",cfp->f.p) ;
#endif

	if (cfp->f.p) {
	    rs = config_reader(cfp) ;
	} /* end if (active) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_imail/config_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (config_read) */


static int config_reader(CONFIG *cfp)
{
	PROGINFO	*pip = cfp->pip ;
	PARAMFILE	*pfp = &cfp->params ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	int		rs ;
	int		rs1 ;
	if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	    LOCINFO	*lip = pip->lip ;
	    EXPCOOK	*ckp = &cfp->cooks ;
	    const int	plen = PBUFLEN ;
	    const int	elen = EBUFLEN ;
	    int		oi ;
	    int		kl, vl ;
	    int		el ;
	    int		v ;
	    cchar	*pr = pip->pr ;
	    cchar	*sn = pip->searchname ;
	    cchar	*ccp ;
	    cchar	*kp, *vp ;
	    char	pbuf[PBUFLEN + 1] ;
	    char	ebuf[EBUFLEN + 1] ;
	    char	tbuf[MAXPATHLEN + 1] ;

	    while (rs >= 0) {
	        kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;
	        if (rs < 0) break ;

	        kp = pe.key ;
	        vp = pe.value ;
	        vl = pe.vlen ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("b_imail/config_read: enum k=%t\n",kp,kl) ;
#endif

	        if ((oi = matpstr(params,2,kp,kl)) >= 0) {

	            ebuf[0] = '\0' ;
	            el = 0 ;
	            if (vl > 0) {
	                el = expcook_exp(ckp,0,ebuf,elen,vp,vl) ;
	                if (el >= 0)
	                    ebuf[el] = '\0' ;
	            } /* end if */

	            if (el < 0)
	                continue ;

	            switch (oi) {

	            case param_logsize:
	                if ((! pip->final.logsize) && (el > 0)) {
	                    pip->have.logsize = TRUE ;
	                    rs = cfdecmfi(ebuf,el,&v) ;
	                    pip->logsize = v ;
	                }
	                break ;

	            case param_to:
	                if ((! lip->final.to) && (el > 0)) {
	                    lip->have.to = TRUE ;
	                    rs = cfdecti(ebuf,el,&v) ;
	                    lip->to = v ;
	                }
	                break ;

	            case param_logfile:
	                if (! pip->final.lfname) {
	                    cchar	*ld = LOGDNAME ;
	                    pip->have.lfname = TRUE ;
	                    rs1 = prsetfname(pr,tbuf,ebuf,el,TRUE,ld,sn,"") ;
	                    ccp = pip->lfname ;
	                    if ((ccp == NULL) || (strcmp(ccp,tbuf) != 0)) {
	                        cchar	**vpp = &pip->lfname ;
	                        pip->changed.lfname = TRUE ;
	                        rs = proginfo_setentry(pip,vpp,tbuf,rs1) ;
	                    }
	                }
	                break ;

	            case param_folder:
	                if ((! lip->final.folder) && (el > 0)) {
	                    rs = locinfo_setfolder(lip,ebuf,el) ;
	                }
	                break ;

	            case param_copy:
	                if ((! lip->final.cmbname) && (el > 0)) {
	                    lip->have.cmbname = TRUE ;
	                    lip->f.cmbname = TRUE ;
	                    rs = locinfo_setcmb(lip,ebuf,el) ;
	                }
	                break ;

	            case param_postmail:
	                if ((! lip->final.postmail) && (el > 0)) {
	                    cchar	**vpp = &lip->postmail ;
	                    rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                }
	                break ;

	            case param_sendmail:
	                if ((! lip->final.sendmail) && (el > 0)) {
	                    cchar	**vpp = &lip->sendmail ;
	                    rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                }
	                break ;

	            case param_mailhost:
	                if ((! lip->final.mailhost) && (el > 0)) {
	                    cchar	**vpp = &lip->mailhost ;
	                    rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                }
	                break ;

	            case param_mailer:
	                if ((! lip->final.xmailer) && (el > 0)) {
	                    lip->have.xmailer = TRUE ;
	                    lip->f.xmailer = TRUE ;
	                    if ((rs1 = optbool(ebuf,el)) >= 0) {
	                        lip->f.xmailer = (rs1 > 0) ;
	                    } else {
	                        cchar	**vpp = &lip->xmailer ;
	                        rs = locinfo_setentry(lip,vpp,ebuf,el) ;
	                    }
	                }
	                break ;

	            case param_org:
	                if (! lip->final.org) {
	                    lip->have.org = TRUE ;
	                    lip->f.org = TRUE ;
	                    if (el > 0) {
	                        if ((rs1 = optbool(ebuf,el)) >= 0) {
	                            lip->f.org = (rs1 > 0) ;
	                        } else if (pip->org == NULL) {
	                            cchar	**vpp = &pip->org ;
	                            rs = proginfo_setentry(pip,vpp,ebuf,el) ;
	                        }
	                    }
	                }
	                break ;

	            case param_useclen:
	                if (! lip->final.useclen) {
	                    lip->have.useclen = TRUE ;
	                    lip->final.useclen = TRUE ;
	                    lip->f.useclen = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.useclen = (rs > 0) ;
	                    }
	                }
	                break ;

	            case param_useclines:
	                if (! lip->final.useclines) {
	                    lip->have.useclines = TRUE ;
	                    lip->final.useclines = TRUE ;
	                    lip->f.useclines = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.useclines = (rs > 0) ;
	                    }
	                }
	                break ;

	            case param_subjrequired:
	                if (! lip->final.reqsubj) {
	                    lip->have.reqsubj = TRUE ;
	                    lip->final.reqsubj = TRUE ;
	                    lip->f.reqsubj = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.reqsubj = (rs > 0) ;
	                    }
	                }
	                break ;

	            case param_allok:
	                if (! lip->final.allok) {
	                    lip->have.allok = TRUE ;
	                    lip->final.allok = TRUE ;
	                    lip->f.allok = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.allok = (rs > 0) ;
	                    }
	                }
	                break ;

	            case param_addsender:
	                if (! lip->final.addsender) {
	                    lip->have.addsender = TRUE ;
	                    lip->final.addsender = TRUE ;
	                    lip->f.addsender = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lip->f.addsender = (rs > 0) ;
	                    }
	                }
	                break ;

	            } /* end switch */

	        } /* end if (valid option) */

	        if (rs < 0) break ;
	    } /* end while (enumerating) */

	    rs1 = paramfile_curend(pfp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramfile-cur) */
	return rs ;
}
/* end subroutine (config_reader) */


static int msgdata_start(MSGDATA *mdp,int mn)
{
	int		rs = SR_OK ;
	int		n, i ;

	if (mdp == NULL) return SR_FAULT ;

	memset(mdp,0,sizeof(MSGDATA)) ;
	mdp->mn = mn ;
	for (i = 0 ; i < msgloghdr_overlast ; i += 1) {
	    mdp->naddrs[i] = -1 ;
	}

	for (i = 0 ; i < msgloghdr_overlast ; i += 1) {
	    rs = ema_start(mdp->addrs + i) ;
	    if (rs < 0) break ;
	    mdp->naddrs[i] = 0 ;
	} /* end for */
	n = i ;
	if (rs < 0) {
	    for (i = 0 ; i < n ; i += 1) {
	        if (mdp->naddrs[i] >= 0) {
	            ema_finish(mdp->addrs + i) ;
	            mdp->naddrs[i] = -1 ;
	        }
	    }
	} /* end if (error) */

	return rs ;
}
/* end subroutine (msgdata_start) */


static int msgdata_setmid(MSGDATA *mdp,cchar *midp,int midl)
{
	int		rs = SR_OK ;
	cchar		*cp ;

	if (mdp == NULL) return SR_FAULT ;
	if (midp == NULL) return SR_FAULT ;

	if (mdp->hdrmid == NULL) {
	    if ((rs = uc_mallocstrw(midp,midl,&cp)) >= 0) {
	        mdp->hdrmid = cp ;
	    }
	} else
	    rs = strlen(mdp->hdrmid) ;

	return rs ;
}
/* end subroutine (msgdata_setmid) */


static int msgdata_loadrecips(MSGDATA *mdp,MAILMSGSTAGE *msp,int mi)
{
	int		rs = SR_OK ;
	int		hi ;
	int		ai ;
	int		c = 0 ;
	cchar		*kn ;

#if	CF_DEBUGS
	debugprintf("msgdata_loadrecips: ent\n") ;
#endif

	for (ai = 0 ; (rs >= 0) && ((hi = msgloghi[ai]) >= 0) ; ai += 1) {
	    kn = mailmsghdrs_names[hi] ;
#if	CF_DEBUGS
	    debugprintf("msgdata_loadrecips: kn{%p}=%s\n",kn,kn) ;
#endif
	    rs = msgdata_loadrecip(mdp,ai,msp,mi,kn) ;
	    c += rs ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("msgdata_loadrecips: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msgdata_loadrecips) */


static int msgdata_loadrecip(MSGDATA *mdp,int ai,MAILMSGSTAGE *msp,int mi,
cchar *kn)
{
	int		rs = SR_OK ;
	int		hi = 0 ;
	int		hl ;
	int		c = 0 ;
	cchar		*hp ;

	while (rs >= 0) {
	    hl = mailmsgstage_hdrival(msp,mi,kn,hi++,&hp) ;
	    if (hl == SR_NOTFOUND) break ;
	    rs = hl ;
	    if ((rs >= 0) && (hl > 0)) {
	        rs = msgdata_loadrecipadd(mdp,ai,hp,hl) ;
	        c += rs ;
	    }
	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msgdata_loadrecip) */


static int msgdata_loadrecipadd(MSGDATA *mdp,int ai,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (mdp->naddrs[ai] < 0) {
	    if ((rs = ema_start(mdp->addrs + ai)) >= 0) {
	        mdp->naddrs[ai] = 0 ;
	    }
	}

	if (rs >= 0) {
	    EMA		*alp = (mdp->addrs + ai) ;
	    if ((rs = ema_parse(alp,sp,sl)) >= 0) {
	        c = rs ;
	        mdp->naddrs[ai] += c ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msgdata_loadrecipadd) */


static int msgdata_checksubject(MSGDATA *mdp,MAILMSGSTAGE *msp,int mi)
{
	int		rs ;
	int		hl ;
	int		sl = 0 ;
	cchar		*kn = HN_SUBJECT ;
	cchar		*hp ;

	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) >= 0) {
	    cchar	*sp ;
	    hl = rs ;
	    if ((sl = sfshrink(hp,hl,&sp)) > 0) {
	        mdp->f.subject = TRUE ;
	    } /* end if (positive) */
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	} /* end if (mailmsgstage_hdrval) */

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (msgdata_checksubject) */


#ifdef	COMMENT
static int msgdata_loadbodytype(MSGDATA *mdp,MAILMSGSTAGE *msp,int mi)
{
	CTYPE		ct ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		hl ;
	cchar		*kn = HN_CTYPE ;
	cchar		*hp ;

	if ((rs = mailmsgstage_hdrval(msp,mi,kn,&hp)) > 0) {
	    hl = rs ;
	    rs1 = ctype_decode(&ct,hp,hl) ;
	} else if (isHdrEmpty(rs)) {
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (msgdata_loadbodytype) */
#else /* COMMENT */
static int msgdata_loadbodytype(MSGDATA *mdp,MAILMSGSTAGE *msp,int mi)
{
	int		rs = SR_OK ;

	if (mi >= 0) rs = mi ;
	return rs ;
}
/* end subroutine (msgdata_loadbodytype) */
#endif /* COMMENT */


/* mark (or not mark) those senders that match FROM addreses */
/* ARGSUSED */
static int msgdata_procsender(MSGDATA *mdp,LOCINFO *lip)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("b_imail/msgdata_procsender: ent\n") ;
#endif
	if (mdp->naddrs[msgloghdr_from] > 0) {
	    EMA		*flp = (mdp->addrs + msgloghdr_from) ;
	    const int	f_onesender = lip->f.onesender ;
	    int		fl ;
	    cchar	*fp ;
	    if ((rs = ema_first(flp,&fp)) > 0) {
	        int	c = 0 ;
	        int	si = 0 ;
	        fl = rs ;
#if	CF_DEBUGS
	        debugprintf("b_imail/msgdata_procsender: fl=%d f=>%t<\n",
	            fl,fp,fl) ;
#endif
	        if (mdp->naddrs[msgloghdr_sender] > 0) {
	            EMA		*slp = (mdp->addrs + msgloghdr_sender) ;
	            int		sl ;
	            cchar	*sp ;
	            while ((sl = ema_getbestaddr(slp,si,&sp)) >= 0) {
#if	CF_DEBUGS
	                debugprintf("b_imail/msgdata_procsender: i=%u\n") ;
	                debugprintf("b_imail/msgdata_procsender: sl=%d s=?%t<\n",
	                    sl,sp,sl) ;
#endif
	                if (sl > 0) {
	                    if ((fl != sl) || (stremacmp(fp,sp) != 0)) {
	                        c += 1 ;
	                        rs = msgdata_setvsender(mdp,si) ;
	                    }
	                }
	                if ((sl > 0) && f_onesender) break ;
	                if (rs < 0) break ;
	                si += 1 ;
	            } /* end while */
	        } /* end if (positive) */
#if	CF_DEBUGS
	        debugprintf("msgdata_procsender: si=%u\n",si) ;
#endif
	        if ((rs >= 0) && (si == 0) && lip->f.addsender) {
	            if ((rs = locinfo_mkhdrsender(lip)) >= 0) {
	                const int	hl = rs ;
	                const int	alen = MAILADDRLEN ;
	                cchar		*hp = lip->hdraddr_sender ;
	                char		*abuf ;
#if	CF_DEBUGS
	                debugprintf("msgdata_procsender: fl=%d\n",fl) ;
	                debugprintf("msgdata_procsender: fp=%s\n",fp) ;
	                debugprintf("msgdata_procsender: hl=%d\n",hl) ;
	                debugprintf("msgdata_procsender: hp=%s\n",hp) ;
#endif
	                if ((rs = uc_malloc((alen+1),&abuf)) >= 0) {
	                    if ((rs = mkbestaddr(abuf,alen,hp,hl)) > 0) {
	                        const int	al = rs ;
	                        cchar		*ap = abuf ;
	                        if ((fl == al) && (stremacmp(fp,ap) == 0)) {
	                            mdp->f.disallowsender = TRUE ;
	                        }
	                    } /* end if (mkbestaddr) */
	                    uc_free(abuf) ;
	                } /* end if (m-a-f) */
	            } /* end if (locinfo_mkhdrsender) */
	        } /* end if (add-sender) */
	        mdp->nvsenders = c ;
	    } /* end if (ema_first) */
	} /* end if (positive) */
#if	CF_DEBUGS
	debugprintf("msgdata_procsender: ret rs=%d f_dis=%u\n",rs,
	    mdp->f.disallowsender) ;
#endif
	return rs ;
}
/* end subroutine (msgdata_procsender) */


static int msgdata_getnvsenders(MSGDATA *mdp)
{
	return mdp->nvsenders ;
}
/* end subroutine (msgdata_getnvsenders) */


static int msgdata_getnsenders(MSGDATA *mdp)
{
	const int	at = msgloghdr_sender ;
	return mdp->naddrs[at] ;
}
/* end subroutine (msgdata_getnsenders) */


static int msgdata_setvsender(MSGDATA *mdp,int si)
{
	int		rs = SR_OK ;
	if (! mdp->f.senders) {
	    if ((rs = bits_start(&mdp->senders,1)) >= 0) {
	        mdp->f.senders = TRUE ;
	    }
	}
	if (rs >= 0) {
	    rs = bits_set(&mdp->senders,si) ;
	}
	return rs ;
}
/* end subroutine (msgdata_setvsender) */


static int msgdata_isvsender(MSGDATA *mdp,int si)
{
	int		rs = SR_OK ;
	if (mdp->f.senders) {
	    rs = bits_test(&mdp->senders,si) ;
	}
	return rs ;
}
/* end subroutine (msgdata_isvsender) */


static int msgdata_sethlen(MSGDATA *mdp,int hlen)
{
	if (mdp == NULL) return SR_FAULT ;
	mdp->hlen = hlen ;
	return SR_OK ;
}
/* end subroutine (msgdata_sethlen) */


static int msgdata_setsubject(MSGDATA *mdp)
{
	int		rs = SR_OK ;
	if (mdp == NULL) return SR_FAULT ;
	mdp->f.subject = TRUE ;
	return rs ;
}
/* end subroutine (msgdata_setsubject) */


static int msgdata_setnrecips(MSGDATA *mdp,int nrecips)
{
	if (mdp == NULL) return SR_FAULT ;
	mdp->nrecips = nrecips ;
	return SR_OK ;
}
/* end subroutine (msgdata_setnrecips) */


static int msgdata_getnrecips(MSGDATA *mdp)
{
	int		rs ;
	if (mdp == NULL) return SR_FAULT ;
	rs = mdp->nrecips ;
	return rs ;
}
/* end subroutine (msgdata_getnrecips) */


static int msgdata_getsubject(MSGDATA *mdp,cchar **rpp)
{
	int		rs = SR_OK ;
	if (mdp == NULL) return SR_FAULT ;
	if (mdp->f.subject) {
	    rs = 1 ;
	}
	if (rpp != NULL) *rpp = NULL ;
	return rs ;
}
/* end subroutine (msgdata_getsubject) */


static int msgdata_finish(MSGDATA *mdp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (mdp == NULL) return SR_FAULT ;

	if (mdp->f.senders) {
	    mdp->f.senders = FALSE ;
	    rs1 = bits_finish(&mdp->senders) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (mdp->env != NULL) {
	    rs1 = uc_free(mdp->env) ;
	    if (rs >= 0) rs = rs1 ;
	    mdp->env = NULL ;
	}

	if (mdp->envfrom != NULL) {
	    rs1 = uc_free(mdp->envfrom) ;
	    if (rs >= 0) rs = rs1 ;
	    mdp->envfrom = NULL ;
	}

	if (mdp->envdate != NULL) {
	    rs1 = uc_free(mdp->envdate) ;
	    if (rs >= 0) rs = rs1 ;
	    mdp->envdate = NULL ;
	}

	if (mdp->retpath != NULL) {
	    rs1 = uc_free(mdp->retpath) ;
	    if (rs >= 0) rs = rs1 ;
	    mdp->retpath = NULL ;
	}

	if (mdp->hdrmid != NULL) {
	    rs1 = uc_free(mdp->hdrmid) ;
	    if (rs >= 0) rs = rs1 ;
	    mdp->hdrmid = NULL ;
	}

	for (i = 0 ; i < msgloghdr_overlast ; i += 1) {
	    if (mdp->naddrs[i] >= 0) {
	        rs1 = ema_finish(mdp->addrs + i) ;
	        if (rs >= 0) rs = rs1 ;
	        mdp->naddrs[i] = -1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (msgdata_finish) */


#ifdef	COMMENT
static int msgdata_setfrom(MSGDATA *mdp,cchar *sp,int sl)
{
	int		rs ;
	cchar		*cp ;

	if (mdp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (mdp->hdrfrom != NULL) return SR_NOANODE ;

	if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	    mdp->hdrfrom = cp ;
	}

	return rs ;
}
/* end subroutine (msgdata_setfrom) */
#endif /* COMMENT */


#if	CF_MSGOPTSNONE
static int msgopts_none(MSGOPTS *optp)
{
	memset(optp,0,sizeof(MSGOPTS)) ;
	return SR_OK ;
}
/* end subroutine (msgopts_none) */
#endif


static int msgopts_all(MSGOPTS *optp)
{
	memset(optp,0,sizeof(MSGOPTS)) ;
	optp->mkenv = TRUE ;
	optp->mkclen = TRUE ;
	optp->mkclines = TRUE ;
	optp->mkfrom = TRUE ;
	optp->mkbcc = TRUE ;
	optp->mksubj = TRUE ;
	return SR_OK ;
}
/* end subroutine (msgopts_all) */


static int msgopts_dead(MSGOPTS *optp)
{
	memset(optp,0,sizeof(MSGOPTS)) ;
	optp->mkenv = TRUE ;
	optp->mkclen = TRUE ;
	optp->mkclines = TRUE ;
	optp->mkbcc = TRUE ;
	return SR_OK ;
}
/* end subroutine (msgopts_dead) */


static int loadorg(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		ol = 0 ;
	if ((pip->org == NULL) || (pip->org[0] == '\0')) {
	    const int	olen = ORGLEN ;
	    cchar	*pr = pip->progname ;
	    cchar	*un = pip->username ;
	    char	obuf[ORGLEN+1] ;
	    if ((rs = localgetorg(pr,obuf,olen,un)) >= 0) {
	        cchar	**vpp = &pip->org ;
	        ol = rs ;
	        rs = proginfo_setentry(pip,vpp,obuf,ol) ;
	    }
	} else {
	    ol = strlen(pip->org) ;
	}
	return (rs >= 0) ? ol : rs ;
}
/* end subroutine (loadorg) */


static int loadpath(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*varpath = VARPATH ;
	cchar		*tp ;

/* process environment */

	if ((tp = getourenv(pip->envv,varpath)) != NULL) {
	    rs = loadpathspec(pip,tp) ;
	    c += rs ;
	}

/* system-default path */

#if	CF_CSPATH
	if (rs >= 0) {
	    rs = loadpathcs(pip) ;
	}
#endif /* CF_CSPATH */

/* our program root */

	if (rs >= 0) {
	    rs = loadpathpr(pip) ;
	    c += rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


#if	CF_CSPATH
static int loadpathcs(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*plp ;
	plp = &lip->epath ;
	return vecstr_addcspath(plp) ;
}
/* end subroutine (loadpathcs) */
#endif /* CF_CSPATH */


static int loadpathpr(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	for (i = 0 ; prbins[i] != NULL ; i += 1) {
	    rs = loadpathprbin(pip,prbins[i]) ;
	    c += rs ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathpr) */


static int loadpathprbin(PROGINFO *pip,cchar bname[])
{
	int		rs ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs = mkpath2(tbuf,pip->pr,bname)) >= 0) {
	    const int	pl = rs ;
	    rs = loadpathcomp(pip,tbuf,pl) ;
	    c += rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathprbin) */


static int loadpathspec(PROGINFO *pip,cchar *pp)
{
	int		rs = SR_OK ;
	int		pl ;
	int		c = 0 ;
	cchar		*tp ;

	while ((tp = strpbrk(pp,":;")) != NULL) {
	    pl = (tp - pp) ;
	    rs = loadpathcomp(pip,pp,pl) ;
	    c += rs ;
	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {
	    rs = loadpathcomp(pip,pp,-1) ;
	    c += rs ;
	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathspec) */


static int loadpathcomp(PROGINFO *pip,cchar pbuf[],int plen)
{
	LOCINFO		*lip = pip->lip ;
	vecstr		*plp ;
	int		rs ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	plp = &lip->epath ;
	if ((rs = pathclean(tbuf,pbuf,plen)) >= 0) { /* allow zero-length */
	    const int	rsn = SR_NOTFOUND ;
	    const int	pl = rs ;
	    cchar	*pp = tbuf ;
	    if ((rs = vecstr_findn(plp,pp,pl)) == rsn) {
	        c += 1 ;
	        rs = vecstr_add(plp,pp,pl) ;
	    }
	} /* end if (pathclean) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathcomp) */


static int vrecipsch(const void	**v1pp,const void **v2pp)
{
	cchar		**e1pp = (cchar **) v1pp ;
	cchar		**e2pp = (cchar **) v2pp ;
	int		rc = 0 ;
#if	CF_DEBUGS && CF_DEBUGSRCH
	debugprintf("main/vrecipsch: ent\n") ;
#endif
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            cchar	*s1p = *e1pp ;
	            cchar	*s2p = *e2pp ;
#if	CF_DEBUGS && CF_DEBUGSRCH
	            debugprintf("main/vrecipsch: s1=%s\n",s1p) ;
	            debugprintf("main/vrecipsch: s2=%s\n",s2p) ;
#endif
	            if ((rc = (*s1p - *s2p)) == 0) {
	                rc = stremacmp(s1p,s2p) ;
	            }
#if	CF_DEBUGS && CF_DEBUGSRCH
	            debugprintf("main/vrecipsch: rc=%d\n",rc) ;
#endif
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
#if	CF_DEBUGS && CF_DEBUGSRCH
	debugprintf("main/vrecipsch: ret rc=%d\n",rc) ;
#endif
	return rc ;
}
/* end subroutine (vrecipsch) */


static int ishigh(int ch)
{
	return (ch & 0x80) ;
}
/* end subroutine (ishigh) */


static int hasMailNote(cchar *hp,int hl,cchar *ts)
{
	const int	tl = strlen(ts) ;
	if (hl < 0) hl = strlen(hp) ;
	return (hl >= tl) && (strncmp(hp,ts,tl) == 0) ;
}
/* end subroutine (hasMailNote) */


static int isBadDate(int rs)
{
	return isOneOf(rsdatebad,rs) ;
}
/* end subroutine (isBadDate) */


static int isHdrEmpty(int rs)
{
	const int	rsn = SR_NOTFOUND ;
	return ((rs == 0) || (rs == rsn)) ;
}
/* end subroutine (isHdrEmpty) */


static int isHup(int rs)
{
	return isOneOf(rshup,rs) ;
}
/* end subroutine (isHup) */


#if	CF_DEBUGOFF && (CF_DEBUG || CF_DEBUGS)
static int debugfilebuf_printoff(FILEBUF *fbp,cchar *n,cchar *s,int wlen)
{
	offset_t	coff ;
	filebuf_tell(fbp,&coff) ;
	debugprintf("%s: s=%s coff=%llu wlen=%u\n",n,s,coff,wlen) ;
	return SR_OK ;
}
#endif /* CF_DEBUG */


