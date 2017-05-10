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

delete(dummy,messnum)
   char dummy[];
   int messnum;
{
	messnum = messord[messnum];    /* convert to internal number */
	messdel[messnum] = 1;
	return(0);
}
