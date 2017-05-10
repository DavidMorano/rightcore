#define VERSION  "3.0g"
/*
	BWNFSD is an RPC daemon used in conjunction with BWNFS (a client NFS
for DOS based PCs. BWNFSD provides Authentication, Print Spooling,
DOS 3.1 Locking, DOS 3.1 Sharing, GID name mapping, UID name mapping services
for BWNFS and associated applications on the PC. BWNFSD is being used also
by Macintosh NFS clients.

	The BWNFSD code is originally copyright Beame & Whiteside Software Ltd.
and now is released to the Public Domain. The intent is that all server vendors
included a version of BWNFSD with their operating system. BWNFSD can run
simultanteously with PCNFSD, but provides more features.

	Please send modifications to:

		Beame & Whiteside Software Ltd.
		P.O. Box 8130
		Dundas, Ontario
		Canada L9H 5E7
		+1 (416) 765-0822

	Please modify by including "ifdefs" for your particular operating
system, with appropriate modifications to the "makefile".

MODIFICATION HISTORY
--------------------
11/11/91 gsw  Added some debug statements
24/03/92 gsw  Changed SHADOW to SHADW since it conflicted on some SYS-V's
31/07/92 fjw  Changed printers code to check for abnormal conditions
31/07/92 fjw  Cleaned up the code and repaired the printer parsing
01/08/92 fjw  Merged in SVR4 code from Frank Glass (gatech!holos0!fsg)
31/08/92 cfb  Major changes to support pcnfsdv2-style queues, etc.
31/08/92 fjw  Changed all the ifdefs around for cleaner code
31/08/92 fjw  Added user cacheing in authorize code
08/10/92 fjw  Fixed code relating to printing and debugmode
05/11/92 fjw  Added some pretty print in debug mode

*/

/*
 *  Uncomment the #define line below only if your system uses a Shadow Password
 *  Database and you have received an "Authorization Error" message after
 *  attempting to Link using a valid username & password combination!
 */

#ifndef SHADW
/* #define SHADW */
#endif

#define MAX_NUM_PRINTERS 500
#define POLL_INTERVAL 60
#define MAX_NUM_CANCEL_CACHE 10
#define NUM_USER_CACHE 3

#include <stdio.h>
#include <sys/types.h>
#ifdef AIX
#include <sys/select.h>
#endif
#include <sys/stat.h>
#include <rpc/rpc.h>
#ifndef HP8_PRINTING	/* to keep HPUX includefile brain-death from erroring */
#include <rpc/clnt.h>	/* added for svr4 */
#include <rpc/svc.h>	/* added for svr4 */
#endif
#include <errno.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <grp.h>
#include <string.h>
#include <netinet/in.h>   /* This may be problematic on some systems */

#ifdef SHADW
#   include <shadow.h>
#endif

#ifdef SGI
extern struct group *getgrent();
#endif

char *crypt();
char *my_strdup();
void free_printer();
void get_printers();

#ifdef INT_SIG
#define Void int
#define Return return(0)
#else
#define Void void
#define Return return
#endif

#ifdef WINNFS
#   include <tiuser.h>
#endif

#ifdef SVR4_PRINTING
#define ALLOW 0
#define DENY  1
#define CLASS 2

struct {
	int access;
	int count;
	union {
		char **users;
		int  *printers;
	} n;
} printer_access[MAX_NUM_PRINTERS];
#endif

#define BW_NFS_PROGRAM 0x2f00dbadL
#define BW_NFS_VERSION 1L

#define SPOOL_INQUIRE  1L	/* Return fhandle of spool directory */
#define SPOOL_FILE     2L	/* Spool file */
#define AUTHORIZE      3L	/* Authorize and return UID and GIDs (max 16) */
#define GRP_NAME_TO_NUMB 4L	/* Convert group name to number */
#define GRP_TO_NUMBER  5L	/* Convert group number(s) to name(s) */
#define RETURN_HOST    6L	/* Convert IP to hostname */
#define UID_TO_NAME    7L	/* Convert UID(s) to name(s) */
#define NAME_TO_UID    8L	/* Convert name(s) to UID(s)  L. Yen */
#define SHARE	      20L	/* DOS 3.1 Share function */
#define UNSHARE       21L	/* DOS 3.1 UnShare function */
#define LOCK	      22L	/* DOS 3.1 Lock request */
#define REMOVE	      23L	/* Remove all locks/shares for PC */
#define UNLOCK	      24L	/* DOS 3.1 Unlock request */
#define GET_PRINTERS  30L	/* Get a list of printers */
#define GET_PRINTQ    31L	/* Get print queue entries and status */
#define CANCEL_PRJOB  32L	/* Cancel a print job */

#define NUSRS  10		/* Maximum names to convert */

#define WNPRQ_ACTIVE	0
#define WNPRQ_PAUSE	1
#define WNPRQ_ERROR	2
#define WNPRQ_PENDING	3
#define WNPRQ_PROBLEM	4
#define WN_SUCCESS	0
#define WN_BAD_QUEUE	0x43

static	char	dir[32],g_string[32];
static	char	spool_dir[132], file[256], file1[256];
static	char	remote_host[16];
static	int	num_printers;
static	char	printer[12];
static	char	*jobname;
static	char	file_name[9];
static	char	*hostname,*p;
static	int	extension, lgids[NGRPS+1];
static	int	luids[NUSRS+1];
static	int	A_flag=0, s_flag=0, do_print;
int debugmode;
static	u_long	my_ip;
static	u_long	incomming_ip;

struct queueentry {
	int jobid;
	char *username;
	char *params;
	int queueposition;
	int jobstatus;
	unsigned long timesubmitted;
	unsigned long size;
	int copies;
	char *jobcomment;
	struct queueentry *next;
};

struct printertype {
	char *queuename;
	char *queuecomment;
	int queuestatus;
	int numphysicalprinters;
	unsigned long lasttime;
	struct queueentry *head;
} printers[MAX_NUM_PRINTERS];

static struct mm_ips {		/* Valid hosts to authenticate for */
	char	type;
	u_long	ip;
	u_long	mask;
	u_long	redirect_ip;
} **ips;

static struct mm_uids { 	/* Cache of uid to name translation */
	int	mm_uid;
	char	*mm_username;
} **uds;

static struct mm_gidss {	/* Cache of uid and associated gids */
	int mm_uid;
	int mm_gid;
	int mm_gid_count;
	int mm_gids[NGRPS];
} **gds;

static int gids_count=0, uids_count=0;
static int ips_count=0;

static struct locking {
	unsigned long	cookie; 	/* 4 bytes */
	char		name[17];	/* 16 character name */
	char		fh[32]; 	/* 32 bytes */
	int		mode;
	int		access;
	unsigned long	offset;
	unsigned long	length;
	int		stat;
	int		sequence;
} lock = { 0 };

static struct printqs {
	char queuename[32];
	char username[32];
	int printer_number;
} printq;

static struct cancaches {
	char queuename[32];
	char username[32];
	int jobid;
	int response;
} cancache[MAX_NUM_CANCEL_CACHE] = { 0 };

int current_cache=0;

int
my_strnicmp(p1, p2, num)
	char *p1, *p2;
	int num;
{
char c, d;

	for ( ; num>0; num--) {
		c = *p1++;
		d = *p2++;
		c = islower(c)?toupper(c):c;
		d = islower(d)?toupper(d):d;
		if (c != d)
			return(1);
	}
	return(0);
}

#ifdef SVR4_PRINTING
int
check_access_printer(pn,name)
	int pn;
	char *name;
{
int i;
char **p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [check_access_printer] pn = %d, name = %s\n", pn, name);

	if (printer_access[pn].count == -1) {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [check_access_printer] deny all requests\n");
		return(0);				     /* Deny all */
	}
	p = printer_access[pn].n.users;
	for (i=0; i<printer_access[pn].count; i++) {
		if (strcmp(*p++,name) == 0) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [check_access_printer] request %s\n", printer_access[pn].access==ALLOW?"allowed":"denied");
			return(printer_access[pn].access == ALLOW);
		}
	}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [check_access_printer] request %s\n", printer_access[pn].access==DENY?"allowed":"denied");
	return(printer_access[pn].access == DENY);
}

int
check_access(pn, name)
	int pn;
	char *name;
{
int i;
int *pi;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [check_access] pn = %d, name = %s\n", pn, name);

	if (printer_access[pn].access != CLASS)
		return(check_access_printer(pn,name));
	pi = printer_access[pn].n.printers;
	for (i=0; i<printer_access[pn].count; i++)
		if (check_access_printer(*pi++,name)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [check_access] returns 1\n");
			return(1);
		}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [check_access] returns 0\n");
	return(0);
}
#endif

Void
clear(unused)
	int unused;
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [clear]\n");
	for (i=0; i<uids_count; i++) {
		free((*(uds+i))->mm_username);
		free(*(uds+i));
	}
	if (uids_count > 0) {
		free(uds);
		uids_count = 0;
	}
	endpwent();
	for (i=0; i<gids_count; i++)
		free(*(gds+i));
	if (gids_count > 0) {
		free(gds);
		gids_count = 0;
	}
	endgrent();
	for (i=0; i<num_printers; i++) {
		free_printer(i);
		free(printers[i].queuename);
		if (printers[i].queuecomment != NULL)
			free(printers[i].queuecomment);
	}
	get_printers();
	(void) signal(SIGHUP, clear);
	Return;
}

Void
free_child(unused)
	int unused;
{
int pstatus;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [free_child] called\n");

	(void) wait(&pstatus);
#ifdef SIGCHLD
	(void) signal(SIGCHLD,free_child);
#endif
#ifdef SIGCLD
	(void) signal(SIGCLD,free_child);
#endif
	Return;
}

