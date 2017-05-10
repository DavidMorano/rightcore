/* fdesc */


#ifndef	FDESC_INCLUDE
#define	FDESC_INCLUDE	1




#define	FDESC		struct fdesc_head
#define	FDESC_ARGS	struct fdesc_a




struct fdesc_a {
	int	fd ;
	int	pid ;
	int	flags ;
} ;

struct fdesc_head {
	unsigned long	magic ;
	int	fd ;
	int	pid ;
	int	flags ;
} ;


#endif /* FDESC_INCLUDE */



