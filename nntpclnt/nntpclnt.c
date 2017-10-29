#ifndef lint
static char     *rcsid = "@(#)$Id: nntpclnt.c,v 1.6 1992/08/03 04:54:23 sob Exp sob $";
#endif

/*
 * This software is Copyright 1992 by Stan Barber. 
 *
 * Permission is hereby granted to copy, reproduce, redistribute or otherwise
 * use this software as long as: there is no monetary profit gained
 * specifically from the use or reproduction or this software, it is not
 * sold, rented, traded or otherwise marketed, and this copyright notice is
 * included prominently in any copy made. 
 *
 * The author make no claims as to the fitness or correctness of this software
 * for any use whatsoever, and it is provided as is. Any use of this software
 * is at the user's own risk. 
 *
 */
/*
 * NNTP client routines.
 */

/*
 * Include configuration parameters only if we're made in the nntp tree.
 */

#ifdef MSDOS
#include <stdlib.h>
#include <io.h>
#endif
#include <stdio.h>
#ifdef PCTCP
#include <pctcp/types.h>
#include <pctcp/pctcp.h>
#include <pctcp/ipconfig.h>
#include <pctcp/sockets.h>
#include <pctcp/options.h>
#include <pctcp/error.h>
#ifdef SOCK_TCP_NNTP
#undef SOCK_TCP_NNTP            /* defined wrong in the development kit! */
#endif
#define SOCK_TCP_NNTP 119
#endif
#ifdef PCNFS
#include <tklib.h>
#endif
#include <sys/types.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef PCNFS
#include <in_addr.h>
#endif
#ifdef LWP                      /* Lan Workplace for DOS from Novell */
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <sys/sockio.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/uio.h>
#define net_read(a,b,c,d,e)        soread(a,b,c)
#define net_write(a,b,c,d)	   sowrite(a,b,c)
#endif

#include	<time.h>

#if defined(USG) || defined(MSDOS)
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef NONETDB
# define        IPPORT_NNTP     ((unsigned short) 119)
#else
# include       <netdb.h>       /* All TLI implementations may not have this */
#endif

#if defined(USG) || defined(MSDOS)
#ifndef index
# define        index   strchr
#endif
#ifndef bcopy
# define        bcopy(a,b,c)   memcpy(b,a,c)
#endif
#ifndef bzero
# define        bzero(a,b)     memset(a,'\0',b)
#endif
#endif /* USG or MSDOS */

#ifdef DATAKIT
#include <dk.h>
#include <errno.h>
#include <memory.h>
#define min(A, B)       ((A) < (B)? (A): (B))
#endif /* DATAKIT */

#ifdef EXCELAN
#if __STDC__
int connect(int, struct sockaddr *);
unsigned short htons(unsigned short);
unsigned long rhost(char **);
int rresvport( int );
int socket( int, struct sockproto *, struct sockaddr_in *, int );
#endif
#endif

#ifdef SGI4DDN
#include <fcntl.h>
#include <dn/defs.h>
#endif
#ifdef DECNET
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif /* DECNET */

/* * AT&T NETNEWS SERVICE ADDITIONS */
#include <pwd.h>


#include	<bfile.h>


#ifdef NNTPSRC
#include "config.h"
#endif /* NNTPSRC */

#include "nntpclnt.h"


/* * AT&T NETNEWS SERVICE ADDITIONS */
#include "md5.h"



extern struct passwd *getpwuid();


#define	PASS_GOOD_TIME	(12*60*60)	/* length of time the .nntpauth file */
					/* is valid. After this, the person  */
					/* will be prompted to re-enter the  */
					/* NNTP password. Set at 12 hours.   */
char	*getpass();
/*
 * END OF AT&T NETNEWS SERVICE ADDITIONS
 */


#ifdef __STDC__
unsigned long inet_addr(char *x);
int get_tcp_socket(char *machine);
#else
unsigned long inet_addr();
#endif

#include "nntp.h"


#if defined(PCTCP) || defined(LWP)
int     nd = -1;                /* network descriptor */
#else
FILE    *ser_rd_fp = NULL;
FILE    *ser_wr_fp = NULL;
#endif


#define	BUFLEN		1024



	char    *index();


/*
 * getserverbyfile      Get the name of a server from a named file.
 *                      Handle white space and comments.
 *                      Use NNTPSERVER environment variable if set.
 *
 *      Parameters:     "file" is the name of the file to read.
 *
 *      Returns:        Pointer to static data area containing the
 *                      first non-ws/comment line in the file.
 *                      NULL on error (or lack of entry in file).
 *
 *      Side effects:   None.
 */