void
strlwr(p)
	char *p;
{
	do {
		if ((*p >= 'A') && (*p <= 'Z'))
			*p += 'a'-'A';
	} while (*(++p) != '\0');
}

void
getmask(ip,mask)
	u_long *ip,*mask;
{
	if (*ip == 0L)
		*mask = 0L;
	else
		if (*ip <= 0xffL) {
			*ip <<= 24;
			*mask = 0xff000000L;
		} else
			if (*ip <= 0xffffL) {
				*ip <<= 16;
				*mask = 0xffff0000L;
			} else
				if (*ip <= 0xffffffL) {
					*ip <<= 8;
					*mask = 0xffffff00L;
				} else
					*mask = 0xffffffffL;
}

unsigned long
my_inet_addr(cp)
	char *cp;
{
static union {
	 unsigned long inn;
	 struct {
		 unsigned char b1, b2, b3, b4;
	 } i;
} in;
int b1, b2, b3, b4;

	b1 = b2 = b3 = b4 = -1;
	if (sscanf(cp,"%d.%d.%d.%d", &b1, &b2, &b3, &b4) == -1)
		return(~0L);
	if (b4 == -1) {
		b4 = b3;
		b3 = b2;
		b2 = 0;
	}
	if (b4 == -1) {
		b4 = b3;
		b3 = 0;
	}
	if (((unsigned) b1 > 255) || ((unsigned) b2 > 255)
	  ||((unsigned) b3 > 255) || ((unsigned) b4 > 255))
		return(~0L);
	in.i.b1 = (unsigned char) b1;
	in.i.b2 = (unsigned char) b2;
	in.i.b3 = (unsigned char) b3;
	in.i.b4 = (unsigned char) b4;
	return(ntohl(in.inn));
}

/*
 *  For -s parameter, returns IP addresses and the associated mask to determine
 *  if the IP address of the destination machine/network is matches an entry
 *  in the -s file.
 */
int
convert_ip(s,ip,mask)
	char *s;
	u_long *ip,*mask;
{
u_long ia,lip,lmask;
struct hostent *he;
struct netent *ne;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [convert_ip] s = %s\n", s);

	if ((ia=inet_network(s)) != (~0L)) {
		lip = ia;
		getmask(&lip,&lmask);
	} else {
		if ((ia=my_inet_addr(s)) != (~0L)) {
			lip = ia;
			lmask = 0xffffffffL;
		} else {
			if ((ne=getnetbyname(s)) != NULL) {
				lip = ne->n_net;
				getmask(&lip,&lmask);
			} else {
				if ((he=gethostbyname(s)) != NULL) {
					lip = * (u_long *) he->h_addr;
					lmask = 0xffffffffL;
				} else {
					if (debugmode)
						fprintf(stdout, "bwnfsd: [convert_ip] returns 0\n");
					return(0);
				}
			}
		}
	}
	*ip = lip;
	if (mask != NULL)
		*mask = lmask;
	if (debugmode)
		fprintf(stdout, "bwnfsd: [convert_ip] (1) ip = %lu, mask = %lu\n", ip, mask);
	return(1);
}

/*
 *	 Read the -s file which lists rules for which hosts can be
 *	 authenticated. Each line is converted to a IP address and mask. Any
 *	 incoming address is ANDed with the mask and compared to IP address,
 *	 if it matches, the action associated with the line is taken.
 *
 *	 Valid actions:
 *		 "+" - Perform Validatation check
 *		 "-" - Return Authorization error
 *		 "=" - Tell client to attempt Authorization at new address
 */
void
read_ips(file)
	char *file;
{
FILE *f;
char s1[255], s2[255], line[255], type;
int i;
u_long ip, mask, redirect_ip;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [read_ips] file = %s\n", file);

	if ((f=fopen(file,"r")) == NULL) {
		fprintf(stdout, "bwnfsd: [read_ips] \"%s\" file not found.\n",file);
		fflush(stdout);
		exit(1);
	}

	ips = (struct mm_ips **) malloc(1);
	while (fgets(line,sizeof(line)-1,f) != NULL) {
		if ((type=line[0]) != '#') {
			i = sscanf(&line[1],"%s %s",s1,s2);
			if (debugmode)
				fprintf(stdout, "bwnfsd: [read_ips] parsing <%s>\n", line[1]);

			if ((i == EOF) || (i == 0) || (i > 2) || ((type != '+') &&
			    (type != '-') && (type != '=')) || ((type == '=') &&
			    (i != 2)) || ((type != '=') && (i != 1))) {
				fprintf(stdout, "bwnfsd: [read_ips] syntax error, line ignored : %s",line);
			} else {
				if (!convert_ip(s1,&ip,&mask)) {
					fprintf(stdout, "bwnfsd: [read_ips] \"%s\" invalid address.\n",s1);
					continue;
				}
				if (i == 2)
					if (!convert_ip(s2, &redirect_ip, NULL)) {
					       fprintf(stdout, "bwnfsd: [read_ips] \42%s\42 invalid address.\n",s1);
					       continue;
					}
				ips = (struct mm_ips **) realloc(ips,sizeof(ips)*(ips_count+1));
				*(ips+ips_count) = (struct mm_ips *) malloc(sizeof(struct mm_ips));
				(*(ips+ips_count))->type	= type;
				(*(ips+ips_count))->ip		= ip;
				(*(ips+ips_count))->mask	= mask;
				(*(ips+ips_count))->redirect_ip = redirect_ip;
				ips_count++;
		       }
		}
	}
	fclose(f);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [read_ips] exiting with %u values\n", ips_count);
}

/*
 *	 Add UID to USERNAME translation to cache
 */
void
add_uid(uid,username)
	int	uid;
	char	*username;
{
	if (debugmode)
		fprintf(stdout, "bwnfsd: [add_uid] uid <%d>, name <%s>\n", uid, username);
	if (uids_count++ == 0)
		uds = (struct mm_uids **) malloc(1);

	uds				   = (struct mm_uids **) realloc(uds,sizeof(uds)*uids_count);
	(*(uds+uids_count-1))		   = (struct mm_uids *) malloc(sizeof(struct mm_uids));
	(*(uds+uids_count-1))->mm_uid	   = uid;
	(*(uds+uids_count-1))->mm_username = (char *) malloc(strlen(username)+1);
	(void) strcpy((*(uds+uids_count-1))->mm_username,username);
}

/*
 *	 Lookup USERNAME given a UID, also add translation to cache
 */
char *
get_ui_name(uid)
	int	uid;
{
struct passwd *pww;
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_ui_name] uid <%u>\n", uid);

	for (i=0; i<uids_count; i++)
		if ((*(uds+i))->mm_uid == uid)
			return((*(uds+i))->mm_username);
	if ((pww=getpwuid(uid)) == NULL) {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [get_ui_name] getpwuid returned NULL\n");
		return(NULL);
	}
	add_uid(uid,pww->pw_name);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_ui_name] returns %s\n", pww->pw_name);
	return(pww->pw_name);
}

/*
 *	 Given a UID, GID and USERNAME, create a list of GIDs associated
 *	 with the UID, a maximum of NGRPS GIDS are used.
 */
void
fill_gid(gd,uid,gid,username)
	struct mm_gidss *gd;
	int	uid,gid;
	char	*username;
{
struct group *gr;
struct group *getgrent();
char   **p;
int    flag;

	add_uid(uid,username);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [fill_gid] uid = %u, gid = %u, user = %s\n", uid, gid, username);
	flag		 = 0;
	gd->mm_uid	 = uid;
	gd->mm_gid	 = gid;
	gd->mm_gid_count = 0;
	(void) setgrent();
	while ((gr=getgrent()) != NULL) {
		p = gr->gr_mem;
		while (((*p) != NULL) && (gd->mm_gid_count < NGRPS)) {
			if (strcmp((*p),username) == 0) {
				gd->mm_gids[gd->mm_gid_count++] = gr->gr_gid;
				if (gr->gr_gid == gid)
					flag = 1;
				break;
			}
			p++;
		}
	}
	if ((flag == 0) && (gd->mm_gid_count < NGRPS))
		gd->mm_gids[gd->mm_gid_count++] = gid;
	(void) endgrent();
}

/*
 *	 Return a list of GIDs for a given UID. The cache of GIDS/UID is
 *	 updated and used.
 */
struct mm_gidss *
get_gids(uid,gid,username)
	int uid,gid;
	char *username;
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [mm_gidss] uid = %u, gid = %u, name = %s\n", uid, gid, username);

	for (i=0; i<gids_count; i++)
		if ((*(gds+i))->mm_uid == uid) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [mm_gidss] gids_count = %u\n", gids_count);
			return(*(gds+i));
		}

	if (gids_count++ == 0)
		gds = (struct mm_gidss **) malloc(1);

	gds = (struct mm_gidss **) realloc(gds,sizeof(gds)*gids_count);
	(*(gds+gids_count-1)) = (struct mm_gidss *) malloc(sizeof(struct mm_gidss));
	(void) fill_gid((*(gds+gids_count-1)), uid, gid, username);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [mm_gidss] gids_count = %u\n", gids_count);
	return(*(gds+gids_count-1));
}

static struct my_groups {
	int my_gid;
	char	*my_name;
} **g;

static int group_count=0;

/*
 *	 Return the text name of a GID, if none is found, then the GID
 *	 is convert to a text number.
 */
