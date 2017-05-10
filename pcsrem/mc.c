/* Modified by Mesut Gunduc on 11/28/84 */

/* mc(since_time): module to check for mail notification
* J. A. Kutsch  ho 43233  x-3059
* January 1981
*
* for all users in the active list,
* mc checks to see if their mail file has been written since
* the "since_time" time.
* if so, and if the length of the mail file is not zero,
* the user is notified that the mailbox has been written.
*/

#define REMDIR "/usr/add-on/pcs/lib/remdata"

#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
struct ustat sbuf;
mc(since)
time_t since;
{
     char mailfile[50];
     char userfile[50];
     char user[10];
     register FILE *ufile;

     strcpy(userfile,REMDIR);
     strcat(userfile,"/mc_users");
     if ((ufile=fopen(userfile,"r"))==0) return;
     while (fgets(user,10,ufile)!=0) {
          user[9]='\0';
          user[strlen(user)-1]='\0';
          strcpy (mailfile,"/usr/spool/mail/");
          strcat (mailfile,user);
          if (stat(mailfile,&sbuf)==0) {
               if (sbuf.st_mtime>=since && sbuf.st_size>0)
                    notify(user,"\n  new mail arrived\n");
          }
     }
     fclose (ufile);
}


