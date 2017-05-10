/* mroff.h */


struct mroff_flags {
	uint	concatenate : 1 ;
	uint	reference : 1 ;
	uint	headers : 1 ;
} ;

struct mroff {
	struct mroff_flags	f ;
	char			*headerstring ;
	char			*footerstring ;
	int			coffset ;
	int			xoffset, yoffset ;
	int			blanklines ;
	int			maxlines ;
} ;