char *
get_gr_name(gid)
	int	gid;
{
static char gd[32];
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_gr_name] for gid %u\n", gid);

	for (i=0; i<group_count; i++)
		if ((*(g+i))->my_gid == gid) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_gr_name] returns %s\n", ((*(g+i))->my_name));
			return ((*(g+i))->my_name);
		}
	sprintf(gd,"%u",gid);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_gr_name] returns %s\n", gd);
	return(gd);
}

/*
 *	 Given a group name, return the GID associated with it.
 */
int *
get_gr_gid(name)
	char *name;
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_gr_gid] for %s\n", name);
	for (i=0; i<group_count; i++)
		if (strcmp((*(g+i))->my_name,name) == 0) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_gr_gid] returns %s\n", (*(g+i))->my_gid);
			return(&(*(g+i))->my_gid);
		}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_gr_gid] returns NULL\n");
	return (NULL);
}

/*
 *	 Cache a list of all group names and their associated GIDS.
 */
void
get_groups()
{
struct group *gr;
struct group *getgrent();

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_groups] called\n");

	g = (struct my_groups **) malloc(1);
	(void) setgrent();
	while ((gr=getgrent()) != NULL) {
		group_count++;
		g = (struct my_groups **) realloc(g,sizeof(g)*group_count);
		(*(g+group_count-1)) = (struct my_groups *) malloc(sizeof(struct my_groups));
		(*(g+group_count-1))->my_name = (char *) malloc(strlen(gr->gr_name)+1);

		(void) strcpy((*(g+group_count-1))->my_name,gr->gr_name);
		(*(g+group_count-1))->my_gid = gr->gr_gid;
	}
	(void) endgrent();
}

#ifdef SVR4_PRINTING
void
get_class_list(char *name, int pn)
{
FILE *f;
char line[128];
char file_name[64];
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_class_list] name = %s, pn = %u\n", name, pn);
	printer_access[pn].access     = CLASS;
	printer_access[pn].n.printers = NULL;
	printer_access[pn].count      = 0;
	sprintf(file_name,"/usr/spool/lp/admins/lp/classes/%s",name);

	if ((f=fopen(file_name,"r")) == NULL) {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [get_class_list] open file %s returned NULL\n", file_name);
		return;
	}
	while (fgets(line,128,f) != NULL) {
		*(line+strlen(line)-1) = '\0';
		for (i=0; i<pn; i++)
			if (strcmp(line,printers[i].queuename) == 0) {
				if (printer_access[pn].count++ == 0)
					printer_access[pn].n.printers = (int *) malloc(sizeof(int));
				else
					printer_access[pn].n.printers = (int *) realloc(printer_access[pn].n.printers,printer_access[pn].count * sizeof(int));
				*(printer_access[pn].n.printers+printer_access[pn].count-1) = i;
				break;
			}
	}
	fclose(f);
}


void
get_access(char *name, int pn)
{
FILE *f;
char line[128];
char file_name[64];

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_access] name = %s, pn =%u\n", name, pn);
	printer_access[pn].access  = ALLOW;
	printer_access[pn].n.users = NULL;
	printer_access[pn].count   = 0;
	sprintf(file_name,"/usr/spool/lp/admins/lp/printers/%s/users.allow",name);
	if ((f=fopen(file_name,"r")) != NULL) {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [get_access] reading %s\n", file_name);
		while (fgets(line,128,f) != NULL) {
			*(line+strlen(line)-1) = '\0';
			if (strcmp(line,"any") == 0) {
				printer_access[pn].n.users = NULL;
				printer_access[pn].count = 0;
				break;
			}
			if (printer_access[pn].count++ == 0)
				printer_access[pn].n.users = (char **) malloc(sizeof(char**));
			else
				printer_access[pn].n.users = (char **) realloc(printer_access[pn].n.users,printer_access[pn].count * sizeof(char**));
			*(printer_access[pn].n.users+printer_access[pn].count-1) = strdup(line);
		}
		fclose(f);
		if (printer_access[pn].count != 0) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_access] access count = %u\n", printer_access[pn].count);
			return;
		}
	} else {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [getaccess] unable to open %s\n", file_name);
	}
	printer_access[pn].access = DENY;
	sprintf(file_name,"/usr/spool/lp/admins/lp/printers/%s/users.deny",name);
	if ((f=fopen(file_name,"r")) != NULL) {
		while (fgets(line,128,f) != NULL) {
			*(line+strlen(line)-1) = '\0';
			if (strcmp(line,"any") == 0) {
				printer_access[pn].n.users = NULL;
				printer_access[pn].count   = -1;
				break;
			}
			if (printer_access[pn].count++ == 0)
				printer_access[pn].n.users = (char **) malloc(sizeof(char**));
			else
				printer_access[pn].n.users = (char **) realloc(printer_access[pn].n.users,printer_access[pn].count * sizeof(char**));
			*(printer_access[pn].n.users+printer_access[pn].count-1) = strdup(line);
		}
		fclose(f);
	} else {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [getaccess] unable to open %s\n", file_name);
	}

}


void
get_printer_list(status_file, type)
char *status_file;
int  type;
{
FILE *f;
char line[128],name[32];
int  i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_printer_list] file = %s, type = %u\n", status_file, type);
	if ((f=fopen(status_file,"r")) == NULL) {
		if (debugmode)
			fprintf(stdout, "bwnfsd: [get_printer_list] unable to open %s\n", status_file);
		return;
	}
	while (fgets(line,128,f) != NULL)
		if (strncmp(line,"=====",5) == 0) {
			if (fgets(name,32,f) == NULL)
				break;
			*(name+strlen(name)-1) = '\0';
			if (fgets(line,128,f) == NULL)
				break;
			*(line+strlen(line)-1) = '\0';
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_printer_list] name = %s, line = %s\n", name, line);
			for (i=0; i<(int)strlen(line); i++)
				if (strncmp(&line[i],"accepting",9) == 0)
					break;
			if (i == strlen(line))
				continue;
			if (strlen(name) > 11) {
				fprintf(stdout, "bwnfsd: [get_printer_list] Printer name \"%s\" exceeds 11 characters; not available for linking\n", name);
				continue;
			}
			if (num_printers >= MAX_NUM_PRINTERS) {
				if (debugmode)
					fprintf(stdout, "bwnfsd: [get_printer_list] Max number of printers (%d) exceeded.  Recompile with larger value\n", MAX_NUM_PRINTERS);
				break;
			}
			printers[num_printers].queuename = strdup(name);
			printers[num_printers].queuecomment = NULL;
			if (type == 0)
				get_access(name,num_printers++);
			else
				get_class_list(name,num_printers++);
		}
	fclose(f);
}
#endif

/*
 *	 Cache a list of available printers, BSD code searchs the /etc/printcap
 *	 file.	Creating a dummy /etc/printcap can also work.
 */
void
get_printers()
{
#ifndef SVR4_PRINTING
static char line[1024];
#ifdef AIX
static char stanza[255];
#endif
char *p, *p1, *end;
FILE *f;
int start;
#endif
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_printers] called\n");

	num_printers = 0;
#ifdef SVR4_PRINTING
	get_printer_list("/usr/spool/lp/system/pstatus",0);
	get_printer_list("/usr/spool/lp/system/cstatus",1);
#else
#ifdef AIX
	if ((f=fopen("/etc/qconfig","r")) == NULL) {
		fprintf(stdout, "bwnfsd: File /etc/qconfig was not found.\n");
		fprintf(stdout, "        No printers are available for use.\n");
		fflush(stdout);
		return;
	}

	stanza[0] = '\0';
	while (fgets(line,sizeof(line)-1,f) != NULL) {
		line[sizeof(line)-1] = '\0';
		for (p=line; isspace(*p); )
			p++;
		if (*p == '*')
			continue;
		for (p1=&line[strlen(line)-1]; isspace(*p1); --p1)
			*p1 = '\0';
		if (strchr(p, ':') != NULL) {
			strcpy(stanza, p);
			if (stanza[strlen(stanza)-1] == ':')
				stanza[strlen(stanza)-1] = '\0';
			continue;
		}
		if (my_strstr(p, "device") != NULL) {
			if (stanza[0] != '\0') {
				if (num_printers >= MAX_NUM_PRINTERS) {
					fprintf(stdout, "bwnfsd: [get_printers] Max number of printers (%d) exceeded.\nRecompile with larger value\n", MAX_NUM_PRINTERS);
					fflush(stdout);
					goto outoutout;
				}
				if (strlen(stanza) <= 11) {
					printers[num_printers].queuename = my_strdup(stanza);
					printers[num_printers].queuecomment = NULL;
					printers[num_printers].queuestatus = WNPRQ_ACTIVE;
					printers[num_printers].numphysicalprinters = 0;
					printers[num_printers].lasttime = 0L;
					printers[num_printers].head = NULL;
					num_printers++;
				} else
					fprintf(stdout, "bwnfsd: [get_printer_list] Printer name \"%s\" exceeds 11 characters; not available for linking\n", stanza);
				stanza[0] = '\0';
			}
		}
	}
outoutout:
	fclose(f);
