/* process */


#define	CF_DEBUGS	0


/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)process.c	1.5	96/05/03 SMI"	/* SVr4.0 1.2	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


/******************************************************************************

    process.c handles the requests, which can be of three types:

		ANNOUNCE - announce to a user that a talk is wanted

		LEAVE_INVITE - insert the request into the table
		
		LOOK_UP - look up to see if a request is waiting in
			  in the table for the local user

		DELETE - delete invitation


******************************************************************************/


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <utmpx.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <syslog.h>

#include	<vsystem.h>
#include	<tmpx.h>

#include	"config.h"
#include	"defs.h"
#include "ctl.h"


/* local defines */

#define	O_TERM		(O_WRONLY | O_NOCTTY | O_NDELAY)



/* external subroutines */

extern CTL_MSG *find_request(CTL_MSG *request);
extern CTL_MSG *find_match(CTL_MSG *request);

extern int delete_invite(int id_num);
extern int announce(struct proginfo *,CTL_MSG *request, char *remote_machine);
extern int new_id(void);

extern void insert_table(CTL_MSG *request, CTL_RESPONSE *response);


/* forward references */

static int find_user(char *, char *) ;

static void do_announce(struct proginfo *,
		CTL_MSG *request, CTL_RESPONSE *response);






void process_request(pip,request, response)
struct proginfo	*pip ;
CTL_MSG		*request ;
CTL_RESPONSE	*response ;
{
    CTL_MSG *ptr;


    response->type = request->type;
    response->id_num = 0;

    switch (request->type) {

	case ANNOUNCE:
	    do_announce(pip,request, response);

	    break;

	case LEAVE_INVITE:
	    ptr = find_request(request);
	    if (ptr != (CTL_MSG *) 0) {
		response->id_num = ptr->id_num;
		response->answer = SUCCESS;
	    } else {
		insert_table(request, response);
	    }
	    break;

	case LOOK_UP:
	    ptr = find_match(request);
	    if (ptr != (CTL_MSG *) 0) {
		response->id_num = ptr->id_num;
		response->addr = ptr->addr;
		response->answer = SUCCESS;
	    } else {
		response->answer = NOT_HERE;
	    }
	    break;

	case DELETE:
	    response->answer = delete_invite(request->id_num);
	    break;

	default:
	    response->answer = UNKNOWN_REQUEST;
	    break;

    } /* end switch */

}
/* end subroutine (process_request) */



/* LOCAL SUBROUTINES */



static void do_announce(pip,request,response)
struct proginfo	*pip ;
CTL_MSG		*request ;
CTL_RESPONSE	*response ;
{
    struct hostent *hp;

    CTL_MSG *ptr;

    int result;


#if	CF_DEBUGS
	debugprintf("do_announce: entered\n") ;
#endif

	/* see if the user is logged */

    result = find_user(request->r_name, request->r_tty) ;

#if	CF_DEBUGS
	debugprintf("do_announce: find_user() result=%d\n",result) ;
#endif

    if (result != SUCCESS) {

#if	CF_DEBUGS
	debugprintf("do_announce: find_user() not-SUCCESS\n") ;
#endif

	response->answer = result;
	return;
    }

    hp = gethostbyaddr((const char *)&request->ctl_addr.sin_addr,
			  sizeof(struct in_addr), AF_INET);

    if ( hp == (struct hostent *) 0 ) {
	response->answer = MACHINE_UNKNOWN;
	return;
    }

    ptr = find_request(request);

    if (ptr == (CTL_MSG *) 0) {

#if	CF_DEBUGS
	debugprintf("do_announce: find_request() ZERO\n") ;
#endif

	insert_table(request,response);

	response->answer = announce(pip,request, hp->h_name);

    } else if (request->id_num > ptr->id_num) {

#if	CF_DEBUGS
	debugprintf("do_announce: find_request() request ID larger\n") ;
#endif

	    /*
	     * this is an explicit re-announce, so update the id_num
	     * field to avoid duplicates and re-announce the talk 
	     */
	ptr->id_num = response->id_num = new_id();

	response->answer = announce(pip,request, hp->h_name);

    } else {

#if	CF_DEBUGS
	debugprintf("do_announce: find_request() ELSE\n") ;
#endif

	    /* a duplicated request, so ignore it */
	response->id_num = ptr->id_num;
	response->answer = SUCCESS;

    }

    return;
}
/* end subroutine (do_announce) */


/* * Search utmp for the local user */
static int find_user(char *name, char *tty)
{
    struct utmpx *ubuf;

    int tfd;
	int	f ;
	int	f_ok = FALSE ;

    char dev[100];


    setutxent();		/* reset the utmpx file */

    while ((ubuf = getutxent()) != NULL) {

 	if ((ubuf->ut_type == USER_PROCESS) &&
  	    (strncmp(ubuf->ut_user, name, sizeof(ubuf->ut_user)) == 0)) {

	    /* check if this entry is really a tty */
	    strcpy(dev, "/dev/");

	    strncat(dev, ubuf->ut_line, sizeof(ubuf->ut_line));

	    if ((tfd = open(dev, O_TERM)) == -1) {
		continue;
	    }

	    f = isatty(tfd) ;

		close(tfd) ;

	    if (! f) {

		openlog("talk", 0, LOG_AUTH);
		syslog(LOG_CRIT, "%.*s in UTMP is not a terminal\n",
			sizeof(ubuf->ut_line), ubuf->ut_line);
		closelog();
		continue;
	    }

	    if (*tty == '\0') {
		    /* no particular tty was requested */

		(void) strcpy(tty, ubuf->ut_line);

		f_ok = TRUE ;

    		endutxent();		/* close the utmpx file */
		return(SUCCESS);

	    } else if (strncmp(ubuf->ut_line, tty,TMPX_LLINE) == 0) {

		f_ok = TRUE ;

	    }
	}

    } /* end while */

    endutxent();		/* close the utmpx file */

    return (f_ok) ? SUCCESS : NOT_HERE ;
}
/* end subroutine (find_user) */



