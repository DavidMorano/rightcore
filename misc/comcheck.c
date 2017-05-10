/* last modified %G% version %I% */

/* comment counter */

/*
	Author :
	Doug Daudelin
	08/29/88

*/


/*******************************************************************

It counts comment occurrences, rather than numbers of lines with comments,
and will also not be fooled by things like a '/*/' string - which is only
a comment beginning and not also a comment end.


*********************************************************************/


#include	<stdio.h>

int main()
{
	int c, count1, count2, arm1, arm2;

	count1 = count2 = arm1 = arm2 = 0;
	while ((c = getchar()) != EOF) {

		if(c == '\/') {	

			if(arm2 == 1) {

				++count2;
				arm2 = 0;

			} else {

				arm1 = 1;
			}

		} else if(c == '\*') {

			if(arm1 == 1) {

				++count1;
				arm1 = 0;

			} else {

				arm2 = 1;
			}

		} else {

			arm1 = arm2 = 0;
		}
	}
	printf("%d comment beginnings, %d comment endings\n", count1, count2);
}