char *
getserverbyfile(file)
char    *file;
{
	register FILE   *fp;
	register char   *cp;
	static char     buf[256];
	char            *index();
	char            *getenv();
	char            *strcpy();

	if (cp = getenv("NNTPSERVER")) {
		(void) strcpy(buf, cp);
		return (buf);
	}

	if (file == NULL)
		return (NULL);

	fp = fopen(file, "r");
	if (fp == NULL)
		return (NULL);

	while (fgets(buf, sizeof (buf), fp) != NULL) {
		if (*buf == '\n' || *buf == '#')
			continue;
		cp = index(buf, '\n');
		if (cp)
			*cp = '\0';
		(void) fclose(fp);
		return (buf);
	}

	(void) fclose(fp);
	return (NULL);                   /* No entry */
}


/*
 * server_init  Get a connection to the remote news server.
 *
 *      Parameters:     "machine" is the machine to connect to.
 *
 *      Returns:        -1 on error
 *                      server's initial response code on success.
 *
 *      Side effects:   Connects to server.
 *                      "ser_rd_fp" and "ser_wr_fp" are fp's
 *                      for reading and writing to server.
 */

server_init(machine)
char    *machine;
{
	bfile	passfile, *pfp = &passfile ;

	struct passwd	client ;

	int     sockt_rd, sockt_wr;
	int	rs ;

	char	buf[BUFLEN + 1] ;
	char    line[256];
	char	*username = "dam" ;

/*
 * AT&T NETNEWS SERVICE ADDITIONS
 */
	int	init_signon;
	char	authstring[80];
	struct 	passwd	*client_pass;
	FILE	*authfp;
	char	authfile[256];
	char	authtemp[80];
	long	curr_time;
	long	base_time;
	long	time_diff = 0L;
	char	*nntppass;
	char	e_nntppass[80];
	int	i;
	MD5_CTX	mdContext;
/*
 * END OF AT&T NETNEWS SERVICE ADDITIONS
 */

#if defined(DECNET) || defined(SGI4DDN)
	char    *cp;

	cp = index(machine, ':');

	if (cp && cp[1] == ':') {
		*cp = '\0';
		sockt_rd = get_dnet_socket(machine);
	} else
		sockt_rd = get_tcp_socket(machine);
#else
# ifdef DATAKIT
	sockt_rd = get_dk_socket(machine);
# else
	sockt_rd = get_tcp_socket(machine);
# endif
#endif
	if (sockt_rd < 0)
		return (-1);

#if defined(PCTCP) || defined(LWP)
	sockt_wr = nd = sockt_rd;          /* save the network descriptor */
#else
	sockt_wr = dup(sockt_rd);
	/*
	 * Now we'll make file pointers (i.e., buffered I/O) out of
	 * the socket file descriptor.  Note that we can't just
	 * open a fp for reading and writing -- we have to open
	 * up two separate fp's, one for reading, one for writing.
	 */

	if ((ser_rd_fp = fdopen(sockt_rd, "r")) == NULL) {
		perror("server_init: fdopen #1");
		return (-1);
	}

	if ((ser_wr_fp = fdopen(sockt_wr, "w")) == NULL) {
		perror("server_init: fdopen #2");
		ser_rd_fp = NULL;               /* from above */
		return (-1);
	}

#endif
	/* Now get the server's signon message */

	(void) get_server(line, sizeof(line));
/*
 * AT&T NETNEWS SERVICE ADDITIONS
 *
 * provide authorization information to the server before continuing
 * with the rest of the NNTP conversation.
 */
	init_signon=atoi(line);

	if ( init_signon == OK_CANPOST || init_signon == OK_NOPOST ) {

	    /* 
	     * get information from $HOME/.nntpauth
	     */

#ifdef	COMMENT
	    client_pass = getpwuid(getuid());
#else
#ifdef	SYSV
		client_pass = getpwnam_r(username,&client,buf,BUFLEN) ;
#else
		client_pass = getpwnam(username) ;
#endif
#endif

	    /*
	     * probably should put some error checking here
	     */
	    sprintf(authfile,"%s/.nntpauth", client_pass->pw_dir);

	    if ( (authfp = fopen(authfile,"r")) != NULL) {

		fgets(authtemp, sizeof(authtemp), authfp);     
		base_time = atol(authtemp);
		fgets(e_nntppass, sizeof(e_nntppass), authfp);
		curr_time = time((long*)0);
		time_diff = (curr_time - base_time);
	    }
	    /*
	     * if (file does not exist) or (information in file is too old)
	     *    prompt user to input NNTP password
	     *    store base_time and encrypted NNTP password in .nntpauth
	     * fi
	     */
	    if ( (authfp == NULL) || (time_diff > PASS_GOOD_TIME) ) {

		fclose(authfp);

#ifdef	COMMENT
		if (authfp == NULL) {

		    printf("No authorization file found. Please re-enter\n");
		    printf("your NNTP password if you have one, or hit\n");
		    printf("the <return> key to receive instructions on\n");
		    printf("how to become authorized to read netnews.\n\n");

		} else {

		    printf("Your NNTP authorization has expired.\n");
		    printf("Please re-enter your NNTP password.\n\n");
		}

		nntppass = getpass("NNTP Password: ");
#else
		nntppass = "" ;
		if (bopen(pfp,"/home/dam/.nntppasswd","r",0666) >= 0) {

			if ((rs = breadline(pfp,buf,8)) >= 0) {

				if (buf[rs - 1] == '\n') rs -= 1 ;

					buf[rs - 1] = '\0' ;
					nntppass = buf ;

			}

			bclose(pfp) ;

		} /* end if (getting NNTP password from file) */
#endif

		if ((nntppass != NULL) && *nntppass) {

		    base_time = time((long*)0);
		    authfp = fopen(authfile,"w");
		    if (!authfp)
		    {
			printf("Cannot create %s: \n", authfile);
			return(-1);
		    }
		    fprintf(authfp, "%ld\n", base_time);
/*
 * MD5 encryption stuff done here
 */
		    e_nntppass[0] = '\0';
		    MD5Init(&mdContext);
		    MD5Update(&mdContext, nntppass, strlen(nntppass));
		    MD5Final(&mdContext);
		    for (i = 0; i < 16; i++) {

		        sprintf(authtemp,"%02x", mdContext.digest[i]);
		        strcat(e_nntppass,authtemp);
		    }
/*
 * END MD5 encryption stuff
 */
		    fprintf(authfp, "%s", e_nntppass);
		    fclose(authfp);
		    chmod(authfile,0400|0200);

		} else {

		    e_nntppass[0] = 'x';
		    e_nntppass[1] = '\0';
		}
	    }
	    /*
	     * send to server a string consisting of the following:
	     *   1. the keyword "authinfo"
	     *   2. the keyword "md5" to indicate the type of authorization
	     *	    being done
	     *   3. the person's userid
	     *   5. the person's encrypted NNTP password
	     */
#ifdef	COMMENT
	    sprintf(authstring,"authinfo md5 %s %s", 
		client_pass->pw_name,
		e_nntppass);
#else
	    sprintf(authstring,"authinfo md5 %s %s", 
		username,
		e_nntppass);
#endif

	    put_server(authstring);
	    (void) get_server(line, sizeof(line));
	}
/*
 * END OF AT&T NETNEWS SERVICE ADDITIONS
 */
	return (atoi(line));
}

