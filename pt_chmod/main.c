/* main (pt_chmod) */


/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pt_chmod.c	1.8	97/01/22 SMI"	/* SVr4.0 1.1	*/

/* updated (enhanced):

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


/* * change the owner and mode of the pseudo terminal slave device.  */


#include	<stdlib.h>
#include	<grp.h>


/* local defines */

#define	DEFAULT_TTY_GROUP	"tty"


/* external subroutines */


/* exported subroutines */


int main(int argc,const char **argv,const char **envv)
{
	int		fd;
	int		ex = -1 ;
	char		*cp ;

	if (argc >= 2) {
	    struct group	*gr_name_ptr ;
	    gid_t		gid ;

	    if ((gr_name_ptr = getgrnam(DEFAULT_TTY_GROUP)) != NULL) {
		gid = gr_name_ptr->gr_gid;
	    } else {
		gid = getgid();
	    }

	    fd = atoi(argv[1]);

	    if ((cp = ptsname(fd)) != NULL) {
	        const uid_t	uid = getuid() ;
	        if (chown(cp,uid,gid) == 0) {
	            if (chmod(cp, 00620) == 0) {
	                ex = 0 ;
	            }
	        }
	    }

	} /* end if (good arguments) */

	exit(ex) ;
}
/* end subroutine (main) */


