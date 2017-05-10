/* profile */

/************************************************************************
 *                                                                      
 * The information contained herein is for use of   AT&T Information    
 * Systems Laboratories and is not for publications. (See GEI 13.9-3)   
 *                                                                      
 *     (c) 1984 AT&T Information Systems                                
 *                                                                      
 * Authors of the contents of this file:                                
 *                                                                      
 *                      Jishnu Mukerji                                  
 *									

 ***********************************************************************/


/* profile definitions */

#define DRDMAILOPTS "+all_scan:+long_scan:+confirm:+page"

 /* user profile options */
struct profopts {
		 char name[30];
		 int  value;
		 };




 /* user profile options  (with defaults, 1=on  0=off) */
static struct profopts userprof [] = {
	  "all_scan",    0,		/* scan mailbox when enter */
	  "each_scan",   0,		/* scan current mess before prompt */
	  "long_scan",   0,		/* 80 column scan (no=50 col) */
	  "fifo",        0,		/* mess ordering  (no=lifo) */
	  "sort_all",    0,		/* presort mess (no=in newbox only) */
	  "confirm",     0,             /* ask before deleting mess at quit */
	  "convert",     0,		/* convert UNIX message formats */
	  "page",	 0,		/* display one page at a time*/
	  ""
	  };


