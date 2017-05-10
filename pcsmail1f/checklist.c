/* checklist */


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
		David A.D. Morano


  checkreclist() checks for validity of all the currently specified
  recipients of the mail message.


************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<baops.h>
#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external subroutines */

extern char	*strtoken() ;


/* external variables */

extern struct global	g ;


/* new global data */

int	*cl_va = NULL ;



#define firstname()	tokenlen = 0 ;\
		realname = strtoken(sbuf,",",&tokenlen);\
	        *selectname = '\0'; *curaddr = '\0';\
	        first = 1

#define nextname()	if(!first){ *npt++ = ','; *npt++ = ' ';\
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
	        realname = strtoken(sbuf,",",&tokenlen);


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

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: called\n") ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checklist: about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;
#endif

	state = error = 0;
	ambiguous = 0;		/* be optimistic */
	if (tonames <= 0)
	    printf("WARNING: There are no primary recipients\n");

/* check recipient list */

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to check for recipients - t=%d c=%d b=%d\n",
		tonames,ccnames,bccnames) ;
#endif

	if (tonames > 0 || ccnames > 0 || bccnames > 0) {

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: goto some recipients - state=%d\n",
		state) ;
#endif

	    while (state < 3) {

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: inside state while about to switch - state=%d\n",
		state) ;
#endif

	        switch (state) {

	        case 0:
	            strcpy(sbuf,recipient);

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: state=%d about to call 'beginlist()' w/ recip=\%s\"\n",
		state,recipient) ;
#endif

	            beginlist(recipient);

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: finished w/ 'beginlist()'\n") ;
#endif

	            state++;
	            break;

	        case 1:
	            state++;
	            if(*copyto == '\0') continue;

	            strcpy(sbuf,copyto);

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: state=%d about to call 'beginlist()' w/ recip=\%s\"\n",
		state,recipient) ;
#endif

	            beginlist(copyto);

	            break;

	        case 2:
	            state++;
	            if (*bcopyto == '\0') continue;

	            strcpy(sbuf,bcopyto);

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: state=%d about to call 'beginlist()' w/ recip=\%s\"\n",
		state,recipient) ;
#endif

	            beginlist(bcopyto);

	            break;

	        default:
	            logfile_printf(&g.eh,"checkreclist: This should not happen!\n");

	            rmtemp(TRUE,"checkreclist") ;

			return BAD ;
	        }

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: exited switch about to call 'firstname()'\n") ;
#endif

/* The first word is always a name in state 1*/

	        firstname();

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: returned from 'firstname()'\n") ;
#endif

	        if (ex_mode == EM_PCSMAIL) {

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: execution mode is PCSMAIL\n") ;
#endif

/* If the first name cannot be mapped to a valid address
		 * and a return path has been provided, then try to use
		 * the return path instead
*/

	            if (isret && (realname != NULL) && (mode == 0) &&
	                (*realname != NULL) &&
	                g.f.interactive && (state == 1) &&
	                (strchr(realname,'<') == NULL)) {

	                error = 0;	/* JM 2/22/85 */

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to call 'checkname()'\n") ;
#endif

	                if (checkname(realname,mode) != 1) {

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
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

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to call 'nextname()' ca=\"%s\" sn=\"%s\"\n",
	curaddr,selectname) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: about to do it\n") ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: &sbuf=%08X tl=%d\n",
	(int) sbuf,tokenlen) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: did it\n") ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: 'nextname()' sbuf=\"%s\" sl=%d tl=%d\n",
	sbuf,strlen(sbuf),tokenlen) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: &s1=%08X &npt=%08X\n",
	s1buf,npt) ;
#endif

	                nextname();

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: out of 'nextname()' in the if\n") ;
#endif

	            }

/*PAS-JM 2/8/85*/

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to enter while loop\n") ;
#endif

	            while ((realname != NULL) && (*realname != '\0')) {

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to call 'checkname()' w/\n") ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist:\trn=\"%s\" mode=%d\n",
		realname,mode) ;
#endif

	                if (checkname(realname,mode) != 1) {

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: 'checkname()' B returned != 1\n") ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: well &v=%08X\n",&value) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;
#endif

	                    error = 1;
	                }

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to call 'nextname()' ca=\"%s\" sn=\"%s\"\n",
	curaddr,selectname) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: this one - about to do it\n") ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: &sbuf=%08X\n",
	(int) sbuf) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: about to do &tokenlen=%08X v=%d\n",&tokenlen,value) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: tl=%d\n",
	tokenlen) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: did it\n") ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: 'nextname()' sbuf=\"%s\" sl=%d tl=%d\n",
	sbuf,strlen(sbuf),tokenlen) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
 	"checkreclist: &s1=%08X &npt=%08X\n",
	s1buf,npt) ;

	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: 'nextname()' s1buf=\"%s\" sl(s1buf)=%d\n",
	s1buf,strlen(s1buf)) ;
#endif

	                nextname();

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: out of 'nextname()' in while loop\n") ;
#endif

	            } /* end while */

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: out of while loop\n") ;
#endif

	        } /* end if (execution mode is PCSMAIL) */

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to call 'endlist()'\n") ;
#endif

	        endlist();

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: returned from 'endlist()'\n") ;
#endif

	        if (strlen(s1buf) >= ((unsigned) BUFSIZE)) {

	            s1buf[BUFSIZE-1] = '\0';
	            *strrchr(s1buf, ',') = '\0';

	            logfile_printf(&g.eh,
	                "checkreclist: warning - too many recipients\n",
	                g.progname);

	            printf(
	                "%s: warning - too many recipients, list truncated\n",
	                g.progname);

	        }

	        strcpy(str,s1buf) ;

	    } /* end while */

	} /* end if */

/* setup the name lists properly */

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: about to check for ambiguous\n") ;
#endif

	if (ambiguous) {

	    tonames = getnames(recipient);

	    ccnames = getnames(copyto);

	    bccnames = getnames(bcopyto);

	}
	*realto = '\0';

#if	CF_DEBUG
	if (g.debuglevel > 0) logfile_printf(&g.eh,
	"checkreclist: exiting\n") ;
#endif

	return OK ;
}
/* end subroutine (checklist) */