#ifdef DATAKIT
/*
 * get_dk_socket -- get a socket (actually a file descriptor) connected
 *                  to the netnews server machine.
 *
 *       Parameters:    "machine" is the dial string to reach the service.
 *
 *       Returns:       file descriptor for the service, or a "-1" in
 *                      case of failure.
 *
 *       Side effects:  connects to service.
 */
 
get_dk_socket(machine)
char *machine;
{
	static short block[3] = {DKR_BLOCK};
	char *maphost(), *dialstring, *cuserid();
	int dkfd;
 
	if ( (dialstring = maphost(machine, 'n', "nntp", "", getlogin())) == (char *)NULL )
		return(-1);

	if ( (dkfd = dkdial(dialstring)) >= 0 )
		ioctl(dkfd, DIOCRMODE, block);
	else
		dkfd = -1;

	return(dkfd);
}
 
#else /* !DATAKIT */

/*
 * get_tcp_socket -- get us a socket connected to the news server.
 *
 *      Parameters:     "machine" is the machine the server is running on.
 *
 *      Returns:        Socket connected to the news server if
 *                      all is ok, else -1 on error.
 *
 *      Side effects:   Connects to server.
 *
 *      Errors:         Printed via perror.
 */

int
get_tcp_socket(machine)
char    *machine;       /* remote host */
{
	int     s;
#ifdef PCTCP
	in_name fhost;
	struct  addr a;
	fhost = nm_res_name(machine,(char *) NULL,0);
	if (fhost == 0){
		pneterror(machine);
		return(-1);
	}
	/*
	 * setup address structure 
	 */
	 a.lsocket = 0;
	 a.fsocket = SOCK_TCP_NNTP;
	 a.fhost = fhost;
	 /* make connection */ 
	 s = net_connect(-1,STREAM,&a);
	 if (s == -1) {
		pneterror("net_connect");
		return(-1);
	}
#else
	struct  sockaddr_in sin;
#ifdef NONETDB
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
#else
	struct  servent *getservbyname(), *sp, h_sp;
	struct  hostent *gethostbyname(), *hp, h_hp;
#ifdef h_addr
	int     x = 0;
	register char **cp;
	static char *alist[1];
#endif /* h_addr */
	unsigned long inet_addr();
	static struct hostent def;
	static struct in_addr defaddr;
	static char namebuf[ 256 ];

	if ((sp = getservbyname("nntp", "tcp")) ==  NULL) {
		fprintf(stderr, "nntp/tcp: Unknown service.\n");
		return (-1);
	}
	h_sp = *sp;	/* struct assign */
	sp = &h_sp;
	/* If not a raw ip address, try nameserver */
	if (!isdigit(*machine) ||
	    (long)(defaddr.s_addr = inet_addr(machine)) == -1)
	{
		hp = gethostbyname(machine);
		h_hp = *hp;	/* struct assign */
		hp = &h_hp;
	} else {
		/* Raw ip address, fake  */
		(void) strcpy(namebuf, machine);
		def.h_name = namebuf;
#ifdef h_addr
		def.h_addr_list = alist;
#endif
		def.h_addr = (char *)&defaddr;
		def.h_length = sizeof(struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	if (hp == NULL) {
		fprintf(stderr, "%s: Unknown host.\n", machine);
		return (-1);
	}
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = sp->s_port;
#endif /* NONETDB */

	/*
	 * The following is kinda gross.  The name server under 4.3
	 * returns a list of addresses, each of which should be tried
	 * in turn if the previous one fails.  However, 4.2 hostent
	 * structure doesn't have this list of addresses.
	 * Under 4.3, h_addr is a #define to h_addr_list[0].
	 * We use this to figure out whether to include the NS specific
	 * code...
	 */

#ifdef  h_addr

	/* get a socket and initiate connection -- use multiple addresses */

	for (cp = hp->h_addr_list; cp && *cp; cp++) {
		s = socket(hp->h_addrtype, SOCK_STREAM, 0);
		if (s < 0) {
			perror("socket");
			return (-1);
		}
		bcopy(*cp, (char *)&sin.sin_addr, hp->h_length);
		
		if (x < 0)
			fprintf(stderr, "trying %s\n", inet_ntoa(sin.sin_addr));
		x = connect(s, (struct sockaddr *)&sin, sizeof (sin));
		if (x == 0)
			break;
		fprintf(stderr, "connection to %s: ", inet_ntoa(sin.sin_addr));
		perror("");
		(void) close(s);
	}
	if (x < 0) {
		fprintf(stderr, "giving up...\n");
		return (-1);
	}
#else   /* no name server */
#ifdef EXCELAN
	if ((s = socket(SOCK_STREAM,(struct sockproto *)NULL,&sin,SO_KEEPALIVE)) < 0)
	{
		/* Get the socket */
		perror("socket");
		return (-1);
	}
	bzero((char *) &sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(IPPORT_NNTP);
	/* set up addr for the connect */

	if ((sin.sin_addr.s_addr = rhost(&machine)) == -1) {
		fprintf(stderr, "%s: Unknown host.\n", machine);
		return (-1);
	}
	/* And then connect */

	if (connect(s, (struct sockaddr *)&sin) < 0) {
		perror("connect");
		(void) close(s);
		return (-1);
	}
#else /* not EXCELAN */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	/* And then connect */

	bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
	if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		perror("connect");
		(void) close(s);
		return (-1);
	}

#endif /* !EXCELAN */
#endif /* !h_addr */
#endif /* !PCTCP */
	return (s);
}

#endif /* !DATAKIT */

#ifdef DECNET
/*
 * get_dnet_socket -- get us a socket connected to the news server.
 *
 *      Parameters:     "machine" is the machine the server is running on.
 *
 *      Returns:        Socket connected to the news server if
 *                      all is ok, else -1 on error.
 *
 *      Side effects:   Connects to server.
 *
 *      Errors:         Printed via nerror.
 */

get_dnet_socket(machine)
char    *machine;
{
	int     s, area, node;
	struct  sockaddr_dn sdn;
	struct  nodeent *getnodebyname(), *np;

	bzero((char *) &sdn, sizeof(sdn));

	switch (s = sscanf( machine, "%d%*[.]%d", &area, &node )) {
		case 1: 
			node = area;
			area = 0;
		case 2: 
			node += area*1024;
			sdn.sdn_add.a_len = 2;
			sdn.sdn_family = AF_DECnet;
			sdn.sdn_add.a_addr[0] = node % 256;
			sdn.sdn_add.a_addr[1] = node / 256;
			break;
		default:
			if ((np = getnodebyname(machine)) == NULL) {
				fprintf(stderr, 
					"%s: Unknown host.\n", machine);
				return (-1);
			} else {
				bcopy(np->n_addr, 
					(char *) sdn.sdn_add.a_addr, 
					np->n_length);
				sdn.sdn_add.a_len = np->n_length;
				sdn.sdn_family = np->n_addrtype;
			}
			break;
	}
	sdn.sdn_objnum = 0;
	sdn.sdn_flags = 0;
	sdn.sdn_objnamel = strlen("NNTP");
	bcopy("NNTP", &sdn.sdn_objname[0], sdn.sdn_objnamel);

	if ((s = socket(AF_DECnet, SOCK_STREAM, 0)) < 0) {
		nerror("socket");
		return (-1);
	}

	/* And then connect */

	if (connect(s, (struct sockaddr *) &sdn, sizeof(sdn)) < 0) {
		nerror("connect");
		close(s);
		return (-1);
	}

	return (s);
}
#endif
#ifdef SGI4DDN
/*
 * get_dnet_socket -- get us a socket connected to the news server.
 * (Silicon Graphics Version)
 *      Parameters:     "machine" is the machine the server is running on.
 *
 *      Returns:        link ("socket") connected to the news server if
 *                      all is ok, else -1 on error.
 *
 *      Side effects:   Connects to server.
 *
 *      Errors:         Printed via dn_perror.
 */

get_dnet_socket(machine)
char    *machine;
{
	int     s, area, node;
	OpenBlock sdn;

	bzero((char *) &sdn, sizeof(sdn));
	strcpy(sdn.op_node_name,machine);
	strcpy(sdn.op_task_name,"NNTP");

	if ((s = open(DN_LINK,O_RDWR)) < 0) {
		dn_perror("Open Fail: ");
		return (-1);
	}

	if (ioctl(s, SES_LINK_ACCESS, &sdn) < 0) {
		dn_perror("connect");
		close(s);
		return (-1);
	}

	return (s);
}
#endif



/*
 * handle_server_response
 *
 *      Print some informative messages based on the server's initial
 *      response code.  This is here so inews, rn, etc. can share
 *      the code.
 *
 *      Parameters:     "response" is the response code which the
 *                      server sent us, presumably from "server_init",
 *                      above.
 *                      "nntpserver" is the news server we got the
 *                      response code from.
 *
 *      Returns:        -1 if the error is fatal (and we should exit).
 *                      0 otherwise.
 *
 *      Side effects:   None.
 */

handle_server_response(response, nntpserver)
int     response;
char    *nntpserver;
{
    switch (response) {
	case OK_NOPOST:         /* fall through */
		printf(
	"NOTE: This machine does not have permission to post articles.\n");
		printf(
	"      Please don't waste your time trying.\n\n");

	case OK_CANPOST:
		return (0);
		break;
/*
 * CHANGES FOR THE AT&T NETNEWS SERVICE
 */
	case CONT_OLDAUTHI:
/*
 * CONT_OLDAUTHI indicates that the user has NNTPSERVER pointed to an 
 * old NNTP server
 */
printf("\nThe environmental variable NNTPSERVER is set incorrectly to use this\n");
printf("news reading program. Please enter the command \"unset NNTPSERVER\".\n"); 
printf("The default NNTP server (as specified by your local system administrator\n");
printf("when this program was installed) will then be used.\n\n");
		sleep(5);
		return (-1);
		break;

	case ERR_ACCESS:
/*
 * ERR_ACCESS indicates that the authorization file(s) on the NNTP server do 
 * not contain the necessary authorization information for this person.
 */
printf("\nYou are not authorized to use %s as an NNTP server.\n\n",nntpserver);
printf("To become authorized for NNTP service, you need to type the\n");
printf("following command:\n\n");
#ifdef DATAKIT
printf("\tdk %s.nntpauth\n\n",nntpserver);
#else
printf("\ttelnet %s 1119\n\n",nntpserver);
#endif
printf("You'll then be stepped through a program that will gather the necessary\n");
printf("information to establish your authorization for NNTP service.\n\n");
printf("If you are still having problems after doing the above ");
#ifdef DATAKIT
printf("dk command,\n");
#else
printf("telnet command,\n");
#endif
printf("please call the Computing Technology Center Helpdesk at 708-979-3333.\n\n");
		sleep(5);
		return (-1);
		break;

	case ERR_AUTHREJ:
/*
 * ERR_AUTHREJ indicates that this person has the necessary authorization
 * information on the NNTP server, but the information supplied by the
 * client did not sync with the information contained on the server.
 */
printf("\nThe NNTP password provided is not the same as the one stored on\n");
printf("the NNTP server for you. Please remove the file '.nntpauth' in\n");
printf("your home directory, and try again.\n\n");
printf("If you still can not access the NNTP server after trying this,\n");
printf("please contact the Computing Technology Center Helpdesk\n");
printf("at 708-979-3333.\n\n");
		sleep(5);
		return (-1);
		break;
	case ERR_AUTHSYS:
/*
 * ERR_AUTHSYS indicates that there is something vitally wrong with the
 * authorization system on the NNTP server.
 */
printf("There is a problem with the authorization data files\n");
printf("on the %s NNTP server.\n\n",nntpserver);
printf("Please call the Computing Technology Center Helpdesk to\n");
printf("inform them of the problem. The number is 708-979-3333.\n\n");
		sleep(5);
		return (-1);
		break;
	case ERR_AUTHEXP:
/*
 * ERR_AUTHEXP indicates that the authorization information on the NNTP server
 * has expired. Instructions need to be given so this information can be
 * made current.
 */
printf("Your authorization information on the NNTP server %s\n",nntpserver);
printf("has expired.\n\n");
printf("To rejuvenate your authorization information,  you need enter the command:\n\n");
#ifdef DATAKIT
printf("\tdk %s.nntpauth\n\n",nntpserver);
#else
printf("\ttelnet %s 1119\n\n",nntpserver);
#endif
printf("on the machine you are using as your NNTP client.\n\n");
printf("You'll be stepped through a program that will gather the necessary\n");
printf("information to rejuvenate your authorization for NNTP service.\n\n");
printf("If you still have problems after going through this rejuvenation\n"); 
printf("process, please call the Computing Technology Center Helpdesk\n"); 
printf("at 708-979-3333.\n\n");
		sleep(5);
/*
 * END OF CHANGES FOR THE AT&T NETNEWS SERVICE
 */
		return (-1);
		break;

	default:
		printf("Unexpected response code from %s news server: %d\n",
			nntpserver, response);
		return (-1);
		break;
    }
	/*NOTREACHED*/
}


/*
 * put_server -- send a line of text to the server, terminating it
 * with CR and LF, as per ARPA standard.
 *
 *      Parameters:     "string" is the string to be sent to the
 *                      server.
 *
 *      Returns:        Nothing.
 *
 *      Side effects:   Talks to the server.
 *
 *      Note:           This routine flushes the buffer each time
 *                      it is called.  For large transmissions
 *                      (i.e., posting news) don't use it.  Instead,
 *                      do the fprintf's yourself, and then a final
 *                      fflush.
 */

void
put_server(string)
char *string;
{
#ifdef DEBUG
	fprintf(stderr, ">>> %s\n", string);
#endif
#if defined(PCTCP) || defined(LWP)
	net_write(nd,string,strlen(string),0);
	net_write(nd,"\r\n",2,0);
/*      net_flush(nd); */
#else
	fprintf(ser_wr_fp, "%s\r\n", string);
	(void) fflush(ser_wr_fp);
#endif
	return;
}

#ifdef DATAKIT
/*
 * dkfgets - DATAKIT substitute for fgets
 *
 * The basic problem is that fgets is incompatible with alarm().  When
 * the file stream used by fgets() (or any stdio) function is a "slow"
 * device, i.e. not disk, signals generated by the alarm clock will
 * interrupt a read and cause data to be thrown away.  This means that
 * get_server will return an error.  The return code is not checked by
 * the callers of get_server and the ensuing ARTICLE loop ends up leaving
 * the server flow-controlled and both sides hung.  In these additions
 * we use this local routine which disables the alarm before doing the read, 
 * and restore them after the read.
 */
char *
dkfgets(string, size, fp)
char    *string;
int     size;
FILE    *fp;
{
	static struct dk_io {
		char *ptr;
		char *buffr;
		int avail;
	} io_p = { 0, 0, 0};
	register struct dk_io *io = &io_p;
	register char *cp;
	register to_move;
	char *save_string = string;
 
	if (!io->buffr) {
		/* get initial buffer */
		io->buffr = (char *) malloc(BUFSIZ);
		if (io->buffr == NULL)
			return(NULL);
	}
	while (size) {
		if (io->avail == 0) {
			unsigned alarm_time;
			/* try to fill buffer */
			alarm_time = (unsigned) alarm(0);
			if ((io->avail = read(fileno(fp), io->buffr, BUFSIZ)) <
0) {
				fprintf(stderr, "Read Failure on NNTP\n");
				return(NULL);
			}
			if (alarm_time)
				(void) alarm(alarm_time);
			io->ptr = io->buffr;
		}
		cp = memchr(io->ptr, '\n', min(size, io->avail));
		if (cp) {
			/* this buffer contains a new line */
			to_move = cp - io->ptr + 1;
			size = 0;
		}
		else {
			to_move = min(size, io->avail);
			size -= to_move;
		}
		(void) memcpy(string, io->ptr, to_move);
 
		io->avail -= to_move;
		io->ptr += to_move;
		string += to_move;
	}
	return(save_string);
}
#endif /* DATAKIT */

/*
 * get_server -- get a line of text from the server.  Strips
 * CR's and LF's.
 *
 *      Parameters:     "string" has the buffer space for the
 *                      line received.
 *                      "size" is the size of the buffer.
 *
 *      Returns:        -1 on error, 0 otherwise.
 *
 *      Side effects:   Talks to server, changes contents of "string".
 */

get_server(string, size)
char    *string;
int     size;
{
	register char *cp;
	char *index();

#if defined(PCTCP) || defined(LWP)
	int i = 0;
	bzero(string,size);
	cp = string;
	while(i < size){
		if (net_read(nd,cp,1,(struct addr *) NULL,0) < 0){
			if (i > 0)
				break;
			else 
				return(-1);
		}
		if (*cp == '\r' || *cp == '\n'){
			*cp = '\0';
			if (i>0)
				break;
			else
				continue;
		}
		cp++;
		i++;
	}
#else
# ifdef DATAKIT
	if (dkfgets(string, size, ser_rd_fp) == NULL)
# else /* !DATAKIT */
	if (fgets(string, size, ser_rd_fp) == NULL)
# endif /* !DATAKIT */
		return (-1);
	if ((cp = index(string, '\r')) != NULL)
		*cp = '\0';
	else if ((cp = index(string, '\n')) != NULL)
		*cp = '\0';
#endif
#ifdef DEBUG
/*	fprintf(stderr, "<<< %s\n", string); */
#endif

	return (0);
}


/*
 * close_server -- close the connection to the server, after sending
 *              the "quit" command.
 *
 *      Parameters:     None.
 *
 *      Returns:        Nothing.
 *
 *      Side effects:   Closes the connection with the server.
 *                      You can't use "put_server" or "get_server"
 *                      after this routine is called.
 */

void
close_server()
{
	char    ser_line[256];

#if !defined(PCTCP) && !defined(LWP)
	if (ser_wr_fp == NULL || ser_rd_fp == NULL)
		return;
#endif

	put_server("QUIT");
#ifndef DATAKIT
	(void) get_server(ser_line, sizeof(ser_line));
#endif
#ifdef LWP
	(void) soclose(nd);
#else
#ifdef PCTCP
	(void) net_eof(nd);
	(void) net_releaseall();
#else
#ifndef EXCELAN
	shutdown(fileno(ser_rd_fp),2);
#endif
	(void) fclose(ser_rd_fp);
	(void) fclose(ser_wr_fp);
#endif
#endif
	return;
}

#ifdef NONETDB
/*
 * inet_addr for EXCELAN (which does not have it!)
 *
 */
unsigned long
inet_addr(cp)
register char   *cp;
{
	unsigned long val, base, n;
	register char c;
	unsigned long octet[4], *octetptr = octet;
#ifndef htonl
	extern  unsigned long   htonl();
#endif  /* htonl */
again:
	/*
	 * Collect number up to ``.''.
	 * Values are specified as for C:
	 * 0x=hex, 0=octal, other=decimal.
	 */
	val = 0; base = 10;
	if (*cp == '0')
		base = 8, cp++;
	if (*cp == 'x' || *cp == 'X')
		base = 16, cp++;
	while (c = *cp) {
		if (isdigit(c)) {
			val = (val * base) + (c - '0');
			cp++;
			continue;
		}
		if (base == 16 && isxdigit(c)) {
			val = (val << 4) + (c + 10 - (islower(c) ? 'a' : 'A'));
			cp++;
			continue;
		}
		break;
	}
	if (*cp == '.') {
		/*
		 * Internet format:
		 *      a.b.c.d
		 *      a.b.c   (with c treated as 16-bits)
		 *      a.b     (with b treated as 24 bits)
		 */
		if (octetptr >= octet + 4)
			return (-1);
		*octetptr++ = val, cp++;
		goto again;
	}
	/*
	 * Check for trailing characters.
	 */
	if (*cp && !isspace(*cp))
		return (-1);
	*octetptr++ = val;
	/*
	 * Concoct the address according to
	 * the number of octet specified.
	 */
	n = octetptr - octet;
	switch (n) {

	case 1:                         /* a -- 32 bits */
		val = octet[0];
		break;

	case 2:                         /* a.b -- 8.24 bits */
		val = (octet[0] << 24) | (octet[1] & 0xffffff);
		break;

	case 3:                         /* a.b.c -- 8.8.16 bits */
		val = (octet[0] << 24) | ((octet[1] & 0xff) << 16) |
			(octet[2] & 0xffff);
		break;

	case 4:                         /* a.b.c.d -- 8.8.8.8 bits */
		val = (octet[0] << 24) | ((octet[1] & 0xff) << 16) |
		      ((octet[2] & 0xff) << 8) | (octet[3] & 0xff);
		break;

	default:
		return (-1);
	}
	val = htonl(val);
	return (val);
}
#endif /* NONETDB */


/*
 ***********************************************************************
 ** md5.c -- the source code for MD5 routines                         **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 1/91 SRD,AJ,BSK,JT Reference C Version                   **
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **                                                                   **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.                                        **
 **                                                                   **
 ** License is also granted to make and use derivative works          **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.              **
 **                                                                   **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **                                                                   **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.                                    **
 ***********************************************************************
 */

/* #include "md5.h" */

/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContext using MD5Init        **
 **    (2) Call MD5Update on mdContext and M                          **
 **    (3) Call MD5Final on mdContext                                 **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */

/* forward declaration */
static void Transform ();

static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }

/* The routine MD5Init initializes the message-digest context
   mdContext. All fields are set to zero.
 */
void MD5Init (mdContext)
MD5_CTX *mdContext;
{
  mdContext->i[0] = mdContext->i[1] = (UINT4)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (UINT4)0x67452301;
  mdContext->buf[1] = (UINT4)0xefcdab89;
  mdContext->buf[2] = (UINT4)0x98badcfe;
  mdContext->buf[3] = (UINT4)0x10325476;
}

/* The routine MD5Update updates the message-digest context to
   account for the presence of each of the characters inBuf[0..inLen-1]
   in the message whose digest is being computed.
 */
void MD5Update (mdContext, inBuf, inLen)
MD5_CTX *mdContext;
unsigned char *inBuf;
unsigned int inLen;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((UINT4)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((UINT4)inLen << 3);
  mdContext->i[1] += ((UINT4)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
                (((UINT4)mdContext->in[ii+2]) << 16) |
                (((UINT4)mdContext->in[ii+1]) << 8) |
                ((UINT4)mdContext->in[ii]);
      Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

/* The routine MD5Final terminates the message-digest computation and
   ends with the desired message digest in mdContext->digest[0...15].
 */
void MD5Final (mdContext)
MD5_CTX *mdContext;
{
  UINT4 in[16];
  int mdi;
  unsigned int i, ii;
  unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* pad out to 56 mod 64 */
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
  MD5Update (mdContext, PADDING, padLen);

  /* append length in bits and transform */
  for (i = 0, ii = 0; i < 14; i++, ii += 4)
    in[i] = (((UINT4)mdContext->in[ii+3]) << 24) |
            (((UINT4)mdContext->in[ii+2]) << 16) |
            (((UINT4)mdContext->in[ii+1]) << 8) |
            ((UINT4)mdContext->in[ii]);
  Transform (mdContext->buf, in);

  /* store buffer in digest */
  for (i = 0, ii = 0; i < 4; i++, ii += 4) {
    mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
    mdContext->digest[ii+1] =
      (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
    mdContext->digest[ii+2] =
      (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
    mdContext->digest[ii+3] =
      (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
  }
}

/* Basic MD5 step. Transforms buf based on in.
 */
static void Transform (buf, in)
UINT4 *buf;
UINT4 *in;
{
  UINT4 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226); /* 61 */
  II ( d, a, b, c, in[11], S42, 3173756917); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}

/*
 ***********************************************************************
 ** End of md5.c                                                      **
 ******************************** (cut) ********************************
 */
