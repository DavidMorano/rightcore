/* getoptions */



/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		T.S.Kennedy						*
 *		J.Mukerji						*
 *									*

 ************************************************************************/



#include	<string.h>
#include	<stdio.h>

#include	"config.h"
#include	"smail.h"
#include	"c_parse.h"
#include	"c_cmd_mat.h"



/* externals */

extern char		c_parse() ;



/* local data */

struct c_parsed		c_p ;



/* define command option words */

#define	O_DELIVERY	1
#define	O_EDIT		2
#define	O_FILECOPY	3
#define	O_STANDARD	5
#define	O_VERIFY	6
#define	O_?		7
#define	O_WAIT		8
#define	O_INTERNET	9
#define	O_ACKNOLEDGE	10
#define	O_ORIGEDIT	11
#define	O_NAME		12
#define	O_FULLNAME	13
#define	O_NOPROMPT	14
#define	O_VERSION	15


static char *options[] = {
	"delivery",		/* 1 */
	"edit",		/* 2 */
	"filecopy",		/* 3 */
	"notify",		/* 4 */
	"standard",		/* 5 */
	"verify",		/* 6 */
	"?",			/* 7 */
	"wait",		/* 8 */
	"internet",		/* 9 */
	"acknowledge",	/*10 */
	"original_edit",	/*11 *//*PAS-JM 2/8/85*/
	"name",
	"fullname",
	"noprompt",
	"version",
	"END",
	NULL
} ;


/* define command keywords */

#define	K_SENDMAIL	15


static char *keywords[] = {
	"append_file",		/* 1 */
	"copy_to",		/* 2 */
	"file",			/* 3 */
	"keywords",		/* 4 */
	"message",		/* 5 */
	"subject",		/* 6 */
	"time",			/* 7 */
	"dappend_file",		/* 8 */ /*jm*/
	"return_path",		/* 9 */ /*1227*/
	"eappend_file",		/* 10*/
	"ereference",		/* 11*/
	"blindcopy_to",		/* 12*/
	"bcc",			/* 13*/
	"original",		/* 14*/
	"sendmail",
	"END",
	NULL
} ;



