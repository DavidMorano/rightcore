/* This software is Copyright 1992 by Stan Barber. 
 *
 * Permission is hereby granted to copy, reproduce, redistribute or otherwise
 * use this software as long as: there is no monetary profit gained
 * specifically from the use or reproduction of this software, it is not
 * sold, rented, traded or otherwise marketed, and this copyright notice is
 * included prominently in any copy made. 
 *
 * The author make no claims as to the fitness or correctness of this software
 * for any use whatsoever, and it is provided as is. Any use of this software
 * is at the user's own risk. 
 */


static char	*rcsId = "@(#)$Id: uname.c,v 1.6 1992/08/03 04:55:23 sob RELEASE sob $";

#include <stdio.h>
#include "config.h"

#ifdef USG
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef DOUNAME
# define DONE
#endif /* DOUNAME */

#ifdef GHNAME
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

uname(uptr)
char	*uptr;
{
	struct hostent *he;

	gethostname(uptr, 256);

	he = gethostbyname(uptr);
	strncpy(uptr, he->h_name, 255);
	uptr[255] = '\0';
}
# define DONE
#endif
#ifdef PHOSTNAME
uname(uptr)
char *uptr;
{
	FILE *popen();
	FILE *uucpf;
	register char *p;
	if (((uucpf = popen(PHOSTNAME, "r")) == NULL ||
		fgets(uptr, 256, uucpf) == NULL) {
			fprintf(stderr, "%s command failed. Contact your systems administrator.\n", PHOSTNAME);
			pclose(uucpf);
			exit(1);
	}
	p = index(uptr, '\n');
	if (p)
		*p = '\0';
	if (uucpf != NULL)
		pclose(uucpf);
}
#define DONE
#endif /* PHOSTNAME */

#ifdef HOSTFILE
#ifndef	GHNAME
uname(uptr)
char *uptr;
{
    FILE * uucpf;
    char * hostname;
    register char *p;

    if ((uucpf = fopen(HOSTFILE,"r")) == NULL) {
	fprintf(stderr,"couldn't open \"%s\" to determine hostname\n", 
		HOSTFILE); 
	exit(1);
    } else {
	fgets(uptr, 256, uucpf);
    }
	p = index(uptr, '\n');
	if (p)
		*p = '\0';
	if (uucpf != NULL)
		fclose(uucpf);
}
#define DONE
#endif
#endif
#ifdef WHOAMI
#define	HDRFILE "/usr/include/whoami.h"

uname(uptr)
char *uptr;
{
	char buf[BUFSIZ];
	FILE *fd;
	
	fd = fopen(HDRFILE, "r");
	if (fd == NULL) {
		fprintf(stderr, "Cannot open %s to determine hostname. Contact your system administrator.\n", HDRFILE);
		exit(1);
	}
	
	for (;;) {	/* each line in the file */
		if (fgets(buf, sizeof buf, fd) == NULL) {
			fprintf(stderr, "%s is corrupted. Please contact your system administrator\n", HDRFILE);
			fclose(fd);
			exit(1);
		}
		if (sscanf(buf, "#define sysname \"%[^\"]\"", uptr) == 1) {
			fclose(fd);
			return;
		}
	}
}
#define DONE
#endif
#ifndef DONE
#ifdef SITENAME
uname(uptr)
char* uptr;
{
	strcpy(uptr,SITENAME);
	return;
}
#endif
#endif
