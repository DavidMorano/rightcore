
extern int	isdigitlatin(int) ;


static int maxidletime(pip,sp,sl)
struct proginfo	*pip ;
const char	*sp ;
int		sl ;
{
	int	rs = SR_OK ;
	int	v ;
	int		ch = MKCHAR(sp[0]) ;

	                            if (isdigitlatin(ch)) {
	                                rs = cfdecti(sp,sl,&v) ;

	                            } else if ((tolower(sp[0]) == 'i') ||
	                                (sp[0] == '-')) {
	                                v = INT_MAX ;

	                            } else
	                                rs = SR_INVALID ;

	if (rs >= 0)
		pip->maxidle = v ;

	return rs ;
}
/* end subroutine (maxidletime) */

