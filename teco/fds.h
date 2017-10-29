/*	static char h_fds[] = "@(#) fds.h:  4.2 12/12/82";	*/
/* fds.h
 *
 *	Declare TECO file descriptor numbers.
 *	This file specifies the file descriptor numbers for various
 *	kinds of input/output that TECO does.
 */


#define FILEIN	0		/* file input for ER, EP commands. */
#define FILEOUT 1		/* file output for EW, EA commands. */
#define ERROUT 2		/* error message output:  standard
				 * TECO errors and internal errors */
#define TTYIN 3			/* terminal input */
#define TTYOUT 4		/* terminal output:  input echo,
				 * ^A, T :G n^T, V W = ? LF BS
				 */
#define EIFILE 5		/* input from EI command file */
