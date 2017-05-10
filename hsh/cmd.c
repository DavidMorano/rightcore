/* cmd */

/* command mode */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0


/* revision history:

	= 88/01/10, David A­D­ Morano

	This subroutine was based on the SHELL program for
	the PPI type computers and others like them.


*/


/************************************************************************

	This module provides the command mode portion of a small 
	portable screen editor for use by microcomputer based 
	applications.



***************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<ascii.h>

#include	"localmisc.h"
#include	"q.h"
#include	"defs.h"



/* local defines */

#define	READ_TIMEOUT	1000L



/* external subroutines */

extern int	tty_read(), tty_control(), tty_write() ;
extern int	ghp() ;
extern int	setjmp(), longjmp() ;
extern int	cfhexi() ;
extern int	hi_insert(), hi_backup(), hi_copy() ;
extern int	editline() ;



/* line mode command table */

#define	CMD_REPEAT	0
#define	CMD_HISTORY	1
#define	CMD_HELP	2
#define	CMD_TERM	3
#define	CMD_EXIT	4
#define	CMD_QUIT	5
#define	CMD_NC		6		/* number of commands */
#define	CMD_CL		4		/* maximum final compare length */



static char	comtab[][6] = {
	1,4,'R','E','P','E',
	2,4,'H','I','S','T',

	1,4,'H','E','L','P',
	1,4,'T','E','R','M',

	1,4,'E','X','I','T',
	1,4,'Q','U','I','T',
} ;


static unsigned long	cterms[8] = {
	0xFEC0FCFFL, 0x80000000L, 
	0x00000000L, 0x40000000L,
	0x0809C000L, 0x00000000L,
	0x00000000L, 0x00000000L
} ;

static char	sexit[] = "exit\n" ;








int command_mode(gdp)
struct gdata	*gdp ;
{
	bfile	*ifp = gdp->ifp ;
	bfile	*ofp = gdp->ofp ;
	bfile	*efp = gdp->efp ;

	struct field	fsb ;
	struct history	*hp = gdp->hp ;
	struct linebuf	*lbp ;

	long		environ[16] ;	/* state save array */
	long		lw ;
	long		old_mode ;

	int		len, llen ;
	int		i, j ;
	int		n ;
	int		iw, iw2, rs ;
	int		cllr ;
	int		cur_pos ;
	int		fildes[2] ;
	int		sifd, sofd, sefd ;
	int		csifd, csofd, csefd ;

	short		*swp, sw ;

	uchar		c ;
	uchar		*bp, buffer[LINESIZE], buf[LINESIZE], tbuf[10] ;
	uchar		*cbp ;
	uchar		*cbuf, comline[CBL+1] ;
	uchar		outbuf[2100], *obp = outbuf ;
	uchar		inbuf[100], *ibp = inbuf ;
	uchar		bg ;


/* open pipe for SHELL command input */

	mknod("cip",S_IFIFO | 0664,0) ;

	sifd = open("cip",O_RDONLY,0) ;

	csifd = open("cip",O_WRONLY,0) ;


/* open pipe for SHELL error output */

	pipe(fildes) ;

	csefd = fildes[0] ;
	sefd = fildes[1] ;


	bflush(efp) ;

	if (fork() == 0) {

	    close(0) ;

#ifdef	COMMENT
	    close(1) ;
#endif

	    close(2) ;

	    if (dup(sifd) != 0) {

	        bprintf(efp,"%s: DUP didn't work properly\n",
			gdp->progname) ;

	        bclose(efp) ;

	        return BAD ;
	    }


#ifdef	COMMENT
	    dup(sofd) ;
#endif

	    dup(sefd) ;

	    close(sifd) ;

#ifdef	COMMENT
	    close(sofd) ;
#endif

	    close(sefd) ;

	    execl("/bin/ksh","ksh","-i","cip",0)  ;
	}


/* terminal setup */

	tty_control(gdp->ifd,FM_SETMODE,gdp->term_mode) ;


/* wait for shell to be activated */

	while (read(csefd,&c,1) > 0) {

	    if (c == '$') {

	        read(csefd,&c,1) ;

	        if (c == ' ') goto more ;

	        continue ;
	    }

	    bputc(efp,c) ;

	    if (c == '\n') bflush(efp) ;

	}

	goto shell_exit ;


/* get more commands from the user */
more:
	setjmp(environ) ;

	bflush(ofp) ;

	bflush(efp) ;


	len = tty_aread(gdp->ifd,FM_SETMODE,
	    comline,CBL,
	    READ_TIMEOUT,cterms,CPP,CPL) ;

	if (len < 0) {

	    bprintf(efp,"%s: problems w/ input\n",
		gdp->progname) ;

	    goto exit ;
	}


	if (len == 0) {

	    tty_write(gdp->ofd," shell timeout\r\n",16) ;

	    goto exit ;
	}

	c = comline[len - 1] ;
	if (c == CH_ESC || c == CH_CSI || c == CH_SS2 || c == CH_SS3) {

	    iw = (int) len ; 
	    iw -= 1 ;
	    editline(gdp,hp,comline,&iw,CPP,(int) CPL,c) ;

	    llen = (long) iw ;
	    if (llen == 0) 
		goto more ;

	    goto gotline ;
	}


