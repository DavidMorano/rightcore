/* emit_article */

/* emit (process) an article */


#define	CF_DEBUG	1		/* run-time debugging */
#define	CF_DELREMOTE	0		/* ? */


/* revision history:

	= 1994-11-01, David A­D­ Morano
	- added a mode to intercept for mailbox use

	= 1994-12-01, David A­D­ Morano
        - modified to only print out header fields that a user is normally
        interested in

	= 1995-07-01, David A­D­ Morano
	- extensively modified to add:
		article follow-up capability
		article previous
		article printing
		article piping & redirecting

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is one of the "EMIT" subroutines used for
	"emitting" articles in different ways.

	Synopsis:

	int emit_article(pip,dsp,ai,ap,ngdir,af)
	struct proginfo	*pip ;
	MKDIRLIST_ENT	*dsp ;
	int		ai ;
	ARTLIST_ENT	*ap ;
	char		ngdir[] ;
	char		af[] ;

	Arguments:

	pip		program information pointer
	dsp		user structure pointer
	ai		article index within newsgroup
	ap		article ARTLIST_ENT pointer
	ngdir		directory (relative to spool directory) of article
	af		article base file name

	Returns:

	EMIT_OK		0
	EMIT_NEXT	0
	EMIT_BAD	-1
	EMIT_DONE	-2
	EMIT_PREVIOUS	-3
	EMIT_QUIT	-4
	EMIT_SKIP	-5
	EMIT_SAVE	-6


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<termios.h>
#include	<setjmp.h>
#include	<unistd.h>
#include	<time.h>
#include	<strings.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"headerkeys.h"
#include	"mkdirlist.h"
#include	"artlist.h"


/* local defines */

#define	ANSLEN		100
#define	INDENTLEN	16
#define	HVLEN		LINEBUFLEN
#define	LINEOVERHEAD	3		/* unusable terminal lines */

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	(MAXPATHLEN * 3)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	getfiledirs(const char *,const char *,const char *,vecstr *) ;

extern int	cmd_save() ;
extern int	cmd_reply() ;
extern int	cmd_printout() ;
extern int	cmd_follow() ;
extern int	cmd_output() ;
extern int	bbcpy(char *,char *) ;
extern int	hmatch() ;
extern int	term_linecheck(struct proginfo *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern const char	*monthname[] ;


/* forward references */

static int	deluser() ;
static int	delremote() ;
static int	isus() ;
static int	hastabs(const char *) ;

static void	rslowit() ;
static void	onintr() ;


/* local variables */

static jmp_buf		jmpenv ;

/* users who can delete articles */
static const char	*deleteusers[] = {
	"pcs",
	"root",
	"special",
	"daemon",
	"sys",
	"adm",
	"admin",
	"uucp",
	"listen",
	"news",
	"dam",
	"morano",
	NULL
} ;


/* exported subroutines */


int emit_article(pip,dsp,ai,ap,ngdir,af)
struct proginfo	*pip ;
MKDIRLIST_ENT	*dsp ;
int		ai ;
ARTLIST_ENT	*ap ;
const char	ngdir[] ;
const char	af[] ;
{
	struct passwd	*pp ;

	struct tm	ts, *timep ;

	struct ustat	sb ;

	bfile		afile, *afp = &afile ;
	bfile		helpfname, *hfp = &helpfname ;
	bfile		savefile, *sfp = &savefile ;

	int		rs = SR_OK ;
	int		args, i, j, sl, len ;
	int		lines, showlines ;
	int		c, badreads ;
	int		ip, op ;
	int		indent = INDENT ;
	int		f_continue ;
	int		f_text = TRUE ;
	int		f_header ;
	int		f_a, f_b, f_c, f_d, f_e, f_f ;
	int		f_board ;
	int		f_newsgroups ;
	int		f_date = FALSE ;
	int		f_from = FALSE ;
	int		f_articleid = FALSE ;
	int		f_shown = FALSE ;
	const char	*cp, *cp2 ;
	const char	*sp, *fmt ;
	const char	*oflags ;
	char		linebuf[LINEBUFLEN + 1], *lbp ;
	char		outbuf[(2*LINEBUFLEN) + 1], *obp ;
	char		hv_subject[LINEBUFLEN + 1] ;
	char		hv_date[LINEBUFLEN + 1] ;
	char		hv_articleid[LINEBUFLEN + 1] ;
	char		hv_newsgroups[MAXPATHLEN + 1] ;
	char		buf[BUFLEN + 1] ;
	char		s[BUFSIZE + 1] ;
	char		toks[10][COLS] ;
	char		afname[MAXPATHLEN + 1] ;
	char		response[ANSLEN + 1] ;
	char		indent_string[INDENTLEN + 1] ;
	char		*ofname ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: ent\n") ;
#endif

	if (ngdir == NULL)
	    return EMIT_DONE ;

	if (af == NULL)
	    return EMIT_OK ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: ng=\"%s\" af=\"%s\"\n",
	        ngdir,af) ;
#endif

	showlines = pip->termlines - LINEOVERHEAD ;
	if (pip->showlines >= 4) {

	    if (pip->showlines < showlines)
	        showlines = pip->showlines ;

	}

	if (pip->f.interactive)
	    indent = 0 ;

	hv_subject[0] = '\0' ;
	hv_subject[LINEBUFLEN] = '\0' ;
	mkpath3(afname, pip->newsdname,ngdir,af) ;

	if (pip->f.mailbox) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("emit_article: mailbox a=\"%s\"\n",afname) ;
#endif

	    cmd_save(gp,ap,ngdir,afname, SMODE_OUT,NULL) ;

	    bflush(pip->ofp) ;

	    return EMIT_OK ;

	} else if (pip->f.catchup)
	    return EMIT_OK ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: regular stuff, a=\"%s\"\n",afname) ;
