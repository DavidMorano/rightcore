/* checklist */


#define	DEBUG	1


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
		David A.D. Morano


  checkreclist() checks for validity of all the currently specified
  recipients of the mail message.


************************************************************************/



#include	<string.h>
#include	<stdio.h>

#include	<baops.h>

#include	"config.h"
#include	"smail.h"



/* external subroutines */

extern char	*strtoken() ;


/* external variables */

extern struct global	g ;


/* new global data */

int	*cl_va = NULL ;



#define firstname()	tokenlen = 0 ;\
		errprintf("firstname-1: (sbuf + tl)=\"%s\"\n", \
			sbuf + tokenlen) ; \
		realname = strtoken(sbuf,",",&tokenlen);\
		errprintf("firstname-2: (sbuf + tl)=\"%s\"\n", \
			sbuf + tokenlen) ; \
	        *selectname = '\0'; *curaddr = '\0';\
	        first = 1

#define nextname()	errprintf("nextname-1: (sbuf + tl)=\"%s\"\n", \
		sbuf + tokenlen) ; \
	if(!first){ *npt++ = ','; *npt++ = ' ';\
	                *npt = '\0';}\
	        else first = 0;\
	        {\
	            char *d1,*d2;\
	            d1 = selectname; d2 = curaddr;\
	            while (*npt++ = *d1++) ;\
	            npt--;\
	            if( *d2 != '\0' )\
	            {\
	                *npt++ = ' ';\
	                *npt++ = '<';\
	                while (*npt++ = *d2++) ;\
	                npt--;\
	                *npt++ = '>';\
	                *npt = '\0';\
	            }\
	        }\
	        *selectname = '\0'; *curaddr = '\0';\
	        realname = strtoken(sbuf,",",&tokenlen); \
		errprintf("nextname-2: (sbuf + tl)=\"%s\"\n",sbuf + tokenlen) ;


#define endlist()	*npt = '\0'

#define beginlist(name)	npt = s1buf ; str = name ; *npt = '\0'



checkreclist(mode)
int	mode ;
{
	int	state ;
	int	tokenlen ;
	int	value = 31415926 ;

	char	sbuf[BUFSIZE * 2 + 8] ;
	char	s1buf[BUFSIZE * 2 + 8] ;
	char *realname;
	char *str;
	char first;
	char *npt;


	cl_va = &value ;

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: called\n") ;
#endif

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;
#endif

	state = error = 0;
	ambiguous = 0;		/* be optimistic */
	if (tonames <= 0)
	    printf("WARNING: There are no primary recipients\n");

/* check recipient list */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to check for recipients - t=%d c=%d b=%d\n",
		tonames,ccnames,bccnames) ;
#endif

	if (tonames > 0 || ccnames > 0 || bccnames > 0) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: goto some recipients - state=%d\n",
		state) ;
#endif

	    while (state < 3) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: inside state while about to switch - state=%d\n",
		state) ;
#endif

	        switch (state) {

	        case 0:
	            strcpy(sbuf,recipient);

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: state=%d about to call 'beginlist()' w/ recip=\%s\"\n",
		state,recipient) ;
#endif

	            beginlist(recipient);

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: finished w/ 'beginlist()'\n") ;
#endif

	            state++;
	            break;

	        case 1:
	            state++;
	            if(*copyto == '\0') continue;

	            strcpy(sbuf,copyto);

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: state=%d about to call 'beginlist()' w/ recip=\%s\"\n",
		state,recipient) ;
#endif

	            beginlist(copyto);

	            break;

	        case 2:
	            state++;
	            if(*bcopyto == '\0') continue;

	            strcpy(sbuf,bcopyto);

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: state=%d about to call 'beginlist()' w/ recip=\%s\"\n",
		state,recipient) ;
#endif

	            beginlist(bcopyto);

	            break;

	        default:
	            errprintf("checkreclist: This should not happen!\n");
	            rmtemp(0);

	        }

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: exited switch about to call 'firstname()'\n") ;
#endif

