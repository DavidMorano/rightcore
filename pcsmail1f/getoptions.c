/* getoptions */


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
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									

 ************************************************************************/



#include	<sys/utsname.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"c_parse.h"
#include	"c_cmd_mat.h"



/* externals subroutines */

extern int		c_parse() ;

extern char		*malloc_sbuf() ;


/* external variables */

extern struct global	g ;

extern int		isfile ;


/* local data */

struct c_parsed		c_p ;



/* define command option words */

#define	OPT_V		0
#define	OPT_DELIVERY	1
#define	OPT_EDIT	2
#define	OPT_FILECOPY	3
#define	OPT_STANDARD	5
#define	OPT_VERIFY	6
#define	OPT_QQQ		7
#define	OPT_WAIT	8
#define	OPT_INTERNET	9
#define	OPT_ACKNOWLEDGE	10
#define	OPT_ORIGEDIT	11
#define	OPT_NAME	12
#define	OPT_FULLNAME	13
#define	OPT_NOPROMPT	14
#define	OPT_VERSION	15
#define	OPT_NOSEND	16


static char *options[] = {
	"V",
	"delivery",		/* 1 */
	"edit",			/* 2 */
	"filecopy",		/* 3 */
	"notify",		/* 4 */
	"standard",		/* 5 */
	"verify",		/* 6 */
	"?",			/* 7 */
	"wait",			/* 8 */
	"internet",		/* 9 */
	"acknowledge",		/*10 */
	"original_edit",	/*11 *//*PAS-JM 2/8/85*/
	"name",
	"fullname",
	"noprompt",
	"version",
	"nosend",
	NULL
} ;


/* define command keywords */

#define	KW_SENDMAIL	15


static char	*keywords[] = {
	"empty",
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
	NULL
} ;



int getoptions( stringform, argform )
char	*stringform ;
char	*argform[] ;
{
	int	reciplen ;
	int	c_recip = 0 ;
	int	j,k, l, jv ;
	int	f_exit = FALSE ;

	char	sendopt[512] ;
	char	eformat[800] ;
	char	*option, *sdopt ;


#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("getoptions: entered\n") ;
#endif

	runcmd = YES ;			/* be optimistic! */
	reciplen = strlen(recipient) ;

	if (stringform != NULL) {

	    strcpy(eformat, 
	        "%s: %s \"%c%s\" ; respecify SMAILOPTS\n") ;

	    strcpy(sendopt, stringform) ;

	    option = strtok(sendopt,":") ;

	} else {

	    strcpy(eformat, 
	        "%s: %s \"%c%s\" ; retype the command line\n") ;

	    argform += 1 ;		/* skip the command name */
	    option = *argform++ ;	/* pick first word after name */

	}

	while (option != NULL) {

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("getoptions: top of while\n") ;
#endif

	    if (stringform != NULL) sdopt = strtok(0,"") ;

/* switch on "spec" */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("getoptions: top of switch, option=\"%s\"\n",
	            option) ;
#endif

	    jv = 0 ;		/* assume spec is + (see PLUS:) */
	    switch (c_parse(option,&c_p)) {

	    case MINUS:
#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: switch MINUS\n") ;
#endif

	        jv = -1 ;		/* set to 0 on fall thru */

/* - entered alone */

	        if (strcmp(c_p.opt, NULLSTR) == 0) {

	            break ;
	        }

/* fall through to the next case */

	    case PLUS:
#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: switch PLUS\n") ;
#endif

	        jv += 1 ;  		/* set +1 */
	        if (strcmp(c_p.opt,NULLSTR) == 0) {

/* + entered standalone: process */
	            fprintf(stderr,eformat,comm_name, 
	                "invalid option",c_p.spec,"") ;

	            break ;

	        } /* end if */

/* both "plus" and "minus" above really wanted to execute this next */

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: switch top of PLUS & MINUS\n") ;
#endif

	        if (c_cmd_match(c_p.opt,options) == 0) {

/* unknown option: process error */

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: switch unknown option\n") ;
#endif

	            logfile_printf(&g.lh,"unknown invocation option\n") ;

	            bprintf(g.efp,"%s: unknown invocation option\n",
	                g.progname) ;

	            bflush(g.efp) ;

	            bprintf(g.efp, eformat, comm_name,
	                "unknown option",
	                c_p.spec,c_p.opt) ;

	            f_exit = TRUE ;
	            g.f.exit = TRUE ;
	            break ;

	        } /* end if */

/* ambiguous option: process error */

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: about to check for ambiguous\n") ;
#endif

	        if ((imatch[0] != OPT_V) && (imat > 1)) {

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

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: about to switch \n") ;
#endif

/* valid option: switch on imatch[0] */
/* and process accordingly	     */

	        switch (imatch[0]) {

	        case OPT_V:
	        case OPT_VERSION:
	            g.f.version = TRUE ;
	            break ;

/* "acknowledge" */
/* "delivery" */
	        case OPT_ACKNOWLEDGE:
	        case 1:
	            break ;

/* "edit" */
	        case 2:
	            if (jv) isedit = 2 ;

	            else isedit = 0 ;

	            break ;

/* "filecopy" */
	        case 3:
	            copy = jv ;
	            break ;

/* "notify" */
	        case 4:
	            break ;

/* "standard" */
	        case 5:
	            standard = jv ;
	            break ;

/* "verify" */
	        case 6:
	            verify = jv ;
	            break ;

/* ignore as an option */
	        case OPT_QQQ:
	            break ;

/* "wait" */
	        case 8:
	            iswait = jv ;
	            break ;

/* "internet" */
	        case 9:
	            f_internet = jv ;
	            break ;

/* "original_edit" */
	        case 11:
	            orig_edit = jv ;
	            break ;

	        case OPT_NAME:
	            f_name = jv ;
	            break ;

	        case OPT_FULLNAME:
	            f_fullname = jv ;
	            break ;

	        case OPT_NOPROMPT:
	            f_noprompt = jv ;
	            break ;

	        case OPT_NOSEND:
	            g.f.nosend = TRUE ;
	            break ;

	        } /* end switch */

	        break ;

	    case EQUAL:

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: EQUAL\n") ;
#endif

	        if (strcmp(c_p.opt,NULLSTR) == 0) {

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
	            f_exit = TRUE ;
	            g.f.exit = TRUE ;
	            break ;
	        }

/* valid keyword */

	        switch (imatch[0]) {

/* "append_file" */
	        case 1:
	            if(c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "append_file=") ;

	            }

	            isappend = 1 ;
	            strwcpy(appfile,c_p.value,SYSLEN) ;

	            appfile[SYSLEN - 1] = '\0' ;
	            break ;

/* "copy_to" */
	        case 2:
	            strwcpy(copyto,c_p.value,BUFSIZE) ;

	            copyto[BUFSIZE - 1] = '\0' ;
	            cc = 1 ;
	            break ;

/* "file" */
	        case 3:

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: EQUAL file\n") ;
#endif

	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "file=") ;

	                runcmd = NO ;
	            }

	            isfile = 1 ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: turned 'isfile' ON\n") ;
#endif

	            strwcpy(filename,c_p.value,SYSLEN) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: after 'strncpy' isfile=%d\n",isfile) ;
#endif

	            filename[SYSLEN - 1] = '\0' ;
	            break ;

/* "keywords" */
	        case 4:
	            keyword = 1 ;
	            strwcpy(keys,c_p.value,BUFSIZE) ;

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

	                strwcpy(message,c_p.value,BUFSIZE) ;

	                message[BUFSIZE-1] = '\0' ;

	            }

	            break ;

/* "subject" */
	        case 6:

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: got a subject\n") ;
#endif

	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "subject=") ;
	                runcmd = NO ;

	            } else {

	                strwcpy(subject,c_p.value,BUFSIZE) ;

	                subject[BUFSIZE-1] = '\0' ;

	            }

	            break ;