#endif

/* start of code block, return here to start reading this article again */
start:
	lines = 0 ;
	if ((rs = bopen(afp,afname, "r",0666)) < 0)
	    return EMIT_DONE ;

	if (setjmp(jmpenv))
	    goto finish ;

	signal(SIGINT, onintr) ;

/* if we are an ANSI terminal, pop the screen! */

	if (pip->f.popscreen)
	    bprintf(pip->ofp,"%H") ;

/* what about the old "indent" junk ? */

	for (i = 0 ; (i < indent) && (i < INDENTLEN) ; i += 1)
	    indent_string[i] = ' ' ;

	indent_string[i] = '\0' ;

/* go through the header loops */

	f_header = TRUE ;
	f_continue = FALSE ;
	f_a = f_b = f_c = f_d = f_e = f_f = FALSE ;
	f_board = FALSE ;
	f_newsgroups = FALSE ;
	while ((len = breadline(afp,linebuf,LINEBUFLEN)) > 0) {

	    if (linebuf[0] == '\n') 
		break ;

	    if (linebuf[len - 1] == '\n') 
		len -= 1 ;

	    linebuf[len] = '\0' ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("emit_article: top while, f_newsgroups=%d\n",
	            f_newsgroups) ;
	        debugprintf("emit_article: >%s<\n",linebuf) ;
	    }
#endif /* CF_DEBUG */

	    if ((f_c = (hmatch(HK_SUBJECT,linebuf) > 0)) ||
	        (f_c = (hmatch(HK_TITLE,linebuf) > 0)) ||
	        (f_c = (hmatch(HK_SUBJ,linebuf) > 0)) ||
	        (f_e = (hmatch(HK_FROM,linebuf) > 0)) ||
	        (f_d = (hmatch(HK_DATE,linebuf) > 0)) ||
	        (f_a = (hmatch(HK_BOARD,linebuf) > 0)) ||
	        (f_b = (hmatch(HK_NEWSGROUPS,linebuf) > 0)) ||
	        (f_f = (hmatch(HK_ARTICLEID,linebuf) > 0)) ||
	        (f_continue && CHAR_ISWHITE(linebuf[0]))) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) debugprintf(
	            "emit_article: special header, f_continue=%d F_IW=%d\n",
	            f_continue,CHAR_ISWHITE(linebuf[0])) ;
