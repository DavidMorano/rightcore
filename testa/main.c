/* main (TESTA) */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<mallocstuff.h>

#include	<localmisc.h>



int main()
{
	int		a[20][2] ;
	int		typesize, size ;
	int		n, i, j ;
	int		*p ;
	char		*(*keys)[2] ;

	n = 2 * 20 ;

	typesize = sizeof(uint (*)[2]) ;

	printf("1 typesize=%u\n",typesize) ;

	typesize = sizeof(char *) ;

	printf("2 typesize=%u\n",typesize) ;

	size = n * typesize ;
	(void) uc_malloc(size,&keys) ;

	memset(keys,0,size) ;

	printf("typesize=%u n=%u size=%u\n",typesize,n,size) ;

	keys[0][0] = "hello" ;
	keys[0][1] = "there" ;

	keys[1][0] = "good" ;
	keys[1][1] = "bye" ;

	for (i = 0 ; i < 2 ; i += 1) {

		if (keys[i][0] != NULL)
	    printf("k=%s v=%s\n",
		keys[i][0],keys[i][1]) ;

	}


	size = n * sizeof(int) ;
	(void) memset(a,0,size) ;

	p = (int *) a ;
	for (i = 0 ; i < n ; i += 1)
		p[i] = i ;

	for (i = 0 ; i < 20 ; i += 1) {

		for (j = 0 ; j < 2 ; j += 1)
			printf("[%2d,%2d]=%d\n",
				i,j,a[i][j]) ;

	}

	sub(a,2) ;

	fclose(stdout) ;

	return 0 ;
}


int sub(a,rn)
int	rn ;
int	(*a)[2] ;
{
	int		i, j ;

	for (i = 0 ; i < 20 ; i += 1) {
	    for (j = 0 ; j < 2 ; j += 1) {
			printf("sub [%2d,%2d]=%d\n",
				i,j,a[i][j]) ;
	    }
	}

	return 0 ;
}


