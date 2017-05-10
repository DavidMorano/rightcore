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
	static final int	debuglevel = 0 ;
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

	final double	delta = 0.005 ;

	Line[]		lines ;

	Random		r = new Random() ;

	LineParam	p = new LineParam() ;

	Graphics	g ;

	String		is ;

	Color		bg_color ;

	Thread		t = null ;

	int		alive ;

	boolean		f_lastline = false ;
	boolean		f_running = false ;
	boolean 	f_destroy = false ;

/* constructor */

	public	MovingLines(int n,double speed,int length,String s) {

	    lines = new Line[n] ;


	    if (Debug.debuglevel > 2) {

	        System.err.println("MovingLines/constructor: entered") ;

	        System.err.println("MovingLines/constructor: s=" + s) ;

		}

	    alive = n ;
		is = s ;

/* initialize all of the lines that we want */

	    p.r = r ;
	    p.speed = speed ;
	    p.length = (double) length ;

	} /* end constructor (MovingLines) */

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

	    for (i = 0 ; i < lines.length ; i += 1)
	        lines[i] = new Line(p) ;

	    g.setColor(Color.black) ;

	    g.drawString(is,50,50) ;

	} /* end subroutine (init) */

	public void	start() {

	    f_running = true ;
	    if (t == null) {

	        t = new Thread(this) ;

	        t.start() ;

	    }

	} /* end subroutine (start) */

	public void	stop() {

	    f_running = false ;

	} /* end subroutine (stop) */

	public void	destroy() {

	    f_running = false ;
	    f_destroy = true ;
	    while (f_destroy)
	        DamUtil.msleep(200) ;	/* wait for it to exit */

	    t = null ;

	} /* end subroutine (destroy) */

/* go into a loop moving "Line"s until they are all gone */

	public void	run() {

	    long	daytime ;
	    long	lastpaint = System.currentTimeMillis() ;

	    int	linesleft ;
	    int	tics ;


	    tics = 0 ;
	    linesleft = alive ;
	    while (! f_destroy) {

	        while ((! f_running) && (! f_destroy))
	            DamUtil.sleep(1) ;

	        DamUtil.msleep((int) (delta * 1000.0)) ;

	        if (Debug.debuglevel > 4)
	            System.err.println("JavaLines/main: after sleep") ;

		daytime = System.currentTimeMillis() ;

	        linesleft = movethem(delta) ;

	        if ((((daytime - lastpaint) / 1000) > 4) && 
			(linesleft < 50)) {

			lastpaint = daytime ;

/* we are NOT in the drawing thread so just schedule the actual draw */

			repaint() ;

		}

	    } /* end while */

	    f_destroy = false ;

	} /* end subroutine (run) */

	synchronized int	movethem(double delta) {

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

	public void	update(Graphics g) {

		paint(g) ;

	} /* end subroutine (update) */

/* this gets called from the drawing thread */

	synchronized public void	paint(Graphics g) {

	    int	i ;


	    for (i = 0 ; i < lines.length ; i += 1) {

	        if (lines[i] != null)
	            lines[i].draw(g) ;

	    } /* end for */

/*
	    if (alive < 50)
	        remaining() ;
*/

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

	double	ox1, oy1 ;
	double	ox2, oy2 ;

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

/* get our foreground color but it shouldn't be close to the background */

	    while (closeColor(fg_color,bg_color)) 
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

	} /* end constructor (Line) */

	public void	draw(Graphics g) {

	    int	ix1, iy1, ix2, iy2 ;


	    ix1 = (int) (x1 + 0.5) ;
	    iy1 = (int) (y1 + 0.5) ;
	    ix2 = (int) (x2 + 0.5) ;
	    iy2 = (int) (y2 + 0.5) ;

	    g.setColor(fg_color) ;

	    g.drawLine(ix1,iy1, ix2,iy2) ;

	} /* end subroutine (draw) */

	public void	eraseold(Graphics g) {

	    int	ix1, iy1, ix2, iy2 ;

	    ix1 = (int) (ox1 + 0.5) ;
	    iy1 = (int) (oy1 + 0.5) ;
	    ix2 = (int) (ox2 + 0.5) ;
	    iy2 = (int) (oy2 + 0.5) ;

	    g.setColor(bg_color) ;

	    g.drawLine(ix1,iy1, ix2,iy2) ;

	} /* end subroutine (erase) */

	public void	erase(Graphics g) {

	    int	ix1, iy1, ix2, iy2 ;

	    ix1 = (int) (x1 + 0.5) ;
	    iy1 = (int) (y1 + 0.5) ;
	    ix2 = (int) (x2 + 0.5) ;
	    iy2 = (int) (y2 + 0.5) ;

	    g.setColor(bg_color) ;

	    g.drawLine(ix1,iy1, ix2,iy2) ;

	} /* end subroutine (erase) */

	public void	move(Graphics g,double delta) {

	ox1 = x1 ;
	oy1 = y1 ;
	ox2 = x2 ;
	oy2 = y2 ;

//	    erase(g) ;

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

	eraseold(g) ;

	    draw(g) ;

//	eraseold(g) ;

	} /* end subroutine (move) */

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

	} /* end subroutine (within) */

/* we want to avoid colors "close" to the background color */

	private boolean	closeColor(Color a,Color b) {

	    int	closeness = 170 ;

	    int	a_red, a_blue, a_green ;
	    int	b_red, b_blue, b_green ;


	    a_red = a.getRed() ;
	    a_blue = a.getBlue() ;
	    a_green = a.getGreen() ;

	    b_red = b.getRed() ;
	    b_blue = b.getBlue() ;
	    b_green = b.getGreen() ;

	    if ((Math.abs(a_red - b_red) < closeness) &&
	        (Math.abs(a_blue - b_blue) < closeness) &&
	        (Math.abs(a_green - b_green) < closeness))
	        return true ;

	    return false ;

	} /* end subroutine (closeColor) */

}
/* end class (Line) */



