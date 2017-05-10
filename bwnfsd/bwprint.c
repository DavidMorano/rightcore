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

BWPRINT.C provides the print spool services for BWNFS. This is where most
modification needs to be done as it is coded mainly for BSD systems, not
System V.

MODIFICATION HISTORY
--------------------
27/06/92 fjw  Made default_print readable by breaking out #defined sections
01/08/92 fjw  Merged in SVR4 code from Frank Glass (gatech!holos0!fsg)
31/08/92 cfb  Added code for queueing.	It's a little ugly :-)
31/08/92 fjw  Mucked about cleaning the ifdefs up and making queues right
31/08/92 fjw  Thanks to David Marceau for help with some obscure SCO bits
08/10/92 fjw  Fixed problem with forking processes failing
09/10/92 fjw  Added HPUX 8.07 support

*/

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

char *my_strdup();
char *find_host();
extern int debugmode;

#ifdef INT_SIG
#define Void int
#define Return return(0)
#else
#define Void void
#define Return return
#endif

#ifdef BSD_QUEUE
#   define LP_STATUS "/usr/ucb/lpq -P%s"
#   define LP_REMOVE "/usr/ucb/lprm -P%s %d", printers[thdebugprinter].queuename
#   define LP_REMSUCCESS "dequeued"
#endif
#ifdef AIX_QUEUE
#   define LP_STATUS "/bin/enq -q -P %s"
#   define LP_REMOVE "/bin/enq -P %s -x%d", printers[thdebugprinter].queuename
#   define LP_REMFAILURE "enq"
#endif
#if defined(SVR4_QUEUE)
#   define LP_STATUS "/usr/bin/lpstat -p%s"
#   define LP_REMOVE "/usr/bin/cancel %d"
#   define LP_REMSUCCESS "cancelled"
#endif
#ifdef SCO_QUEUE
#   define LP_STATUS "/usr/bin/lpstat -p%s -u"
#   define LP_REMOVE "/usr/bin/cancel %s-%d", printers[thdebugprinter].queuename
#   define LP_REMSUCCESS "cancelled"
#endif

#define PARSE_UNKNOWN	0
#define PARSE_BSD	1
#define PARSE_BWLPDOS	2
#define PARSE_AIX	3
#define PARSE_SCO	4

#define  WN_SUCCESS		0x00
#define  WN_NET_ERROR		0x02
#define  WN_ACCESS_DENIED	0x07
#define  WN_JOB_NOT_FOUND	0x41
#define  WN_BAD_QUEUE		0x43
#define  WN_ALREADY_LOCKED	0x46

int
my_fork(file)
char *file;
{
int counter=0, pid;

	srand((int)time(NULL));
	while (((pid=fork()) == -1) && (counter++ < 10)) {
		if (debugmode) {
			fprintf(stdout,"bwnfsd: [my_fork] had to sleep for %s\n",file);
			fflush(stdout);
		}
		sleep(rand()%10);
	}
	if ((pid == -1) && debugmode)
		fprintf(stdout,"bwnfsd: [my_fork] gave up on %s\n",file);

	return(pid);
}

#if defined(SVR4_PRINTING) || defined(HP8_PRINTING)
void
default_print(file, printer, jobname)
char *file, *printer, *jobname;
{
char *argys[20];
int ac, pid;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] called\n");

	ac = 0;
	argys[ac++] = "/usr/bin/lp";
	argys[ac++] = "lp";
	argys[ac++] = "-d";          /* destination printer */
	argys[ac++] = printer;
	argys[ac++] = "-c";          /* copy files rather than print inplace */
	if (!debugmode)
		argys[ac++] = "-s";  /* suppress request-id message */
	if (jobname != NULL) {
		argys[ac++] = "-t";  /* title */
		argys[ac++] = jobname;
	}
	argys[ac++] = file;
	argys[ac++] = NULL;

	chmod(file, 0777);
	signal(SIGCLD, SIG_IGN);
	if (debugmode) {
		fprintf(stdout, "bwnfsd: [default_print] execv(%s,\"", argys[0]);
		for (ac=1; argys[ac]!=NULL; ac++)
			fprintf(stdout, "%s ", argys[ac]);
		fprintf(stdout, "\")\n");
		fflush(stdout);
	}
	if ((pid=my_fork(file)) == 0) {
		execv(argys[0],&argys[1]);
		exit(1);
	}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] calling waitpid %d\n", pid);
	waitpid(pid, NULL, 0);

