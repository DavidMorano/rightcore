/* rem_pgm: reminder service daemon
* J. A. Kutsch  ho 43233  x-3059
* July 1980
*/

#define REMDIR "/usr/add-on/pcs/lib/remdata"
#define REMUID 470
#include "time.h"
#include <stdio.h>
#include <signal.h>
#include "remfile.h"
#include <sys/types.h>

static time_t prev_time,now_raw;
char addcmd[50];

main()
{
     char remfile[50];
     char outmsg[256];
     struct tm *now, *wait5(), *currenttime();
     int msgfile;
     struct msgrec rec;
     int i;

     switch (fork())		/* TLH - now don't have to add an & */
         {
	 case -1:	fprintf (stderr, "rem_pgm: Cannot fork!\n");
	 		exit (2);
	 case 0:	break;
	 default:	exit (0);
	 }

     signal(SIGHUP, SIG_IGN);	/* TLH - when /etc/rc is done on 3B's, */
     signal(SIGINT, SIG_IGN);	/* init sends different signals than on */ 
     signal(SIGQUIT, SIG_IGN);	/* a vax. This fix is good for both machines. */

     setuid(REMUID);	/* set to reminder service uid */

     strcpy(remfile,REMDIR);
     strcat(remfile,"/remfile");
     strcpy(addcmd,REMDIR);
     strcat(addcmd,"/rem_add&");

     currenttime();
     prev_time=now_raw;
     for (;;) {
          now=wait5();	/* wait till 5 minute multiple */

          if ((msgfile=open(remfile,0))>=0) {	/* if open succeeds */
               /* read file and check for time */
               while (r_read(msgfile,rec.name,100)>0) {
                    if (telltime(now->tm_hour,now->tm_min,cv_t2i(rec.hour),cv_t2i(rec.min),cv_t2i(rec.t1),cv_t2i(rec.t2),
                        cv_t2i(rec.t3))) { /*if time to give reminder */
                         i=8;
                         while (rec.name[i]==' ') rec. name[i--]='\0';


                         sprintf(outmsg,"\nmessage from reminder service ...\n   %2.2s:%2.2s %s\n   Current time is %02d:%02d\nEOF\n",
                             rec.hour,rec.min,rec.text,now->tm_hour,now->tm_min);
                         notify (rec.name,outmsg);	/* send to user */
                    }
               }
               close (msgfile);
               sync();
               mc(prev_time);	/* check for new mail */
               prev_time=now_raw;	/* reset previous time */
          }
     }
}

struct tm *
wait5()
{
     struct tm *now, *currenttime();

     system (addcmd);	/* add new items and remove dups */

     now=currenttime();
     sleep (((4-(now->tm_min%5))*60)+(60-now->tm_sec));
     now=currenttime();
     return (now);
}

struct tm *
currenttime()
{
     time_t clock;
     struct tm *localtime();
     time(&clock);
     now_raw=clock;
     return(localtime(&clock));
}

r_read(f,s,n)
int f,n;
char *s;
{
     while (read(f,s,1)) {
          if (*s++ == '\n') {
               *--s = '\0';
               return(1);
          }
     }
     return(0);
}

telltime(ch,cm,th,tm,t1,t2,t3)
int ch,cm,th,tm,t1,t2,t3;
{
     /* check for current time = test time */
     /* or test time - t1 or t2 or t3 */
     /* odd test  times are truncated to previous 5 minute interval */

     int test, curr;

     tm -= (tm%5);	/* set back to previous 5 min */
     t1 -= (t1%5);	/* set back to previous 5 min */
     t2 -= (t2%5);	/* set back to previous 5 min */
     t3 -= (t3%5);	/* set back to previous 5 min */


     test = th*60 + tm;
     curr = ch*60 + cm;
#ifdef DBG
     printf ("*telltime: curr= %d:%d  rec= %d:%d\n",ch,cm,th,tm);
#endif

     if (curr==test || curr==test-t1 || curr==test-t2 || curr==test-t3)
          return(1);
     return(0);
}

cv_t2i(s)
char *s;
{
     char *num = "0123456789";
     int d1,d2;
     for (d1=0;d1<10;d1++)
          if (num[d1]==*s) break;
     s++;
     for (d2=0;d2<10;d2++)
          if (num[d2]==*s) break;
     return ((d1*10)+d2);
}