#else
	if ((f=fopen("/etc/printcap","r")) == NULL) {
		fprintf(stdout, "bwnfsd: File /etc/printcap was not found.\n");
		fprintf(stdout, "        No printers are available for use.\n");
		fflush(stdout);
		return;
	}

	while (fgets(line,sizeof(line)-1,f) != NULL) {
		start=num_printers;
multi_line:
		if ((p=strchr(line,'#')) != NULL)
			*p = '\0';
		if ((p=strchr(line,':')) != NULL)
			*p = '\0';
		if ((end=line+strlen(line)) == line)
			continue;
		if (*(end-1) == '\n')
			*(--end) = '\0';
		if (*(end-1) == '\\') {
			*(--end) = '\0';
			if ((sizeof(line)-(end-line)) <= 0) {
				fprintf(stdout, "bwnfsd: [get_printers] Multi-line line too long in /etc/printcap.\n");
				fflush(stdout);
				break;
			}
			if (fgets(end,sizeof(line)-(end-line),f) != NULL)
				goto multi_line;
		}
		for (p=line; p<=end; ) {
			while ((*p == ' ') || (*p == '\t'))
				p++;
			if (!*p)
				break;
			p1 = p;
			while (strchr(" \t|",*p1) == NULL)
				p1++;
			if ((*p1 == ' ') || (*p1 == '\t')) {
				while (strchr("|",*p1) == NULL)
					p1++;
				*p1 = '\0';
				while ((p[strlen(p)-1] == ' ') || (p[strlen(p)-1] == '\t'))
					p[strlen(p)-1] = '\0';
				if ((strchr(p,' ') != NULL) || (strchr(p,'\t') != NULL)) {
					for (i=start; i<num_printers; i++)
						printers[i].queuecomment = my_strdup(p);
					start = num_printers;
					p = p1 + 1;
					continue;
				}
			} else
				*p1 = '\0';
			if (*p) {
				if (num_printers >= MAX_NUM_PRINTERS) {
					fprintf(stdout, "bwnfsd: [get_printers] Max number of printers (%d) exceeded.\nRecompile with larger value\n", MAX_NUM_PRINTERS);
					fflush(stdout);
					goto outoutout;
				}
				if (strlen(p) <= 11) {
					printers[num_printers].queuename = my_strdup(p);
					printers[num_printers].queuecomment = NULL;
					printers[num_printers].queuestatus = WNPRQ_ACTIVE;
					printers[num_printers].numphysicalprinters = 0;
					printers[num_printers].lasttime = 0L;
					printers[num_printers].head = NULL;
					num_printers++;
				} else
					fprintf(stdout, "bwnfsd: [get_printer_list] Printer name \"%s\" exceeds 11 characters; not available for linking\n", p);
			}
			p = p1 + 1;
		}
	}
outoutout:
	fclose(f);
#endif
#endif
	if (debugmode) {
		  fprintf(stdout, "bwnfsd: [get_printers] Following devices are available:\n");
		  for (i=0; i<num_printers; i++)
			  fprintf(stdout, "bwnfsd:                 %s\n", printers[i].queuename);
	}

}

static struct var_file {
	int	status;
	int	dir_size;
	char	*dir_handle;
} f;

/*
 *	 DOS 3.1 Sharing and Unsharing code. Uses the same format for rpc.lockd
 *	 as BWNFSD, so some parameters are ignored. The cookie under rpc.lockd
 *	 must be 4 bytes long under BWNFSD.
 */
xdr_share(xdrsp,l)
	XDR *xdrsp;
	struct locking *l;
{
int  i;
char g[32];
char *p;

	if (debugmode > 1) {
		fprintf(stdout, "bwnfsd: [xdr_share] called for %s, ofs = %ld, len = %ld\n", l->name, l->offset, l->length);
		fprintf(stdout, "bwnfsd:             mode = %u, access = %u, stat = %u, sequence = %u\n", l->mode, l->access, l->stat, l->sequence);
	}
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);
	if (l->cookie != 4)
		return(0);
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);

	p = l->name;
	if (!xdr_string(xdrsp,&p,17))
		return(0);
	p = l->fh;
	if (!xdr_bytes(xdrsp,&p,&i,32))
		return(0);
	p = g;
	if (!xdr_bytes(xdrsp,&p,&i,32))
		return(0);
	if (!xdr_int(xdrsp, &l->mode))
		return(0);
	if (!xdr_int(xdrsp, &l->access))
		return(0);

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_share] returns (1)\n");
	return(1);
}


/*
 *	 Get Printers response
 */
int
xdr_printers(xdrsp,l)
	XDR *xdrsp;
	char *l;
{
int i;
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_printers] called\n");
	i = num_printers;
	if (!xdr_int(xdrsp, &i))
		return(0);
	for (i=0; i<num_printers; i++) {
		p = printers[i].queuename;
		if (!xdr_string(xdrsp, &p, 24))
			return(0);
		p = printers[i].queuecomment;
		if (p == NULL)
			p = "";
		if (!xdr_string(xdrsp, &p, 255))
			return(0);
	}

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_printers] returns (1)\n");
	return(1);
}

/*
 *	 DOS 3.1 Share response.
 */
int
xdr_shareres(xdrsp,l)
	XDR *xdrsp;
	struct locking *l;
{
int i;
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_shareres] called by %s\n", l->name);
	i = 4;
	if (!xdr_int(xdrsp, &i))
		return(0);
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);
	if (!xdr_int(xdrsp,&l->stat))
		return(0);
	l->sequence++;
	if (!xdr_int(xdrsp,&l->sequence))
		return(0);

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_shareres] returns (1)\n");
	return(1);
}


/*
 *	 DOS 3.1 locking request. Note that several parameters are ignored.
 */
int
xdr_lock(xdrsp,l)
	XDR *xdrsp;
	struct locking *l;
{
int  i;
char g[32];
char *p;

	if (debugmode) {
		fprintf(stdout, "bwnfsd: [xdr_lock] called by %s\n", l->name);
		if (debugmode > 1)
			fprintf(stdout, "bwnfsd:            ofs = %lu, len = %lu\n", l->offset, l->length);
	}
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);
	if (!xdr_int(xdrsp,&i))
		return(0);
	if (!xdr_int(xdrsp,&i))
		return(0);
	p = l->name;
	if (!xdr_string(xdrsp,&p,17))
		return(0);
	p = l->fh;
	if (!xdr_bytes(xdrsp,&p,&i,32))
		return(0);
	p = g;
	if (!xdr_bytes(xdrsp,&p,&i,32))
		return(0);
	if (!xdr_int(xdrsp,&i))
		return(0);
	if (!xdr_int(xdrsp, &l->offset))
		return(0);
	if (!xdr_int(xdrsp, &l->length))
		return(0);
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_lock] returns (1)\n");
	return(1);
}


/*
 *	 DOS 3.1 unlock request. Note that several parameters are ignored.
 */
xdr_unlock(xdrsp,l)
	XDR *xdrsp;
	struct locking *l;
{
int  i;
char g[32];
char *p;

	if (debugmode) {
		fprintf(stdout, "bwnfsd: [xdr_unlock] called by %s\n", l->name);
		if (debugmode > 1)
			fprintf(stdout, "bwnfsd:              ofs = %lu, len = %lu\n", l->offset, l->length);
	}
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);

	p = l->name;
	if (!xdr_string(xdrsp,&p,17))
		return(0);
	p = l->fh;
	if (!xdr_bytes(xdrsp,&p,&i,32))
		return(0);
	p = g;
	if (!xdr_bytes(xdrsp,&p,&i,32))
		return(0);
	if (!xdr_int(xdrsp,&i))
		return(0);
	if (!xdr_int(xdrsp, &l->offset))
		return(0);
	if (!xdr_int(xdrsp, &l->length))
		return(0);
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_unlock] returns (1)\n");
	return(1);
}

/*
 *	 Send Locking response.
 */
int
xdr_res(xdrsp,l)
	XDR *xdrsp;
	struct locking *l;
{
int i;
char *p;

	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_res] called\n");
	i = 4;
	if (!xdr_int(xdrsp, &i))
		return(0);
	if (!xdr_int(xdrsp, &l->cookie))
		return(0);
	if (!xdr_int(xdrsp,&l->stat))
		return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_res] returns (1)\n");
	return(1);
}


/*
 *	 Remove all DOS 3.1 locks and shares, note that the PC's name is
 *	 a maximum of 16 chaarcters.
 */
int
xdr_get_lock_name(xdrsp,l)
	XDR *xdrsp;
	char *l;
{
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_get_lock_name] called\n");
	return(xdr_string(xdrsp,&l,17));
}

/*
 *	 Send a redirect to a new IP during an authorization request.
 */
int
xdr_new_ip(xdrsp, ip)
	XDR *xdrsp;
	u_long *ip;
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_new_ip] called\n");
	i = 3;
	if (!xdr_int(xdrsp, &i))
		return(0);
	if (!xdr_int(xdrsp, ip))
		return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_new_ip] returns (1)\n");
	return(1);
}

/*
 *	 Get GROUP name.
 */
int
xdr_g_string(xdrsp, st)
	XDR *xdrsp;
	char *st;
{
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_g_string] called\n");
	p = st;
	if (!xdr_string(xdrsp,&p,32))
		return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_g_string] returns (1)\n");
	return(1);
}

/*
 *	 Send Status and GID
 */
int
xdr_send_gid(xdrsp, st)
	XDR *xdrsp;
	char *st;
{
int   *i, j;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_gid] called with st = %s\n", st);
	i = get_gr_gid(st);
	if (i == NULL) {
		j = 2;
		if (!xdr_int(xdrsp,&j))
			return(0);
	} else {
		j = 0;
		if (!xdr_int(xdrsp,&j))
			return(0);
		if (!xdr_int(xdrsp,i))
			return(0);
	}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_gid] returns (1)\n");
	return(1);
}

/*
 *	 Get list of UIDs to translate to text.
 */
int
xdr_nuids(xdrsp, luids)
	XDR *xdrsp;
	int luids[NUSRS+1];
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_nuids] called\n");
	if (!xdr_int(xdrsp,&luids[0]))
		return(0);
	if ((luids[0] < 0) || (luids[0] > 16))
		luids[0] = 0;
	for (i=1; i<=luids[0]; i++)
	   if (!xdr_int(xdrsp,&luids[i]))
		return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_nuids] returns (1)\n");
	return(1);
}


