#ifndef lint
static char *sccsid = "@(#)$Id: inews.c,v 1.6 1992/08/03 04:51:45 sob Exp sob $";
#endif

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

#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "nntp.h"
#include "nntpclnt.h"
#ifdef GETPWENT
#include <pwd.h>
#ifndef __STDC__
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();
#endif
#endif
#ifdef USG
#include <string.h>
#else /* not USG */
#include <strings.h>
#endif /* not USG */
/* for gen_frompath() */
#define FROM 1
#define PATH 2

#include <sys/utsname.h>
#ifndef SYS_NMLN
#define SYS_NMLN 257
#endif

#define	MAX_SIGNATURE	4

extern	FILE	*ser_wr_fp;
extern	char	*getlogin();

#if defined(AUTHSIMPLE) || defined(AUTHOLD)
FILE *passfile;
#endif /* AUTHSIMPLE || AUTHOLD */

char	host_name[SYS_NMLN];
char 	*username;
struct utsname t;
#ifdef GETPWENT
struct	passwd *passwd;
#else
char passwd[BUFSIZ];
char *homedir;
char *fullname;
#endif

main(argc, argv)
int	argc;
char	*argv[];
{
	char	line[NNTP_STRLEN], s[NNTP_STRLEN];
	int	seen_fromline, in_header, seen_header, seen_pathline;
	int	response;
	char	*server;
	char	*getserverbyfile();
	register char	*cp;
#ifndef GETPWENT
	int i;
#endif
	/* find out who they REALLY are */
	username = getlogin();
	if (username == NULL)
#ifdef GETPWENT
		{
			passwd = getpwuid(getuid());
			username = passwd->pw_name;
		}
	else
		passwd = getpwnam(username);
	if (passwd == NULL){
		fprintf(stderr,"Can't get user information. Please contact your system adminstrator.\n");
		exit(1);
	}
#else
		{
			int i;
			if (getpw(getuid(), passwd) != 0){
				fprintf(stderr,"Can't get user information. Please contact your system adminstrator.\n");
				exit(1);
			}
			cp = index(passwd,':');
			*cp = '\0';
			username = passwd;
		} 
	else
		{
		    FILE *pfp = fopen("/etc/passwd","r");
		    int foundit = 0;
			if (pfp == NULL)
			{
				fprintf(stderr,"Can't open password file. Please contact your system adminstrator.\n");
				exit(1);
			}
		    while (fgets(passwd,sizeof passwd,pfp) != NULL) {
			if (strncmp(username,passwd,strlen(username)))
				continue;
			if (passwd[(strlen(username)-1)] != ':')
				continue;
			foundit = 1;
			break;
			}
		    fclose(pfp);
		    if (foundit == 0) {
			fprintf(stderr,"%s appears to be an invalid username.\n", username);
			exit(1);
		    }
		    cp = index(passwd,':');
		    *cp = '\0';
		}
	cp++;
	for (fullname=cp, i=4; i; i--) { /* magic cookie XXX */
		if (fullname)
   				fullname = index(s,':')+1;
	}
	cp = index(fullname,':');
	*cp = '\0';
	cp++;
	homedir = cp;
	cp = index(homedir,':');
	*cp = '\0';
#endif


#if defined(AUTHSIMPLE) || defined(AUTHOLD)
	/*
	 * we have to be setuid to news to open the file, but not afterwards
	 */
	passfile = fopen(PASSFILE, "r");
#ifdef SETEUID
	seteuid(getuid());
	setegid(getgid());
#else
	setuid(getuid());
	setgid(getgid());
#endif
#endif /* AUTHSIMPLE || AUTHOLD */

	++argv;
	while (argc > 1)
		if (*argv[0] == '-') {
			++argv;
			--argc;
		} else
			break;

	if (argc > 1) {
		if (frdebugopen(*argv, "r", stdin) == NULL) {
			perror(*argv);
			exit(1);
		}
	}

#ifdef DOUNAME
	uname(&t);
	t.nodename[sizeof(t.nodename) - 1] = 0;
	strcpy(host_name, t.nodename);
#else
	uname(host_name);
#endif

	server = getserverbyfile(SERVER_FILE);
	if (server == NULL) {
		fprintf(stderr,
			"Can't get the name of the news server from %s.\n",
			SERVER_FILE);
		fprintf(stderr,
	       "Either fix this file, or put NNTPSERVER in your enviroment.\n");
		exit(1);
	}

	response = server_init(server);
	if (response < 0) {
		printf("Couldn't connect to %s news server, try again later.\n",
			server);
		exit(1);
	}

	if (handle_server_response(response, server) < 0
	    || response == OK_NOPOST) {
		close_server();
		exit(1);
	}

again:
	put_server("POST");
	(void) get_server(line, sizeof(line));
	if (*line != CHAR_CONT) {
		if (atoi(line) == ERR_NOPOST) {
			close_server();
			fprintf(stderr,
				"Sorry, you can't post from this machine.\n");
			exit(1);
		}
#ifdef AUTH
		if ((atoi(line) == ERR_AUTHSYS)
#ifdef AUTHOLD
			||(atoi(line) == ERR_OLDNOAUTH)
#endif
		) {
			if (postauth(server,line) < 0){
				close_server();
				fprintf(stderr,"Authorization failed. Posting aborted.\n");
				exit(1);
			}
			goto again;
		}
#endif
		close_server();
		fprintf(stderr, "Remote error: %s\n", line);
		exit(1);
	}

	in_header = 1;
	seen_header = 0;
	seen_fromline = 0;
	seen_pathline = 0;


	while (fgets(s, sizeof(s), stdin) != NULL) {
		/* Strip trailing newline */
		cp = s + strlen(s) - 1;
		if (cp >= s && *cp == '\n')
			*cp = '\0';
		if (s[0] == '.')    /* Single . is eof, so put in extra one */
			(void) fputc('.', ser_wr_fp);
		if (in_header && !strncasecmp(s, "From:", sizeof("From:")-1)) {
	                seen_header = 1;
			seen_fromline = 1;
		}
		if (in_header && !strncasecmp(s, "Path:", sizeof("Path:")-1)) {
	                seen_header = 1;
			seen_pathline = 1;
		}
		if (in_header && s[0] == '\0') {
	                if (seen_header) {
		                in_header = 0;
				if (!seen_pathline)
					gen_frompath(PATH);
				if (!seen_fromline)
					gen_frompath(FROM);
				else
					fprintf(ser_wr_fp, "Originator: %s@%s\r\n", username, host_name);
			} else {
			        continue;
			}
		} else if (in_header) {
		        if (valid_header(s))
			        seen_header = 1;
	                else
                                continue;
		}
		fprintf(ser_wr_fp, "%s\r\n", s);
	}
	if (in_header) {
		/* Still in header after EOF?  Hmm... */
		in_header = 0;
		if (!seen_pathline)
			gen_frompath(PATH);
		if (!seen_fromline)
			gen_frompath(FROM);
		else
			fprintf(ser_wr_fp, "Originator: %s@%s\r\n", username, host_name);
		fprintf(ser_wr_fp, "\r\n");
	}
  
	append_signature();

	fprintf(ser_wr_fp, ".\r\n");
	(void) fflush(ser_wr_fp);
	(void) get_server(line, sizeof(line));
	if (*line != CHAR_OK) {
		if (atoi(line) == ERR_POSTFAIL) {
			char c = '\n';
			close_server();
			printf("Article not accepted by server; not posted.\n");
			for (cp = line + 4; *cp && *cp != '\r'; cp++) {
				c  =  *cp=='\\' ? '\n' : *cp;
				putchar(c);
			}
			if (c != '\n')
				putchar('\n');
			exit(1);
		} else {
			close_server();
			fprintf(stderr, "Remote error: %s\n", line);
			exit(1);
		}
	}

	/*
	 * Close server sends the server a
	 * "quit" command for us, which is why we don't send it.
	 */

	close_server();

	exit(0);
}

