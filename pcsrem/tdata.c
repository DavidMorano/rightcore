/* takes string "in" and returns 2 character array "out" with the
  zero-filled right-justified number.
*/

#include <stdio.h>
#include <ctype.h>


tdata (in,out)
 char *in,*out;
{
	int k,dout;

	/* check input */
	for (k=0; k<strlen(in); k++)
		if (!isdigit(in[k]))
		{     out[0]='0';  out[1]='0';    return;   }

	dout = atoi (in);
	if (dout > 99)
	{     out[0]='0';   out[1]='0';   return;   }

	/* get (at most) two digits */
	switch (strlen(in))
	{
	case 0:   out[0]='0';   out[1]='0';     break;
	case 1:   out[0]='0';   out[1]=in[0];   break;
	case 2:   out[0]=in[0]; out[1]=in[1];   break;
	}
	return;
}


	