#endif

	        if (f_a) 
			f_board = TRUE ;

	        if (f_b) 
			f_newsgroups = TRUE ;

	        if (f_c) {

	            cp = linebuf ;

/* fast forward to the value of the header */

	            while (*cp && (*cp != ':')) 
			cp += 1 ;

	            if (*cp == ':') 
			cp += 1 ;

	            while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

/* remove trailing EOL (if present) */

	            sl = len - (cp - linebuf) ;
	            if ((sl > 0) && (cp[sl - 1] == '\n'))
	                sl -= 1 ;

/* copy it in */

	            if (sl > HVLEN) sl = HVLEN ;

	            strncpy(hv_subject,cp,sl) ;

	            hv_subject[sl] = '\0' ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
			debugprintf("emit_article: s=\"%s\" sl=%d\n",
			hv_subject,sl) ;
#endif

	        } /* end if (save subject header value) */

	        if (f_d) 
			f_date = TRUE ;

	        if (f_e) 
			f_from = TRUE ;

	        if (f_f) 
			f_articleid = TRUE ;

	        f_continue = TRUE ;
	            cp = linebuf ;
	        if (f_a && pip->f.interactive) {

	            while (*cp && (*cp != ':')) 
			cp += 1 ;

	            if (*cp == ':') 
			cp += 1 ;

	            while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

	            lines += 1 ;
	            if (pip->f.popscreen) {
	                fmt = (hastabs(cp)) ? "%s%s: %K%s\n" : "%s%s: %s%K\n" ;

	            } else
	                fmt = "%s%s: %s\n" ;

	            bprintf(pip->ofp,fmt,
	                indent_string,HK_NEWSGROUPS,cp) ;

	            f_a = FALSE ;
	            f_newsgroups = TRUE ;
	            f_shown = TRUE ;

	        } else if (f_b && (! pip->f.interactive)) {

/* this is for backward compatibility with old software */

#if	CF_DEBUG
	            if (pip->debuglevel > 1) 
			debugprint("emit_article: compatibility code\n") ;
#endif

	            while (*cp && (*cp != ':')) 
			cp += 1 ;

	            if (*cp == ':') 
			cp += 1 ;

	            while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

	            lines += 1 ;
	            if (pip->f.popscreen) {
	                fmt = (hastabs(cp)) ? "%s%s%K%s\n" : "%s%s%s%K\n" ;

	            } else
	                fmt = "%s%s%s\n" ;

	            bprintf(pip->ofp,fmt,
	                indent_string,"BOARD: ",cp) ;

	            f_b = FALSE ;
	            f_shown = TRUE ;

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel > 1) {
			debugprintf(
			    "emit_article: no special 1, f_newsgroups=%d\n",
	                f_newsgroups) ;
			debugprintf("emit_article: cp=%s\n",cp) ;
		}
#endif

	            lines += 1 ;
	            if (pip->f.popscreen)
	                fmt = (hastabs(cp)) ? "%s%K%s\n" : "%s%s%K\n" ;

	            else
	                fmt = "%s%s\n" ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1) {
			debugprintf("emit_article: fmt=?\n") ;
			debugprintf("emit_article: fmt=>%s<\n",fmt) ;
			}
#endif

	            bprintf(pip->ofp,fmt,
			indent_string,linebuf) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
			debugprintf("emit_article: output header leader\n") ;
#endif

	        } /* end if (handle special cases) */

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
			debugprintf("emit_article: is/was a header\n") ;
#endif

	    } else if (strncasecmp(linebuf,HK_CTYPE,HKL_CTYPE) == 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) 
			debugprintf("emit_article: linebuf=\"%s\"\n",
	            linebuf) ;
#endif

	        sl = HKL_CTYPE ;
	        while ((sl < len) && CHAR_ISWHITE(linebuf[sl]))
	            sl += 1 ;

	        if ((sl < len) && (linebuf[sl] == ':'))
	            sl += 1 ;

	        sl = sfshrink((linebuf + sl),(len - sl),&cp) ;

	        cp[sl] = '\0' ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
		    debugprintf("emit_article: got some content type \"%s\"\n",
	            cp) ;
#endif

	        if ((cp2 = strpbrk(cp,"/;\t ")) != NULL)
			*cp2 = '\0' ;

	        if (strncasecmp(cp,"text",4) != 0) {

	            f_text = FALSE ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1) 
			debugprintf("emit_article: got some non-text\n") ;
#endif

	        }

	    } else {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
		    debugprintf("emit_article: unrecognized header, f_newsgroups=%d\n",
	            f_newsgroups) ;
#endif

	        f_continue = FALSE ;

	    } /* end if (recognized header or not) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf("emit_article: bottom while, f_newsgroups=%d\n",
	        f_newsgroups) ;
#endif

	} /* end while (header loops) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("emit_article: done w/ headers, f_shown=%d\n",
	    f_shown) ;
#endif

/* do we want to create a "newsgroups" header line for the viewer ? */

	if ((! f_shown) && (! f_newsgroups)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf(
	            "emit_article: showing \"newsgroups\"\n") ;
#endif

	    for (i = 0 ; ngdir[i] ; i += 1) {

	        if (ngdir[i] == '/')
	            hv_newsgroups[i] = '.' ;

	        else
	            hv_newsgroups[i] = ngdir[i] ;

	    } /* end for */

	    hv_newsgroups[i] = '\0' ;
	    lines += 1 ;
	    if (pip->f.popscreen)
	        fmt = (hastabs(cp)) ? "%s%s: %K%s\n" : "%s%s: %s%K\n" ;

	    else
	        fmt = "%s%s: %s\n" ;

	    bprintf(pip->ofp,fmt,
	        indent_string,HK_NEWSGROUPS,hv_newsgroups) ;

	} /* end if (creating a "newsgroups" header line) */

