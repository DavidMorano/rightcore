/* print */


/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)print.c	1.3	96/05/03 SMI"	/* SVr4.0 1.1	*/

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

/* debug print routines */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "ctl.h"



/* extern subroutines */


/* external variables */

extern int	debug ;






void
print_request(CTL_MSG *request)
{


    printf("type is %d, l_user %s, r_user %s, r_tty %s\n",
	    request->type, request->l_name, request->r_name,
	    request->r_tty);
    printf("        id = %d\n", request->id_num);
    fflush(stdout);
}


void
print_response(CTL_RESPONSE *response)
{


    printf("type is %d, answer is %d, id = %d\n\n", response->type, 
	    response->answer, response->id_num);
    fflush(stdout);
}


void
print_error(char *string)
{
    FILE *cons;
    char *err_dev = "/dev/console";
    char *sys;
    pid_t val, pid;


    if (debug) err_dev = "/dev/tty";

    if ((sys = strerror(errno)) == (char *) NULL)
	    sys = "Unknown error";

	/* don't ever open tty's directly, let a child do it */
    if ((pid = fork()) == 0) {
	cons = fopen(err_dev, "a");
	if (cons != NULL) {
	    fprintf(cons, "Talkd : %s : %s(%d)\n\r", string, sys,
		    errno);
	    fclose(cons);
	}
	exit(0);
    } else {
	    /* wait for the child process to return */
	do {
	    val = wait(0);
	    if (val == (pid_t)-1) {
		if (errno == EINTR) {
		    continue;
		} else if (errno == ECHILD) {
		    break;
		}
	    }
	} while (val != pid);
    }
}
/* end subroutine (print_error) */