/*
 * If we get here, it means that the print failed. We unlink the
 * printfile to prevent lots of queued files. This may not be appropriate
 * for your application or for debugging purposes.
 */

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] unlinking print-file %s\n", file);
	(void) unlink(file);
}
#else

#ifdef AIX
void
default_print(file, printer, jobname)
char *file, *printer, *jobname;
{
char *argys[20];
int ac;
char zots[255], zoots[255];

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] called\n");

	ac = 0;
	argys[ac++] = "/bin/enq";
	argys[ac++] = "enq";
	strcpy(zots, "-P");           /* Printer name */
	strcat(zots, printer);
	argys[ac++] = zots;
	argys[ac++] = "-r";           /* remove file after printing */
	argys[ac++] = "-c";           /* copy files rather than print inplace */
	if (jobname != NULL) {
		strcpy(zoots, "-T");  /* title */
		strcat(zoots, jobname);
		argys[ac++] = zoots;
	}
	argys[ac++] = file;
	argys[ac++] = NULL;

	chmod(file, 0777);
	if (debugmode) {
		fprintf(stdout, "bwnfsd: [default_print] execv(%s,\"", argys[0]);
		for (ac=1; argys[ac]!=NULL; ac++)
			fprintf(stdout, "%s ", argys[ac]);
		fprintf(stdout, "\")\n");
	}
	execv(argys[0],&argys[1]);

/*
 *  If you are encountering defunct (zombie) processes, then
 *  this might be a good place to start looking
 */

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] print failed, unlinking %s\n", file);
	(void) unlink(file);
}
#else

#ifdef SGI
void
default_print(file, printer, jobname)
char *file, *printer, *jobname;
{
char *argys[20];
int ac;
char zots[255], zoots[255];

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] called\n");

	ac = 0;
	argys[ac++] = "/usr/bin/lp";
	argys[ac++] = "lp";
	strcpy(zots, "-d");            /* destination printer */
	strcat(zots, printer);
	argys[ac++] = zots;
	argys[ac++] = "-c";            /* copy files rather than print inplace */
	if (!debugmode)
		argys[ac++] = "-s";    /* suppress messages from lp */
	if (jobname != NULL) {
		strcpy(zoots, "-T");   /* title */
		strcat(zoots, jobname);
		argys[ac++] = zoots;
	}
	argys[ac++] = file;
	argys[ac++] = NULL;

	chmod(file,0777);
	if (debugmode) {
		fprintf(stdout, "bwnfsd: [default_print] execv(%s,\"", argys[0]);
		for (ac=1; argys[ac]!=NULL; ac++)
			fprintf(stdout, "%s ", argys[ac]);
		fprintf(stdout, "\")\n");
	}
	if (my_fork(file) == 0)
		execv(argys[0],&argys[1]);
	wait(NULL);

/*
 *  If you are encountering defunct (zombie) processes, then
 *  this might be a good place to start looking
 */

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] unlinking %s\n", file);
	(void) unlink(file);
}
#else

#ifdef SYSV32
void
default_print(file, printer, jobname)
char *file, *printer, *jobname;
{
char *argys[20];
int ac;
char zots[255], zoots[255];

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] called\n");

	ac = 0;
	argys[ac++] = "/usr/bin/lp";
	argys[ac++] = "lp";
	strcpy(zots, "-d");            /* destination printer */
	strcat(zots, printer);
	argys[ac++] = zots;
	argys[ac++] = "-c";            /* copy files rather than print inplace */
	if (!debugmode)
		argys[ac++] = "-s";    /* suppress messages from lp */
	if (jobname != NULL) {
#ifdef AVION
		strcpy(zoots, "-t");   /* title */
#else
		strcpy(zoots, "-T");   /* title */
#endif
		strcat(zoots, jobname);
		argys[ac++] = zoots;
	}
	argys[ac++] = file;
	argys[ac++] = NULL;

	chmod(file, 0777);
	if (debugmode) {
		fprintf(stdout, "bwnfsd: [default_print] execv(%s,\"", argys[0]);
		for (ac=1; argys[ac]!=NULL; ac++)
			fprintf(stdout, "%s ", argys[ac]);
		fprintf(stdout, "\")\n");
	}
	if (my_fork(file) == 0)
		execv(argys[0], &argys[1]);
	wait(NULL);

