
#defin		CONBUFLEN	4000


struct conhead {
	long	magic ;
	long	systab ;
	long	nsys ;
	long	funtab ;
	long	nfun ;
	long	exptab ;
	long	nexp ;
	long	conbuf ;
	long	conbuflen ;
} ;

struct system {
	char	name ;
	char	gtdev ;
	char	gtdev2 ;
	char	filtfile ;
	char	mapfile ;
	char	dictfile ;
} ;


