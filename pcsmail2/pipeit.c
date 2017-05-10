/* pipeit */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		J.Mukerji						
 *									

 ************************************************************************/



#include	<string.h>
#include	<signal.h>
#include	<setjmp.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"header.h"




/* external subroutines */

extern struct global	g ;


/* local subroutines */

void		sigpipe() ;


/* local data */

jmp_buf	env ;

int	pipejump = 0 ;



int pipeit(command, file, header, f_autoreply)
char	*command, *file, *header ;
int	f_autoreply ;
{
	bfile	msgfile, *mfp = &msgfile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	*fpa[3] ;

	FILE	*pf ;
	FILE	*tf ;

	int	f_inenvelope ;
	int	htemp ;
	int	f_header = TRUE ;

	char	deadfname[MAXPATHLEN + 1] ;
	char	tempbuf[BUFSIZE + 1] ;

	void	(*savepsig)() ;


#if	CF_DEBUG
	if (g.f.debug)
		logfile_printf(&g.eh,
		"pipeit: entered\n\tc=\"%s\"\n\tf=\"%s\"\n\th=\"%s\"\n",
	        command,file,header) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	fileinfo(file,"pipeit") ;
#endif

	if (g.f.nosend) {

	    bopen(mfp,file,"r",0666) ;

	    sprintf(deadfname,"%s/spool/pcsmail/%s",g.pcs,DEADFILE) ;

	    bopen(tfp,deadfname,"wct",0666) ;

	    bcopyblock(mfp,tfp,BUFSIZE) ;

	    bclose(mfp) ;

	    bclose(tfp) ;

	    return 0 ;
	}

	f_inenvelope = TRUE ;
	if ((file != NULL) && (file[0] != '\0')) {

	    if ((tf = fopen(file, "r")) == NULL) {

	        return -1 ;
	    }

	} else 
	    tf = NULL ;

	savepsig = (void (*)()) signal(SIGPIPE,sigpipe) ;

	if (setjmp(env) == 0) {

	    pipejump = 1 ;
	    if ((pf = popen( command, "w" )) != NULL ) {

	        fputs(header, pf) ;

	        if (tf != NULL) {

/* this is done only if a real message is to be sent */

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"pipeit: header=\"%s\"\n", header) ;
#endif

	            while (fgetline(tf,tempbuf, BUFSIZE) > 0) {

#if	CF_DEBUG && 0
	                if (g.debuglevel > 0)
	                    logfile_printf(&g.eh,
				"pipeit: ptempbuf=\"%s\"\n", tempbuf) ;
#endif

	                if (f_inenvelope) {

/* escape any existing envelope lines */

	                    if (((strncmp(tempbuf, "From",4) == 0) &&
	                        (tempbuf[4] != ':')) ||
	                        (strncmp(tempbuf, ">From", 5) == 0)) {

	                        if ( tempbuf[0] != '>' )
	                            fputs(">", pf) ;

	                        fputs( tempbuf, pf ) ;

	                        continue ;

	                    } else
	                        f_inenvelope = 0 ;

			} /* end if (escaping envelope lines) */

/* now strip off the old header or as much of it as has
			been included in the new header */

#if	CF_DEBUG
	                if (g.debuglevel > 0)
	                    logfile_printf(&g.eh,
				"pipeit: f_autoreply=%d f_header=%d\n",
	                        f_autoreply,f_header) ;
#endif

/* skip all header lines */

	                if (f_header && (! f_autoreply)) {

/* if end of header, output new header */

	                    if (! isheader(tempbuf)) {

	                        f_header = FALSE ;
	                        putheader( pf, 0 ) ;

	                    }

	                    continue ;

	                } /* end if */

/* don't send out any BCC or CC lines!!		*/

	                if ((htemp = header_line(tempbuf)) == MH_BCC)
	                    continue ;

	                if (htemp == MH_CC)
	                    continue ;

/* escape "From" lines */

	                if (strncmp( tempbuf, "From", 4 ) == 0 )
	                    fputs( ">", pf ) ;

	                fputs( tempbuf, pf ) ;

	                if (htemp >= 0) fputs( "\n", pf ) ;

	            } /* end while (copying tempfile to the pipe) */

	        } /* end if (got a file) */

	        if (pclose(pf) == 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	                logfile_printf(&g.eh,"pipeit: pipe closed successfully\n") ;
#endif

	            if (tf != NULL) fclose(tf) ;

	            pipejump = 0 ;
	            signal(SIGPIPE,savepsig) ;

	            return 0 ;
	        }

	    } /* end if (popen succeeded) */

	} /* end if ('setjmp' block) */

#if	CF_DEBUG
	            if (g.debuglevel > 0)
	logfile_printf(&g.eh,"pipeit: pipe close failed\n") ;
#endif

	pipejump = 0 ;
	signal(SIGPIPE,savepsig) ;

	if (tf != NULL) fclose(tf) ;

	return -2 ;
}
/* end subroutine (pipeit) */


void sigpipe(i)
int	i ;
{


	signal( i, SIG_IGN ) ;

	if ((i == SIGPIPE) && pipejump) longjmp(env,1) ;

}
/* end subroutine (sigpipe) */



