/* unix_truerand */

/*
 *	Almost "true" random numbers (very nearly uniform)
 *	Based on code by D. P. Mitchell
 *	Modified by Matt Blaze 7/95, 11/96
 *	Version 2.1
 *
 *	This is completely unsupported software.
 *
 */
/*
 * The authors of this software are Don Mitchell and Matt Blaze.
 *              Copyright (c) 1995, 1996 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 
 *
 * This software may be subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * Truerand is a dubious, unproven hack for generating "true" random
 * numbers in software.  It is at best a good "method of last resort"
 * for generating key material in environments where there is no (or
 * only an insufficient) source of better-understood randomness.  It
 * can also be used to augment unreliable randomness sources (such as
 * input from a human operator).
 *
 * The basic idea behind truerand is that between clock "skew" and
 * various hard-to-predict OS event arrivals, counting a tight loop
 * will yield a little bit (maybe one bit or so) of "good" randomness
 * per interval clock tick.  This seems to work well in practice even
 * on unloaded machines.  If there is a human operator at the machine,
 * you should augment trand with other measurements, such as keyboard
 * event timing.  On server machines (e.g., where you need to generate
 * a Diffie-Hellman secret but have no operator to type keys) trand
 * alone may (or may not) be sufficient.
 *
 * Because the randomness source is not well-understood, I've made
 * fairly conservative assumptions about how much randomness can be
 * extracted in any given interval.  Based on a cursory analysis of
 * the BSD kernel, there seem to be about 100-200 bits of unexposed
 * "state" that changes each time a system call occurs and that affect
 * the exact handling of interrupt scheduling, plus a great deal of
 * slower-changing but still hard-to-model state based on, e.g., the
 * process table, the VM state, etc.  There is no proof or guarantee
 * that some of this state couldn't be easily reconstructed, modeled
 * or influenced by an attacker, however, so we keep a large margin
 * for error.  The truerand API assumes only 0.3 bits of entropy per
 * interval interrupt, amortized over 24 intervals and whitened with
 * SHA.
 *
 * The trurand API is in randbyte.c, and consists of trand8(),
 * trand16(), and trand32().  Do not use raw_truerand() directly.
 * 
 * WARNING: depending on the particular platform, raw_truerand()
 * output may be biased or correlated.  In general, you can expect no
 * more than 8-16 bits of "pseudo-entropy" out of each 32 bit word.
 * Always run the output through a good post-whitening function (like
 * SHA, MD5 or DES or whatever) before using it to generate key
 * material.  The API does this for you, providing 8, 16, and 32 bit
 * properly "whitened" random numbers (trand8(), trand16(), and
 * trand32(), respectively).  Use the trand calls instead of calling
 * raw_truerand() directly.
 *
 * Test truerand on your own platform before fielding a system based
 * on this software or these techniques.
 *
 * This software seems to work well (at 10 or so bits per
 * raw_truerand() call) on a Sun Sparc-20 under SunOS 4.1.3 and on a
 * P100 under BSDI 2.0.  You're on your own elsewhere.
 *
 * This version of truerand doesn't clobber the ITIMER, so any
 * scheduled alarms will still occur (though alarms cannot occur while
 * raw_truerand is running and will be delayed by about 250ms per
 * raw_truerand call.
 */

#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#include <math.h>



static jmp_buf env;
static unsigned count;
static unsigned ocount;
static unsigned buffer;



static int tick()
{
	struct itimerval it;


	timerclear(&it.it_interval);
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 16665;
	if (setitimer(ITIMER_REAL, &it, NULL) < 0)
		perror("tick");
}

static void
interrupt()
{


	if (count)
		longjmp(env, 1);
	(void) signal(SIGALRM, interrupt);
	tick();
}

static unsigned long
roulette()
{


	if (setjmp(env))
		return count;
	(void) signal(SIGALRM, interrupt);
	count = 0;
	tick();
	for (;;)
		count++;	/* about 1 MHz on VAX 11/780 */
}

/*
 * basic interface to 32 bit truerand.
 * note that any scheduled SIGNALRM will be delayed by about .3 secs.
 */
unsigned long raw_truerand()
{
	void (*oldalrm)();
	struct itimerval it;
	unsigned long counts[12];
	unsigned char *qshs();
	unsigned char *r;
	unsigned long buf;
	int i;


	getitimer(ITIMER_REAL, &it);
	oldalrm = signal(SIGALRM, SIG_IGN);
	for (i=0; i<12; i++) {
		counts[i]=0;
		while ((counts[i] += roulette()) < 512)
			;
	}
	signal(SIGALRM, oldalrm);
	setitimer(ITIMER_REAL, &it, NULL);

	r = qshs(counts,sizeof(counts));
	buf = *((unsigned long *) r);

	return buf;
}
/* end subroutine (raw_truerand) */


int raw_n_truerand(n)
int n;
{
	int slop, v;

	slop = 0x7FFFFFFF % n;
	do {
		v = raw_truerand() >> 1;
	} while (v <= slop);
	return v % n;
}