/*
 *  If you are encountering defunct (zombie) processes, then
 *  this might be a good place to start looking
 */

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] unlinking %s\n", file);
	(void) unlink(file);
}
#else

void
default_print(file, printer, jobname)
char *file, *printer, *jobname;
{
char *argys[20];
int ac;

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] called\n");

	ac = 0;
	argys[ac++] = "/usr/ucb/lpr";
	argys[ac++] = "lpr";
	argys[ac++] = "-P";             /* Printer destination */
	argys[ac++] = printer;
	argys[ac++] = "-r";             /* remove file after printing */
	if (jobname != NULL) {
		argys[ac++]="-T";       /* title */
		argys[ac++] = jobname;
	}
	argys[ac++] = file;
	argys[ac++] = NULL;

	if (debugmode) {
		fprintf(stdout, "bwnfsd: [default_print] execv(%s,\"", argys[0]);
		for (ac=1; argys[ac]!=NULL; ac++)
			fprintf(stdout, "%s ", argys[ac]);
		fprintf(stdout, "\")\n");
	}
	execv(argys[0],&argys[1]);

/*
 * If we get here, it means that the print failed. We unlink the
 * printfile to prevent lots of queued files. This may not be appropriate
 * for your application or for debugging purposes.
 */

	if (debugmode)
		fprintf(stdout, "bwnfsd: [default_print] print failed, unlinking print-file %s\n", file);
	(void) unlink(file);
}
#endif
#endif
#endif
#endif

void
print_it(file, printer, jobname)
char *file, *printer, *jobname;
{
struct printies {
	char *name;
	void (*routine)();
};

static	struct printies print_list[] = {
	"lp", default_print,
/*
 *	"your printer", your_printer_routine,
 */
};

#define PRINT_COUNT sizeof(print_list)/sizeof(struct printies)

int	i,tmp;
struct ustat buf;

	if (debugmode) {
		fprintf(stdout, "bwnfsd: [print_it] called\n");
		fprintf(stdout, "bwnfsd: Filename = %s\n", file);
		fprintf(stdout, "bwnfsd: Jobname = %s\n", jobname);
		fprintf(stdout, "bwnfsd: Printer = %s\n", printer);
	}
	if (stat(file,&buf) == 0) {
		setgid(buf.st_gid);
		setuid(buf.st_uid);
	}

	for (i=0; i<PRINT_COUNT; i++)
		if (strcmp(printer,print_list[i].name) == 0) {
			if (debugmode)
				fprintf(stdout, "bwnfsd: [print_it] using %s\n", printer);
					  (*print_list[i].routine)(file,printer,jobname);
			return;
		}
	if (debugmode)
		fprintf(stdout, "bwnfsd: [print_it] using default_print\n");
	default_print(file, printer, jobname);
}

#define WNPRQ_ACTIVE	0
#define WNPRQ_PAUSE	1
#define WNPRQ_ERROR	2
#define WNPRQ_PENDING	3
#define WNPRQ_PROBLEM	4

#define WNPRJ_QS_QUEUED 0
#define WNPRJ_QS_PAUSE	1
#define WNPRJ_QS_SPOOLING 2
#define WNPRJ_QS_PRINTING 3

#define WNPRJ_DS_COMPLETE	0x8
#define WNPRJ_DS_INTERV 	0x10
#define WNPRJ_DS_ERROR		0x20
#define WNPRJ_DS_DESTOFFLIN	0x40
#define WNPRJ_DS_DESTPAUSED	0x80
#define WNPRJ_DS_NOTIFY 	0x100
#define WNPRJ_DS_DESTNOPAPER	0x200
#define WNPRJ_DS_DESTFORMCHG	0x400
#define WNPRJ_DS_DESTCRTCHG	0x800
#define WNPRJ_DS_DESTPENCHG	0x1000

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
};

extern struct printertype printers[];

void
free_printer(thdebugprinter)
	int thdebugprinter;
{
struct queueentry *p1, *p2;

	p1 = printers[thdebugprinter].head;
	while (p1) {
		p2 = p1->next;
		if (p1->username)
			free(p1->username);
		if (p1->params)
			free(p1->params);
		if (p1->jobcomment)
			free(p1->jobcomment);
		free(p1);
		p1 = p2;
	}
	printers[thdebugprinter].head = NULL;
}

int no_response=0;
#ifdef SYSV32
pid_t child_pid;
#else
int child_pid;
#endif
FILE *thepipe;
#ifndef SCO
struct itimerval timer;
#endif

