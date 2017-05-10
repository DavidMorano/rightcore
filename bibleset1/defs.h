/* defs */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vecstr.h>
#include	<localmisc.h>

#include	"linecenter.h"
#include	"biblebook.h"
#include	"biblemeta.h"


#ifndef	nelem
#ifdef	nelements
#define	nelem		nelements
#else
#define	nelem(n)	(sizeof(n) / sizeof((n)[0]))
#endif
#endif

#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif

#ifndef	MAXPATHLEN
#define	MAXPATHLEN	2048
#endif

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	USERNAMELEN
#ifndef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#ifndef	LOGNAME_MAX
#define	GROUPNAMELEN	LOGNAME_MAX
#else
#define	GROUPNAMELEN	32
#endif
#endif

#ifndef	LOGNAMELEN
#ifndef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	PROJNAMELEN
#ifndef	LOGNAME_MAX
#define	PROJNAMELEN	LOGNAME_MAX
#else
#define	PROJNAMELEN	32
#endif
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

/* timezone (zoneinfo) name */
#ifndef	TZLEN
#define	TZLEN		60
#endif

#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	NYEARS_CENTURY
#define	NYEARS_CENTURY	100
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	BUFLEN		MAX((MAXPATHLEN + MAXNAMELEN),MAXHOSTNAMELEN)

#undef	HDRBUFLEN
#define	HDRBUFLEN	200

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags
#define	PROGINFO_POUT	struct proginfo_pout
#define	PROGINFO_POFF	struct proginfo_poff
#define	PROGINFO_COUNTS	struct proginfo_counts

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo

#define	TROFFSTRS	struct troffstrs 


struct troffstrs {
	const char	*slash3 ;	/* three slashes */
	const char	*int2 ;		/* interpolate two-char string-name */
	const char	*linecomment ;	/* introduce a line comment */
	const char	*infont_r ;	/* in-line font switching */
	const char	*infont_i ;
	const char	*infont_b ;
	const char	*infont_x ;
	const char	*infont_cw ;
	const char	*infont_p ;
} ;

enum words {
	word_chapter,
	word_psalm,
	word_bookindex,
	word_page,
	word_booktitle,
	word_thebookof,
	word_book,
	word_overlast
} ;

struct proginfo_flags {
	uint		progdash:1 ;
	uint		akopts:1 ;
	uint		aparams:1 ;
	uint		quiet:1 ;
	uint		outfile:1 ;
	uint		errfile:1 ;
	uint		biblebook:1 ;
	uint		biblemeta:1 ;
	uint		cover:1 ;	/* cover EPS file */
	uint		frontmatter:1 ;
	uint		backmatter:1 ;
	uint		pagenums:1 ;	/* provide page numbering */
	uint		ff:1 ;		/* font-family */
	uint		ibz:1 ;		/* ignore-book-zero */
	uint		tc:1 ;		/* table-of-contents */
	uint		pagetitle:1 ;
	uint		hyphenate:1 ;
	uint		ha:1 ;
	uint		ps:1 ;		/* point-size */
	uint		vs:1 ;		/* vertical-size (leading) */
	uint		vzlw:1 ;	/* verse-zero line-width */
	uint		vzlb:1 ;	/* verse-zero line-width */
	uint		inbook:1 ;	/* saw a book */
	uint		inchapter:1 ;
	uint		inverse:1 ;
	uint		inversezero:1 ;
	uint		inkeep:1 ;
	uint		setchapter:1 ; /* set the chapter TROFF-string */
	uint		setverse:1 ;	/* set the verse TROFF-string */
	uint		chapterzero:1 ; /* saw a zero-chapter */
	uint		versezerohalf:1 ;
	uint		chapterbegin:1 ; /* at beginning of chapter */
	uint		octetbegin:1 ;	/* psalm-octet begin */
	uint		reduced:1 ;	/* font-size reduced */
	uint		quoteblock:1 ;	/* quoted-block */
	uint		tmpshortcol:1 ;	/* temporary short output */
	uint		preverseone:1 ;	/* previous to verse=1 */
	uint		maintextheader:1 ;	/* main-text header */
	uint		maintextfooter:1 ;	/* main-text footer */
} ;