/*
 *	 Get list of GIDs to translate to text.
 */
int
xdr_ngids(xdrsp, lgids)
	XDR *xdrsp;
	int lgids[NGRPS+1];
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_ngids] called\n");

	if (!xdr_int(xdrsp,&lgids[0]))
		return(0);
	if ((lgids[0] < 0) || (lgids[0] > 16))
		lgids[0] = 0;
	for (i=1; i<=lgids[0]; i++)
		if (!xdr_int(xdrsp,&lgids[i]))
			return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_ngids] returns (1)\n");
	return(1);
}


/*
 *	 Send Usernames translated from UIDS.
 */
xdr_send_usernames(xdrsp, luids)
	XDR *xdrsp;
	int luids[NUSRS+1];
{
int i, j;
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_usernames] called\n");
	for (i=1; i<=luids[0]; i++) {
		p = get_ui_name(luids[i]);
		if (debugmode)
			fprintf(stdout, "bwnfsd: [xdr_send_usernames] p = %s\n", p);
		if (p == NULL) {
			j = 0;
			if (!xdr_int(xdrsp,&j))
				return(0);
		} else {
			if (!xdr_string(xdrsp,&p,32))
				return(0);
		}
	}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_usernames] returns (1)\n");
	return(1);
}

/*
 *	 Send Group names translated from GIDS.
 */
int
xdr_send_names(xdrsp, lgids)
	XDR *xdrsp;
	int lgids[NGRPS+1];
{
int i, j;
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_names] called\n");
	for (i=1; i<=lgids[0]; i++) {
		p = get_gr_name(lgids[i]);
		if (debugmode)
			fprintf(stdout, "bwnfsd: [xdr_send_names] p = %s\n", p);
		if (p == NULL) {
			j = 0;
			if (!xdr_int(xdrsp,&j))
				return(0);
		} else {
			if (!xdr_string(xdrsp,&p,32))
				return(0);
		}
	}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_names] returns (1)\n");
	return(1);
}


/*
 *	 Return fhandle of spool directory. Note a loop of XDR_INTS is
 *	 used to allow this to compile on all systems.
 */
xdr_file(xdrsp, file_var)
	XDR *xdrsp;
	struct var_file *file_var;
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_file] called\n");
	if (!xdr_int(xdrsp,&file_var->status))
		return(0);
	if (file_var->status != 0) {
		if (debugmode > 1)
			fprintf(stdout, "bwnfsd: [xdr_file] status = %u, returns (1)\n", file_var->status);
		return(1);
	}
	for (i=0; i<32; i+=4)
		if (!xdr_int(xdrsp, (&file_var->dir_handle[0]) + i))
			return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_file] returns (1)\n");
	return(1);
}


/*
 *	Spool a file to the printer:
 *
 *		Parameters:
 *			printer_name (fixed 11 bytes)
 *			file_name    (fixed 8 bytes) (Internet Address in Hex).
 *			extension     xdr_int (filename is form C02A0405.xxx)
 */
int
xdr_print(xdrsp)
	XDR *xdrsp;
{
struct ustat buf;
struct passwd *pw1;
char *p;
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_print] called\n");
	i = 11;
	p = printer;
	if (!xdr_bytes(xdrsp,&p,&i, 11)) {
		if (debugmode > 1)
			fprintf(stdout, "bwnfsd: [xdr_print] printer = %s, file = %s, ext = %s\n", printer, file_name, extension);
		return(0);
	}
	printer[11] = ' ';
	*strchr(printer, ' ') = '\0';
	i = 8;
	p = file_name;
	if (!xdr_bytes(xdrsp,&p,&i, 8)) {
		if (debugmode > 1)
			fprintf(stdout, "bwnfsd: [xdr_print] printer = %s, file = %s, ext = %s\n", printer, file_name, extension);
		return(0);
	}
	file_name[8] = '\0';
	if (!xdr_int(xdrsp,&extension)) {
		if (debugmode > 1)
			fprintf(stdout, "bwnfsd: [xdr_print] printer = %s, file = %s, ext = %s\n", printer, file_name, extension);
		return(0);
	}
	strlwr(file_name);
	sprintf(file,"%s/%s.%03d",spool_dir,file_name,extension);
#ifdef FOURTEEN_CHAR
	sprintf(file1,"%s/%s.%03dq",spool_dir,file_name,extension);
#else
	sprintf(file1,"%s/%s.%03dqueued",spool_dir,file_name,extension);
#endif
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_print] f = %s, f1 = %s\n", file, file1);
	jobname  = NULL;
	do_print = -1;
	if (stat(file, &buf) == 0) {
		do_print = rename(file, file1);
		if (debugmode && do_print)
			fprintf(stdout, "bwnfsd: [xdr_print] rename failed, error = %d\n", errno);
		pw1 = getpwuid((int) buf.st_uid);
		if (pw1 != NULL) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [xdr_print] jobname = %s\n", pw1->pw_name);
			jobname = pw1->pw_name;
		}
	} else
		if (debugmode)
			if (errno == ENOENT)
				fprintf(stdout, "bwnfsd: [xdr_print] duplicate close request\n");
			else
				fprintf(stdout, "bwnfsd: [xdr_print] stat failed, errno = %d\n",errno);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_print] returns (1)\n");
	return(1);
}

struct auth_struct {
	u_long ip;
	int type;
	char device[13];
	char login_text[64];
	char *username;
	char *password;
} auth_request;

/*
 *	 Request Authorization:
 *		 Parameters:
 *			 IP_Address of server host (XDR_INT)
 *			 Type of request (XDR_INT) (4 - disk, 3 - printer)
 *			 Device to link  (XDR_BYTES max 12) (printer name)
 *			 Username/Password (XDR_BYTES max 64)
 */
int
xdr_auth_request(xdrsp, auth)
	XDR *xdrsp;
	struct auth_struct *auth;
{
int   i, j, x, y;
char  *p;

	if (!xdr_int(xdrsp,&auth->ip))
		return(0);
	if (!xdr_int(xdrsp,&auth->type))
		return(0);
	i = 11;
	p = auth->device;
	if (!xdr_bytes(xdrsp,&p,&i, 12))
		return(0);
	auth->device[11] = ' ';
	p=strchr(auth->device, ' ');
	if (p == NULL)
		return(0);
	*p = '\0';
	i = 64;
	p = auth->login_text;
	if (!xdr_bytes(xdrsp,&p,&i,64))
		return(0);
	if (i < 6)
		auth->type = -1;
	p = auth->login_text;
	for (x= -1,j=0; j<i; j++) {
		y = *p;
		x ^= *p;
		*p++ = x;
		x = y;
	}
	auth->username = auth->login_text + 2;
	auth->password = auth->username + strlen(auth->username) + 1;
	if (debugmode) {
		fprintf(stdout, "bwnfsd: [xdr_auth_request] called\n");
		fprintf(stdout, "bwnfsd:   Link to %d.%d.%d.%d\n",
			   (auth->ip >> 24) & 0xff, (auth->ip >> 16) & 0xff,
			   (auth->ip >>  8) & 0xff, (auth->ip)	     & 0xff);
		fprintf(stdout, "bwnfsd:   Request type : %u\n", auth->type);
		fprintf(stdout, "bwnfsd:   Device name  : %s\n", auth->device);
		fprintf(stdout, "bwnfsd:   Username     : %s\n", auth->username);
/*
 *		fprintf(stdout, "bwnfsd:   Password     : %s\n", auth->password);
 */
		fprintf(stdout, "bwnfsd: [xdr_auth_request] returns (1)\n");
	}
	return(1);
}

struct name_struct {
	char usr_login[64];
	char *usr_name;
} name_request;

int
xdr_name_request(xdrsp, name)
	XDR *xdrsp;
	struct name_struct *name;
{
char *p;

	p = name->usr_login;
	name->usr_name = name->usr_login;
	if (!xdr_string(xdrsp,&p,32))
		return(0);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_name_request] called\n");
	return(1);
}

/*
 *	 Send successful authorization reply.
 *		 Results:
 *		   UID (XDR_INT)
 *		   GID (XDR_INT)
 *		   GID count (XDR_INT)
 *		   GIDs (XDR_INT)	 (GID count entries)
 */
int
xdr_pw(xdrsp,pwd)
	XDR   *xdrsp;
	struct passwd *pwd;
{
int   i;
struct mm_gidss *gg;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_pw] called\n");
	i  = 0;
	gg = get_gids(pwd->pw_uid,pwd->pw_gid,pwd->pw_name);
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: uid = %u, gid = %u, gids = %u\n", gg->mm_uid, gg->mm_gid, gg->mm_gids[0]);
	if (!xdr_int(xdrsp,&i))
		return(0);
	i = pwd->pw_uid;
	if (!xdr_int(xdrsp,&i))
		return(0);
	i = pwd->pw_gid;
	if (!xdr_int(xdrsp,&i))
		return(0);
	if (!xdr_int(xdrsp,&gg->mm_gid_count))
		return(0);
	for (i=0; i<gg->mm_gid_count; i++)
		if (!xdr_int(xdrsp,&gg->mm_gids[i]))
			return(0);

	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_pw] returns (1)\n");
	return(1);
}

int
xdr_get_printq(xdrsp,pq)
	XDR *xdrsp;
	struct printqs *pq;
{
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_get_printq] called\n");
	p = pq->queuename;
	if (!xdr_string(xdrsp,&p,32))
		return(0);
	p = pq->username;
	if (!xdr_string(xdrsp,&p,32))
		return(0);
	return(1);
}