/* The first word is always a name in state 1*/

	        firstname();

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: returned from 'firstname()'\n") ;
#endif

	        if (ex_mode == EM_PCSMAIL) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: execution mode is PCSMAIL\n") ;
#endif

/* If the first name cannot be mapped to a valid address
		 * and a return path has been provided, then try to use
		 * the return path instead
*/

	            if (isret && (realname != NULL) && (mode == 0)
	                && (*realname != NULL)	/*PAS-JM 2/8/85*/
	                && f_interactive && (state == 1)
	                && (strchr(realname,'<') == NULL)) {

	                error = 0;	/* JM 2/22/85 */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to call 'checkname()'\n") ;
#endif

	                if (checkname(realname,mode) != 1) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: 'checkname()' A returned != 1\n") ;
#endif

/* "\nWill try to use the return address from the mail envelope.\n\n"); */

	                    if (checkname(retpath,mode) == 1) {

	                        strcpy(selectname, realname);

#ifdef	COMMENT
	                        printf(
	                            "The address being used for %s is %s\n",
	                            selectname,curaddr);
#endif

	                    } else {

/* "The return address on the envelope doesn't make any sense either!\n"); */

	                        error = 1;
	                    }
	                }

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to call 'nextname()' ca=\"%s\" sn=\"%s\"\n",
	curaddr,selectname) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"about to do it\n") ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"&sbuf=%08X tl=%d\n",
	(int) sbuf,tokenlen) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"did it\n") ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: 'nextname()' sbuf=\"%s\" sl=%d tl=%d\n",
	sbuf,strlen(sbuf),tokenlen) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"&s1=%08X &npt=%08X\n",
	s1buf,npt) ;
#endif

	                nextname();

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: out of 'nextname()' in the if\n") ;
#endif

	            }

/*PAS-JM 2/8/85*/

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to enter while loop\n") ;
#endif

	            while ((realname != NULL) && (*realname != '\0')) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;
#endif

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to call 'checkname()' w/\n") ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist:\trn=\"%s\" mode=%d\n",
		realname,mode) ;
#endif

	                if (checkname(realname,mode) != 1) {

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: 'checkname()' B returned != 1\n") ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"checkreclist: well &v=%08X\n",&value) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"checkreclist: about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;
#endif

	                    error = 1;
	                }

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to call 'nextname()' ca=\"%s\" sn=\"%s\"\n",
	curaddr,selectname) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"this one - about to do it\n") ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"&sbuf=%08X\n",
	(int) sbuf) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"tl=%d\n",
	tokenlen) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"did it\n") ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: 'nextname()' sbuf=\"%s\" sl=%d tl=%d\n",
	sbuf,strlen(sbuf),tokenlen) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
 	"&s1=%08X &npt=%08X\n",
	s1buf,npt) ;

	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: 'nextname()' s1buf=\"%s\" sl(s1buf)=%d\n",
	s1buf,strlen(s1buf)) ;
#endif

	                nextname();

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: out of 'nextname()' in while loop\n") ;
#endif

	            } /* end while */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: out of while loop\n") ;
#endif

	        } /* end if (execution mode is PCSMAIL) */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to call 'endlist()'\n") ;
#endif

	        endlist();

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: returned from 'endlist()'\n") ;
#endif

	        if (strlen(s1buf) >= BUFSIZE) {

	            s1buf[BUFSIZE-1] = '\0';
	            *strrchr(s1buf, ',') = '\0';

	            errprintf(
	                "%s: warning - too many recipients, list truncated\n",
	                g.progname);

	            printf(
	                "%s: warning - too many recipients, list truncated\n",
	                g.progname);

	        }

	        strcpy(str,s1buf) ;

	    } /* end while */

	} /* end if */

/* setup the name lists properly */

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: about to check for ambiguous\n") ;
#endif

	if (ambiguous) {

	    tonames = getnames(recipient);

	    ccnames = getnames(copyto);

	    bccnames = getnames(bcopyto);

	}
	*realto = '\0';

#if	DEBUG
	if (BATST(g.uo,UOV_DEBUG)) errprintf(
	"checkreclist: exiting\n") ;
#endif

}
/* end subroutine (checklist) */