/* print out a "from" if not present */

	if (((! f_from) || (! f_date)) &&
	    (bcontrol(afp,BC_STAT,&sb) >= 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf(
	            "emit_article: optional FROM/DATE processing\n") ;
#endif

/* do we need a "from" header */

	    if ((! f_from) &&
	        ((pp = getpwuid(sb.st_uid)) != NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
		    debugprintf(
	                "emit_article: got a PASSWD entry on the file\n") ;
#endif

	        if ((strcmp(pp->pw_name,"pcs") != 0) &&
	            (strcmp(pp->pw_name,"uucp") != 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
			debugprintf(
	                    "emit_article: printing the FROM line\n") ;
#endif

	            lines += 1 ;
	            if (pip->f.popscreen)
	                fmt = "%s%s:       %s!%s%K\n" ;

	            else
	                fmt = "%s%s:       %s!%s\n" ;

	            bprintf(pip->ofp,fmt,
	                indent_string,HK_FROM,
	                pip->mailhost,pp->pw_name) ;

	        } /* end if */

	    } /* end if (from) */

/* print a date if not present */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf("emit_article: about to look at the DATE\n") ;
#endif

	    if (! f_date) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
			debugprintf("emit_article: getting file time \n") ;
#endif

#ifdef	SYSV
	        timep = 
	            (struct tm *) localtime_r((time_t *) &sb.st_mtime,
	            &ts) ;
#else
	        timep = 
	            (struct tm *) localtime((time_t *) &sb.st_mtime) ;
#endif

	        sprintf(hv_date,"%02d %3s %4d %02d:%02d %s",
	            timep->tm_mday,
	            monthname[timep->tm_mon],
	            timep->tm_year + 1900,
	            timep->tm_hour,
	            timep->tm_min,
	            ((timep->tm_isdst <= 0) ? tzname[0] : tzname[1])) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
		    debugprintf("emit_article: about to print the DATE\n") ;
#endif

	        lines += 1 ;
	        if (pip->f.popscreen)
	            fmt = "%s%s:       %s%K\n" ;

	        else
	            fmt = "%s%s:       %s\n" ;

	        bprintf(pip->ofp,fmt,
	            indent_string,HK_DATE,
	            hv_date) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
		    debugprintf("emit_article: printed the DATE\n") ;
#endif

	    } /* end if (date) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf("emit_article: bottom of optional stuff\n") ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: past optional processing\n") ;
#endif

/* do we need the article ID header ? */

	if (! f_articleid) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf("emit_article: optional article ID processing\n") ;
#endif

	    lines += 1 ;
	    if (pip->f.popscreen)
	        fmt = "%s%s: <%s>%K\n" ;

	    else
	        fmt = "%s%s: <%s>\n" ;

	    bprintf(pip->ofp,fmt,
	        indent_string,HK_ARTICLEID,af) ;

	} /* end if (printint out the article ID) */


/* print a blank line between the headers and the rest of the message */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: pop screen ?\n") ;
#endif

	lines += 1 ;
	if (pip->f.popscreen)
	    bprintf(pip->ofp,"%K\n") ;

	else
	    bprintf(pip->ofp,"\n") ;

/* OK, if this was a multimedia message, we want to ask user about it */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: METAMAIL or not ?\n") ;
#endif

	if (pip->f.interactive && (! f_text) && 
	    (getenv("NOMETAMAIL") == NULL)) {

	    struct termios	ttystatein, ttystateout ;

	    char	Cmd[CMDBUFLEN + 1] ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf("emit_article: running METAMAIL\n") ;
#endif

	    if (pip->debuglevel > 1)
	        bprintf(pip->efp,
		    "%s: running metamail on article \"%s\"\n",
	            pip->progname,af) ;

	    bprintf(pip->ofp,"about to run METAMAIL, return to continue %J") ;

	    breadline(pip->ifp,response,ANSLEN) ;

	    bufprintf(Cmd,CMDBUFLEN, "%s -m BB -p -q %s", 
	        pip->prog_metamail,afname) ;

	    tcgetattr(0,&ttystatein) ;

	    tcgetattr(1,&ttystateout) ;

	    (void) system(Cmd) ;

	    tcsetattr(0,TCSADRAIN,&ttystatein) ;

	    tcsetattr(1,TCSADRAIN,&ttystateout) ;

	    goto finish ;

	} /* end if (using METAMAIL to display interatively) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: past possible METAMAIL\n") ;
#endif

/* continue with old normal display of this message */


/* process the rest of this article */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: processing rest of article \n") ;
#endif

	op = 0 ;
	ip = indent ;
	while ((len = breadline(afp,linebuf,LINEBUFLEN)) > 0) {

	    linebuf[len] = '\0' ;
	    lbp = linebuf ;
	    obp = outbuf ;
	    while ((c = *lbp++) != '\0') {

	        switch (c) {

	        case '\r':
	        case '\n':
	            if (pip->f.popscreen) {

	                bufprintf(buf,20,"%K") ;	/* expand code */

	                obp = strwcpy(obp,buf,-1) ;

	            }

	            *obp++ = c ;
	            op = 0 ;
	            ip = indent ;
	            if (! pip->f.interactive)
			break ;

	            if (! pip->f.nopage) {

	                lines += 1 ;
	                if (lines > showlines) {

	                    if (pip->f.popscreen)
	                        bprintf(pip->ofp,"%K\n") ;

	                    else
	                        bprintf(pip->ofp,"\n") ;

	                    if (pip->f.popscreen) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: popscreen prompt\n") ;
#endif

	                            fmt = "hit CR to continue, \"q\" for "
				    "prompt> %J" ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: popscreen prompt fmt=>%s<\n",
		fmt) ;
#endif

	                        bprintf(pip->ofp, fmt) ;

	                    } else
	                        bprintf(pip->ofp, 
	                            "hit CR to continue, \"q\" for prompt> ") ;

	                    if (pip->f.interactive)
	                        bflush(pip->ofp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("emit_article: funny change thing\n") ;
#endif

	                    badreads = 0 ;
			    while (1) {

	                    	sl = breadline(pip->ifp,response,ANSLEN) ;

				if (sl >= 0) 
					break ;

	                    	if (badreads++ > 10)
					goto bad ;

	                    } /* end while */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("emit_article: past funny change thing, sl=%d\n",
		sl) ;
#endif

	                    term_linecheck(gp) ;

	                    showlines = pip->termlines - LINEOVERHEAD ;
	                    if (pip->showlines >= 4) {

	                        if (pip->showlines < showlines)
	                            showlines = pip->showlines ;

	                    }

	                    if (response[0] == '\n') {

	                        lines = 0 ;
	                        if (pip->f.popscreen) {

	                            showlines -= 1 ;
	                            bprintf(pip->ofp,"%A%K%A") ;

	                        } else
	                            bprintf(pip->ofp,"\n") ;

	                        break ;

	                    } else
	                        goto finish ;

	                } else
	                    break ;

	            } /* end if (nopage) */

	            ip -= 1 ;

/* fall through to the next case */
/* FALLTHROUGH */

	        case ' ':
	            ip += 1 ;
	            break ;

	        case '\b':
	            if (ip > indent) 
			ip -= 1 ;

	            break ;

	        case '\t':
	            ip = ((ip - indent + 8) & -8) + indent ;
	            break ;

	        default:
	            if (((c & 0x7F) < 0x20) || (c == 0x7F))
	                break ;

	            while (ip < op) {

	                *obp++ = '\b' ;
	                op -= 1 ;

	            } /* end while */

#ifdef	COMMENT
	            while ((ip & -8) > (op & -8)) {

	                *obp = '\t' ;

	                op = (op + 8) & -8 ;

	            }
#endif /* COMMENT */

	            while (ip > op) {

	                *obp++ = ' ' ;
	                op += 1 ;
	            }

	            *obp++ = c ;
	            ip += 1 ;
	            op += 1 ;
	            break ;

	        } /* end switch */

	    } /* end while (inner) */

	    bwrite(pip->ofp,outbuf,(obp - outbuf)) ;

	} /* end while (reading article body lines) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: done processing rest of article \n") ;
#endif



/* we are finished reading this article */
finish:
	bclose(afp) ;

	if (pip->f.interactive) {

	    if (pip->f.popscreen)
	        bprintf(pip->ofp,"%K\n") ;

	    else
	        bputc(pip->ofp, '\n') ;

	    bflush(pip->ofp) ;

	}

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
	    signal(SIGINT, SIG_DFL) ;

	if (! pip->f.interactive)
	    return EMIT_OK ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: before prompt\n") ;
#endif


/* continue if we are interactive */
prompt:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: prompt\n") ;
#endif

	s[0] = '\0' ;
	bprintf(pip->ofp,
	    "next, previous, review, follow, reply,") ;

	if (pip->f.popscreen) {
	    bprintf(pip->ofp,
	        " save, print, quit ? [next] %J") ;

	} else
	    bprintf(pip->ofp,
	        " save, print, quit ? [next] ") ;

	if (pip->f.interactive)
	    bflush(pip->ofp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: before 'breadline()'\n") ;
#endif

	badreads = 0 ;
	while ((sl = breadline(pip->ifp,response,ANSLEN)) < 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("emit_article: top loop\n") ;
#endif

	    if (badreads++ > 10) 
		goto bad ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("emit_article: bottom loop\n") ;
#endif

	} /* end while (reading an interactive response) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: past 'breadline()'\n") ;
#endif

	term_linecheck(gp) ;

	showlines = pip->termlines - LINEOVERHEAD ;
	if ((sl > 0) && (response[sl - 1] == '\n'))
	    sl -= 1 ;

	response[sl] = '\0' ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article: C=\"%s\"\n",response) ;
#endif

	cp = response ;
	while (CHAR_ISWHITE(*cp)) 
		cp += 1 ;

/* switch on the user entered command */

	switch (cp[0]) {

/* "replay" or "review" */
	case 'r':

/* reply */

	    if ((strcmp(cp,"rep") == 0) || (strcmp(cp,"reply") == 0)) {

	        cmd_reply(gp,ap,ngdir,af) ;

	        goto prompt ;
	    }

/* review */

	    if ((strcmp(cp,"rev") == 0) || (strcmp(cp,"review") == 0)) {

	        bputc(pip->ofp,
	            '\n') ;

	        goto start ;
	    }

	    bprintf(pip->ofp,"ambiguous option, please respecify\n") ;

	    goto prompt ;

/* save the article to a file */
	case '>':
	    cp += 1 ;
	    oflags = "wct" ;
	    if (*cp == '>') {
	        oflags = "wca" ;
	        cp += 1 ;
	    }

	    sl = sfshrink(cp,-1,&ofname) ;
	    ofname[sl] = '\0' ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("emit_article: > %s\n",ofname) ;
#endif

	    if (bopen(sfp,ofname,oflags,0666) >= 0) {

	        if (bopen(afp,afname,"r",0666) >= 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("emit_article: opened file \n") ;
#endif

	            rs = bcopyblock(afp,sfp,-1) ;

	            bclose(afp) ;

	        } /* end if (opening the article file) */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("emit_article: rs=%d\n",rs) ;
#endif

	        bclose(sfp) ;

	    } else
	        bprintf(pip->efp,"%s: could not open file \"%s\"\n",
	            pip->progname,ofname) ;

	    goto prompt ;

/* pipe the article to a program */
	case '|':
	    cp += 1 ;

	    sl = sfshrink(cp,-1,&cp2) ;

	    cp2[sl] = '\0' ;
	    cmd_output(gp,ap,ngdir,afname, cp2) ;

	    goto prompt ;

/* follow (up) */
	case 'f':
	    bprintf(pip->ofp,"calling BBPOST ...\n") ;

	    if ((rs = cmd_follow(gp,ap,ngdir,afname, hv_subject)) < 0) {

	        bprintf(pip->ofp,
	            "could not append attachment\n") ;

	        bprintf(pip->ofp,
	            "possible full file system\n") ;

	    }

	    bprintf(pip->ofp,"\nreturned to BB ...\n") ;

	    goto prompt ;

/* "unsubscribe" */
	case 'u':
	    dsp = usp->dsp ;
	    dsp->f.subscribe = FALSE ;
	    goto prompt ;

/* "save" or "subscribe" */
	case 's':
	    if (cp[1] == 'u') {

/* handle making a subscription here */

	        dsp = usp->dsp ;
	        dsp->f.subscribe = TRUE ;
	        goto prompt ;

	    } /* end if (handling "subscribe") */

/* fall through to the next case below */

/* save this messages */
	case 'm':
	    sp = strtok(cp, " ") ;

	    args = 0 ;
	    strcpy(toks[2],"new") ;

	    while (sp != NULL) {

	        strcpy(toks[++args],sp) ;

	        sp = strtok(0," ") ;

	    } /* end while */

	    if (strcmp(toks[2],"new") == 0)
	        args = 2 ;

	    for (j = 2 ; j <= args ; j += 1) {

	        rs = cmd_save(gp,ap,ngdir,afname, SMODE_MAILBOX,toks[j]) ;

		if (rs < 0)
	            bprintf(pip->ofp,"could not save article (rs %d)\n",
	                rs) ;

	    } /* end for */

	    goto prompt ;

/* "print" or "prt" or "previous" */
	case 'p':
	    if ((strncmp(cp,"pri",3) == 0) || (strncmp(cp,"prt",3) == 0)) {

/* print */
	        while (*cp && (! CHAR_ISWHITE(*cp))) 
			cp += 1 ;

/* skip over white space also, if present */

	        while (CHAR_ISWHITE(*cp)) 
			cp += 1 ;

/* print it out with any arguments that user may have specified */

	        cmd_printout(gp,ap,ngdir,afname, cp) ;

	        goto prompt ;

	    } else {

/* handle the "previous" command */

	        if (ai > 0)
	            rs = EMIT_PREVIOUS ;

	        else {

	            bprintf(pip->ofp,
	                "\nno previous articles available in newsgroup\n\n") ;

	            goto prompt ;
	        }

	    } /* end if (print or previous) */

	    break ;

/* possibly skip the rest of this newsgroup entirely */
	case 'S':
	    if ((cp[1] == '\0') || (cp[1] == 'k')) 
		return EMIT_SKIP ;

	    if (cp[1] == 'a') 
		return EMIT_SAVE ;

	    bprintf(pip->ofp,
	        "unknown command, please try again\n") ;

	    goto prompt ;


/* help */
	case '?':
	    bprintf(pip->ofp,"\n") ;

	    if ((rs = bopen(hfp,pip->helpfname,"r",0666)) >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("emit_article: helpfname file=%s\n",
		pip->helpfname) ;
#endif

	        while ((sl = breadline(hfp,linebuf,LINEBUFLEN)) > 0) {

	            if (linebuf[0] != '#') {

	                linebuf[sl] ='\0' ;
	                bwrite(pip->ofp,linebuf,sl) ;

	            }

	        } /* end while */

	        bclose(hfp) ;

	    } else {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("emit_article: default help\n") ;
#endif

	        bprintf(pip->ofp,
	            "   %-10s%s\n","review",
	            "print article again") ;

	        bprintf(pip->ofp,
	            "   %-10s%s\n","reply",
	            "reply to article") ;

	        bprintf(pip->ofp,
	            "   %-10s%s\n","next",
	            "go to next article") ;

	        bprintf(pip->ofp,
	            "   %-10s%s\n","save",
	            "save article in mailbox") ;

	        bprintf(pip->ofp,
	            "   %-10s%s\n","Skip",
	            "skip the rest of this newsgroup and mark as read") ;

	        bprintf(pip->ofp,
	            "   %-10s%s\n","Save",
	            "save (suspend) the session for reading later") ;

	        bprintf(pip->ofp,
	            "   %-10s%s\n","quit",
	            "exit program") ;

	    } /* end if */

	    bprintf(pip->ofp,"\n") ;

	    goto prompt ;

/* show the user where she is */
	case '=':
	    bbcpy(hv_newsgroups,ngdir) ;

	    bprintf(pip->ofp,"\ncurrent newsgroup> %s\n\n",hv_newsgroups) ;

	    goto prompt ;

/* next */
	case 'n':
	case '\0':
	case '\n':
	    rs = EMIT_OK ;
	    break ;

/* quit */
	case 'q':
	    rs = EMIT_QUIT ;
	    break ;

/* delete the article */
	case 'd':

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("emit_article: possible delete \n") ;

	        debugprintf("emit_article: cmd=%s\n",cp) ;

	        debugprintf("emit_article: user=%s\n",pip->uip->username) ;

	    }
#endif /* CF_DEBUG */

	    if ((strncmp(cp,"del",3) == 0) && 
	        deluser(gp,deleteusers,pip->uip->username)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("emit_article: delete \n") ;
#endif

	        u_unlink(afname) ;

#if	CF_DELREMOTE
	        delremote(gp,afname) ;
#endif

	        rs = EMIT_OK ;
	        break ;

	    } else
	        bprintf(pip->ofp,
	            "unknown command, please try again\n") ;

	    goto prompt ;

/* end case */

/* edit the article in place, only for trusted users */
	case 'e':
	    if (deluser(gp,deleteusers,pip->uip->username)) {

	        char	cmdbuf[CMDBUFLEN + 1] ;


	        if (getfiledirs(NULL,pip->prog_editor,"x",NULL) > 0) {

	            bufprintf(cmdbuf,CMDBUFLEN,"%s %s",
	                pip->prog_editor,afname) ;

	            system(cmdbuf) ;

	        } else
	            bprintf(pip->ofp,
	                "could not execute the editor \"%s\"\n",
	                pip->prog_editor) ;

	    } else
	        bprintf(pip->ofp,
	            "operation is only available for trusted users\n") ;

	    goto prompt ;

	default:
	    bprintf(pip->ofp,
	        "unknown command, please try again\n") ;

	    goto prompt ;

	} /* end switch (on user command) */


/* exit this article */
exit:
	bputc(pip->ofp,
	    '\n') ;

	return rs ;

/* bad stuff comes here */
bad:
	return EMIT_BAD ;
}
/* end subroutine (emit_item) */


