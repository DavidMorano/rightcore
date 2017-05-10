/* isalpha */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


/* exported subroutines */


int isalpha(int ch)
{
	int	mch = (ch | 040) ;
	return ((mch >= 'a') && (mch <= 'z')) ;
}
/* end subroutine (isalpha) */


