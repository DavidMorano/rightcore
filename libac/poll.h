/* poll */

/* poll header stuff */


/* revistion history:

	= 2015-04-12, David A­D­ Morano
	We needed this stuff so that user's had something to include
	in order to use the 'u_poll(3dam)' subroutine!


*/

/* Copyright (c) 2015 David A.D. Morano. All rights reserved. */

/******************************************************************************

	This file has the stuff that is needed to implement a user-level
	knockoff of the 'poll(2)' system call.	We will be implemeting
	this call with 'select(3c)' but the user is also expecting to
	include this file to get the structure and the defines so we
	needed to do something here to provide that stuff.


******************************************************************************/


#ifndef POLL_INCLUDE
#define	POLL_INCLUDE	1


#if	(! defined(SYSHAS_POLL)) || (SYSHAS_POLL == 0)


#ifdef	__cplusplus
extern "C" {
#endif

typedef struct pollfd {
	int		fd;		/* file desc to poll */
	short		events;		/* events of interest on fd */
	short		revents;	/* events that occurred on fd */
} pollfd_t;

typedef unsigned long	nfds_t;

/* * Testable select events */

#define	POLLIN		0x0001		/* fd is readable */
#define	POLLPRI		0x0002		/* high priority info at fd */
#define	POLLOUT		0x0004		/* fd is writeable (won't block) */
#define	POLLRDNORM	0x0040		/* normal data is readable */
#define	POLLWRNORM	POLLOUT
#define	POLLRDBAND	0x0080		/* out-of-band data is readable */
#define	POLLWRBAND	0x0100		/* out-of-band data is writeable */

#define	POLLNORM	POLLRDNORM

/*
 * Non-testable poll events (may not be specified in events field,
 * but may be returned in revents field).
 */
#define	POLLERR		0x0008		/* fd has error condition */
#define	POLLHUP		0x0010		/* fd has been hung up on */
#define	POLLNVAL	0x0020		/* invalid pollfd entry */

#define	POLLREMOVE	0x0800	/* remove a cached poll fd from /dev/poll */



#ifdef	__cplusplus
}
#endif

extern int	poll(pollfd_t *,nfds_t,int) ;

#endif /* (! defined(SYSHAS_POLL)) || (SYSHAS_POLL == 0) */


#endif	/* POLL_INCLUDE */


