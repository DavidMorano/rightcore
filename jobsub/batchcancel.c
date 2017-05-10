/*
 * batchcancel [-i] [-q queue_name] [jobid...]
 *	Cancel pending or running batch jobs.
 *
 *      -q queue_name - the queue to be searched (default all).
 *      jobid - job number to be cancelled (default is most recent job).
 *		Jobs or queues may be ed(1) style regular expressions.
 *	-i	- ask for confirmation before cancelling.
 */


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include	<time.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include "bat_common.h"

char *progname, *q;
int debug, verify, nfound, ncancelled, lastjob, first;



int main(argc, argv) 
int	argc ; 
char	*argv[] ; 
{
	int c;
	char *qpattern = NULL;
	extern int optind, nextq();
	extern char *optarg;
#ifndef __STDC__
	extern char *ctime();
#endif
	static int qflag = 0;


	progname = argv[0];
	while ((c = getopt(argc, argv, "diq:")) != EOF)
		switch (c) {
		case 'd':
			debug++;
			break;
		case 'q':
			qpattern = optarg;
			break;
		case 'i':
			verify++;
			break;
		default:
			fprintf(stderr, "Usage: %s [-i] [-q queue] jobid ...\n",
				progname);
			exit(1);
		}
	if (argc == optind)
		lastjob++;	/* remove user's most recent job */
	if (optind == argc-1 && strcmp(argv[optind], "-") == 0)
		optind++;	/* remove all possible jobs */
	if (chdir(SPOOLDIR) < 0)
		p_error(SPOOLDIR);
	while ((qflag = nextq(qflag)) > 0) {
		if (q[0] == '.')
			continue;
		if (qpattern && !match(q, qpattern))
			continue;
		if (chdir(q) < 0)
			continue;
		if (chdir(Q_CFDIR) == 0) {
			doqueue(q, argv+optind);
			if (chdir("..") < 0)
				p_error("chdir ..");
		}
		if (chdir("..") < 0)
			p_error("chdir ..");
		if (lastjob < 0)
			break;
	}
	if (nfound == 0) {
		if (qpattern)
			fprintf(stderr, 
			"%s: %s: No matching queue/jobs found\n",
				progname, qpattern);

		else
			fprintf(stderr, "%s: No jobs to cancel\n", progname);

		exit(1);
	}
	exit(ncancelled > 0 ? 0 : 1);
}


char lastjobname[BUFSIZ];

doqueue(qname, joblist)
	char *qname;
	char **joblist;
{
	DIR *dir;
	char *job, **pp;
	struct_dir *dp;
	struct ustat stb;
	time_t lastmtime;


	if (debug)
		printf("Checking queue %s\n", qname);

	first = 1;
	lastmtime = 0;
	if ((dir = opendir(".")) == NULL)
		p_error(qname);
	while ((dp = readdir(dir)) != NULL) {
		job = dp->d_name;
		if (strncmp(job, "cf", 2) != 0)
			continue;
		if (debug)
			printf("Job %s\n", job);
		if (lastjob) {
			if (stat(job, &stb) == 0)
				if (stb.st_mtime > lastmtime) {
					lastmtime = stb.st_mtime;
					strcpy(lastjobname, job);
				}
			continue;
		}
		if (*joblist) {
			for (pp = joblist; *pp; pp++)
				if (match(job+2, *pp))
					goto okay;
			continue;
		}

	   okay:
		cancel(qname, job);
	}
	if (lastjob && lastjobname[0]) {
		lastjob = -1;
		cancel(qname, lastjobname);
	}
	closedir(dir);
}