static Void
cancel_pipe(unused)
	int unused;
{
	no_response = 1;
	fclose(thepipe);
	kill(child_pid, SIGKILL);
	Return;
}

void
start_pipe_timer(n)
	int n;
{
	signal(SIGALRM, cancel_pipe);
#ifndef SCO
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = n;
	setitimer(ITIMER_REAL, &timer, NULL);
#else
	alarm(n);
#endif
	no_response = 0;
}

void
stop_pipe_timer()
{
#ifndef SCO
	timer.it_interval.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	timer.it_value.tv_sec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
#else
	alarm(0);
#endif
	signal(SIGALRM, SIG_IGN);
}

FILE *
my_popen(thecmd, timeout)
	char *thecmd;
	int timeout;
{
int fds[2], parent_fd, child_fd, pid, i;

	if (pipe(fds) < 0)
		return(NULL);
	parent_fd = fds[0];
	child_fd = fds[1];
	if ((pid=fork()) == 0) {
		for (i=0; i<10; i++)
			if (i != child_fd)
				close(i);
		if (child_fd != 1) {
			dup2(child_fd, 1);
			close(child_fd);
		}
		dup2(1, 2);
		execl("/bin/sh", "sh", "-c", thecmd, NULL);
		exit(-1);
	}
	if (pid == -1) {
		close(parent_fd);
		close(child_fd);
		return(NULL);
	}
	child_pid = pid;
	close(child_fd);
	start_pipe_timer(timeout);
	thepipe = fdopen(parent_fd, "r");
	return(thepipe);
}

void
my_pclose(fd)
	FILE *fd;
{
int pid, status;

	stop_pipe_timer();
	fclose(fd);
	if (child_pid == -1)
		return;
	while (((pid=wait(&status)) != child_pid) && (pid != -1))
		;
	return;
}

char *
my_strstr(str1, str2)
	char *str1, *str2;
{
char *a, *b, *c, *d;
int len1=strlen(str1), len2=strlen(str2);

	if (len2 > len1)
		return(NULL);
	d = str1+(len1-len2)+1;
	a = str1;
	while (a < d) {
		if (*a == *str2) {
			b = a;
			c = str2;
			while (*c && (*b == *c)) {
				b++;
				c++;
			}
			if (*c == 0)
				return(a);
		}
		a++;
	}
	return(NULL);
}

