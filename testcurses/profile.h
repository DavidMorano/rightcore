/* profile definitions */

#define DRDMAILOPTS "+all_scan:+long_scan:+confirm:+page"

 /* user profile options */
struct profopts {
		 char name[30];
		 int  value;
} ;




 /* user profile options  (with defaults, 1=on  0=off) */
static struct profopts	userprof[] = {
	  "all_scan",    0,		/* scan mailbox when enter */
	  "each_scan",   0,		/* scan current mess before prompt */
	  "long_scan",   1,		/* 80 column scan (no=50 col) */
	  "fifo",        1,		/* mess ordering (no=lifo) */
	  "sort_all",    0,		/* presort mess (no=in newbox only) */
	  "confirm",     0,             /* ask before deleting mess at quit */
	  "convert",     0,		/* convert UNIX message formats */
	  "page",	 0,		/* display one page at a time*/
	"replyinc",	0,		/* include original message in reply */
	  ""
} ;