int
xdr_cancel_job(xdrsp,pq)
	XDR *xdrsp;
	struct printqs *pq;
{
char *p;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_cancel_job] called\n");
	p = pq->queuename;
	if (!xdr_string(xdrsp,&p,32))
		return(0);
	p = pq->username;
	if (!xdr_string(xdrsp,&p,32))
		return(0);
	if (!xdr_int(xdrsp,&pq->printer_number))
		return(0);
	return(1);
}

int
xdr_send_printq(xdrsp,pq)
	XDR *xdrsp;
	struct printqs *pq;
{
int i, pn;
struct queueentry *qe;
char *pp;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_send_printq] called\n");
	pn = pq->printer_number;
	i = pn == -1;
	if (!xdr_int(xdrsp,&i))
		return(0);
	if (i != 0)
		return(1);
	pp = printers[pn].queuecomment;
	if (pp == NULL)
		pp = "";
	if (!xdr_string(xdrsp,&pp,32))
		return(0);
	if (!xdr_int(xdrsp,&printers[pn].queuestatus))
		return(0);
	if (!xdr_int(xdrsp,&printers[pn].numphysicalprinters))
		return(0);
	qe = printers[pn].head;
	i = 1;
	while (qe) {
		if ((*pq->username != '\0') && (strcmp(pq->username,qe->username) != 0)) {
			qe=qe->next;
			continue;
		}
		if (!xdr_int(xdrsp,&i))
			return(0);
		if (!xdr_int(xdrsp,&qe->jobid))
			return(0);
		pp = qe->username;
		if (pp == NULL)
			pp = "";
		if (!xdr_string(xdrsp,&pp,32))
			return(0);
		pp = qe->params;
		if (pp == NULL)
			pp = "";
		if (!xdr_string(xdrsp,&pp,255))
			return(0);
		if (!xdr_int(xdrsp,&qe->queueposition))
			return(0);
		if (!xdr_int(xdrsp,&qe->jobstatus))
			return(0);
		if (!xdr_long(xdrsp,&qe->timesubmitted))
			return(0);
		if (!xdr_long(xdrsp,&qe->size))
			return(0);
		if (!xdr_int(xdrsp,&qe->copies))
			return(0);
		pp = qe->jobcomment;
		if (pp == NULL)
			pp = "";
		if (!xdr_string(xdrsp,&pp,255))
			return(0);
		qe=qe->next;
	}
	i = 0;
	if (!xdr_int(xdrsp,&i))
		return(0);
	return(1);
}

#ifdef SCO
#define SecureWare 1
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#endif

char *
find_host(r_ip)
	u_long r_ip;
{
char *p;
u_long incomming_ip;
struct hostent *h;

	if (r_ip == my_ip)
		p = hostname;
	else {
		incomming_ip = ntohl(r_ip);
		h = gethostbyaddr((char *) &incomming_ip,4,AF_INET);
		if (h == NULL) {
			sprintf(remote_host,"%u.%u.%u.%u",
				       (incomming_ip	  ) & 0xff,
				       (incomming_ip >>  8) & 0xff,
				       (incomming_ip >> 16) & 0xff,
				       (incomming_ip >> 24) & 0xff);
			p = remote_host;
		} else
			p = h->h_name;
	}
	return(p);
}

struct user_cache_struct {
	char username[32];
	int uid, gid;
	char password[32];
	unsigned long successes;
} user_cache[NUM_USER_CACHE] = { 0 };

int cache_cycle=0;

void
add_cache(pw, p)
	struct passwd *pw;
	char *p;
{
int i,pos;
unsigned long smin= ~0L;

	pos = 0;
	cache_cycle = (cache_cycle+1) % NUM_USER_CACHE;
	for (i=0; i<NUM_USER_CACHE; i++)
		if ( (user_cache[i].successes < smin) ||
		     ((user_cache[i].successes == smin) && (i <= cache_cycle))) {
			pos=i;
			smin=user_cache[i].successes;
		}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [addcache] <%s> added to position %d replacing <%s(%ld)>\n",
			pw->pw_name, pos, user_cache[pos].username, user_cache[pos].successes);
	strcpy(user_cache[pos].username,pw->pw_name);
	strcpy(user_cache[pos].password,p);
	user_cache[pos].uid = pw->pw_uid;
	user_cache[pos].gid = pw->pw_gid;
	user_cache[pos].successes=1;
}

struct passwd *
getcache(request)
	struct auth_struct *request;
{
static struct passwd pas;
int i;

	for (i=0; i<NUM_USER_CACHE; i++) {
		if (strcmp(request->username, user_cache[i].username) == 0) {
			if (strcmp(crypt(request->password,user_cache[i].password), user_cache[i].password) == 0) {
				pas.pw_name = user_cache[i].username;
				pas.pw_uid = user_cache[i].uid;
				pas.pw_gid = user_cache[i].gid;
				user_cache[i].successes++;
				if (debugmode)
					fprintf(stdout, "bwnfsd: [getcache] <%s(%ld)> found in cache\n", request->username, user_cache[i].successes);
				return(&pas);
			} else {
				user_cache[i].username[0] = '\0';
				user_cache[i].successes = 0;
				if (debugmode)
					fprintf(stdout, "bwnfsd: [getcache] <%s> wrong password in cache\n", request->username);
				return(NULL);
			}
		}
	}
	return(NULL);
}

char *
reqname(req)
	int req;
{
static char buff[100];

	switch(req) {
	case NULLPROC:
		strcpy(buff, "Null Procedure");
		break;
	case SPOOL_INQUIRE:
		strcpy(buff, "Return fhandle of spool directory");
		break;
	case SPOOL_FILE:
		strcpy(buff, "Spool a file");
		break;
	case AUTHORIZE:
		strcpy(buff, "Authorize and return UID and GIDs");
		break;
	case GRP_NAME_TO_NUMB:
		strcpy(buff, "Convert group name to number");
		break;
	case GRP_TO_NUMBER:
		strcpy(buff, "Convert group number(s) to name(s)");
		break;
	case RETURN_HOST:
		strcpy(buff, "Convert IP to hostname");
		break;
	case UID_TO_NAME:
		strcpy(buff, "Convert UID(s) to name(s)");
		break;
	case NAME_TO_UID:
		strcpy(buff, "Convert name(s) to UID(s)");
		break;
	case SHARE:
		strcpy(buff, "DOS Share function");
		break;
	case UNSHARE:
		strcpy(buff, "DOS UnShare function");
		break;
	case LOCK:
		strcpy(buff, "DOS Lock request");
		break;
	case REMOVE:
		strcpy(buff, "Remove all locks/shares for PC");
		break;
	case UNLOCK:
		strcpy(buff, "DOS Unlock request");
		break;
	case GET_PRINTERS:
		strcpy(buff, "Get a list of printers");
		break;
	case GET_PRINTQ:
		strcpy(buff, "Get print queue entries and status");
		break;
	case CANCEL_PRJOB:
		strcpy(buff, "Cancel a print job");
		break;
	default:
		sprintf(buff, "Unknown request - %d", req);
		break;
	}
	return(buff);
}

/*
 *	 The main RPC routine.
 */
int
create_spool(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
struct passwd *pw;
int i,j;
u_long r_ip, i_ip;
char *p;
#ifdef SHADW
struct spwd *sp;
#endif
#ifdef SVR4_PRINTING
struct pr_passwd *prpw;
#endif

	if (debugmode)
		fprintf(stdout, "bwnfsd: [create_spool] called with %s\n", reqname(rqstp->rq_proc));
	switch(rqstp->rq_proc) {
	case NULLPROC:
		if (!svc_sendreply(transp, xdr_void, 0))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [nullproc] Can't reply to RPC call\n");
		return;
	case SPOOL_INQUIRE:
		if (!svc_sendreply(transp, xdr_file, &f))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [spool_inquire] Can't reply to RPC call\n");
		return;
	case SPOOL_FILE:
		if (!svc_getargs(transp,xdr_print, NULL)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [spool_file] svc_getargs request failed\n");
			svcerr_decode(transp);
			return;
		}
		i = 0;
		if (do_print == 0) {
			for (i=0; i<num_printers; i++)
				if (strcmp(printers[i].queuename,printer) == 0) {
					printers[i].lasttime = 0L;
					break;
				}
			if (debugmode)
				fflush(stdout);
			if ((i=fork()) == 0) {
				(void) signal(SIGHUP,SIG_IGN);
				(void) signal(SIGINT,SIG_IGN);
				(void) signal(SIGQUIT,SIG_IGN);
#ifdef SIGABRT
				(void) signal(SIGABRT,SIG_IGN);
#endif
				(void) signal(SIGTERM,SIG_IGN);
				print_it(file1,printer,jobname);
				exit(0);
			}
		}
		if (i != -1) {
			if (!svc_sendreply(transp, xdr_void, 0))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [spool_file] Can't reply to RPC call\n");
		} else
			if (debugmode)
				fprintf(stdout, "bwnfsd: [spool_file] Can't fork print process\n");
		return;
	case AUTHORIZE:
		if (!svc_getargs(transp,xdr_auth_request, &auth_request)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [authorize] svc_getargs request failed\n");
			svcerr_decode(transp);
			return;
		}
		if (auth_request.type == -1) {
			i = 1;
			if (!svc_sendreply(transp, xdr_int, &i))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] (1) Can't reply to RPC call\n");
			return;
		}
		if (A_flag == 0) {
			if (s_flag == 0) {
				if (ntohl(auth_request.ip) != my_ip) {
					i = 1;
					if (!svc_sendreply(transp, xdr_int, &i))
						if (debugmode)
							fprintf(stdout, "bwnfsd: [authorize] (2) Can't reply to RPC call\n");
					return;
				}
			} else {
				for (i=0; i<ips_count; i++) {
					if (((*(ips+i))->mask & ntohl(auth_request.ip)) == (*(ips+i))->ip) {
						switch((*(ips+i))->type) {
						case '+' :
							i=ips_count+1;
							break;
						case '-' :
							i = 1;
							if (!svc_sendreply(transp, xdr_int, &i))
								if (debugmode)
									fprintf(stdout, "bwnfsd: [authorize] (3) Can't reply to RPC call\n");
							return;
						case '=' :
							i_ip = (*(ips+i))->redirect_ip;
							if (!svc_sendreply(transp, xdr_new_ip, &i_ip))
								if (debugmode)
									fprintf(stdout, "bwnfsd: [authorize] (4) Can't reply to RPC call\n");
							return;
						}
					}
				}
				if (i == ips_count) {
					i = 1;
					if (!svc_sendreply(transp, xdr_int, &i))
						if (debugmode)
							fprintf(stdout, "bwnfsd: [authorize] (5) Can't reply to RPC call\n");
					return;
				}
			}
		}
#ifdef SCO
		if ((prpw=getprpwnam(auth_request.username)) == NULL) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [authorize] getprpwnam failed\n");
auth_error:
			i = 1;
			if (!svc_sendreply(transp, xdr_int, &i))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] (6) Can't reply to RPC call\n");
			return;
		}
		if (prpw->uflg.fg_lock && (prpw->ufld.fd_lock != 0))
			goto auth_error;
		i = prpw->ufld.fd_max_tries;
		if (i == 0)
			i = 5;
		if (prpw->ufld.fd_nlogins >= (short) i)
			goto auth_error;
		prpw->ufld.fd_encrypt[13] = '\0';
		if (strcmp(crypt(auth_request.password,prpw->ufld.fd_encrypt), prpw->ufld.fd_encrypt) != 0)
			goto auth_error;
		if ((pw=getpwnam(auth_request.username)) == NULL)
			goto auth_error;
