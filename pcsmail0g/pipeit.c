/* pipeit */


#define	DEBUG	1


/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
 *									*

 ************************************************************************/



#include	<string.h>
#include	<signal.h>
#include	<setjmp.h>
#include	<pwd.h>
#include	<stdio.h>

#include	<baops.h>

#include	"config.h"
#include	"smail.h"
#include	"header.h"




/* external subroutines */

extern struct global	g ;


/* local subroutines */

void		sigpipe() ;


/* local data */

jmp_buf	env ;

int	pipejump = 0 ;
int	inheader ;



int pipeit(command, file, header, autoreply)
char	*command, *file, *header, autoreply ;
{
	FILE	*pf, *popen() ;
	FILE	*tf ;

	int	inpostmark ;
	int	*savepsig ;
	int	htemp ;
	int	f_header = TRUE ;

	char	tempbuf[BUFSIZE] ;


#if	 DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	errprintf("pipeit:\tc=\"%s\"\n\tf=\"%s\"\n\th=\"%s\"\n",
	    command,file,header) ;
#endif

	if (BATST(g.uo,UOV_NOSEND)) return 0 ;

	inheader = inpostmark = 1 ;
	if (file[0] != '\0') {

	    if ((tf = fopen(file, "r")) == NULL) {

	        return -1 ;
	    }

	} else tf = NULL ;

	savepsig = (int *) signal(SIGPIPE,sigpipe) ;

	if (setjmp(env) == 0) {

	    pipejump = 1 ;
	    if ((pf = popen( command, "w" )) != NULL ) {

	        fputs(header, pf) ;

	        if (tf != NULL) {

/* Done only if a real message is to be sent */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	            errprintf("pipeit: header=\"%s\"\n", header) ;
#endif
	            while (fgetline(tf,tempbuf, BUFSIZE) > 0) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	                errprintf("pipeit: ptempbuf=\"%s\"\n", tempbuf) ;
#endif

	                if (inpostmark)

	                    if (((strncmp( tempbuf, "From", 4 ) == 0 ) &&
	                        ( tempbuf[4] != ':' )) ||
	                        strncmp( tempbuf, ">From", 5 ) == 0 ) {

	                        if( tempbuf[0] != '>' )
	                            fputs(">", pf) ;

	                        fputs( tempbuf, pf ) ;

	                        continue ;

	                    } else
	                        inpostmark = 0 ;

/* Now strip off the old header or as much of it as has
			been included in the new header */

	                if (f_header && (! autoreply)) {

/* skip all header lines */

	                    if (tempbuf[0] != '\n') {

	                        f_header = FALSE ;

/* end of header, output new header */

	                        putheader( pf, 0 ) ;

	                        continue ;
	                    }

	                }

/* don't send out any BCC or CC lines!!		*/
	                if( (htemp = header_line( tempbuf )) == MH_BCC )
	                    continue ;

	                if (htemp == MH_CC) continue ;

/* Escape From lines */
	                if( strncmp( tempbuf, "From", 4 ) == 0 )
	                    fputs( ">", pf ) ;

	                fputs( tempbuf, pf ) ;

	                if (htemp >= 0) fputs( "\n", pf ) ;

	            }
	        }

	        if (pclose(pf) == 0) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG))
	            errprintf("pipeit: pipe closed successfully\n") ;
#endif

	            if (tf != NULL) fclose(tf) ;

	            pipejump = 0 ;
	            signal(SIGPIPE, (void *) savepsig) ;

	            return 0 ;
	        }
	    }
	}

#if	DEBUG
	errprintf("pipeit: pipe close failed\n") ;
#endif

	pipejump = 0 ;
	signal(SIGPIPE, (void *) savepsig) ;

	if(tf != NULL) fclose(tf) ;

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