getoptions( stringform, argform )
char	*stringform ;
char	**argform ;
{
	int reciplen ;
	int	c_recip = 0 ;
	int j,k,jv ;

	char sendopt[512] ;
	char eformat[80] ;
	char *option, *sdopt ;


	runcmd = YES ;			/* be optimistic! */
	reciplen = strlen(recipient) ;

	f_interactive = FALSE ;
	if (isatty(0)) f_interactive = TRUE ;

	if (stringform != ((char *) 0)) {

	    strcpy(eformat, "%s: %s %c%s ignored  ;respecify SMAILOPTS\n") ;

	    strcpy(sendopt, stringform) ;

	    option = strtok(sendopt,":") ;

	} else {

	    strcpy(eformat, 
	        "%s: %s %c%s ignored  ;retype the command line\n") ;

	    argform++ ;			/* skip the command name */
	    option = *argform++ ;	/* pick first word after name */
	}

	while (option != NULL) {

	    if (stringform != 0) sdopt = strtok(0,"") ;

/* switch on "spec" */

	    jv = 0 ;		/* assume spec is + (see PLUS:) */
	    switch (c_parse(option,&c_p)) {

	    case MINUS:
	        jv = -1 ; /*set to 0 on fall thru */

/* - entered alone */

	        if (strcmp(c_p.opt, NULLSTR) == SAME) {

	            f_interactive = FALSE ;
	            break ;
	        }

/* fall through to the next case */

	    case PLUS:
	        ++jv ;	  /* set +1 */
	        if (strcmp(c_p.opt,NULLSTR) == SAME) {

/* + entered standalone: process */
	            fprintf(stderr,eformat,comm_name, 
	                "invalid option",c_p.spec,"") ;

	            break ;
	        }

/* both "plus" and "minus" above really wanted to execute this next */

	        if (c_cmd_match(c_p.opt,options) == 0) {

/* unknown option: process error */
	            fprintf(stderr, eformat, comm_name,
	                "unknown option",
	                c_p.spec,c_p.opt) ;
	            break ;
	        }

/* ambiguous option: process error */

	        if (imat > 1) {

/* ambiguous option: process error */

	            fprintf(stderr, eformat, comm_name,
	                "ambiguous option",
	                c_p.spec, c_p.opt) ;

	            fprintf(stderr, "option matches -") ;

	            for (k=0;k < imat;k++)
	                fprintf(stderr,
	                    " %s",options[imatch[k]]) ;

	            break ;
	        }

/* valid option: switch on imatch[0] */
/* and process accordingly	     */

	        switch (imatch[0] + 1) {

	        case 10: /* "acknowledge" */
	        case 1:  /* "delivery" */
	            break ;

	        case 2: /* "edit" */
	            if (jv) isedit = 2 ;

	            else isedit = 0 ;

	            break ;

	        case 3: /* "filecopy" */
	            copy = jv ;
	            break ;

	        case 4: /* "notify" */
	            break ;

	        case 5: /* "standard" */
	            standard = jv ;
	            break ;

	        case 6: /* "verify" */
	            verify = jv ;
	            break ;

/* ignore as an option */
	        case 7: /* "?" */
	            break ;

	        case 8: /* "wait" */
	            iswait = jv ;
	            break ;

	        case 9: /* "internet" */
	            f_internet = jv ;
	            break ;

	        case 11:/* "original_edit" */
	            orig_edit = jv ;
	            break ;

	        case O_NAME:
	            f_name = jv ;
			break ;

	        case O_FULLNAME:
	            f_fullname = jv ;
			break ;

	        case O_NOPROMPT:
	            f_noprompt = jv ;
			break ;

		case O_VERSION:
		  f_version = jv ;
		  break ;

	        } /* end switch */

	        break ;

	    case EQUAL:
	        if (strcmp(c_p.opt,NULLSTR) == SAME) {

/* = entered alone: process error */
	            fprintf(stderr, eformat, comm_name,
	                "invalid parameter",
	                ' ', "") ;
	            runcmd = NO ;
	            break ;
	        }

	        if (c_cmd_match(c_p.opt,keywords) == 0) {

/* unknown keyword: process error */
	            fprintf(stderr, eformat, comm_name,
	                "unknown keyword",
	                ' ',c_p.opt) ;
	            runcmd = NO ;
	            break ;
	        }

	        if (imat > 1) {

/* ambiguous keyword: process error */

	            fprintf(stderr, eformat, comm_name,
	                "ambiguous keyword",
	                ' ', c_p.opt) ;

	            fprintf(stderr,"keyword matches -") ;

	            for (k=0;k < imat;k++)
	                fprintf(stderr," %s",keywords[imatch[k]]) ;

	            runcmd = NO ;
	            break ;
	        }

/* valid keyword */

	        switch (imatch[0] + 1) {

/* "append_file" */
	        case 1: 
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "append_file=") ;
	            }
	            isappend = 1 ;
	            strncpy(appfile,c_p.value,SYSLEN) ;
	            appfile[SYSLEN-1] = '\0' ;
	            break ;

/* "copy_to" */
	        case 2: 
	            strncpy(copyto,c_p.value,BUFSIZE) ;
	            copyto[BUFSIZE-1] = '\0' ;
	            cc = 1 ;
	            break ;

/* "file" */
	        case 3: 
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "file=") ;
	                runcmd = NO ;
	            }
	            isfile = 1 ;
	            strncpy(filename,c_p.value,SYSLEN) ;
	            filename[SYSLEN-1] = '\0' ;
	            break ;