/*
 * append_signature -- append the person's .signature file if
 * they have one.  Limit .signature to MAX_SIGNATURE lines.
 * The rn-style DOTDIR environmental variable is used if present.
 */

append_signature()
{
	char	line[256], sigfile[256];
	char	*cp;
	FILE	*fp;
	char	*index(), *getenv();
	int	count = 0;
	char	*dotdir;
	
	if (passwd == NULL)
	  return;
#ifdef DO_DOTDIR
	if ((dotdir = getenv("DOTDIR")) == NULL)
#endif
	{
#ifdef GETPWENT
	  dotdir = passwd->pw_dir;
#else
	  dotdir = homedir;
#endif
	}

	if (dotdir[0] == '~') {
#ifdef GETPWENT
	  (void) strcpy(sigfile, passwd->pw_dir);
#else
	  (void) strcpy(sigfile, homedir);
#endif
	  (void) strcat(sigfile, &dotdir[1]);
	} else {
	  (void) strcpy(sigfile, dotdir);
	}
	(void) strcat(sigfile, "/");
	(void) strcat(sigfile, ".signature");

#ifdef DEBUG
  fprintf(stderr,"sigfile = '%s'\n", sigfile);
#endif

	fp = fopen(sigfile, "r");
	if (fp == NULL)
		return;

#ifdef DEBUG
  fprintf(stderr,"sigfile opened OK\n");
#endif

	fprintf(ser_wr_fp, "-- \r\n");
	while (fgets(line, sizeof (line), fp)) {
		count++;
		if (count > MAX_SIGNATURE) {
			fprintf(stderr,
	      "Warning: .signature files should be no longer than %d lines.\n",
			MAX_SIGNATURE);
			fprintf(stderr,
			"(Only %d lines of your .signature were posted.)\n",
			MAX_SIGNATURE);
			break;
		}
		/* Strip trailing newline */
		cp = line + strlen(line) - 1;
		if (cp >= line && *cp == '\n')
			*cp = '\0';

		fprintf(ser_wr_fp, "%s\r\n", line);
	}
	(void) fclose(fp);
#ifdef DEBUG
	printf(".signature appended (from %s)\n", sigfile);
#endif
}