#else
		if ((pw=getcache(&auth_request)) != NULL)
			goto got_pw;
		if ((pw=getpwnam(auth_request.username)) == NULL) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [authorize] getpwnam failed\n");
			i = 1;
			if (!svc_sendreply(transp, xdr_int, &i))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] (6) Can't reply to RPC call\n");
			return;
		}
#ifdef SHADW
		if ((sp=getspnam(auth_request.username)) == NULL) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [authorize] getspnam request failed\n");
			i = 1;
			if (!svc_sendreply(transp, xdr_int, &i))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] (7) Can't reply to RPC call\n");
			return;
		}
		p = sp->sp_pwdp;
#else
		p = pw->pw_passwd;
#endif
		if (strcmp(crypt(auth_request.password,p),p) != 0) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [authorize] crypt request failed\n");
			i = 1;
			if (!svc_sendreply(transp, xdr_int, &i))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] (8) Can't reply to RPC call\n");
			return;
		}
		add_cache(pw, p);
#endif
got_pw:
		if (auth_request.type == 3) {
			for (i=0; i<num_printers; i++)
				if (strcmp(printers[i].queuename,auth_request.device) == 0)
					break;
			if (i == num_printers) {
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] Printer %s not found\n",auth_request.device);
				i = 2;
				if (!svc_sendreply(transp, xdr_int, &i))
					if (debugmode)
						fprintf(stdout, "bwnfsd: [authorize] (9) Can't reply to RPC call\n");
				return;
			}
#ifdef SVR4_PRINTING
		       if (!check_access(i,auth_request.username)) {
				if (debugmode)
					fprintf(stdout, "bwnfsd: [authorize] Not authorized for %s\n",auth_request.device);
				i = 1;
				if (!svc_sendreply(transp, xdr_int, &i))
					if (debugmode)
						fprintf(stdout, "bwnfsd: [authorize] (10) Can't reply to RPC call\n");
				return;
			}
#endif
		}
		if (!svc_sendreply(transp, xdr_pw, pw))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [authorize] (11) Can't reply to RPC call\n");
		return;
	case GRP_NAME_TO_NUMB:
		if (!svc_getargs(transp,xdr_g_string, g_string)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [grp_name_to_numb] svc_getargs request failed\n");
			svcerr_decode(transp);
			return;
		}
		if (!svc_sendreply(transp, xdr_send_gid, g_string))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [grp_name_to_numb] Can't reply to RPC call\n");
		return;
	case GRP_TO_NUMBER:
		if (!svc_getargs(transp,xdr_ngids, lgids)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [grp_to_number] svc_getargs request failed\n");
			svcerr_decode(transp);
			return;
		}
		if (!svc_sendreply(transp, xdr_send_names, lgids))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [grp_to_number] Can't reply to RPC call\n");
		return;
	case RETURN_HOST:
		if (!svc_getargs(transp,xdr_int, &r_ip)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [return_host] svc_getargs request failed\n");
			svcerr_decode(transp);
			return;
		}
		p = find_host(r_ip);
		if (debugmode)
			fprintf(stdout, "bwnfsd: [return_host] remote = %s\n", p);

		if (!svc_sendreply(transp, xdr_g_string, p))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [return_host] Can't reply to RPC call\n");
		return;
	case UID_TO_NAME:
		if (!svc_getargs(transp,xdr_nuids, luids)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [uid_to_name] svc_getargs failed\n");
			svcerr_decode(transp);
			return;
		}
		if (!svc_sendreply(transp, xdr_send_usernames, luids))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [uid_to_name] Can't reply to RPC call\n");
		return;
	case NAME_TO_UID:
		if (!svc_getargs(transp, xdr_name_request, &name_request)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [name_to_uid] svc_getargs failed\n");
			svcerr_decode(transp);
			return;
		}
		if ((pw=getpwnam(name_request.usr_name)) == NULL) {
			i = 1;
			if (!svc_sendreply(transp, xdr_int, &i))
				if (debugmode)
					fprintf(stdout, "bwnfsd: [name_to_uid] (1) Can't reply to RPC call\n");
			return;
		}
		if (!svc_sendreply(transp, xdr_pw, pw))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [name_to_uid] (2) Can't reply to RPC call\n");
		return;
	case REMOVE:
		if (!svc_getargs(transp,xdr_get_lock_name,lock.name)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [remove] svc_getargs failed\n");
			svcerr_decode(transp);
			return;
		}
		process_lock_request((int) rqstp->rq_proc,lock.name,lock.fh,lock.access,
				     lock.mode,lock.offset,lock.length,lock.cookie);
		if (!svc_sendreply(transp,xdr_void,NULL))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [remove] Can't reply to RPC call\n");
		return;
	case SHARE:
	case UNSHARE:
		if (!svc_getargs(transp,xdr_share,&lock)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [%sshare] svc_getargs failed\n", rqstp->rq_proc==SHARE?"":"un");
			svcerr_decode(transp);
			return;
		}
		lock.stat = !process_lock_request((int) rqstp->rq_proc,lock.name,lock.fh,
				     lock.access,lock.mode,lock.offset,lock.length,lock.cookie);
		if (debugmode) {
			if (lock.stat)
				fprintf(stdout, "bwnfsd: [%sshare] lock status = false\n", rqstp->rq_proc==SHARE?"":"un");
			else
				fprintf(stdout, "bwnfsd: [create_spool] lock status = true\n");
		}
		if (!svc_sendreply(transp,xdr_shareres,&lock))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [%sshare] Can't reply to RPC call\n", rqstp->rq_proc==SHARE?"":"un");
		return;
	case LOCK:
		lock.stat = svc_getargs(transp,xdr_lock,&lock);
		goto check_lock;
	case UNLOCK:
		lock.stat = svc_getargs(transp,xdr_unlock,&lock);
	 check_lock:
		if (!lock.stat)
			if (!svc_getargs(transp,xdr_lock,&lock)) {
				if (debugmode)
					fprintf(stdout, "bwnfsd: [%slock] svc_getargs request failed\n", rqstp->rq_proc==LOCK?"":"un");
				svcerr_decode(transp);
				return;
			}
		lock.stat = !process_lock_request((int) rqstp->rq_proc,lock.name,lock.fh,
				     lock.access,lock.mode,lock.offset,lock.length,lock.cookie);
		if (debugmode)
			if (lock.stat)
				fprintf(stdout, "bwnfsd: [%slock] lock status = false\n", rqstp->rq_proc==LOCK?"":"un");
			else
				fprintf(stdout, "bwnfsd: [create_spool/check_lock] lock status = true\n");
		if (!svc_sendreply(transp,xdr_res,&lock))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [%slock] Can't reply to RPC call\n", rqstp->rq_proc==LOCK?"":"un");
		return;
	case GET_PRINTERS:
		if (!svc_sendreply(transp, xdr_printers, NULL))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_printers] Can't reply to RPC call\n");
		return;
	case GET_PRINTQ:
		if (!svc_getargs(transp,xdr_get_printq,&printq)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_printq] svc_getargs failed\n");
			svcerr_decode(transp);
			return;
		}
		printq.printer_number = -1;
		for (i=0; i<num_printers; i++)
			if (strcmp(printers[i].queuename,printq.queuename) == 0) {
				if (time(NULL) > (printers[i].lasttime+POLL_INTERVAL))
					add_entries(i);
				printq.printer_number = i;
				break;
			}
		if (!svc_sendreply(transp,xdr_send_printq,&printq))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [get_printq] Can't reply to RPC call\n");
		return;
	case CANCEL_PRJOB:
		if (!svc_getargs(transp,xdr_cancel_job,&printq)) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [cancel_prjob] svc_getargs failed\n");
			svcerr_decode(transp);
			return;
		}
		j = WN_BAD_QUEUE;
		for (i=0; i<num_printers; i++)
			if (strcmp(printers[i].queuename,printq.queuename) == 0) {
				for (j=0; j<MAX_NUM_CANCEL_CACHE; j++)
					if ((cancache[j].jobid == printq.printer_number) &&
					    (strcmp(cancache[j].username, printq.username) == 0) &&
					    (strcmp(cancache[j].queuename, printq.queuename) == 0)) {
						j = cancache[j].response;
						goto no_bloody_boolean_handy;
					}
				if ((j=cancel_printjob(i, printq.username, printq.printer_number)) == WN_SUCCESS)
					printers[i].lasttime = 0L;
				cancache[current_cache].jobid = printq.printer_number;
				strcpy(cancache[current_cache].queuename, printq.queuename);
				strcpy(cancache[current_cache].username, printq.username);
				cancache[current_cache].response = j;
				current_cache = ++current_cache%MAX_NUM_CANCEL_CACHE;
				break;
			}
