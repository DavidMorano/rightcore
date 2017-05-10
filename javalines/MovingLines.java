/* MovingLines */


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
	static final int	debuglevel = 2 ;
}



/**
 * LineParam
 *
 * this class is used to store and pass arguments down to the 'Line' class
 *
 *
 */

class LineParam {
	double	width, height ;
	double	length ;
	double	speed ;
	Random	r ;
	Color	bg_color ;
}




/**
 * MovingLines
 *
 * this class implements the idea of a canvas thing with lines in it
 *
 */

public class MovingLines extends Canvas implements Runnable {

	final double	delta = 0.010 ;

	Line[]		lines ;

	    Random	r = new Random() ;

	LineParam	p = new LineParam() ;

	Graphics	g ;

	Color		bg_color ;

	Thread		t ;

	int		nlines ;
	int		alive ;

	boolean	f_lastline = false ;
	boolean		f_running = false ;
		boolean f_destroy = false ;

	public		MovingLines(int n,double speed,int length) {

	    lines = new Line[n] ;


	    if (Debug.debuglevel > 2)
	        System.err.println("MovingLines/constructor: entered") ;

	    alive = nlines = n ;

/* initialize all of the lines that we want */

	    p.r = r ;
	    p.speed = speed ;
	    p.length = (double) length ;

	} ; /* end constructor (MovingLines) */

	public void	init() {

	    Dimension size ;

	    int	i ;


	    size = getSize() ;

	    bg_color = getBackground() ;

	    g = getGraphics() ;

	    g.setPaintMode() ;

	    p.bg_color = bg_color ;

	    p.width = size.width ;
	    p.height = size.height ;

	    for (i = 0 ; i < nlines ; i += 1)
	        lines[i] = new Line(p) ;

	} ; /* end subroutine (init) */

	public void	start() {

		f_running = true ;
		if (t == null) {

			t = new Thread(this) ;

			t.start() ;

		}

	} /* end subroutine (start) */

	public void	stop() {

		f_running = false ;
		f_destroy = false ;

	} /* end subroutine (stop) */

	public void	destroy() {

		f_running = false ;
		f_destroy = true ;
		DamUtil.sleep(2) ;	/* wait for it to die */

	} /* end subroutine (destroy) */

/* go into a loop moving "Line"s until they are all gone */

	public void	run() {

		int	linesleft ;
		int	tics ;


	    tics = 0 ;
	    linesleft = nlines ;
	    while ((! f_destroy) && (linesleft > 1)) {

		while (! f_running)
	            DamUtil.sleep(1) ;

	            DamUtil.msleep((int) (delta * 1000.0)) ;

	            if (Debug.debuglevel > 4)
	                System.err.println("JavaLines/main: after sleep") ;

	            linesleft = movethem(delta) ;

	            if (((tics % 500) == 0) && (linesleft < 50))
	                remaining() ;

	            tics += 1 ;

	    } /* end while */

	} /* end subroutine (run) */

	int	movethem(double delta) {

	    int	i, j, count = 0 ;


/* move all of the lines */

	    for (i = 0 ; i < lines.length ; i += 1) {

	        if (lines[i] != null) {

	            count += 1 ;
	            lines[i].move(g,delta) ;

	        }

	    } /* end for */

/* see if any of the lines have intersected any of the others */

	    for (i = 0 ; i < lines.length ; i += 1) {

	        if (lines[i] != null) {

	            for (j = (i + 1) ; j < lines.length ; j += 1) {

	                if ((lines[j] != null) && 
	                    lines[i].intersect(lines[j])) {

	                    lines[j].erase(g) ;

	                    lines[j] = null ;
			count -= 1 ;

	                } /* end if */

	            } /* end for */

	        } /* end if */

	    } /* end for */

	    if (Debug.debuglevel > 2)
	        System.err.println("MovingLines/movethem: count=" + count) ;

	alive = count ;
	    return count ;

	} /* end subroutine (movethem) */

	public void	paint(Graphics g) {

	    int	i ;


	    if (Debug.debuglevel > 0)
	        System.err.println("Line/paint: entered") ;

	    for (i = 0 ; i < lines.length ; i += 1) {

	        if (lines[i] != null)
	            lines[i].draw(g) ;

	    } /* end for */

	if (alive < 50)
		remaining() ;

	    if (f_lastline)
	        lastline() ;

	} /* end subroutine (paint) */

	void	remaining() {

	    g.setColor(Color.black) ;

	    g.drawString("remaining",50,50) ;

	} /* end subroutine (remaining) */

	void lastline() {

	    f_lastline = true ;
	    g.setColor(Color.black) ;

	    g.drawString("Only One Line Segment Remaining",100,100) ;

	} /* end subroutine (lastline) */

	public int	linesleft() {

		return alive ;
	}

}
/* end class (MovingLines) */


/**
 * Line
 *
 * this class is used to store and operate on a line segment
 *
 */

class Line {

	double	x1, y1 ;
	double	x2, y2 ;
	double	vx, vy ;		/* speed in pixels/milliseconds */
	double	width, height ;		/* canvas dimensions */
	Color	fg_color, bg_color ;

