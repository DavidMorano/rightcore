/* redomsg */


#define		DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
		David A.D. Morano
 *									



 ************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"header.h"



/* external variables */

extern struct global	g ;



/* copy the message body from fp2 to fp1 */

void copymessage(fp1,fp2)
FILE	*fp1, *fp2 ;
{
	int	l, f_header ;

	char	linebuf[LINELEN + 1] ;


#ifdef	COMMENT
	int lheader ;
	int inheader ;


/* NOTE: header_line returns a positive value identifying the	*/
/* type of header line, based on the keyword. If the line is	*/
/* not a header line, then it returns (-1) */

	inheader = TRUE ;
	while (fgetline(fp2,s,BUFSIZE) > 0) {

/* skip existing fields */

	    if (inheader) {

	        if (*s == '\n') {

	            inheader = FALSE ;
	            continue ;
	        }

	        if ((lheader = header_line(s)) >= 0) continue ;

	    } else 
		lheader = header_line(s) ;

	    if ((lheader == CC) || (lheader == BCC)) continue ;

	    fputs(s,fp1) ;

/* 
	I think that the following code is garbage-ola and should be
	removed.  There does not appear to be ANY circumstance
	in this routine where a header should be copied to the
	new message file.  So I comments the junk out !
*/

#ifdef	COMMENT

/* If the line is a header line then add a '\n' 	*/
/* to the output file. This is necessary because	*/
/* header_line chops off the '\n' from the line when	*/
/* it decides that the line is a header line!!!		*/

	    if (lheader >= 0) {

		fputs("\n",fp1) ;

fprintf(stderr,
"added a newline because of header line bug\n") ;

	    }
#endif

	} /* end while */

#else
	f_header = TRUE ;
	while ((l = fgetline(fp2,linebuf,LINELEN)) > 0) {

		linebuf[l] = '\0' ;
		if (! f_header) fputs(linebuf,fp1) ;

		if (linebuf[0] == '\n') f_header = FALSE ;

	}
#endif

}
/* end subroutine (copymessage) */