	if (c == '\n') 
		tty_write(gdp->ofd,"\r\n",2) ;

/* save length of command line */

#ifdef	FUTURE
	if (sb.cs != R_TIMEOUT) 
		llen = len - 1 ;
#else
	llen = len - 1 ;
#endif

	c = comline[0] ;
	if (len == 1) {

	    if (c != '?') 
		goto more ;

	    bprintf(ofp," exit\n") ;

	    write(csifd,sexit,strlen(sexit)) ;

	    goto exit ;
	}


gotline:
	if (breakout(SEM_CY,SEM_INT)) 
		goto more ;

	bflush(ofp) ;

	cllr = llen ;
	cbp = comline ;

/* put this line into the history space */

	hi_insert(hp,comline,(int) llen) ;


/* play some games */

#if	CF_DEBUGS
	bprintf(efp," entered : %W : closing TTY\n",comline,llen) ;

	bflush(efp) ;
#endif

	if (tty_close(gdp->ofd) < 0) {

	    bprintf(efp,"%s: bad close on TTY\n",gdp->progname) ;

	    goto exit ;
	}

/* pass to shell */

	comline[llen] = '\n' ;
	llen += 1 ;

	write(csifd,comline,llen) ;

/* wait for the shell to come back */

try_sync:
	while (read(csefd,&c,1) > 0) {

	    if (c == '$') {

	        read(csefd,&c,1) ;

	        if (c == ' ') 
			goto woke ;

	        continue ;
	    }

	    bputc(efp,c) ;

	    if ((c == '\n') || (c < 0x20)) 
		bflush(efp) ;

	}

	tty_open(gdp->ifd) ;

	goto shell_exit ;

woke:
#if	CF_DEBUGS
	bprintf(efp,"opening TTY\n") ;

	bflush(efp) ;
#endif

	tty_open(gdp->ifd) ;

	goto more ;


shell_exit:
	bprintf(efp,"%s: the shell exited\n",gdp->progname) ;

exit:

#if	CF_DEBUGS
	bprintf(gdp->ofp,"exiting\n") ;
#endif

/* close the pipes */

	close(csifd) ;

	close(sifd) ;

	close(csefd) ;

	close(sefd) ;


/* do not close the files, this will occur in the main routine */

	bflush(ofp) ;

	bflush(efp) ;

	return OK ;
}
/* end subroutine (command_mode) */



int abort(dummy) { 
	return (NO) ; 
}

int interrupt(dummy) { 
	return (NO) ; 
}

int breakout(dum0,dum1) { 
	return (NO) ; 
}




/* subroutine to get some command line arguments as HEX numbers */
int getargs(fsbp,n,p)
struct field	*fsbp ;
int		n, p[] ;
{
	extern int	ghp() ;

	int		i ;


	field_get(fsbp,0L) ;		/* throw away the first argument */

	for (i = 0 ; i < n ; i += 1) {

	    p[i] = 0 ;
	    if (ghp(fsbp,&p[i]) == PARAM_BAD) 
		return (BAD) ;

	}

	return (OK) ;
}
/* end subroutine (getargs) */


/* subroutine to get a value */
int ghp(fsbp,address)
struct field	*fsbp ;
int		*address ;
{


	field_get(fsbp,NULL) ;

	if (fsbp->flen == 0) 
		return PARAM_EMPTY ;

	if (! cfhexi(fsbp->flen,fsbp->fp,address)) 
		return PARAM_OK ;

	return PARAM_BAD ;
}
/* end subroutine (ghp) */


/* sleep for microseconds */
int microsleep(u)
int	u ;
{

	return sleep(u / 1000000) ;
}


int set_term() { 
}


int restore_term() { 
}


int tform(obp,buf,len)
char	*obp ;
char	*buf ;
int	len ;
{
	int	t, ch, i, n = 0 ;


	ch = 0 ;
	for (i = 0 ; i < len ; i += 1) {

	    if (buf[i] == '\t') {

	        for (t = 0 ; t < (8 - (ch & 7)) ; t += 1) {

	            *obp++ = ' ' ;
	            n += 1 ;
	        }

	        ch += (8 - (ch & 7)) ;

	    } else {

	        *obp++ = buf[i] ;
	        n += 1 ;

	        ch += 1 ;
	    }
	}

	return n ;
}
/* end subroutine (tform) */


/* line queue handling */

int insl(qp,ep)
struct linehead	*qp ;
struct linebuf	*ep ;
{


	ep->np = (struct linebuf *) qp ;
	ep->pp = (struct linebuf *) qp->tp ;
	if (qp->hp == ((struct linebuf *) qp)) {

	    qp->hp = qp->tp = ep ;
	    return Q_EMPTY ;

	} else {

	    (qp->tp)->np = ep ;
	    qp->tp = ep ;
	    return Q_OK ;

	}

}

/* remove from line queue */
int reml(qp,epp)
struct linebuf	*qp ;
struct linebuf	*(*epp) ;
{


	if (qp->np == ((struct linebuf *) qp)) 
		return Q_UNDER ;

	(*epp) = qp->np ;
	((*epp)->np)->pp = ((struct linebuf *) qp) ;
	qp->np = (*epp)->np ;
	return Q_OK ;
}



