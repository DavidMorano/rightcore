/* c_parse */


struct c_parsed	{	/* structure returned by c_parse */
	char spec;	/* + | - | = | NULLSTR		 */
	char *opt;	/* value following + | - ; preceeding =  */
	char *value;	/* value following = ; initial string if spec=NULLSTR */
} ;


#define EQUAL	'='
#define MINUS	'-'
#define PLUS	'+'
#define NULLSTR	""
#define NULLPTR	((char *) 0)


