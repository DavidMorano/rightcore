
/* reads the time line and parse out the hour, min, and message.
hour,min are 2 character arrays, message is a string.
acceptable times are, e.g., :
"10 ",  "10:45", "10-11", "10 - 11"
returns 1 if successful, 0 if invalid time format.
*/

#include <stdio.h>
#include <ctype.h>

parse (timeline,hour,min,mess)
char *timeline,*hour,*min,*mess;
{
     char *tp,hourbuild[100],minbuild[100];
     int  nhour,nmin,k,hlen;

     tp = timeline;
     while (*tp == ' '  ||  *tp == '\t')     tp++;

     /* extract the hour */
     hlen = strcspn (tp," -:\0");
     for (k=0; k<hlen; k++)
          if (!isdigit (*tp))
               return(0);
          else	hourbuild[k] = *tp++;
     hourbuild[k] = NULL;

     while (*tp == ' ')   tp++;

     switch (*tp)
     {
     case '-':
     case NULL:   
          strcpy (minbuild,"0");     
          break;
     case ':' :   
          tp++;
          while (*tp == ' ')   tp++;
     default:     
          k=0;			/* extract minutes */
          while (isdigit(*tp)  &&  *tp != NULL)
               minbuild[k++] = *tp++;
          minbuild[k] = NULL;
     }


     /* length checks */
     if (strlen(hourbuild) == 0)    return(0);
     if (strlen(minbuild)  == 0)    strcpy (minbuild,"0");

     /* check for valid ranges */
     nhour = atoi (hourbuild);
     nmin =  atoi (minbuild);
     if (nhour < 0  ||  nhour > 23)   return(0);
     if (nmin  < 0  ||  nmin  > 59)   return(0);

     /* change hours,minutes into proper (2 character) format */
     tdata (hourbuild,hour);
     tdata (minbuild,min);

     /* rest of timeline is the message  (msp already set) */
     k=0;
     while (*tp != NULL)    mess[k++] = *tp++;
     mess[k] = '\0';

     return(1);
}
