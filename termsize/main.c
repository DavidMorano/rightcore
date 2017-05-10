/* main */

/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* compile-time debuggin */
#define	CF_TERMCAP	0
#define	CF_ALARM	0


/* revision history:

	= 1991-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Program to try to figure out the window size of the terminal.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<termio.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"config.h"


/* local defines */

#define	TIMEOUT		15

#define	BUFLEN		100
#define EMULATIONS	2

#define	TT_SUN		0
#define TT_XTERM	1
#define	TT_SXREEN	2
#define	TT_VT520	3


/* external subroutines */

extern int	matostr(cchar **,int,cchar *,int) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	substring(cchar *,int,cchar *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdec(cchar *,int,int *) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* forward references */

static void		onintr(int) ;
static void		onalarm(int) ;

static char		*strindex() ;


/* global variables */

const char	*progname ;

struct gdata {
	const char	*progname ;
	bfile		*efp ;
} gd ;

char *emulator[] = {
	"sun",
	"xterm",
} ;

char *getsize[] = {
	"\033[18t",
	"\033[t",
} ;

char *readsize[] = {
	"\033[8;%d;%d;t",
	"\033[%d;%dT",
} ;

char *restore[] = {
	0,
	"\0338",
} ;


struct termio	ttsave, newtty ;

char		termcap[2048] ;
char		newtc[2048] ;
char		buffer[20] ;


/* local variables */

static cchar	*termtypes[] = {
	"sun",
	"xtern",
	"vt100",
	"vt102",
	"vt200",
	"vt220",
	"vt300",
	"vt320",
	"vt400",
	"vt420",
	"vt500",
	"vt520",
	"ansi",
	NULL
} ;

enum termtypes {
	termtype_sun,
	termtype_xterm,
	termtype_vt100,
	termtype_vt102,
	termtype_vt200,
	termtype_vt220,
	termtype_vt300,
	termtype_vt320,
	termtype_vt400,
	termtype_vt420,
	termtype_vt500,
	termtype_vt520,
	termtype_ansi,
	termtype_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	int		t1, t2;
	int		this_emulator = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rs2 ;
	int		to ;
	int		f_err = FALSE ;
	int		bi, len, blen ;

	cchar		*bn ;
	const char	*env = getenv("TERM") ;
	const char	*cp ;
	char		*ptr ;
	char		buf[BUFLEN], *bp ;

#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs2 = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs2) ;
	}
#endif /* CF_DEBUGS */

	if (bopen(efp,BFILE_STDERR,"wca",0664) < 0) return BAD ;

	if (bopen(ofp,BFILE_STDOUT,"wct",0664) < 0) return BAD ;

#if	CF_DEBUGS
	debugprintf("main: earnest\n") ;
#endif

	progname = argv[0] ;
	gd.progname = progname ;

	{
	    int	ti ;
	    if ((ti = matostr(termtypes,4,env,-1)) >= 0) {
		switch (ti) {
		case termtype_sun:
	    	    this_emulator = TT_SUN ;
		    break ;
		case termtype_vt100:
		case termtype_vt200:
		case termtype_vt220:
		case termtype_vt300:
		case termtype_vt320:
		case termtype_vt400:
		case termtype_vt420:
		case termtype_vt500:
		case termtype_vt520:
	    	    this_emulator = TT_XTERM ;
		    break ;
		default:
	    	    this_emulator = TT_XTERM ;
		    break ;
		} /* end switch */
	    } else {
	    	    this_emulator = TT_XTERM ;
	    } /* end if */
	} /* end block */


	ioctl(0,TCGETA,&ttsave);

	newtty = ttsave ;
	newtty.c_iflag &= ~(ICRNL | IUCLC) ;
	newtty.c_lflag &= ~(ICANON | ECHO) ;
	newtty.c_cflag |= CS8 ;
	newtty.c_cc[VMIN] = 0 ;
	newtty.c_cc[VTIME] = 1 ;
	ioctl(0,TCSETA,&newtty) ;

	signal(SIGINT, onintr) ;

	signal(SIGQUIT, onintr) ;

	signal(SIGTERM, onintr) ;

	signal(SIGALRM,onalarm) ;

	write(0,getsize[this_emulator], strlen(getsize[this_emulator])) ;

#if	CF_ALARM
	alarm(TIMEOUT) ;
#endif

#if	! CF_DEBUGS
	bp = buf ;
	blen = 0 ;
	to = TIMEOUT ;
	rs = OK ;
	while ((to > 0) && (blen <= BUFLEN)) {

	    len = read(0,bp,BUFLEN - blen) ;

	    if (len > 0) {
	        blen += len ;
	        bp += len ;
	        to = TIMEOUT ;
	    } else if (len == 0) {
		to -= 1 ;
	    } else {
	        rs = len ;
	        break ;
	    }

	} /* end while */
#else
	strcpy(buf,"E[4;8T") ;

	rs = OK ;
	blen = 6 ;
#endif

	if (rs != OK) {
	    bprintf(efp,"${0}: bad news\n",progname) ;
	    goto badret ;
	}

#if	CF_DEBUGS
	bprintf(efp,"len read is %d\n",blen) ;

	bflush(efp) ;
#endif

	if (blen >= 6) {

/************************
	"\033[8;%d;%d;t",
	"\033[%d;%dT",
	    if (sscanf(buf,readsize[this_emulator],&t1,&t2) != 2)
	        f_err = TRUE ;
*************************/

	    f_err = TRUE ;
	    bp = buf ;
	    switch (this_emulator) {

	    case TT_SUN:
	        if (substring(buf,blen,"t") > 0) {

	            bp += 4 ;
	            blen -= 4 ;
	            if ((bi = substring(bp,blen,";")) >= 0) {
	                if (cfdeci(bp,bi,&t1) < 0) break ;
	            }

	            bp += (bi + 1) ;
	            blen -= (bi + 1) ;

	            if ((bi = substring(bp,blen,";")) >= 0) {
	                if (cfdeci(bp,bi,&t2) < 0) break ;
	            }

	            f_err = FALSE ;
	        }

	    case TT_XTERM:
	        if (substring(buf,blen,"T") > 0) {

	            bp += 2 ;
	            blen -= 2 ;
	            if ((bi = substring(bp,blen,";")) >= 0) {

#if	CF_DEBUGS
bprintf(efp,"bi %d - buf %t\n",bi,bp,blen) ;
#endif

	                if (cfdeci(bp,bi,&t1) < 0) 
				break ;

#if	CF_DEBUGS
	bprintf(efp,"got lines %d\n",t1) ;
#endif

	            }

	            bp += (bi + 1) ;
	            blen -= (bi + 1) ;

	            if ((bi = substring(bp,blen,"T")) >= 0) {

#if	CF_DEBUGS
bprintf(efp,"bi %d - buf %t\n",bi,bp,blen) ;
#endif

	                if (cfdeci(bp,bi,&t2) < 0) 
				break ;

#if	CF_DEBUGS
	bprintf(efp,"got columns %d\n",t2) ;
#endif

	            }

	            f_err = FALSE ;
	        }

	    default:
	        ;
	    } /* end switch */

	} else {
	    f_err = TRUE ;
	}

	if (f_err) {

	    bprintf(efp,"%s: cannot parse window size information\n",
		progname) ;

	    bflush(efp) ;

	    onintr(0);
	}

#if	CF_ALARM
	alarm(0) ;
#endif

	signal(SIGALRM,SIG_IGN);

	ioctl(0,TCSETA,&ttsave);

/* we have what we want, but check to see if we have to deal with CF_TERMCAP */

	if (*termcap != '\0') {

/* now see if we can jerry-rig up a new termcap entry for it */

	    if ((ptr = strindex(termcap, "co#")) == NULL) {

	        bprintf(efp, "%s: No `co#'\n", progname);

	        bflush(efp) ;

	        exit(1) ;
	    }

	    strncpy(newtc, termcap, ptr - termcap + 3);

	    sprintf(newtc + strlen (newtc), "%d", t2);

	    ptr = strchr(ptr,':');

	    strcat(newtc, ptr);

/* now do lines */

	    if ((ptr = strindex(newtc,"li#")) == NULL) {

	        bprintf(efp, "%s: No `li#'\n", progname);

	        bflush(efp) ;

	        exit(1) ;
	    }

	    strncpy(termcap, newtc, ptr - newtc + 3);

	    sprintf(termcap + ((int) ptr - (int) newtc + 3), "%d", t1);

	    ptr = strchr(ptr, ':');

	    strcat(termcap, ptr);

#if	CF_TERMCAP
	    bprintf(ofp,"TERMCAP=\"") ;

	    bwrite(ofp,termcap,strlen(termcap)) ;

	    bprintf(ofp,"\"\n") ;
#endif

	}

	if (sfbasename(progname,-1,&bn) >= 0) {

	if (strcmp(bn,"winsize") == 0) {
		bprintf(efp,
		"%d:%d\n",t1,t2) ;
	} else {
	    bprintf(ofp,
		"LINES=%d; COLUMNS=%d; export LINES COLUMNS\n",
		t1,t2) ;
	}

	} else {
		bprintf(efp,
		"%s: could not obtain the base name of this progranm\n",
		progname) ;
	}

	bclose(ofp) ;

badret:
retearly:
	bclose(efp) ;

#if	CF_DEBUGS
	debugclose() ;
#endif

	return OK ;
}
/* end subroutine (main) */


/* local subroutines */


/* ARGSUSED */
static void onintr(int sn)
{
	ioctl (0, TCSETA, &ttsave) ;
	exit(1) ;
}


static void onalarm(int sn)
{

#ifdef	COMMENT
	bprintf(gd.efp, "%s: timed out on reply from terminal, aborting\n", 
	    dg.progname);

	bflush(gd.efp) ;

#endif
	onintr(sn) ;
}
/* end subroutine (onalarm) */


/*
	Returns a pointer to the first occurrence of s2 in s1, or NULL 
	if there are none.
*/

static char *strindex (s1, s2)
register char *s1, *s2;
{
	int 		s2len = strlen(s2);
	register char *s3;

	while ((s3 = strchr(s1, *s2)) != NULL) {
	    if (strncmp (s3, s2, s2len) == 0) return s3 ;
	    s1 = ++s3;
	}

	return NULL ;
}


