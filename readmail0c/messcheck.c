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
#include "defs.h"


 /* message marked for deletion */
#define  deleted(mn)   (messdel[messord[(mn)]])
 /* valid external messnum (hence also valid internal) */
#define  inrange(mn)   ((((mn) < 1) || ((mn) > nummess))  ?  0  :  1)


/* checking messages for validity and readjusting message pointer.
  messages marked for deletion won't ever be displayed.
*/





/*  boolean function.  returns 1 if message can be accessed, else 0.
   assumes is passed external messnum.
*/

messvalid (messnum)
   int messnum;
{
	if (! inrange (messnum))
	{
		printf("\n  message number out of range\n");
		return(0);
	}

	if (deleted (messnum))
	{
		printf("\n  message marked for deletion\n");
		return(0);
	}
	else   return(1);
}







/* move the global curr.msgno message pointer to the previous or next
  message, skipping over messages marked for deletion.
  if none, print message and leave pointer where is.
  returns  0 if successful,  1 if error.
*/

messnp (inc)
  int inc;	/* inc is -1 or +1 */
{
	int tempmsgno;
	char direction[10];

	tempmsgno = curr.msgno;
	curr.msgno +=  inc;
	while (inrange (curr.msgno))
	{
		if (! deleted (curr.msgno))
			return(0);	/* found undeleted */
		curr.msgno +=  inc;
	}

	curr.msgno = tempmsgno;
	strcpy (direction, ((inc < 0) ? "previous" : "next"));
	printf("\n  no %s message\n",direction);
	return(1);
}








/* find the next message not marked for deletion and adjust the global
  message pointer  (used after move and delete).
  if there is no next such message, go backwards to try to find one.
  if no undeleted at all, set curr.msgno to 0.
  returns  0 if successful,  1 if error.
*/

messadjust ()
{
	int tempmsgno;

	tempmsgno = curr.msgno;
	 /* try looking in forward direction */
	curr.msgno ++;
	while (inrange (curr.msgno))
	{
		if (! deleted (curr.msgno))
			return (0);		/* found undeleted */
		curr.msgno ++;
	}

	 /* try looking in backwards direction */
	curr.msgno =  tempmsgno;
	while (inrange (curr.msgno))
	{
		if (! deleted (curr.msgno))
			return (0);		/* found undeleted */
		curr.msgno --;
	}

	 /* no undeleted anywhere */
	curr.msgno = 0;
	return (1);
}