/* local subroutines */


/* is the specified user allow to perform deletes ? */
static int deluser(gp,deleteusers,username)
struct proginfo	*gp ;
char		*deleteusers[] ;
char		username[] ;
{
	int		i ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article/deluser: ent\n") ;
#endif

	for (i = 0 ; deleteusers[i] != NULL ; i += 1) {

	    if (strcmp(deleteusers[i],username) == 0)
	        return TRUE ;

	} /* end for */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("emit_article/deluser: no match\n") ;
#endif

	return FALSE ;
}
/* end subroutine (deluser) */


/* delete an article which is on a remote machine */
static int delremote(gp,afname)
struct proginfo	*gp ;
char		afname[] ;
{
	bfile	hostsfile, *hfp = &hostsfile ;
	bfile	notusfile, *nfp = &notusfile ;
	bfile	*fpa[3], file0, file1, file2 ;

	pid_t	cpid ;

	int	rs = SR_OK ;
	int	len ;
	int	f_notus = FALSE ;
	int	childstat ;
	int	n = 0 ;

	char	bbhostsfname[MAXPATHLEN + 1] ;
	char	bbnotusfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	cmd[CMDBUFLEN + 1] ;
	char	*lbp, *cp ;


	fpa[0] = &file0 ;
	fpa[1] = NULL ;
	fpa[2] = NULL ;
	bufprintf(cmd,CMDBUFLEN,"rm -f %s",afname) ;

	mkpath2(bbhostsfname, pip->pr,BBHOSTSFILE) ;

	mkpath2(bbnotusfname, pip->pr,BBNOTUSFILE) ;

	if (bopen(nfp,bbnotusfname,"r",0666) >= 0)
	    f_notus = TRUE ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("delremote: ent afname=%s\n",afname) ;
#endif

	if ((rs = bopen(hfp,bbhostsfname,"r",0666)) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("delremote: opened 'bbhosts' file\n") ;
#endif

	    while ((len = breadline(hfp,linebuf,LINEBUFLEN)) > 0) {

	        if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	        linebuf[len] = '\0' ;
	        lbp = linebuf ;
	        while (CHAR_ISWHITE(*lbp)) 
			lbp += 1 ;

	        if ((*lbp == '\0') || (*lbp == '#')) 
			continue ;

	        cp = lbp ;
	        while (*cp && (! CHAR_ISWHITE(*cp))) 
			cp += 1 ;

	        *cp = '\0' ;

/* is this host in our "not us" list ? */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("delremote: about to check for not us host=%s\n",
	                lbp) ;
#endif

	        if (f_notus && isus(nfp,lbp)) 
			continue ;

/* OK, do it for this host */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("delremote: got a not us host=%s\n",lbp) ;

	        if (pip->debuglevel > 1)
	            debugprintf("delremote: cmd=%s\n",cmd) ;
#endif

#ifdef	COMMENT
	        cpid = bopenrcmd(fpa,lbp,cmd) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("delremote: opened remote w/ cpid=%d\n",cpid) ;
#endif

	        bclose(fpa[0]) ;

	        if (cpid > 0) 
			u_waitpid(cpid,&childstat,0) ;
#endif

/* pop it another way, also! */

	        rslowit(lbp,cmd) ;

	    } /* end while */

	    bclose(hfp) ;

	} /* end if (opened 'bbhosts' file) */

	if (f_notus)
	    bclose(nfp) ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (delremote) */


/* is this host us (lookup in "us" file) */
static int isus(nfp,hostbuf)
bfile	*nfp ;
char	hostbuf[] ;
{
	int	len ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*lbp, *cp ;


	bseek(nfp,0L,SEEK_SET) ;

	while ((len = breadline(nfp,linebuf,LINEBUFLEN)) > 0) {

	    if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	    linebuf[len] = '\0' ;
	    lbp = linebuf ;
	    while (CHAR_ISWHITE(*lbp)) 
		lbp += 1 ;

	    if ((*lbp == '\0') || (*lbp == '#')) 
		continue ;

	    cp = lbp ;
	    while (*cp && (! CHAR_ISWHITE(*cp))) 
		cp += 1 ;

	    *cp = '\0' ;
	    if (strcmp(lbp,hostbuf) == 0) 
		break ;

	} /* end while */

	return ((len <= 0) ? FALSE : TRUE) ;
}
/* end subroutine (isus) */


static void rslowit(hostname,cmd)
char	hostname[] ;
char	cmd[] ;
{
	int	rs1 ;

	char	buf[CMDBUFLEN + 1] ;


	rs1 = bufprintf(buf,CMDBUFLEN,
	    "rslow > /dev/null 2>&1 -Un %s!/usr/spool/uucppublic/rslow %s",
	    hostname,cmd) ;

	if (rs1 >= 0)
	    system(buf) ;

}
/* end subroutine (rslowit) */


static int hastabs(s)
const char	s[] ;
{


	while (*s && (*s != '\t'))
		s += 1 ;

	return ((*s) ? TRUE : FALSE) ;
}
/* end subroutine (hastabs) */


static void onintr()
{


	longjmp(jmpenv, 1) ;
}



