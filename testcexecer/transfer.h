/* transfer */


/* revision history:

	= 1992-03-01, David A­D­ Morano

	This program was originally written.


*/


#ifndef	TRANSFER_INCLUDE
#define	TRANSFER_INCLUDE	1


struct fpstat {
	uint	in : 1 ;
	uint	out : 1 ;
	uint	hup : 1 ;
	uint	final : 1 ;
	uint	eof : 1 ;
} ;


#endif /* TRANSFER_INCLUDE */