/* "keywords" */
	        case 4: 
	            keyword = 1 ;
	            strncpy(keys,c_p.value,BUFSIZE) ;
	            keys[BUFSIZE-1] = '\0' ;
	            break ;

/* "message" */
	        case 5: 
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "message=") ;
	                runcmd = NO ;

	            } else {

	                strncpy(message,c_p.value,BUFSIZE) ;
	                message[BUFSIZE-1] = '\0' ;
	            }
	            break ;

/* "subject" */
	        case 6: 

#ifdef	TEST
fprintf(stderr,"%s: got a subject\n",TESTING) ;
#endif

	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "subject=") ;
	                runcmd = NO ;

	            } else {

	                strncpy(subject,c_p.value,BUFSIZE) ;
	                subject[BUFSIZE-1] = '\0' ;
	            }
	            break ;

/* not yet implemented */
	        case 7: /* "time" */
	            break ;

	        case 8: /* "dappend_file" */
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "dappend_file=") ;
	                break ;
	            }
	            isforward = 1 ;
	            strncpy(forwfile,c_p.value,SYSLEN) ;
	            forwfile[SYSLEN-1] = '\0' ;
	            break ;

/* return path */
	        case 9:
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "return_path=") ;
	                break ;
	            }
	            isret = 1 ;
	            strncpy(retpath,c_p.value,BUFSIZE) ;
	            retpath[BUFSIZE-1] = '\0' ;
	            break ;

/* "eappend_file" */
	        case 10: 
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "eappend_file=") ;
	                break ;
	            }
	            iseforward = 1 ;
	            strncpy(eforwfile,c_p.value,SYSLEN) ;
	            eforwfile[SYSLEN-1] = '\0' ;
	            break ;

/* "ereference" */
	        case 11: 
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "ereference=") ;
	                break ;
	            }
	            isreferenced = 1 ;
	            strncpy(reference,c_p.value,BUFSIZE) ;
	            reference[BUFSIZE-1] = '\0' ;
	            break ;

/* "blindcopy_to" */
	        case 12: 
	        case 13:
	            strncpy(bcopyto,c_p.value,BUFSIZE) ;
	            bcopyto[BUFSIZE-1] = '\0' ;
	            bcc = 1 ;
	            break ;

/* "original" */
	        case 14: 
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "eappend_file=") ;
	                break ;
	            }
	            isoriginal = 1 ;
	            strncpy(origfile,c_p.value,SYSLEN) ;
	            origfile[SYSLEN-1] = '\0' ;
	            break ;
	        }
	        break ;

	    case K_SENDMAIL:

#ifdef	TEST
fprintf(stderr,"%s: got a sendmail\n",TESTING) ;
#endif

	        if (c_p.value == NULLSTR) {

	            fprintf(stderr, eformat, comm_name,
	                "incomplete keyword",
	                ' ', "sendmail=") ;

	            runcmd = NO ;

	        } else {

	            strncpy(s_sendmail,c_p.value,BUFSIZE) ;
	            s_sendmail[BUFSIZE-1] = '\0' ;

#ifdef	TEST
fprintf(stderr,"%s: got to the string copy\n",TESTING) ;
#endif

	        }
	        break ;

/* not a keyword or option: process */

	    default:
	        j = strlen(c_p.value) ;

	        if ((reciplen + j) >= BUFSIZE) break ;

#ifdef	COMMENT
	        if (c_p.value[j-1] != '.') strcat(recipient, " ") ;

	        reciplen += j ;
#else
		if (c_recip > 0) {

			strcat(recipient,",") ;

	        	reciplen += (j + 1) ;

		} else
	        	reciplen += j ;
#endif

	        strcat(recipient, c_p.value) ;

		c_recip += 1 ;

	    } /* end switch */

	    if (stringform != 0) option = strtok(sdopt,":") ;

	    else option = *argform++ ;

	}

	if (! f_interactive ) isedit = 0 ;

}
/* end subroutine (getoptions) */



