/* io */


#define	F_TIMEOUT	(5 * 60)


/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)io.c	1.7	94/10/04 SMI"	/* SVr4.0 1.2	*/

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
 *		All rights reserved.
 *
 */


/*
 * this file contains the I/O handling and the exchange of
 * edit characters. This connection itself is established in ctl.c
 */

#include <sys/time.h>
#include <sys/filio.h>

#include <stdio.h>
#include <errno.h>
#include <libintl.h>

#include "talk.h"



#if	defined(F_TIMEOUT) && F_TIMEOUT
#define	A_LONG_TIME	F_TIMEOUT	/* 'select(3c)' wait timeout */
#else
#define	A_LONG_TIME	10000000	/* 'select(3c)' wait timeout */
#endif

#define	STDIN_MASK (1<<fileno(stdin))	/* the bit mask for standard input */

extern int	sys_nerr;


/*
 * The routine to do the actual talking
 */

void talk()
{
	struct timeval wait;

	time_t	starttime, daytime ;

	int read_template, sockt_mask;
	int	error_set ;
	int read_set, nb;

	char buf[BUFSIZ];


	message(gettext("Connection established"));
	beep(); beep(); beep();
	current_line = 0;

	sockt_mask = (1<<sockt);

	/*
	 * wait on both the other process (sockt_mask) and
	 * standard input ( STDIN_MASK )
	 */

	read_template = sockt_mask | STDIN_MASK;

	forever {

		starttime = time(NULL) ;

		read_set = read_template ;
		error_set = read_template ;

		wait.tv_sec = A_LONG_TIME;
		wait.tv_usec = 0;

		nb = select(32, (fd_set *)&read_set, 0, 
			(fd_set *) &error_set, &wait);

		if (nb <= 0) {

		if (nb < 0) {

			/* We may be returning from an interrupt handler */

			if (errno == EINTR) {
				read_set = read_template;
				continue;
			} else {
				/* panic, we don't know what happened */
				p_error(
				gettext("Unexpected error from select"));
				quit();
			}

		} else {

			daytime = time(NULL) ;

			if ((daytime - starttime) < A_LONG_TIME)
				continue ;

			message("timed out") ;

			quit() ;
		}

		}

		if (read_set & sockt_mask) {

			/* There is data on sockt */
			nb = read(sockt, buf, sizeof (buf));

			if (nb <= 0) {
				message(gettext("Connection closed. Exiting"));

#ifdef	COMMENT
				pause();	/* wait for Ctrl-C */
#endif

				quit();

			} else {
				display(&rem_win, buf, nb);
			}
		}

		if (read_set & STDIN_MASK) {

		/*
		 * we can't make the tty non_blocking, because
		 * curses's output routines would screw up
		 */

			ioctl(0, FIONREAD, (struct sgttyb *) &nb);

			nb = read(0, buf, nb);

			display(&my_win, buf, nb);

			write(sockt, buf, nb);

		/* We might lose data here because sockt is non-blocking */

		}

		if (error_set & sockt_mask) {

				message(gettext("Connection closed. Exiting"));

				quit();

		}

	} /* end forever */

}
/* end subroutine (talk) */


/*
* p_error prints the system error message on the standard location
* on the screen and then exits. (i.e. a curses version of perror)
*/

void
p_error(string)
char *string;
{
	char *sys;


	if (errno < sys_nerr) {
		sys = strerror(errno);
	} else {
		sys = gettext("Unknown error");
	}

	wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
	wprintw(my_win.x_win, "[%s : %s (%d)]\n", string, sys, errno);
	wrefresh(my_win.x_win);
	move(LINES-1, 0);
	refresh();
	quit();
}


/* display string in the standard location */

void
message(string)
char *string;
{
	wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
	wprintw(my_win.x_win, "[%s]\n", string);
	wrefresh(my_win.x_win);
}