void
add_entries(thdebugprinter)
	int thdebugprinter;
{
char buffer[1024], *m, *n;
FILE *p;
int systype=PARSE_UNKNOWN, dropx=0, jobposition;
struct queueentry *qe, **head= &printers[thdebugprinter].head;
u_long ip;

	free_printer(thdebugprinter);
	if (debugmode)
		fprintf(stdout, "bwnfsd: [add_entries] called on %s\n", printers[thdebugprinter].queuename);
	printers[thdebugprinter].queuestatus = WNPRQ_ACTIVE;
	sprintf(buffer, LP_STATUS, printers[thdebugprinter].queuename);
	if ((p=my_popen(buffer, 10)) == NULL)
		return;
	while (fgets(buffer, sizeof(buffer)-1, p) != NULL) {
		if (dropx > 0) {
			dropx--;
			continue;
		}
		if (strlen(buffer) == 0)
			continue;
		m = &buffer[strlen(buffer)-1];
		if (*m == '\n')
			*m-- = '\0';
		while ((m>=buffer) && isspace(*m))
			*m-- = '\0';
		if (m < buffer)
			continue;
		if ((my_strnicmp(buffer,"Warning",7) != 0) && ((m=strchr(buffer,':')) != NULL)) {
			for (n=buffer; n<m; n++)
				if (isspace(*n))
					break;
			if (n >= m)
				memcpy(buffer, m+2, strlen(m+2)+1);
		}
		if (my_strnicmp(buffer,"Warning",7) == 0) {
			systype = PARSE_BSD;
			if ((my_strstr(buffer,"no daemon") != NULL) && (printers[thdebugprinter].queuestatus == WNPRQ_ACTIVE))
				printers[thdebugprinter].queuestatus = WNPRQ_PROBLEM;
			if (my_strstr(buffer,"down") != NULL)
				printers[thdebugprinter].queuestatus = WNPRQ_PAUSE;
			continue;
		}
		if (my_strnicmp(buffer, "Rank ", 5) == 0) {
			systype = PARSE_BSD;
			continue;
		}
		if (strncmp(buffer, "*** Printer ", 12) == 0) {
			systype = PARSE_BWLPDOS;
			printers[thdebugprinter].queuestatus = WNPRQ_PROBLEM;
			dropx = 2;
			jobposition = 1;
			continue;
		}
		if (strncmp(buffer, "*** Exceeded ", 13) == 0) {
			systype = PARSE_BWLPDOS;
			printers[thdebugprinter].queuestatus = WNPRQ_PROBLEM;
			dropx = 2;
			jobposition = 1;
			continue;
		}
		if (strncmp(buffer, "   Job Id      Username", 23) == 0) {
			systype = PARSE_BWLPDOS;
			dropx = 1;
			jobposition = 1;
			continue;
		}
		if (strncmp(buffer, "Queue   Dev   Status   ", 23) == 0) {
			systype = PARSE_AIX;
			dropx = 1;
			continue;
		}
		if (strncmp(buffer, "printer ", 7) == 0) {
			systype = PARSE_SCO;
			jobposition = 1;
			if (my_strstr(buffer,"disabled") != NULL)
				printers[thdebugprinter].queuestatus = WNPRQ_PAUSE;
			continue;
		}
		switch (systype) {
		case PARSE_UNKNOWN:
			continue;
			break;
		case PARSE_AIX:
			if (strncmp(&buffer[14],"DOWN",4) == 0) {
				printers[thdebugprinter].queuestatus = WNPRQ_PROBLEM;
				continue;
			}
			if ((qe=(struct queueentry *) malloc(sizeof(struct queueentry))) == NULL)
				continue;
			qe->jobid = atoi(&buffer[24]);
			for (m= &buffer[48]; !isspace(*m); )
				m++;
			*m = '\0';
			qe->username = my_strdup(&buffer[48]);
			qe->queueposition = atoi(&buffer[76]);
			for (m= &buffer[28]; !isspace(*m); )
				m++;
			*m = '\0';
			qe->params = my_strdup(&buffer[28]);
			qe->size = atol(&buffer[67])*1024;
			qe->copies = atoi(&buffer[73]);
			if ((printers[thdebugprinter].queuestatus == WNPRQ_ACTIVE) && (qe->queueposition == 1))
				qe->jobstatus = WNPRJ_QS_PRINTING;
			else
				qe->jobstatus = WNPRJ_QS_QUEUED;
			m = &buffer[28];
			if ((n=strrchr(m, '/')) == NULL)
				n = m;
			else
				n++;
			if ((sscanf(n,"%lx",&ip) != 1) || (ip < 0x01000000L))
				qe->jobcomment = my_strdup("-");
			else {
				sprintf(buffer,"queued from PC \"%s\"",find_host(ip));
				qe->jobcomment = my_strdup(buffer);
			}
			qe->next = NULL;
			*head = qe;
			head = &qe->next;
			break;
		case PARSE_BWLPDOS:
			if ((qe=(struct queueentry *) malloc(sizeof(struct queueentry))) == NULL)
				continue;
			buffer[6] = '\0';
			qe->jobid = atoi(buffer);
			for (m= &buffer[16]; !isspace(*m); )
				m++;
			*m = '\0';
			qe->username = my_strdup(&buffer[16]);
			qe->queueposition = jobposition++;
			for (m= &buffer[31]; !isspace(*m); )
				m++;
			*m++ = '\0';
			qe->size = atol(&buffer[31]);
			if ((*m == '\0') || (*m == ' '))
				qe->params = my_strdup("-");
			else
				qe->params = my_strdup(m);
			qe->timesubmitted = time(NULL);
			qe->copies = -1;
			if ((printers[thdebugprinter].queuestatus == WNPRQ_ACTIVE) && (qe->queueposition == 1))
				qe->jobstatus = WNPRJ_QS_PRINTING;
			else
				qe->jobstatus = WNPRJ_QS_QUEUED;
			qe->jobcomment = my_strdup("-");
			qe->next = NULL;
			*head = qe;
			head = &qe->next;
			break;
		case PARSE_BSD:
			m = &buffer[strlen(buffer)-1];
			while (isspace(*m) && (m>buffer))
				m--;
			while (!isspace(*m) && (m>buffer))
				m--;
			*m = '\0';
			while (!isspace(*m) && (m>buffer))
				m--;
			if (m <= buffer)
				continue;
			if ((qe=(struct queueentry *) malloc(sizeof(struct queueentry))) == NULL)
				continue;
			qe->size = atol(m);
			while (isspace(*m) && (m>buffer))
				*m-- = '\0';
			m = buffer;
			while (isspace(*m) || isdigit(*m))
				m++;
			while (!isspace(*m))
				m++;
			*m++ = '\0';
			qe->queueposition = atoi(buffer);
			if ((printers[thdebugprinter].queuestatus == WNPRQ_ACTIVE) && (qe->queueposition == 1))
				qe->jobstatus = WNPRJ_QS_PRINTING;
			else
				qe->jobstatus = WNPRJ_QS_QUEUED;
			while (isspace(*m))
				m++;
			n = m;
			while (!isspace(*m))
				m++;
			*m++ = '\0';
			qe->username = my_strdup(n);
			while (isspace(*m))
				m++;
			n = m;
			while (!isspace(*m))
				m++;
			*m++ = '\0';
			qe->jobid = atoi(n);
			while (isspace(*m))
				m++;
			qe->params = my_strdup(m);
			qe->timesubmitted = time(NULL);
			qe->copies = -1;
			if ((n=strrchr(m, '/')) == NULL)
				n = m;
			else
				n++;
			if ((sscanf(n,"%lx",&ip) != 1) || (ip < 0x01000000L))
				qe->jobcomment = my_strdup("-");
			else {
				sprintf(buffer,"queued from PC \"%s\"",find_host(ip));
				qe->jobcomment = my_strdup(buffer);
			}
			qe->next = NULL;
			*head = qe;
			head = &qe->next;
			break;

		case PARSE_SCO:
			if (isspace(buffer[0]))
				continue;
			m = strchr(buffer,'-');
			if (m == NULL)
				continue;
			*m++ = '\0';
			if (strcmp(buffer,printers[thdebugprinter].queuename) != 0)
				continue;
			if ((qe=(struct queueentry *) malloc(sizeof(struct queueentry))) == NULL)
				continue;
			qe->jobid = atoi(m);
			for (; !isspace(*m); )
				m++;
			for (; isspace(*m); )
				m++;
			n = m;
			for (; !isspace(*m); )
				m++;
			*m++ = '\0';
			qe->username = my_strdup(n);
			qe->queueposition = jobposition++;
			for (; isspace(*m); )
				m++;
			qe->size = atol(m);
			for (; !isspace(*m); )
				m++;
			for (; isspace(*m); )
				m++;
			qe->params = my_strdup(m);
			qe->timesubmitted = time(NULL);
			qe->copies = -1;
			if (my_strstr(m," on") != NULL)
				qe->jobstatus = WNPRJ_QS_PRINTING;
			else
				qe->jobstatus = WNPRJ_QS_QUEUED;
			qe->jobcomment = my_strdup("-");
			qe->next = NULL;
			*head = qe;
			head = &qe->next;
			break;
		}
	}
	printers[thdebugprinter].lasttime = time(NULL);
	my_pclose(p);
}

