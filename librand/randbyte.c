/* randbyte */

/*
 *	Random byte interface to truerand()
 *	Matt Blaze 9/95
 *	8, 16, 32 really random bits, at about .35 bits per clock
 *      interrupt.
 *
 *	usage: 
 *		unsigned char r8;
 *		unsigned short r16;
 *		unsigned long r32;
 *		unsigned long trand8(), trand16(), trand32();
 *		r8=trand8();
 *		r16=trand16();
 *		r32=trand32();
 *
 *	randbyte() is the same as trand8().
 *	trand8() takes about .3 seconds on most machines.
 */
/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */



/* external subroutines */

	unsigned long raw_truerand();
	unsigned char *qshs();




unsigned long randbyte()
{
	unsigned long r[2];
	unsigned char *hash;


	r[0]=raw_truerand(); r[1]=raw_truerand();
	hash = qshs(r,sizeof(r));
	return ((int) (*hash)) & 0xff;
}

unsigned long trand8()
{
	return randbyte();
}

unsigned long trand16()
{
	return randbyte() ^ (randbyte()<<8);
}

unsigned long trand32()
{
	return randbyte() ^ (randbyte()<<8)
		^ (randbyte()<<16) ^ (randbyte()<<24);
}



