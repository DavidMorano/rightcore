/* prompt */


#define	CF_DEBUG	0


/************************************************************************
 									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		J.Mukerji						
 									

 * prompt() prompts the user and collects string values for individual
 * header fields.


 ************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"prompt.h"
#include	"header.h"



/* external subroutines */

void		rmtemp() ;


/* external variables */

struct global		g ;


/* local variables */

static char errmes1[] = "enter the names of recipients" ;
static char errmes2[] = "separated by commas\n" ;



int prompt(field)
int	field ;
{
	int flag ;

	char *c ;


	if (! g.f.interactive) return ;

	switch (field) {

	case PC_SENDOPT:
	    bprintf(g.ofp,"send options: standard, verify, filecopy ?\n") ;

	bprintf(g.ofp,"[") ;

	    if (standard) bprintf(g.ofp,"standard") ;

	    if (verify) bprintf(g.ofp," verify") ;

	    if (copy) bprintf(g.ofp," filecopy") ;

	    bprintf(g.ofp,"] ") ;

	    *sendopt = '\0' ;
	    if (fgets(sendopt,BUFSIZE,stdin) == NULL) {

	        rmtemp(TRUE,"prompt") ;

	        return BAD ;
	    }

	    c = strrchr(sendopt, '\n') ;

	    if ( c != NULL) *c = '\0' ;

	    return ;

	case PC_SENDCOM:
	    bprintf(g.ofp,"\nsend, review, check, edit, quit ? [send") ;

	    if (standard) bprintf(g.ofp," standard") ;

	    if (verify) bprintf(g.ofp," verify") ;

	    if (copy) bprintf(g.ofp," filecopy") ;

	    bprintf(g.ofp,"] ") ;

	    if (fgets(command,BUFSIZE,stdin) == NULL) {

	        rmtemp(TRUE,"prompt") ;

	        return BAD ;
	    }

	    c = strrchr(command, '\n') ;

	    if (c != NULL) *c = '\0' ;

	    return ;

/* arguments so ask whom send to */
	case PC_ALL:
	case MH_TO:
	    if (tonames == 0) {

	        flag = 0 ;
	        while (1) {	/* read in one line */

	            bprintf(g.ofp,"To: ") ;

	            if (fgets(recipient,BUFSIZE,stdin) == NULL) {

	        rmtemp(TRUE,"prompt") ;

	                return BAD ;
	            }

	            c = strrchr(recipient,'\n') ;

	            if (c != NULL) *c = '\0' ;

	            if (strcmp("?",recipient) == 0) {

	                bprintf(g.ofp,"%s %s\n",errmes1,errmes2) ;

	                help(2) ;

	                flag = 0 ;
	                continue ;
	            }

	            if(strlen (recipient)  != 0) break ;

	            if (flag++) {

	                bprintf(g.efp,"%s: sorry, some bad error ?\n") ;

	        rmtemp(TRUE,"prompt") ;

	                return BAD ;
	            }

	            bprintf(g.ofp,"%s %s\n",errmes1,errmes2) ;

	            bprintf(g.ofp,"if you need more help, enter \"?\"\n") ;

	        }

/* set up token string in "recipient" */

	        if (strcmp(".",recipient) == 0) {

	            isedit = 1 ;
	            *recipient = '\0' ;

	        } else 
			tonames = getnames(recipient) ;

	    } /* end if */

	    if (field != ALL) return ;

/* fall through to case below */

	case MH_CC:
	    if (cc && (ccnames == 0) && (isedit != 1)) {

/* no arguments so ask whom send to */

	        flag = 0 ;
	        while (TRUE) {	/* read in one line */

	            bprintf(g.ofp,"%s ",Cc) ;

	            if (fgets(copyto,BUFSIZE,stdin) == NULL) {

	        rmtemp(TRUE,"prompt") ;

	                return BAD ;
	            }

	            c = strrchr(copyto,'\n') ;
	            if ( c != NULL ) *c = '\0' ;

	            if (strcmp("?",copyto) == 0) {

	                bprintf(g.ofp,"%s (for copy to) %s",
	                    errmes1,errmes2) ;

	                help(2) ;

	                flag = 0 ;
	                continue ;
	            }

	            if (strcmp("",copyto) == 0) cc = 0 ;

	            break ;

	        } /* end while */

/* set up token string in "copyto" */

	        if (strcmp(".",copyto) == 0) {

	            isedit = 1 ;
	            *copyto = '\0' ;

	        } else {

	            ccnames = getnames(copyto) ;
	        }

	    } /* end if */

	    if (field != ALL) return ;

/* fall through to case below */

/* no arguments, so ask whom send to */

	case MH_BCC:
	    if (bcc && (bccnames == 0) && (isedit != 1)) {

	        flag = 0 ;
	        while (TRUE) {	/* read in one line */

	            bprintf(g.ofp,"%s ",Bcc) ;

	            if (fgets(bcopyto,BUFSIZE,stdin) == NULL) {

	        rmtemp(TRUE,"prompt") ;

	                return BAD ;
	            }

	            c = strrchr(bcopyto, '\n') ;

	            if ( c != NULL ) *c = '\0' ;

	            if (strcmp("?",bcopyto) == 0) {

	                bprintf(g.ofp,"%s (for blind-copy to) %s",
	                    errmes1,errmes2) ;

	                help(2) ;

	                flag = 0 ;
	                continue ;
	            }

	            if (strcmp("",bcopyto) == 0) bcc = 0 ;

	            break ;

	        } /* end while */

/* set up token string in "bcopyto" */

	        if (strcmp(".",bcopyto) == 0) {

	            isedit = 1 ;
	            *bcopyto = '\0' ;

	        } else 
			bccnames = getnames(bcopyto) ;

	    } /* end if */

	    if (field != ALL) return ;

/* fall through to case below */

	case MH_SUBJECT:
	    if (*subject == '\0' && (isedit != 1)) {

	        while (1) {

	            bprintf(g.ofp,"Subject: ") ;

	            if (fgets(subject,BUFSIZE,stdin) == NULL)
	                break ;

	            c = strrchr(subject,'\n') ;

	            if (c != NULL) *c = '\0' ;

	            if (strcmp(".",subject) == 0) {

	                isedit = 1 ;
	                *subject = '\0' ;
	                break ;
	            }

	            if (strcmp("?",subject) != 0) break ;

	            help(3) ;

	        } /* end while */

	    } /* end if */

	    if (field != ALL) return ;

/* fall through to case below */

	case MH_KEYWORDS:
	    if(*keys == '\0' && keyword) {	/* prompt for keywords */

	        bprintf(g.ofp,"Keyword: ") ;

	        if (fgets (keys,BUFSIZE,stdin) == NULL ) break ;

	        c = strrchr(keys,'\n') ;

	        if ( c != NULL ) *c = '\0' ;

	        if ( *keys != '\0') {

	            if (strcmp(".",keys) == 0) isedit = 1 ;

	        }

	    } /* end if */

	    if (field != ALL) return OK ;

/* fall off of the bottom */

/* default cases should just fall through */

	} /* end switch */

	return OK ;
}
/* end subroutine (prompt) */