struct proginfo_pout {
	char		pageinfo[TIMEBUFLEN + 1] ;
	char		hf_pageinfo[HDRBUFLEN + 1] ;
	char		hf_pagetitle[HDRBUFLEN + 1] ;
	char		hf_pagelocation[HDRBUFLEN + 1] ;
	char		hf_pagenum[HDRBUFLEN + 1] ;
} ;

struct proginfo_poff {
	const char	**chartrans ;
} ;

struct proginfo_counts {
	int		book ;
	int		chapter ;
	int		verse ;
	int		vmissing ;	/* "missing" words in a verse */
} ;

struct proginfo {
	vecstr		stores ;
	const char	**envv ;
	const char	*pwd ;
	const char	*progename ;	/* execution filename */
	const char	*progdname ;	/* dirname of arg[0] */
	const char	*progname ;	/* basename of arg[0] */
	const char	*pr ;		/* program root */
	const char	*searchname ;
	const char	*rootname ;	/* distribution name */
	const char	*version ;
	const char	*banner ;
	const char	*nodename ;
	const char	*domainname ;
	const char	*username ;
	const char	*groupname ;
	const char	*tmpdname ;
	const char	*helpfname ;
	const char	*bibledb ;
	const char	*pagetitle ;
	const char	*pageinfo ;
	const char	*frontfname ;
	const char	*coverfname ;	/* cover EPS filename */
	const char	*ff ;		/* font-family */
	char		*word[word_overlast + 1] ;
	void		*efp ;
	PROGINFO_FL	have, f, changed, final ;
	PROGINFO_FL	open ;
	PROGINFO_COUNTS	c ;
	PROGINFO_POUT	pout ;
	PROGINFO_POFF	poff ;
	TROFFSTRS	troff ;
	LINECENTER	cv ;		/* center-verse */
	BIBLEBOOK	bb ;
	BIBLEMETA	bm ;
	time_t		daytime ;
	pid_t		pid ;
	uint		columns ;	/* code columns */
	uint		linewidth ;	/* page linewidth */
	uint		vzlw ;		/* verse-zero line-width */
	uint		vzlb ;		/* verse-zero line-break */
	uint		ps ;		/* point-size */
	uint		vs ;		/* vertical-spacing */
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
	int		ffi ;		/* font-family index */
	int		cckeeps ;	/* count chapter keeps */
} ;

struct pivars {
	const char	*vpr1 ;
	const char	*vpr2 ;
	const char	*vpr3 ;
	const char	*pr ;
	const char	*vprname ;
} ;

struct arginfo {
	const char	**argv ;
	int		argc ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int proginfo_start(struct proginfo *,const char **,const char *,
		const char *) ;
extern int proginfo_setentry(struct proginfo *,const char **,const char *,int) ;
extern int proginfo_setversion(struct proginfo *,const char *) ;
extern int proginfo_setbanner(struct proginfo *,const char *) ;
extern int proginfo_setsearchname(struct proginfo *,const char *,const char *) ;
extern int proginfo_setprogname(struct proginfo *,const char *) ;
extern int proginfo_setexecname(struct proginfo *,const char *) ;
extern int proginfo_setprogroot(struct proginfo *,const char *,int) ;
extern int proginfo_pwd(struct proginfo *) ;
extern int proginfo_rootname(struct proginfo *) ;
extern int proginfo_progdname(struct proginfo *) ;
extern int proginfo_progename(struct proginfo *) ;
extern int proginfo_nodename(struct proginfo *) ;
extern int proginfo_getpwd(struct proginfo *,char *,int) ;
extern int proginfo_getename(struct proginfo *,char *,int) ;
extern int proginfo_getenv(struct proginfo *,const char *,int,const char **) ;
extern int proginfo_finish(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFS_INCLUDE */