int
cancel_printjob(thdebugprinter, username, jobid)
	int thdebugprinter, jobid;
	char *username;
{
struct queueentry *qe=printers[thdebugprinter].head;
char buffer[1024];
FILE *p;

	while (qe) {
		if (qe->jobid == jobid) {
			if (strcmp(qe->username, username) != 0)
				return(WN_ACCESS_DENIED);
			else
				break;
		}
		qe = qe->next;
	}
	if (qe == NULL)
		return(WN_JOB_NOT_FOUND);
	sprintf(buffer, LP_REMOVE, jobid);
	if ((p=my_popen(buffer, 10)) == NULL)
		return(WN_NET_ERROR);
	while (fgets(buffer, sizeof(buffer)-1, p) != NULL) {
		if (buffer[strlen(buffer)-1] == '\n')
			buffer[strlen(buffer)-1] = '\0';
#ifdef LP_REMSUCCESS
		if (my_strstr(buffer, LP_REMSUCCESS) != NULL) {
			my_pclose(p);
			return(WN_SUCCESS);
		}
#else
		if (my_strstr(buffer, LP_REMFAILURE) != NULL) {
			my_pclose(p);
			return(WN_JOB_NOT_FOUND);
		}

#endif
	}
	my_pclose(p);
#ifdef LP_REMSUCCESS
	return(WN_JOB_NOT_FOUND);
#else
	return(WN_SUCCESS);
#endif
}
