/* BattleLines */


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
	final static double	SPEED = 300.0 ;
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

public class BattleLines extends Applet implements Runnable {

	static MovingLines	c ;

	Thread		t = null ;

	Graphics	g ;

	static double	speed = DefParam.SPEED ;

	static int	numlines = DefParam.NUMLINES ;
	static int	linelength = DefParam.LINELEN ;

	int		minlines = DefParam.MINLINES ;
	int		timeout = DefParam.TIMEOUT ;

	boolean		f_running = false ;
	boolean		f_destroy = false ;

	public	BattleLines() {

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

	    BattleLines	us = new BattleLines() ;

	    Random	r = new Random() ;

	    Frame	f ;

	    int		argc = argv.length ;
	    int		intcount = 0 ;
	    int		i ;
	    int		linesleft ;


/* handle the invocation arguments */

	    try {

	        if ((argc > 0) && (argv[0].charAt(0) != '-')) {

	            if (Debug.debuglevel > 0)
	                System.err.println(
	                    "BattleLines/main: numlines=" + argv[0]) ;

	            numlines = Integer.parseInt(argv[0]) ;

	            if (Debug.debuglevel > 0)
	                System.err.println(
	                    "BattleLines/main: numlines=" + numlines) ;

	        }

	        if ((argc > 1) && (argv[1].charAt(0) != '-')) {

	            Double	dd ;


	            if (Debug.debuglevel > 0)
	                System.err.println(
	                    "BattleLines/main: speed=" + argv[1]) ;

	            dd = Double.valueOf(argv[1]) ;

	            speed = dd.doubleValue() ;

	            if (Debug.debuglevel > 0)
	                System.err.println(
	                    "BattleLines/main: speed=" + speed) ;

	        }

	    } catch (Exception e) {

	        System.err.println("BattleLines: bad arguments given") ;

	    }


	    if (numlines < 2) 
		numlines = 2 ;

	    if (speed < 0.0) 
		speed = DefParam.SPEED ;


	            if (Debug.debuglevel > 0) {

	    System.out.println("numlines=" + numlines) ;

	    System.out.println("speed=" + speed) ;

		}


/* continue with the business part of the program */

	    f = new Frame("BattleLines") ;

	    f.setSize(500,500) ;

	    c = new MovingLines(numlines, speed,linelength,
		"Computer Science") ;

	    f.add(c,BorderLayout.CENTER) ;

	    f.setTitle("BattleLines") ;

	    f.setVisible(true) ;

	    us.g = f.getGraphics() ;

/* OK, we initialize the object ! */

	    c.init() ;

		us.drawinitial(us.g,"remaining") ;

/* start it up ! */

	    c.start() ;

/* wait for them to kill each other off */

	    while ((linesleft = c.linesleft()) > 1) {

	        if (Debug.debuglevel > 1)
	            System.out.println("lines left = " + linesleft) ;

	        DamUtil.sleep(4) ;

	    } /* end while */

/* we are almost done */

	    if (Debug.debuglevel > 1)
	        System.out.println("only one segment is left -- exiting") ;

	    DamUtil.sleep(10) ;

/* we end the horror ourself */

	    c.stop() ;

	    c.destroy() ;

	    System.exit(0) ;

	} /* end subroutine (main) */

	void	drawinitial(Graphics g,String s) {

	    g.setPaintMode() ;

	    g.setColor(Color.black) ;

	    g.drawString(s,50,50) ;

	} /* end subroutine (drawinitial) */

	public void	init() {

	    BorderLayout	layout = new BorderLayout() ;


	    if (Debug.debuglevel > 0)
	        System.err.println(
	            "BattleLines/init: entered") ;

	    c = new MovingLines(numlines, speed,linelength,
		"Computer Science") ;

	    setLayout(layout) ;

	    add(c,BorderLayout.CENTER) ;

	    validate() ;

	    c.init() ;

	} /* end subroutine (init) */

	public void	start() {

	    if (Debug.debuglevel > 0)
	        System.err.println(
	            "BattleLines/start: entered") ;

	    c.start() ;

	    f_running = true ;
	    if (t == null) {

	        t = new Thread(this) ;

	        t.start() ;

	    }

	} /* end subroutine (start) */

	public void	stop() {

	    if (Debug.debuglevel > 0)
	        System.err.println(
	            "BattleLines/stop: entered") ;

	    c.stop() ;

	    f_running = false ;

	} /* end subroutine (stop) */

	public void	destroy() {

	    if (Debug.debuglevel > 0)
	        System.err.println(
	            "BattleLines/destroy: entered") ;

	    synchronized (this) {
	        f_destroy = true ;
	    }

	    c.destroy() ;

	    while (f_destroy)
	        DamUtil.msleep(200) ;

	} /* end subroutine (destroy) */

	public void	run() {

	    long	daytime ;
	    long	lasttime = System.currentTimeMillis() ;

	    int	alive_old, alive_new ;

	    boolean f_restart = false ;


	    alive_old = numlines ;
	    while (! f_destroy) {

	        if ((! f_running) && (! f_destroy)) {

	        while ((! f_running) && (! f_destroy))
	            DamUtil.sleep(1) ;

	        	lasttime = System.currentTimeMillis() ;

		}

	        DamUtil.sleep(2) ;

	        alive_new = c.linesleft() ;

	        daytime = System.currentTimeMillis() ;

	        if (alive_new != alive_old)
	            lasttime = daytime ;

	        if ((alive_new < minlines) || (alive_new == alive_old)) {

	            if (((daytime - lasttime) / 1000) > timeout)
	                f_restart = true ;

	        }

	        alive_old = alive_new ;
	        synchronized (this) {

	            if (f_restart && (! f_destroy)) {

	                if (Debug.debuglevel > 0)
	                    System.err.println(
	                        "BattleLines/run: restarting") ;

	                f_restart = false ;
	                c.stop() ;

	                c.destroy() ;

	                c.init() ;

	                c.start() ;

	            } /* end if (restarting the lines) */

	        } /* end synchronized */

	    } /* end while */

	    f_destroy = false ;

	} /* end subroutine (run) */

}
/* end class (BattleLines) */



