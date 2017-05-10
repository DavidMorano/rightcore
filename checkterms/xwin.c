/* checkterms */


#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include	<time.h>
#include <stdio.h>


/* max ttys 256 with a string "/dev/pts/######\0" */
#define MAX_PTS 256
char pts[MAX_PTS][16];
int numpts;


/* time constants */
#define SECOND 1
#define MINUTE (60*SECOND)
#define HOUR (60*MINUTE)

#define TEST 0

#if TEST

#define TIME (1*MINUTE)
#define END_TIME (40*MINUTE) 

#else

/* how often cron looks */
/* REAL time
#define END_TIME (10*HOUR)
*/
#define END_TIME (8*HOUR)

/* What is the longest to go before waking up */
#define TIME (END_TIME-(3*MINUTE*MAX_PTS))

#endif


#define BUFSIZ 1024


static void add_pts(pt)
char *pt;
{
	if( numpts >=  MAX_PTS) {
		fprintf(stderr,"cannot handle more than %d pts\n",MAX_PTS);
		exit(1);
	}
	sprintf(pts[numpts++],"/dev/%s",pt);
}

static void get_pts()
{
	char cmd[BUFSIZ];
	char buf[BUFSIZ];
	char user[BUFSIZ];
	char login[BUFSIZ];
	char ptstr[BUFSIZ];
	char rest[BUFSIZ];
	FILE *ptr;
 
	numpts = 0;
	/* cheap way to get the user id */
	sprintf(cmd,"whoami");
	if ((ptr = popen(cmd, "r")) != NULL){
		if( fgets(buf, BUFSIZ, ptr) == NULL ){
			fprintf(stderr,"could not read cmd %s\n",cmd);
			exit(2);
		}
		sscanf(buf,"%s",&user);
#if TEST
		(void) printf("user = %s\n", user);
#endif
	}
	pclose(ptr);

	/* find out which ttys the user is on */
	sprintf(cmd,"who -a | sed -e s/+// | sed -e s/-//"); 
	if ((ptr = popen(cmd, "r")) != NULL){
		while (fgets(buf, BUFSIZ, ptr) != NULL){
			sscanf(buf,"%s %s",&login,&ptstr);
			if( strcmp(user,login) == 0 ){
#if TEST
				(void) printf("%s %s\n", login, ptstr);
#endif
				add_pts(ptstr);
			}
		}
	}
	pclose(ptr);

}

/* when to wake up to start the update process */
/* also remebers how long we were asleep */
unsigned wakeuptime=0;

/* need the pointer for the rand_r() routine */
unsigned seed = 0;


/* use numttys for two purposes */
/* if it is 0 then set the next wakeup time */
/* if >0 use it to set the remaining average time */

static unsigned next_time(numttys)
int numttys;
{
	unsigned time;
	unsigned average;


	/* do the work in minutes since who reports to the closest minute */
	if( numttys == 0 ){
		/* set the longest we want to wait */
		average = TIME/MINUTE;
	}
	else {
		/* time per process to guarentee we get done */
		average = (END_TIME - wakeuptime)/ (numttys*MINUTE);
		/* don't divde by zero */
		/* should probably print a message here but it would mean 3600 ttys */
		if( average <= 0 ){
			average = 2*MINUTE;
		}
	}
#if TEST
	printf("average time=%d minutes\n",average);
#endif
	time = (unsigned) (rand_r(&seed)%average);
	if( time <= 0 ){
		time = 1;
	}

	/* back to seconds */
	time *= MINUTE;

#if TEST
	fprintf(stderr,"next_time=%d\n",time);
#endif
	return time;
}

/* dummy for the signal handling stuff */
static void touch_int_handler(arg)
int arg;
{
#if TEST
	fprintf(stderr,"Here I am in touch arg=%d\n",arg);
#endif
}

static void alarm_int_handler(arg)
int arg;
{
	char cmd[BUFSIZ];
	int i;

#if TEST
	fprintf(stderr,"Here I am in alarm_int_handler arg=%d\n",arg);
#endif

	/* get the pts the user is on */
	get_pts();
	for(i=0; i<numpts; i++){
		/* setup the command and wait a certain amount of time */
		(void) sprintf(cmd,"touch %s\n",pts[i]);
		(void) sigset( SIGALRM, touch_int_handler); 
		(void) alarm(next_time(numpts));
		(void) pause();
		/* do the command */
#if TEST
		printf("%s\n",cmd);
#endif
		if( system(cmd) !=0 ){
			fprintf(stderr,"sytem(%s) failed\n",cmd);
		}
	}

	/* go to sleep for a long time */
	(void) sigset( SIGALRM, alarm_int_handler); 
	wakeuptime = next_time(0);
	(void) alarm(wakeuptime);
}


int main()
{
/*
	sighold(SIGALRM);
	(void) sigset( SIGALRM, alarm_int_handler); 
*/
	alarm_int_handler(-1);
	while(1){
		(void) pause();
	}

	return 0 ;
}
/* end subroutine (main) */



