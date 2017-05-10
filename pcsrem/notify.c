/* notify(u,m): notifies user "u" with message "m"
* J. A. Kutsch  ho 43233  x-3059
* January 1981
*
* if user is loggd more than once, notification is made to all terminals.
* notification is given without "Message from ..." preface.
* if messages are being denied, notify ignores that terminal
*/

#include "stdio.h"
#include	<sys/types.h>
#include	"utmp.h"
struct utmp	utmp;
notify(user,msg)
char *user, *msg;
{
     register FILE	*iop;
     FILE *port;
     char dev[25];
     char tty[9];
     int i;
     if ((	iop = fopen("/etc/utmp", "r"))!=0) {
          while(fread(&utmp, sizeof(utmp), 1, iop))
               if(strncmp(user, utmp.ut_name, 8) == 0) {
                    for (i=0;i<sizeof(utmp.ut_line);i++)
                         tty[i]=utmp.ut_line[i];
                    tty[i]='\0';
                    strcpy (dev,"/dev/");
                    strcat (dev,tty);
                    if ((port=fopen(dev,"w"))!=0) {
                         fprintf(port,"\n%s\n",msg);
                         fclose (port);
                    }
               }
          fclose (iop);
     }
}