/* "time" */ /* not yet implemented */
	        case 7:
	            break ;

/* "dappend_file" */
	        case 8:
#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: option \"dappend\"\n") ;
#endif

	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "dappend_file=") ;

	                break ;
	            }
	            isforward = 1 ;
	            strwcpy(forwfile,c_p.value,SYSLEN) ;

	            forwfile[SYSLEN-1] = '\0' ;

	            break ;

/* return path */
	        case 9:

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: EQUAL return\n") ;
#endif

	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "return_path=") ;
	                break ;
	            }

	            isret = 1 ;
	            strwcpy(retpath,c_p.value,BUFSIZE) ;

	            retpath[BUFSIZE-1] = '\0' ;

	            break ;

/* "eappend_file" */
	        case 10:
#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: option \"eappend\"\n") ;
#endif

	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "eappend_file=") ;

	                break ;
	            }
	            iseforward = 1 ;
	            strwcpy(eforwfile,c_p.value,SYSLEN) ;

	            eforwfile[SYSLEN - 1] = '\0' ;

	            break ;

/* "ereference" */
	        case 11:
	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "ereference=") ;

	                break ;
	            }
	            isreferenced = 1 ;
	            strwcpy(reference,c_p.value,BUFSIZE) ;

	            reference[BUFSIZE-1] = '\0' ;
	            break ;

/* "blindcopy_to" */
	        case 12:
	        case 13:
	            strwcpy(bcopyto,c_p.value,BUFSIZE) ;

	            bcopyto[BUFSIZE-1] = '\0' ;

	            bcc = 1 ;
	            break ;

/* "original" */
	        case 14:
	            if (c_p.value == NULLSTR) {

	                fprintf(stderr, eformat,
	                    comm_name,
	                    "incomplete keyword",
	                    ' ', "eappend_file=") ;

	                break ;
	            }
	            isoriginal = 1 ;
	            strwcpy(origfile,c_p.value,SYSLEN) ;

	            origfile[SYSLEN-1] = '\0' ;
	            break ;
	        }
	        break ;

/* specify the delivery daemon program */
	    case KW_SENDMAIL:

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: got a sendmail\n") ;
#endif

	        if (c_p.value == NULLSTR) {

	            fprintf(stderr, eformat, comm_name,
	                "incomplete keyword",
	                ' ', "sendmail=") ;

	            runcmd = NO ;

	        } else {

	            l = strlen(c_p.value) ;

	            if (l > BUFSIZE) {

	                l = BUFSIZE ;
	                c_p.value[l] = '\0' ;

	            }

	            g.prog_sendmail = malloc_sbuf(c_p.value,l) ;

#if	CF_DEBUG
	            if (g.f.debug)
	                debugprintf("getoptions: got to the string copy\n") ;
#endif

	        }

	        break ;

/* not a keyword or option: process */
	    default:

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("getoptions: EQUAL default\n") ;
#endif

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

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("getoptions: after EQUAL switch\n") ;
#endif

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("getoptions: isfile=%d\n",isfile) ;
#endif

	    if (stringform != 0)
	        option = strtok(sdopt,":") ;

	    else 
	        option = *argform++ ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("getoptions: bottom of while\n") ;
#endif

	} /* end while */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("getoptions: out of while\n") ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("getoptions: isfile=%d\n",isfile) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("getoptions: exiting isfile=%d\n",isfile) ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("getoptions: exiting w/ exit=%d\n",f_exit) ;
#endif

	if (f_exit) return BAD ;

	return OK ;
}
/* end subroutine (getoptions) */