no_bloody_boolean_handy:
		if (!svc_sendreply(transp,xdr_int,&j))
			if (debugmode)
				fprintf(stdout, "bwnfsd: [cancel_prjob] Can't reply to RPC call\n");
		return;
	default:
		svcerr_noproc(transp);
		return;
	}
}

static struct fhstatus {
	int status;
	char dir[32];
} mount_return;

/*
 *	 Response from rpc.mountd for mount request of SPOOL directory.
 */
int
xdr_fhstatus(xdrsp, fhstat)
	XDR *xdrsp;
	struct fhstatus *fhstat;
{
int i;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_fhstatus] called\n");
	if (!xdr_int(xdrsp, &fhstat->status))
		return(0);
	if (fhstat->status == 0)
		for (i=0; i<32 ; i+=4)
			if (!xdr_int(xdrsp, &(fhstat->dir[0])+i))
				return(0);
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_fhstatus] returns (1)\n");
	return(1);
}

static struct text_string {
	int length;
	char *dir;
} request_text;

/*
 *	 Request to mount SPOOL directory.
 */
int
xdr_text(xdrsp, req)
	XDR *xdrsp;
	struct text_string *req;
{
	if (debugmode)
		fprintf(stdout, "bwnfsd: [xdr_text] mount request for %s\n", req->dir);
	if (!xdr_bytes(xdrsp, &req->dir, &req->length,255))
		return(0);
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [xdr_text] returns (1)\n");
	return(1);
}


/*
 *	 Use rpc.mountd to determine the fhandle of the spool directory,
 *	 Can be replaced with extracted code from rpc.mountd.
 */
int get_mount(dir,directory)
	char *dir,*directory;
{
struct hostent *hp;
struct timeval pentry_timeout, total_timeout;
struct sockaddr_in server_addr;
int sock= RPC_ANYSOCK;
register CLIENT *client;
enum clnt_stat clnt_stat;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_mount] called\n");
	if (debugmode > 1)
		fprintf(stdout, "bwnfsd: [get_mount] dir = \"%s\", directory = \"%s\"\n", dir, directory);
	request_text.length = strlen(directory);
	request_text.dir    = directory;
	if ((hp=gethostbyname(hostname)) == NULL) {
		fprintf(stdout, "bwnfsd: [get_mount] \"%s\" is not in the /etc/hosts file.\n", hostname);
		fprintf(stdout, "bwnfsd:             please entered it and re-run.\n");
		fflush(stdout);
		exit(-1);
	}
	pentry_timeout.tv_sec  = 3;
	pentry_timeout.tv_usec = 0;
	bcopy(hp->h_addr, (caddr_t) &server_addr.sin_addr,hp->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port   = 0;
	if ((client=clntudp_create(&server_addr, 100005L, 
		1L,pentry_timeout, &sock)) == NULL) {

		fprintf(stdout, "bwnfsd: [get_mount] clntupd_create failed\n");
		clnt_pcreateerror("clntudp_create");
		fflush(stdout);
		exit(-1);
	}
	if (bindresvport(sock, (struct sockaddr_in *) NULL))
		if (debugmode) fprintf(stdout, 
"bwnfsd: [get_mount] cannot allocate reserved port ... not critical.\n");

	client->cl_auth       = authunix_create_default();
	total_timeout.tv_sec  = 20;
	total_timeout.tv_usec = 0;
	clnt_stat	      = clnt_call(client, 1L, xdr_text, &request_text,
					   xdr_fhstatus, &mount_return,
					   total_timeout);
	if (clnt_stat != RPC_SUCCESS) {
		fprintf(stdout, "bwnfsd: [get_mount] clnt_stat failed\n");
		clnt_perror(client,"rpc");
		fflush(stdout);
		exit(-1);
	}
	auth_destroy(client->cl_auth);
	clnt_destroy(client);
	if (mount_return.status == 0L)
		bcopy(&mount_return.dir[0],dir,32);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [get_mount] returns mount status %u\n",mount_return.status);
	return(mount_return.status);
}


/*
 *	 Daemon should be killed with a simple KILL and not KILL -9 so
 *	 that it may unmap itself
 */
Void
die(unused)
	int unused;
{
	fprintf(stdout, "bwnfsd: [die] Daemon is dying\n\n");
	fflush(stdout);
	(void) pmap_unset(BW_NFS_PROGRAM, BW_NFS_VERSION);
	exit(1);
}


int
main(argc,argv)
	int argc;
	char **argv;
{
SVCXPRT *transp;
char *p,*prog;
int   i;
struct hostent *hp;

#ifdef SCO
	set_auth_parameters(1,"/");
	setluid(0);
#endif

	fprintf(stdout, "BWNFSD Authentication Daemon Vsn %s starting\n\n", VERSION);
	prog = argv[0];
	debugmode = 0;
	argc--;
	argv++;

another:
	if ((argc > 0) && (strcmp(*argv,"-A") == 0)) {
		argc--;
		argv++;
		A_flag=1;
		goto another;
	}

	if ((argc > 0) && (strcmp(*argv,"-d") == 0)) {
		argc--;
		argv++;
		debugmode++;
		goto another;
	}

	if ((argc > 0) && (strcmp(*argv,"-s") == 0)) {
		argc--;
		argv++;
		if (argc < 2) {
			fprintf(stdout, "usage: %s [-A] [-d] [-s file] spool_area_mount_point\n", prog);
			fflush(stdout);
			exit(1);
		}
		read_ips(*argv);
		s_flag = 1;
		argc--;
		argv++;
		goto another;
	}
	if (argc != 1) {
		fprintf(stdout,"usage: %s [-A] [-d] [-s file] spool_area_mount_point\n", prog);
		fflush(stdout);
		exit(1);
	}

	init_locks();
	hostname = (char *) malloc(255);
	(void) gethostname(hostname,255);
	if ((hp=gethostbyname(hostname)) == NULL) {
		fprintf(stdout, "bwnfsd: host \"%s\" is not in the /etc/hosts file\n", hostname);
		fprintf(stdout, "bwnfsd: Please add it to the file and restart %s\n", prog);
		fflush(stdout);
		exit(-1);
	}
	bcopy(hp->h_addr, (caddr_t) &my_ip,hp->h_length);
	if (get_mount(dir,*argv) != 0) {
		fprintf(stdout, "bwnfsd: Spool area mount point %s could not be mounted.\n",  *argv);
		fprintf(stdout, "bwnfsd: Check that %s is in the /etc/exports file & world-writable\n", *argv);
		fflush(stdout);
		exit(1);
	}

	if (!debugmode) {
		if (fork() != 0)
			exit(0);
		for (i=0; i<10; i++)
			close(i);
		if ((i=open("/dev/null", 2)) < 0)
			exit(1);
		dup2(i, 4);
		close(i);
		dup2(4, 0);
		dup2(4, 1);
		dup2(4, 2);
		close(4);
#ifndef SCO
#ifdef TIOCNOTTY
		i = open("/dev/tty", 2);
		if (i >= 0) {
		   (void) ioctl(i, TIOCNOTTY, 0);
		   close(i);
		}
#endif
#endif
	}

	(void) setpgrp(0,0);
	p = strcpy(spool_dir,*argv) + strlen(*argv);
	if (*(p-1) == '/')
		*(--p) = '\0';
	get_printers();
	get_groups();
	f.status     = 0;
	f.dir_size   = 32;
	f.dir_handle = dir;

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf(stdout, "bwnfsd: [main] Unable to create an RPC server\n");
		fflush(stdout);
		exit(1);
	}
	(void) pmap_unset(BW_NFS_PROGRAM, BW_NFS_VERSION);
	if (!svc_register(transp,BW_NFS_PROGRAM, BW_NFS_VERSION,create_spool, IPPROTO_UDP)) {
		fprintf(stdout, "bwnfsd: [main] Can't register BW-NFS service\n");
		fflush(stdout);
		exit(1);
	}
	(void) signal(SIGHUP, clear);
	(void) signal(SIGINT, die);
	(void) signal(SIGQUIT, die);
#ifdef SIGABRT
	(void) signal(SIGABRT, die);
#endif
	(void) signal(SIGTERM, die);
#ifdef SIGCHLD
	(void) signal(SIGCHLD, free_child);
#endif
#ifdef SIGCLD
#ifdef SCO
	(void) signal(SIGCLD, SIG_IGN);
#else
	(void) signal(SIGCLD, free_child);
#endif
#endif
	svc_run();
	fprintf(stdout, "bwnfsd: [main] svc_run returned!\n");
	fflush(stdout);
	exit(1);
}