cancel(qname, job)
char *qname, *job;
{
	FILE *fp;
	long t;
	int pgrp, c;
	char buf[BUFSIZ], *cp;
	struct ustat stb;
	static int uid = -1;
	extern int errno;
	extern char *uidname();

	if (debug) printf("cancel %s/%s\n", qname, job);
	if ((fp = fopen(job, "r+")) == NULL)
		return;
	fstat(fileno(fp), &stb);
	if (uid < 0)
		uid = getuid();
	if (uid && uid != stb.st_uid)
		{ fclose(fp); return; }
	for (cp = CANCEL_FIRSTLINE; *cp; cp++)
		if (getc(fp) != *cp)
			break;
	if (*cp == '\0')
		{ fclose(fp); return; } /* job already cancelled */
	nfound++;
	if (first) {
		printf(">>-- %s queue --<<\n", qname); 
		first = 0;
	}
	printf("Cancelling job %s, owner %s, submitted %s",
		job+2, uidname(stb.st_uid), ctime(&stb.st_ctime));
	/* print user's command list */
	while(fgets(buf, sizeof buf, fp) != NULL)
		if (strcmp(buf, CONTROL_STR) == 0)
			while ((c = getc(fp)) != EOF)
				putchar(c);
	if (verify) {
		printf("\nCancel? ");
		fflush(stdout);
		if (fgets(buf, sizeof buf, stdin) == NULL ||
		    (buf[0] != 'y' && buf[0] != 'Y'))
			{ fclose(fp); return; }
	}
	ncancelled++;
	rewind(fp);
	fputs(CANCEL_FIRSTLINE, fp);
	time(&t);
	fprintf(fp, "%s %s at %s",
		CANCEL_SECONDLINE, uidname(uid), ctime(&t));
#ifdef CANCEL_THIRDLINE
	fputs(CANCEL_THIRDLINE, fp);
#endif
	fflush(fp);
	ftruncate(fileno(fp), ftell(fp));
	fchmod(fileno(fp), 0744);	/* make it generally readable */
	fclose(fp);
	/* Try to read process group from ../ef* file, and kill it */
	sprintf(buf, "../e%s", job+1);
	if ((fp = fopen(buf, "r")) != NULL) {
		if (fscanf(fp, "pgrp %d", &pgrp) == 1) {
			printf("Job is running; killing it.\n");
			(void) killpg(pgrp, SIGHUP);
			(void) killpg(pgrp, SIGCONT);
			sleep(1);
			if (killpg(pgrp, SIGKILL) < 0 && errno != ESRCH) {
				fprintf(stderr, "%s: Can't kill running job (process group %d)", progname, pgrp);
				perror("");
			}
		}
		fclose(fp);
	}
}


p_error(s)
	char *s;
{
	fprintf(stderr, "%s: ", progname);
	perror(s);
	exit(1);
}


char * uidname(uid)
{
	struct passwd *pw;
	static char buf[16];

	if ((pw = getpwuid(uid)) != NULL)

		return pw->pw_name;

	sprintf(buf, "#%d", uid);
	return buf;
}


match(string, pattern)
char *string, *pattern;
{
#ifdef SUNOS5
#include <libgen.h>
	char *p, *re = regcmp(pattern, 0);
	if (re == 0)
		return 0;
	p = regex(re, string);
	free(re);
	return p != 0;
#else
#ifdef SYSVR4
#include <regex.h>
	int status;
	regex_t re;

	if (regcomp(&re,pattern,REG_EXTENDED|REG_NOSUB) != 0)
		return(0);

	status = regexec(&re,string,(size_t)0,NULL,NULL);
	regfree(&re);
	if (status != 0)
		return(0);

	return(1);
#else
	extern char *re_comp();

	return re_comp(pattern) == 0 && re_exec(string) == 1;
#endif
#endif /* SUNOS5 */
}


int nextq(qflag)
int qflag;
{
	static FILE *fp;
	static char buf[BUFSIZ];
	static DIR *dir;
	static struct_dir *dp;

	/* First time, try to open SPOOLDIR/.queueorder first,
	 * then read the SPOOLDIR directory if that fails.
	 */
	if (qflag == 0) {
		if ((fp = fopen(".queueorder", "r")) != NULL) 
			qflag = 1;
		else if ((dir = opendir(".")) != NULL)
			qflag = 2;
		else
			p_error(SPOOLDIR);
	}

	if (debug)
		printf("In nextp, qflag is %d\n", qflag);

	/* Read the next line of the .queueorder file */
	if (qflag == 1) {
/*		if ((q = fgets(buf, sizeof buf, fp)) == NULL) {	*/
		if (fscanf(fp, "%s", buf) == 1)
			q = buf;
		else {
			qflag = -1;
			fclose(fp);
		}
	}
	/* Read the next directory entry */
	else {
		if ((dp = readdir(dir)) != NULL)
			q = dp->d_name;
		else {
			qflag = -1;
			closedir(dir);
		}
	}

	return (qflag);
}

#if	defined(SUNOS5) && defined(COMMENT)
killpg(pg, sig)
{
	return kill(-pg, sig);
}
#endif



