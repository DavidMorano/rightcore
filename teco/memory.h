/*	static char h_memory[] = "@(#) memory.h:  4.2 12/12/82";	*/
/* memory.h
**
** This header file mimics the operation of the /usr/include/memory.h
** in UNIX 5.0 systems if MEMFUNCTION is undefined.
*/

#ifdef MEMFUNCTION

#include <memory.h>			/* use standard include */

#else

extern char * memcpy();			/* copy bytes */
extern char * memchr();			/* find character */
extern int memcmp();			/* compare strings */

#endif
