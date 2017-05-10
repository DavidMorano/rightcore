/* genname */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		T.S.Kennedy						
 *		J.Mukerji						
 *									

 ************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* generates the filename from the group, ignoring first char prefix */

int genname (group,groupfile,gsize)				/*JM030185*/
char *group;
char groupfile[];
int gsize;	/* max length of groupfile string */
{
	FILE *fp, *fopen();

	struct table *aname;

	int i;
	int glenth;	/* current length of groupfile string */
/*JM030185*/
	int flag,iflag,start;

	char *getenv(), filename[BUFSIZE];
	char list[256];
	char mlist[BUFSIZE];
	char *list1,*list2;


	strcpy(mlist,maillist);

	list1 = list2 = mlist;
	glenth = 0;						/*JM030185*/
	while (1) {

/* get mail.list directory */

	    if (*list2 == '\0') break;

	    while (*list2 != '\0') {

	        if (*list2 == ':') {

	            *list2++ = '\0';
	            break;
	        }
	        list2++;
	    }

/* check for valid group name */

	    if (*list1 == ':')
	        strcpy(filename,group) ;

	    else sprintf(filename,"%s/%s",list1,group);

	    fp = fopen(filename,"r") ;

	    if (fp != NULL) {

	        strcpy(groupfile,filename) ;

	        fclose(fp) ;

	        return 1 ;
	    }

/* translate group name eg, ~jam to ~j.a.miller */

	    start = 0 ;
	    flag = 0 ;
	    while ((iflag = findname(group,&start,TT_USER)) >= 0) {

	        aname = name[start] ;
	        if (*list1 == ':')
	            strcpy(filename,aname->realname) ;

	        else sprintf(filename,"%s/%s",list1,aname->realname) ;

	        fp = fopen (filename,"r") ;

	        if (fp != NULL) {

	            flag += 1 ;
	            if (flag == 1) {

/* assume first filename will not */
/* be longer gsize		  */
	                strcpy(groupfile,filename);

	                glenth = strlen(groupfile);

	            } else {

	                i = strlen(filename) + 1 ;

	                if ((glenth + i) < gsize) {

	                    sprintf(groupfile+glenth,":%s",
	                        filename);

	                    glenth += i;

	                } else {

	                    printf(
	                        "warning: truncated group list\n");

	                    fclose(fp);

	                    break;
	                }
	            }
	            fclose(fp);
	        }
	        start++;
	        if (iflag == 0) break;

	    }
	    if (flag > 0) return flag ;

	    list1 = list2;
	}

	return 0 ;
}
/* end subroutine (genname) */


