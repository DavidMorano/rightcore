/* Include file for extra stuff we stuck in for HP-UX 8.0  */
/*   By. G. Strachan Jan 1992                              */

/* The following resource limits aren't really defined in hp-ux but we */
/* fake them for now                                                   */

#define RLIMIT_CPU   0
#define RLIMIT_FSIZE 1
#define RLIMIT_DATA  2
#define RLIMIT_STACK 3
#define RLIMIT_CORE  4
#define RLIMIT_RSS   5

#define	RLIM_INFINITY	0x7fffffff
