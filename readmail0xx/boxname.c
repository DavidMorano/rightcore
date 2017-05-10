/************************************************************************
 *                                                                      *
 * The information contained herein is for use of   AT&T Information    *
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   *
 *                                                                      *
 *     (c) 1984 AT&T Information Systems                                *
 *                                                                      *
 * Authors of the contents of this file:                                *
 *                                                                      *
 *                      Bruce Schatz                                    *
 *									*
 ***********************************************************************/
#include "pwd.h"
#include "defs.h"

 /* this file contains several short functions relating mailbox names to
   UNIX filenames.
 */


 /* returns name of the directory containing mailboxes */
char *  maildir ()
{
	static char temp[LINELEN];
	char *getenv();
	strcpy (temp,getenv("HOME"));
	strcat (temp,"/mail");
	return (temp);
}

/* returns complete UNIX pathname (fullname) of specified mailbox (boxname) */
full_boxname (fullname,boxname)
 char fullname[],boxname[];
{
	boxname= &boxname[strspn(boxname, " ")]; /*kill leading blanks */
	strtok (boxname, " ");		/* kill trailing blanks */
	if (boxname[0] == '\0') strcpy(boxname, "new");
	strcpy (fullname,maildir());
	strcat (fullname,"/");
	strcat (fullname,boxname);
}

 

 /* returns mailbox name (boxname) from complete UNIX pathname (fullname) */
mail_boxname (boxname,fullname)
   char boxname[],fullname[];
{
	char *strrchr();
	fullname= &fullname[strspn(fullname, " ")]; /*kill leading blanks */
	strcpy (boxname, strrchr(fullname,'/')+1);
}
