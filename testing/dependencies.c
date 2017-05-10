


struct a {
	struct b	*bp ;
	int	a ;
} ;


struct b {
	struct a	*ap ;
	int	b ;
} ;


int a_init(ap)
struct a	*ap ;
{


	(void) memset(ap,0,sizeof(struct a)) ;

	return 0 ;
}






