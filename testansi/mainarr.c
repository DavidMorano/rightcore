/* mainarr */

#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>


#define		N	2
#define		M	3

struct node {
	int	i, j ;
} ;

static int sub(int n,int m;struct node [n][m],int n,int m) ;

int main()
{
	struct node	a[N][M] ;
	int		i, j ;
	int		x, y ;
	int		n = N ;
	int		m = M ;

	for (i = 0 ; i < N ; i += 1) {
	   for (j = 0 ; j < M ; j += 1) {
		a[i][j].i = i ;
		a[i][j].j = j ;
	   }
	}

	for (i = 0 ; i < N ; i += 1) {
	   for (j = 0 ; j < M ; j += 1) {
		x = a[i][j].i ;
		y = a[i][j].j ;
		printf("%u,%u\n",x,y) ;
	   }
	}

	sub(a,n,m) ;

	return 0 ;
}
/* end subroutine (main) */

static int sub(int n,int m;struct node a[n][m],int n,int m)
{
	int	i, j ;
	int		x, y ;
	printf("sub\n") ;
	for (i = 0 ; i < n ; i += 1) {
	   for (j = 0 ; j < m ; j += 1) {
		x = a[i][j].i ;
		y = a[i][j].j ;
		printf("%u,%u\n",x,y) ;
	   }
	}
	return 0 ;
}

