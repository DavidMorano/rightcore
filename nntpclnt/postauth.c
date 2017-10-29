/* 
 * postauth: do authorization handshake when posting requires it.
 * This version compatible with the proposed NNTP Version 2 SIMPLE
 * authentication scheme.
 *
 * This software is Copyright 1992 by Stan Barber. 
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
#ifndef lint
static char * rcsid = "$Id: postauth.c,v 1.6 1992/08/03 04:51:45 sob Exp sob $";
#endif
#include <stdio.h>
#include "config.h"
#include "nntp.h"

#ifdef AUTH

int
postauth(host,response)
char *host;
char *response;
{
	char authtype[NNTP_STRLEN];
	int i, rcode;
	if((i = sscanf(response,"%d %s", rcode, authtype)) != 2)
		return(-1);
#ifdef AUTHSIMPLE
	if (!strcasecmp(authtype,"SIMPLE"))
		return(authsimple(host));
#endif
#ifdef AUTHOLD
	return (authold(host));
#else
	return(-1);
#endif
}
#ifdef AUTHSIMPLE
/*
 * a sample implementation of the SIMPLE authentication scheme in NNTP V2 as
 * proposed.
 */
extern FILE *passfile;

int
authsimple(host)
char * host;
{
	char remote[256], user[16], pass[16];
	char buf[BUFSIZ];
	int i;

	if (passfile == NULL)
	{
		fprintf(stderr,"Posting is not allowed from this system.\n");
		return(-1);
	}

	while(fgets(buf, sizeof(buf), passfile))
	{
		if (buf[0] == '#')
			continue;
		
		i = sscanf(buf,"%s %s %s", remote, user, pass);
		/* malformed entry? */
		if (i != 3)
		{
			fprintf(stderr,"Posting Authorization Denied. File format error.\n");
			continue;
		}
		
		/* right host? */
		if (!strcasecmp(remote,host))
			break;
	}
	if (feof(passfile))
	{
		fprintf(stderr,"Posting to %s is not allowed from this system\n", host);
		return(-1);
	}
	
	sprintf(buf,"authinfo simple");
	if (converse(buf, sizeof(buf)) != CONT_AUTH)
	{
		fprintf(stderr,"%s\n", buf);
		return(-1);
	}
	
	sprintf(buf,"%s %s", user,pass);
	if (converse(buf, sizeof(buf)) != OK_AUTH)
	{
		fprintf(stderr,"%s\n", buf);
		return(-1);
	}
	
	return(0);
}

#endif /* AUTHSIMPLE */
#ifdef AUTHOLD
extern FILE *passfile;
authold(host)
char *host;
{
	char remote[64], user[16], pass[16];
	char buf[1024];
	int i;

	if (passfile == NULL)
	{
		fprintf(stderr,"Posting is not allowed from this system.\n");
		return(-1);
	}

	while(fgets(buf, sizeof(buf), passfile))
	{
		if (buf[0] == '#')
			continue;
		i = sscanf(buf,"%s %s %s", remote, user, pass);
		/* malformed entry? */
		if (i != 3)
		{
			fprintf(stderr,"Posting Authorization Denied. File format error.\n");
			continue;
		}
		/* right host? */
		if (!strcasecmp(remote,host))
			break;
	}
	if (feof(passfile))
	{
		fprintf(stderr,"Posting to %s is not allowed from this system\n", host);
		return(-1);
	}
	
	sprintf(buf,"authinfo user %s", user);
	if (converse(buf, sizeof(buf)) != CONT_OLDAUTHD)
	{
		fprintf(stderr,"%s\n", buf);
		return(-1);
	}

	sprintf(buf,"authinfo pass %s", pass);
	if (converse(buf, sizeof(buf)) != OK_OLDAUTH)
	{
		fprintf(stderr,"%s\n", buf);
		return(-1);
	}
	fclose(passfile);
}
#endif
int
converse(buf,buflen)
char *buf;
int buflen;
	{
	put_server(buf);
	get_server(buf,buflen);
	return(atoi(buf));
	}

#ifndef STRCASECMP
strcasecmp(s1, s2)
	char *s1, *s2;
{

	while (toupper(*s1) == toupper(*s2++))
		if (*s1++ == '\0')
			return(0);
	return(toupper(*s1) - toupper(*--s2));
}
#endif
#endif /* AUTH */
