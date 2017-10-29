/* main */

/* (C) Copyright 1993,1994 by Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Carnegie
 * Mellon University not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  Carnegie Mellon University makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */


#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "common.h"
#include "version.h"
#include "xmalloc.h"



/* local defines */

#define MAXADDRESS 100



/* external subroutines */


/* external variables */

extern int optind;

extern char *optarg;


/* forward references */

void	usage() ;





int main(argc, argv)
int	argc ;
char	*argv[] ;
{
    FILE *infile;
    FILE *descfile = 0;

    long maxsize = 0;

    int part;
	int	es = 1 ;
    int opt;
    int i;

    char *fname = 0;
    char *subject = 0;
    char *descfname = 0;
    char *outfname = 0;
    char *newsgroups = 0;
    char *ctype = 0;
    char *headers = 0;
    char *p;
    char sbuf[1024];
    char fnamebuf[MAXPATHLEN + 1];
	char	*cp ;


    if ((p = getenv("SPLITSIZE")) && *p >= '0' && *p <= '9')
	maxsize = atoi(p);


    while ((opt = getopt(argc, argv, "s:d:m:c:o:n:")) != EOF) {

	switch (opt) {

	case 's':
	    subject = optarg;
	    break;

	case 'd':
	    descfname = optarg;
	    break;

	case 'm':
	    maxsize = atoi(optarg);
	    break;

	case 'c':
	    ctype = optarg;
	    break;

	case 'o':
	    outfname = optarg;
	    break;

	case 'n':
	    newsgroups = optarg;
	    break;

	default:
	    usage();

		es = 1 ;
		goto early ;
	}

    }


    if (ctype) {
	if (!cistrncmp(ctype, "text/", 5)) {
	    fprintf(stderr, 
		"This program is not appropriate for encoding textual data\n");
	    exit(1);
	}

    }

    if (optind == argc) {
	fprintf(stderr, "An input file must be specified\n");
	usage();

	es = 1 ;
	goto early ;
    }

    fname = argv[optind++];

    /* Must have exactly one of -o, -n, or destination addrs */
    if (optind == argc) {

	if (outfname && newsgroups) {
	    fprintf(stderr, 
	"The -o and -n switches are mutually exclusive.\n");

	    usage();

		es = 1 ;
		goto early ;
	}

	if (!outfname && !newsgroups) {
	    fprintf(stderr, 
"Either an address or one of the -o or -n switches is required\n");

	    usage();

		es = 1 ;
		goto early ;
	}

	if (newsgroups) {
	    headers = xmalloc(strlen(newsgroups) + 25);
	    sprintf(headers, "Newsgroups: %s\n", newsgroups);
	}

    } else {

	if (outfname) {

	    fprintf(stderr, 
		"The -o switch and addresses are mutually exclusive.\n");

	    usage();

		es = 1 ;
		goto early ;
	}
	if (newsgroups) {

	    fprintf(stderr, 
		"The -n switch and addresses are mutually exclusive.\n");

	    usage();

		es = 1 ;
		goto early ;
	}

	headers = xmalloc(strlen(argv[optind]) + 25);

	sprintf(headers, "To: %s", argv[optind]);

	for (i = optind+1; i < argc; i++) {

	    headers = xrealloc(headers, strlen(headers)+strlen(argv[i]) + 25);

	    strcat(headers, ",\n\t");
	    strcat(headers, argv[i]);

	}
	strcat(headers, "\n");

    } /* end if */

    if (!subject) {

	fputs("Subject: ", stdout);
	fflush(stdout);
	if (!fgets(sbuf, sizeof(sbuf), stdin)) {

	    fprintf(stderr, "A subject is required\n");

	    usage();

		es = 1 ;
		goto early ;
	}

	if (p = strchr(sbuf, '\n')) 
		*p = '\0';

	subject = sbuf;
    }	

    if (!outfname) {

	if (cp = getenv("TMPDIR"))
	    strcpy(fnamebuf, cp) ;

	else
		strcpy(fnamebuf, "/tmp");

	strcat(fnamebuf, "/mpackXXXXXX");
	mktemp(fnamebuf);
	outfname = strsave(fnamebuf);
    }

    infile = fopen(fname, "r");
    if (!infile) {
	os_perror(fname);

		es = 1 ;
		goto early ;
    }

    if (descfname) {
	descfile = fopen(descfname, "r");
	if (!descfile) {

	    os_perror(descfname);

		es = 1 ;
		goto early ;
	}
    }

    if (encode(infile, (FILE *)0, fname, descfile, subject, headers,
	       maxsize, ctype, outfname)) {

		es = 1 ;
		goto early ;
	}

    if (optind < argc || newsgroups) {

	for (part = 0;;part++) {

	    sprintf(fnamebuf, "%s.%02d", outfname, part);
	    infile = fopen(part ? fnamebuf : outfname, "r");
	    if (!infile) {

		if (part) break;
		continue;
	    }
	    if (newsgroups) {
		inews(infile, newsgroups);
	    }
	    else {
		sendmail(infile, argv, optind);
	    }
	    fclose(infile);
	    remove(part ? fnamebuf : outfname);

	}
    }

	es = 0 ;

early:
	fclose(stdout) ;

	fclose(stderr) ;

	return es ;
}
/* end subroutine (main) */


void usage()
{
    fprintf(stderr, "mpack version %s\n", MPACK_VERSION);
    fprintf(stderr, 
"usage: mpack [-s subj] [-d file] [-m maxsize] [-c content-type] file address...\n");
    fprintf(stderr, 
"       mpack [-s subj] [-d file] [-m maxsize] [-c content-type] -o file file\n");
    fprintf(stderr, 
"       mpack [-s subj] [-d file] [-m maxsize] [-c content-type] -n groups file\n");

}

sendmail(infile, addr, start)
FILE *infile;
char **addr;
int start;
{
    int status;
    int pid;

    if (start < 2) abort();

#ifdef SCO
    addr[--start] = "execmail";
#else
    addr[--start] = "-oi";
    addr[--start] = "sendmail";
#endif

    do {
	pid = fork();
    } while (pid == -1 && errno == EAGAIN);
    
    if (pid == -1) {
	perror("fork");
	return;
    }
    if (pid != 0) {
	while (pid != wait(&status));
	return;
    }

    dup2(fileno(infile), 0);
    fclose(infile);
#ifdef SCO
    execv("/usr/lib/mail/execmail", addr+start);
#else
    execv("/usr/lib/sendmail", addr+start);
    execv("/usr/sbin/sendmail", addr+start);
#endif
    perror("execv");

	exit(1) ;
}

inews(infile)
FILE *infile;
{
    int status;
    int pid;

    do {
	pid = fork();
    } while (pid == -1 && errno == EAGAIN);
    
    if (pid == -1) {
	perror("fork");
	return;
    }
    if (pid != 0) {
	while (pid != wait(&status));
	return;
    }

    dup2(fileno(infile), 0);
    fclose(infile);
    execlp("inews", "inews", "-h", "-S", (char *)0);
    execl("/usr/local/news/inews", "inews", "-h", "-S", (char *)0);
    execl("/usr/local/lib/news/inews", "inews", "-h", "-S", (char *)0);
    execl("/etc/inews", "inews", "-h", "-S", (char *)0);
    execl("/usr/etc/inews", "inews", "-h", "-S", (char *)0);
    execl("/usr/news/inews", "inews", "-h", "-S", (char *)0);
    execl("/usr/news/bin/inews", "inews", "-h", "-S", (char *)0);
    perror("execl");

	exit(1) ;

}

warn()
{
    abort();
}




