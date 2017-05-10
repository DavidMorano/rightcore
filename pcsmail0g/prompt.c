static char sccsid[] = "@(#)prompt.c	PCS 3.0";
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
/*
 * prompt() prompts the user and collects string values for individual
 * header fields.
*/


#include	<string.h>
#include	<stdio.h>

#include	"config.h"
#include	"smail.h"
#include	"prompt.h"
#include	"header.h"



static char errmes1[] = "enter the names of recipients";
static char errmes2[] = "separated by commas\n";


prompt(field)
int	field;
{
	int flag;

	char *c;


	if (! f_interactive) return ;

	switch (field) {

	case PC_SENDOPT:
	    printf("send options: standard, verify, filecopy ?\n");

	    if (standard) printf("[standard");

	    if (verify) printf(" verify");

	    if (copy) printf(" filecopy");

	    printf("] ");

	    *sendopt = '\0';
	    if (fgets(sendopt,BUFSIZE,stdin) == NULL) rmtemp(1);

	    c = strrchr(sendopt, '\n');

	    if ( c != NULL) *c = '\0';

	    return;

	case PC_SENDCOM:
	    printf("\nsend, review, check, edit, quit ? [send");

	    if (standard) printf(" standard");

	    if (verify) printf(" verify");

	    if (copy) printf(" filecopy");

	    printf("] ");

	    if (fgets(command,BUFSIZE,stdin) == NULL) rmtemp(1);

	    c = strrchr(command, '\n');

	    if (c != NULL) *c = '\0';

	    return;

/* arguments so ask whom send to */
	case PC_ALL:
	case MH_TO:
	    if (tonames == 0) {

	        flag = 0;
	        while (1) {	/* read in one line */

	            printf("To: ") ;

	            if (fgets(recipient,BUFSIZE,stdin) == NULL) 
			rmtemp(1);

	            c = strrchr(recipient,'\n');

	            if (c != NULL) *c = '\0';

	            if (strcmp("?",recipient) == 0) {

	                printf("%s %s\n",errmes1,errmes2);

	                help(2);

	                flag = 0;
	                continue;
	            }

	            if(strlen (recipient)  != 0) break;

	            if(flag++) {

	                printf("sorry!\n");

	                rmtemp(1);

	            }

	            printf("%s %s\n",errmes1,errmes2);

	            printf("if you need more help, enter \"?\"\n");

	        }

/* set up token string in "recipient" */

	        if (strcmp(".",recipient) == 0) {

	            isedit = 1 ;
	            *recipient = '\0';

	        } else tonames = getnames(recipient);

	    }

	    if (field != ALL) return ;

	case MH_CC:
	    if (cc && (ccnames == 0) && (isedit != 1)) {   

/* no arguments so ask whom send to */

	        flag = 0;
	        while(1) {	/* read in one line */

	            printf("%s ",Cc);
	            if(fgets(copyto,BUFSIZE,stdin) == NULL)
	                rmtemp(1);
	            c = strrchr(copyto,'\n');
	            if( c != NULL ) *c = '\0';
	            if(strcmp("?",copyto) == 0)
	            {
	                printf("%s (for copy to) %s",
	                    errmes1,errmes2);
	                help(2);
	                flag = 0;
	                continue;
	            }
	            if(strcmp("",copyto) == 0) cc = 0;
	            break;
	        }

/* set up token string in "copyto" */

	        if(strcmp(".",copyto) == 0) {

	            isedit = 1 ;
	            *copyto = '\0';

	        } else {

	            ccnames = getnames(copyto);
	        }
	    }

	    if (field != ALL) return ;

/* no arguments, so ask whom send to */

	case MH_BCC:
	    if (bcc && (bccnames == 0) && (isedit != 1)) {   

	        flag = 0;
	        while(1) {	/* read in one line */

	            printf("%s ",Bcc);

	            if(fgets(bcopyto,BUFSIZE,stdin) == NULL)
	                rmtemp(1);

	            c = strrchr(bcopyto, '\n');

	            if( c != NULL ) *c = '\0';

	            if(strcmp("?",bcopyto) == 0) {

	                printf("%s (for blind-copy to) %s",
	                    errmes1,errmes2);

	                help(2);

	                flag = 0;
	                continue;
	            }

	            if(strcmp("",bcopyto) == 0) bcc = 0;

	            break;
	        }

/* set up token string in "bcopyto" */

	        if(strcmp(".",bcopyto) == 0) {

	            isedit = 1 ;
	            *bcopyto = '\0';

	        } else bccnames = getnames(bcopyto);

	    }

	    if (field != ALL) return ;

	case MH_SUBJECT:
	    if (*subject == '\0' && (isedit != 1)) {

	        while (1) {

	            printf("Subject: ");

	            if (fgets(subject,BUFSIZE,stdin) == NULL)
	                break;

	            c = strrchr(subject,'\n');

	            if (c != NULL) *c = '\0';

	            if (strcmp(".",subject) == 0) {

	                isedit = 1 ;
	                *subject = '\0';
	                break;
	            }

	            if (strcmp("?",subject) != 0) break;

	            help(3);
	        }
	    }

	    if (field != ALL) return ;

/* fall through to case below */

	case MH_KEYWORDS:
	    if(*keys == '\0' && keyword) {	/* prompt for keywords */

	        printf("Keyword: ") ;

	        if (fgets (keys,BUFSIZE,stdin) == NULL ) break;

	        c = strrchr(keys,'\n');

	        if ( c != NULL ) *c = '\0';

	        if ( *keys != '\0') {

	            if (strcmp(".",keys) == 0) isedit = 1 ;

	        }
	    }

	    if (field != ALL) return ;

/* default cases should just fall through */

	} /* end switch */

}
/* end subroutine (prompt) */



