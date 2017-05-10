

package	damutil ;


/**
DamUtil package
*
**/


import	java.lang.* ;
import	java.util.* ;



public class DamUtil {

	public final static int	sleep(int delta) {

		long	start, end ;


		start = System.currentTimeMillis() ;

		try {

	            Thread.sleep(delta * 1000) ;

	        } catch (Exception e) { }

		end = System.currentTimeMillis() ;

		return ((int) (end - start)) ;

	} /* end subroutine (sleep) */

	public final static int	msleep(int delta) {

		long	start, end ;


		start = System.currentTimeMillis() ;

		try {

	            Thread.sleep(delta) ;

	        } catch (Exception e) { }

		end = System.currentTimeMillis() ;

		return ((int) (end - start)) ;

	} /* end subroutine (msleep) */


}
/* end class (DamUtil) */


