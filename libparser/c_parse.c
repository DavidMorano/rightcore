/* c_parse */



#include	<string.h>

#include	"localmisc.h"
#include	"c_parse.h"


/*  function c_parse - parses standard argv value into:
		{spec} : + or - for options; = for keywords; NULL if a value
		{opt}  : option name or keyword name; NULLSTR if just a value
		{value}: object of =; just a value; or NULL if option
*/


int c_parse(s,ps)
char		*s ;
struct c_parsed *ps ;
{


	ps->spec = *s ;
	switch (*s) {

	case PLUS:
	case MINUS:
	    ps->opt = s+1;
	    ps->value = NULLSTR;
	    break;

	default:
	    if ((ps->value = strchr(s,EQUAL)) != NULL) {

	        *ps->value = NULL;
	        ps->value += 1 ;
	        ps->opt = s;
	        ps->spec = EQUAL;

	    } else {

	        ps->value = s;
	        ps->opt = NULLSTR;

	    }
	}

	return (int) ps->spec ;
}
/* end subroutine (c_parse) */