	public	Line(LineParam p) {

	    double	angle, speed ;


	    width = p.width ;
	    height = p.height ;

	    bg_color = p.bg_color ;
	    fg_color = new Color(p.r.nextInt() & 0xFFFFFF) ;

	    if (Debug.debuglevel > 2)
	        System.err.println("Line/constructor: w=" + p.width) ;

	    x1 = p.width * p.r.nextDouble() ;
	    y1 = p.height * p.r.nextDouble() ;

	    if (Debug.debuglevel > 2)
	        System.err.println("Line/constructor: x1=" + x1) ;

	    angle = p.r.nextDouble() * 2.0 * Math.PI ;

	    x2 = x1 + p.length * Math.cos(angle) ;
	    y2 = y1 + p.length * Math.sin(angle) ;

	    speed = p.speed ;
	    if (speed <= 0.0) speed = 0.1 ;

	    angle = p.r.nextDouble() * 2.0 * Math.PI ;

	    vx = speed * Math.cos(angle) ;
	    vy = speed * Math.sin(angle) ;

	    if (Debug.debuglevel > 2)
	        System.err.println("Line/*: vx=" + vx + " vy=" + vy) ;

	};  /* end constructor (Line) */

	public void	draw(Graphics g) {

	    int	ix1, iy1, ix2, iy2 ;


	    ix1 = (int) (x1 + 0.5) ;
	    iy1 = (int) (y1 + 0.5) ;
	    ix2 = (int) (x2 + 0.5) ;
	    iy2 = (int) (y2 + 0.5) ;

	    g.setColor(fg_color) ;

	    g.drawLine(ix1,iy1, ix2,iy2) ;

	} ; /* end subroutine (draw) */

	public void	erase(Graphics g) {

	    int	ix1, iy1, ix2, iy2 ;

	    ix1 = (int) (x1 + 0.5) ;
	    iy1 = (int) (y1 + 0.5) ;
	    ix2 = (int) (x2 + 0.5) ;
	    iy2 = (int) (y2 + 0.5) ;

/*
		g.clearRect(0,100,100,100) ;
*/

	    g.setColor(bg_color) ;

/*
		g.fillRect(100,200,100,100) ;
*/

	    g.drawLine(ix1,iy1, ix2,iy2) ;

	} ; /* end subroutine (erase) */

	public void	move(Graphics g,double delta) {

	    if (Debug.debuglevel > 2) {

	        System.err.println("Line/move: entered x1="+x1+"y1="+y1) ;
	        System.err.println("Line/move: entered x2="+x2+"y2="+y2) ;

	    }

	    this.erase(g) ;

	    x1 += (delta * vx) ;
	    y1 += (delta * vy) ;

	    x2 += (delta * vx) ;
	    y2 += (delta * vy) ;

	    if ((x1 < 0.0) && (x2 < 0.0)) {

	        x1 += width ;
	        x2 += width ;

	    } else if ((x1 >= width) && (x2 >= width)) {

	        x1 -= width ;
	        x2 -= width ;

	    }

	    if ((y1 < 0.0) && (y2 < 0.0)) {

	        y1 += height ;
	        y2 += height ;

	    } else if ((y1 >= height) && (y2 >= height)) {

	        y1 -= height ;
	        y2 -= height ;

	    }

	    this.draw(g) ;

	    if (Debug.debuglevel > 2) {

	        System.err.println("Line/move: exiting x1="+x1+" y1="+y1) ;
	        System.err.println("Line/move: exiting x2="+x2+" y2="+y2) ;

	    }

	} ; /* end subroutine (move) */

	public boolean intersect(Line line) {

	    double	a1, b1, c1 ;
	    double	a2, b2, c2 ;
	    double	det ;
	    double	x, y ;


/* calculate our line's values */

	    a1 = y2 - y1 ;
	    b1 = x1 - x2 ;
	    c1 = y2 * x1 - y1 * x2 ;

/* calculate the other line's values */

	    a2 = line.y2 - line.y1 ;
	    b2 = line.x1 - line.x2 ;
	    c2 = line.y2 * line.x1 - line.y1 * line.x2 ;

/* now we calculate where the intersection point of the two lines is */

	    det = a1 * b2 - a2 * b1 ;
	    if ((det >= (- Double.MIN_VALUE)) && (det <= Double.MIN_VALUE))
	        return (((x1 - line.x1) <= Double.MIN_VALUE) &&
	            ((y1 - line.y1) <= Double.MIN_VALUE)) ;

	    x = (c1 * b2 - c2 * b1) / det ;
/* NOT NEEDED	y = (a1 * c2 - a2 * c1) / det ; */

/* now check if the intersection point is on both line segments */

	    return (within(this,x) && within(line,x)) ;

	} ; /* end subroutine (intersect) */

	private boolean within(Line line,double x) {

	    return (((x >= line.x1) && (x <= line.x2)) ||
	        ((x <= line.x1) && (x >= line.x2))) ;

	} ; /* end subroutine (within) */

}
/* end class (Line) */



