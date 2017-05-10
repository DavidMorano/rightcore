/* main */


import	java.util.* ;
import	java.awt.* ;
import	java.applet.* ;
import	damutil.* ;



/* local program data structures */

/**
 * Debug
 *
 * this class stores a debug flag used during debugging
 *
 *
 */

class Debug {
	static final int	debuglevel = 0 ;
}


/**
 * DefParam
 *
 **/

class DefParam {
	final static int	LINELEN = 40 ;
	final static int	NUMLINES = 100 ;
	final static int	MINLINES = 4 ;
	final static int	TIMEOUT = 10 ;
}


/**
 * ArgOpt
 *
 * this class stores argument option strings
 *
 */

class ArgOpt {
	final static String	argopts[] = {
	    "DEBUG",
	    "VERSION",
	} ;
}


/**
 * BattleLines
 *
 * this is the main class in the program used to create a 'MovingLines'
 * class and to animate it
 *
 */

public class Main {

	Thread		t = null ;



	public	Main() {

	    if (Debug.debuglevel > 0)
	        System.err.println(
	            "BattleLines/constructor: entered") ;

	} /* end constructor */

	public String getAppletInfo() {
	    return "BattleLines: David Morano, 1999\n" ;
	}

	public String[][] getParameterInfo() {

	    String[][] info = {
	        { "model", "model filename", "filename of model to render" },
	    } ;


	    return info ;
	}

	public static void main(String[] argv) {


	    int		argc = argv.length ;
	    int		intcount = 0 ;
	    int		i ;
	    int		linesleft ;


/* handle the invocation arguments */





	System.out.print("hello world\n") ;


	} /* end subroutine (main) */





}
/* end class (main) */