/*
 * gen_frompath -- generate From: and Path: lines, in the form
 *
 *	From: user@host.domain (full_name)
 *	Path: host!user
 *
 * This routine should only be called if the message doesn't have
 * a From: line in it.
 */

gen_frompath(which)
int which;
{
	char	*s;
	char	*cp;
	char	*index(), *getenv();
	
	s = getenv("NAME");
	if (s == NULL) {
#ifdef GETPWENT
		s = passwd->pw_gecos;
#else
		s = fullname;
#endif
#ifdef BERKNAMES
    if ((cp = index(s, ',')) != NULL)
	*cp = '\0';
    if ((cp = index(s, ';')) != NULL)
	*cp = '\0';
#else
#ifdef USGNAMES
    if ((cp = index(s, '(')) != NULL)
	*cp = '\0';
    if ((cp = index(s, '-')) != NULL)
	s = cp;
#else
    {
	FILE * tmpfp;
	char namefile[BUFSIZ];
	char buf[BUFSIZ];
#ifdef GETPWENT
	sprintf(namefile,"%s/.fullname",passwd->pw_dir);
#else
	sprintf(namefile,"%s/.fullname",homedir);
#endif
	if ((tmpfp=fopen(namefile,"r")) != NULL) {
		fgets(buf,sizeof buf,tmpfp);
		fclose(tmpfp);
		buf[strlen(buf)-1] = '\0';
		s = buf;
    		} 
	else
		s = "";
	}
#endif
#endif
	}

	if (which == FROM){
#if defined(DOMAIN)
		/* A heuristic to see if we should tack on a domain */

		cp = index(host_name, '.');
		if (cp)
			fprintf(ser_wr_fp, "From: %s@%s (",
				username,
				host_name);
		else
#if defined(HIDDENNET)
			fprintf(ser_wr_fp, "From: %s@%s (",
				username,
				DOMAIN);
#else
			fprintf(ser_wr_fp, "From: %s@%s.%s (",
				username,
				host_name,
				DOMAIN);
#endif
#else
		fprintf(ser_wr_fp, "From: %s@%s (", username, host_name);
#endif /* DOMAIN */
		for (cp = s; *cp != '\0'; ++cp)
			if (*cp != '&')
				putc(*cp, ser_wr_fp);
			else {		/* Stupid & hack.  God damn it. */
				putc(toupper(username[0]), ser_wr_fp);
				fprintf(ser_wr_fp, username+1);
			}
		fprintf(ser_wr_fp, ")\r\n");
	}
	if (which == PATH){
	/* Only the login name - nntp server will add uucp name */
	/* Folks aren't supposed to the Paths for replies, so we don't */
	/* care if it generates something inaccurate for replies! */
	/*	fprintf(ser_wr_fp, "Path: %s\r\n", username); */
		fprintf(ser_wr_fp, "Path: not-for-mail\r\n");
	}
}


/*
 * valid_header -- determine if a line is a valid header line
 * 
 *	Parameters:	"h" is the header line to be checked.
 *
 *	Returns: 	1 if valid, 0 otherwise
 *
 *	Side Effects:	none
 *
 */

int valid_header(h)
register char *h;
{
  char *index();
  char *colon, *space;
  
  /*
   * blank or tab in first position implies this is a continuation header
   */
  if (h[0] == ' ' || h[0] == '\t')
    return (1);

  /*
   * just check for initial letter, colon, and space to make
   * sure we discard only invalid headers
   */
  colon = index(h, ':');
  space = index(h, ' ');
  if (isalpha(h[0]) && colon && space == colon + 1)
    return (1);

  /*
   * anything else is a bad header -- it should be ignored
   */
  return (0);
}
#ifndef STRNCASECMP
/*
 * lower -- convert a character to lower case, if it's
 *	upper case.
 *
 *	Parameters:	"c" is the character to be
 *			converted.
 *
 *	Returns:	"c" if the character is not
 *			upper case, otherwise the lower
 *			case eqivalent of "c".
 *
 *	Side effects:	None.
 */

char lower(c)
register char c;
{
	if (isascii(c) && isupper(c))
		c = c - 'A' + 'a';
	return(c);
}


/*
 * strncasecmp -- determine if two strings are equal in the first n
 * characters, ignoring case.
 *
 *	Parameters:	"a" and "b" are the pointers
 *			to characters to be compared.
 *			"n" is the number of characters to compare.
 *
 *	Returns:	0 if the strings are equal, 1 otherwise.
 *
 *	Side effects:	None.
 */

strncasecmp(a, b, n)
register char *a, *b;
int	n;
{
	char	lower();

	while (n && lower(*a) == lower(*b)) {
		if (*a == '\0')
			return (0);
		a++;
		b++;
		n--;
	}
	if (n)
		return (1);
	else
		return (0);
}
#endif
