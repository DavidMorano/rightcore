/* main (pt_chmod) */


/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pt_chmod.c	1.8	97/01/22 SMI"	/* SVr4.0 1.1	*/



#define	DEFAULT_TTY_GROUP	"tty"


#include	<stdlib.h>
#include <grp.h>


/* external subroutines */




/*
 * change the owner and mode of the pseudo terminal slave device.
 */
int main(argc, argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct	group	*gr_name_ptr;

	gid_t	gid;

	int	fd;

	char	*cp ;


	if (argc < 2) {
		exit(-1);
	}

	if ((gr_name_ptr = getgrnam(DEFAULT_TTY_GROUP)) != NULL)
		gid = gr_name_ptr->gr_gid;
	else
		gid = getgid();

	fd = atoi(argv[1]);

	cp = ptsname(fd);

	if (cp == NULL)
		exit(-1) ;

	if (chown(cp, getuid(), gid))
		exit(-1);

	if (chmod(cp, 00620))
		exit(-1);

	exit(0);
}
/* end subroutine (main) */



