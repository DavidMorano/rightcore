static int maxidletime(pip,sp,sl)
struct proginfo	*pip ;
const char	*sp ;
int		sl ;
{
	int	rs = SR_OK ;
	int	v ;


	                            if (isdigit(sp[0]))
	                                rs = cfdecti(sp,sl,&v) ;

	                            else if ((tolower(sp[0]) == 'i') ||
	                                (sp[0] == '-'))
	                                v = INT_MAX ;

	                            else
	                                rs = SR_INVALID ;

	if (rs >= 0)
		pip->maxidle = v ;

	return rs ;
}
/* end subroutine (maxidletime) */

