/* pipe_ex */

/* special mail program to debug the 'sendmail' problem */


/* revision history:

	= 1990-07-01, David A­D­ Morano


*/

/* Copyright © 1990 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Special debug mailer.

	Synopsis:

	$ dmail <recepient>


************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>


/* local defines */

#define	MAILERRFILE	"/usr/adm/log/mailerr"
#define	MAILLOGFILE	"/usr/adm/log/maillog"
#define	DEFPATH	"PATH=/bin:/usr/bin:/usr/lbin:/etc:/usr/lib:/usr/adm/bin" ;

#define	BUFLEN		100
#define	ENVBUFLEN	(MAXPATHLEN + 100)
#define	TITLE		"dmail"


/* external subroutines */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	pid_t	pid ;

	int	rs = SR_OK ;
	int	i, len ;
	int	rs_child ;
	int	ifd = 0 ;
	int	inpipe[2] ;
	int	t0fd, t1fd ;
	int	pi ;
	int	f_path = FALSE ;

	char	envbuf[ENVBUFLEN + 1], *ebp = envbuf ;
	char	t0name[TIMEBUFLEN + 1], t1name[TIMEBUFLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	*progname, *cp, *sp ;


	progname = TITLE ;
	if (bopen(efp,MAILERRFILE,"wca",0666) < 0) return BAD ;

	if ((rs = bopen(ofp,MAILLOGFILE,"wca",0666)) < 0) goto badout ;

/* start the log file */

	bprintf(ofp,"========== start of entry\n") ;

	bprintf(ofp,"	     real UID : %4d\n",getuid()) ;

	bprintf(ofp,"	effective UID : %4d\n",geteuid()) ;

	bprintf(ofp,"	     real GID : %4d\n",getgid()) ;

	bprintf(ofp,"	effective GID : %4d\n",getegid()) ;

/* write out the environment to the log file */

/*
	Since we are using the 'execvp(2)' call later, let's use this
	opportunity to ensure that the we have a good PATH environment
	variable.
*/

	bprintf(ofp,"- environment\n") ;

	for (i = 0 ; envv[i] != ((char *) 0) ; i += 1) {

	    bprintf(ofp,"	%s\n",envv[i]) ;

/* if an environment variable is messaed up, remove and insert a substitue */

	    len = strlen(envv[i]) ;
	    if ((pi = substring(envv[i],len,"=")) < 0) {

	        envv[i] = "SUB=sub" ;
	        bprintf(ofp,"    *fixed*\n") ;

	    }

#ifdef	COMMENT
/* if we have a PATH variable, add our stuff to the end */

	    if ((pi = substring(envv[i],5,"PATH=")) == 0) {

	        sp = ":/usr/lib:/usr/adm/bin" ;
	        cp = ebp ;
	        len = strlen(envv[i]) ;

	        strcpy(ebp,envv[i]) ;

	        strcat(ebp,sp) ;

	        ebp += (len + strlen(sp)) ;

	        *ebp++ = '\0' ;
	        envv[i] = cp ;

	        f_path = TRUE ;
	    }
	}
#else
/* if we have a PATH variable, replace it with ours */

	    if ((pi = substring(envv[i],5,"PATH=")) == 0) {

	        envv[i] = DEFPATH ;
	        f_path = TRUE ;
	    }
	}
#endif

/* add a path to the environment if there was not one */

	if (! f_path) {

	    sp = DEFPATH ;
	    putenv(sp) ;

	}

	bprintf(ofp,"- command line\n\n") ;

	for (i = 0 ; i < argc ; i += 1) {

	    bprintf(ofp," %s",argv[i]) ;

	}

/* create temporary files for stdout & stderr from the child program */

	pid = getpid() ;

	sprintf(t0name,"/tmp/dm0_%d",pid) ;

	sprintf(t1name,"/tmp/dm1_%d",pid) ;

	t0fd = open(t0name,O_RDWR | O_CREAT | O_TRUNC,0666) ;

	t1fd = open(t1name,O_RDWR | O_CREAT | O_TRUNC,0666) ;

/* spawn the mailer */

	pipe(inpipe) ;

	if ((pid = fork()) == 0) {

	    close(inpipe[1]) ;	/* close output side of the pipe */

	    close(0) ;

	    dup(inpipe[0]) ;

	    close(inpipe[0]) ;

	    close(1) ;

	    dup(t0fd) ;

	    close(2) ;

	    dup(t1fd) ;

	    execvp(argv[0],argv) ;

	    exit(BAD) ;
	}

	close(inpipe[0]) ;

/* continue logging */

	bprintf(ofp,"\n\n- message follows :\n") ;

/* send the data to the mailer & to log file */

	while ((len = read(ifd,buf,BUFLEN)) > 0) {

	    rs = write(inpipe[1],buf,len) ;

	    if (rs != len) goto badexit ;

	    bwrite(ofp,buf,len) ;

	}

	close(ifd) ;

	close(inpipe[1]) ;

/* wait for mailer to finish */

	while (wait(&rs_child) != pid) ;

/* check on child program exit status */

	bprintf(ofp,"\n\n- mailer status :\n") ;

	if (rs_child & 0xFF) {

	    if ((rs_child & 0xFF) == 0177) {

/* child was stopped */

	        bprintf(ofp,"problem stopped w/ status- %d\n",
	            (rs_child >> 8) & 0xFF) ;

	    } else {

/* program terminated due to a signal */

	        bprintf(ofp,
	            "problem terminated due to signal s/ status - %d\n",
	            rs_child & 0xFF) ;

	    }

/* child called 'exit' */

	} else if ((rs_child >> 8) & 0xFF) {

	    bprintf(ofp,"program exited w/ status - %d\n",
	        (rs_child >> 8) & 0xFF) ;

	}

/* log the standard output of the mailer */

	bprintf(ofp,"- standard output :\n") ;

	while ((len = read(t0fd,buf,BUFLEN)) > 0) {

	    bwrite(ofp,buf,len) ;

	}

	close(t0fd) ;

	unlink(t0name) ;

/* log the standard error output of the mailer */

	bprintf(ofp,"- standard error output :\n") ;

	while ((len = read(t1fd,buf,BUFLEN)) > 0) {

	    bwrite(ofp,buf,len) ;

	}

	close(t1fd) ;

	unlink(t1name) ;

/* end the log entry */

	bprintf(ofp,"========== end of entry\n\n") ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badexit:
	close(ifd) ;

	close(inpipe[1]) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badout:
	bprintf(efp,"%s: can't open output file (%d)\n",progname,rs) ;

	goto badexit ;
}
/* end subroutine (main) */


