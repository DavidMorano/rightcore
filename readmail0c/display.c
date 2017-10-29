/* display */

/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Bruce Schatz, Jishnu Mukerji                    
 *									

 ***********************************************************************/



/* do we want the blank lines before and after the interactive prompt */

#define		BLANK_LINES	1



#include	<stdio.h>

#include	"defs.h"
#include	"localmisc.h"



/* function to display a message */
/* Modified 7.14.84 by J.Mukerji to admit dumping a message to any file, not*
 * just stdout								    */

display(messnum, fd, page)
int messnum ;
FILE *fd ;
char page ;
{
	int	line = 0 ;
	int	l ;
	int	overlines = 1 ;
	int	f_from = FALSE ;
	int	f_bol, f_eol, f_more ;

	char	linebuf[LINELEN + 1] ;
	char	temp[LINELEN + 1],last[LINELEN + 1] ;
	char	response[100] ;
	char	*cp ;


#if	BLANK_LINES
	overlines += 2 ;
#endif

	messnum = messord[messnum];        /* convert to internal number */

	fseek(curr.fp,messbeg[messnum],0) ;

/* skip to the "FROM:" line (sendmail).  
	     if none (UNIX mail), use last "From" line.
	  */

/* skip all headers except :

	"from"
	"newsgroups"
	"to"
	"subject"
	"board"
	"date"
	"cc"
	"keywords"

*/


	f_more = FALSE ;
	f_bol = TRUE ;
	while ((l = fgetline(curr.fp,linebuf,LINELEN)) > 0) {

	    f_eol = FALSE ;
	    if (linebuf[l - 1] == '\n') f_eol = TRUE ;

	    linebuf[l] = '\0' ;
	    if (linebuf[0] == '\n') break ;

	    if (! f_bol) {

	        if (f_more) {

	            line += 1 ;
	            fprintf(fd,"%s",linebuf) ;

	        }

	    } else {

	        if (f_more && ISWHITE(linebuf[0])) {

	            line += 1 ;
	            fprintf(fd,"%s",linebuf) ;

	        } else {

	            f_more = FALSE ;
	            if (strncasecmp(linebuf,"from",4) == 0) {

	                f_from = TRUE ;
	                cp = linebuf + 4 ;
	                while (ISWHITE(*cp)) cp += 1 ;

	                if (*cp == ':') {

	                    f_more = TRUE ;
	                    line += 1 ;
	                    fprintf(fd,"%s",linebuf) ;

	                }

	            } else if ((strncasecmp(linebuf,"date",4) == 0) ||
	                (strncasecmp(linebuf,"subject",7) == 0) ||
	                (strncasecmp(linebuf,"cc",2) == 0) ||
	                (strncasecmp(linebuf,"to",2) == 0) ||
	                (strncasecmp(linebuf,"keywords",8) == 0) ||
	                (strncasecmp(linebuf,"board",5) == 0) ||
	                (strncasecmp(linebuf,"newsgroups",10) == 0)) {

	                f_more = TRUE ;
	                line += 1 ;
	                fprintf(fd,"%s",linebuf) ;

	            }

	        } /* end if (header continuation) */

	    } /* end if (BOL or not) */

	    f_bol = f_eol ;

	} /* end while */

	line += 1 ;
	fprintf(fd,"%s",linebuf) ;

/* continue */

#ifdef	COMMENT
	f_from = FALSE ;
	fgets(temp,LINELEN,curr.fp) ;

	strcpy(last,temp) ;

	while (fgets(temp,LINELEN,curr.fp) != NULL) {

	    if ((strncasecmp(temp,"FROM:",5) == 0) ||
	        (strncasecmp(temp,"Date:",5) == 0)) {

	        f_from = TRUE ;
	        break ;
	    }

	    if (( strncmp(temp,"\n",1) == 0) ||
	        ( ftell(curr.fp)>= messend[messnum])) break ;

	}
#endif

#ifdef	COMMENT
	if (! f_from) {

/* this is a non-PCS message */

	    fseek(curr.fp,messbeg[messnum],0) ;

	    while (fgets (temp,LINELEN,curr.fp) != NULL) {

	        if ( (strncasecmp(temp,"From",4) != 0)  &&
	            (strncasecmp(temp,">From",5) != 0)) break ;

	        strcpy (last,temp) ;

	    }

	    if (*last == '>')
	        fprintf (fd,"%s",&last[1]) ;

	    else 
	        fprintf (fd,"%s",last) ;

	}

	fprintf(fd,"%s",temp);	/* first line of message */

#endif

/* print message without paging */

	if ((! profile("page")) || (! page)) {

	    while (ftell(curr.fp) < messend[messnum]) {

	        fgets(temp,LINELEN,curr.fp) ;

	        fprintf(fd,"%s",temp) ;

	    } /* end while */

	    return ;

	} else {

/* print out the message with paging */
begin:
	    while (ftell(curr.fp) < messend[messnum]) {

	        if (line < (maxlines - overlines)) {

	            fgets(temp,LINELEN, curr.fp) ;

	            line += 1 ;
	            fprintf(fd,"%s", temp) ;

	        } else {

#if	BLANK_LINES
	            fprintf(stderr,"\n") ;
#endif

	            fprintf(stderr,
	                "<<type a CR to continue, \"q\" to leave message>> ") ;

	            gets(response) ;

	            if ((strlen(response)) == 0) {

	                line = 0 ;
#if	BLANK_LINES
	                fprintf(stderr,"\n") ;
#endif

	                continue ;

	            } else
	                return ;

	        } /* end if */

	    } /* end while */

	}
}
/* end subroutine (display) */


