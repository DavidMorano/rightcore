/* 'varargs.h' */

/*
	David A.D. Morano
	April 1983
*/

/*
	This header file will work for all machines which use a
	"normal" stack frame type structure.  The stack can grow
	up or down but the arguments must being at the low addresses
	and proceed to higher addresses on the stack.
*/


#define	va_alist		args
#define	va_list			char *
#define	va_dcl			int	args ;

#define va_start(list)		list = (char *) &va_alist
#define	va_arg(list,mode)	*((mode *) list) ; list += sizeof(mode)
#define va_end(list)


