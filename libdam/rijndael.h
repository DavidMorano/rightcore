/*	$OpenBSD: rijndael.h,v 1.12 2001/12/19 07:18:56 deraadt Exp $ */

/**
 * rijndael-alg-fst.h
 *
 * @version 3.0 (December 2000)
 *
 * Optimised ANSI C code for the Rijndael cipher (now AES)
 *
 * @author Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * This code is hereby placed in the public domain.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __RIJNDAEL_H
#define __RIJNDAEL_H


#include	<sys/types.h>



#define	RIJNDAEL	rijndael_ctx


#define RIJNDAEL_MAXKC	(256/32)
#define RIJNDAEL_MAXKB	(256/8)
#define RIJNDAEL_MAXNR	14


#ifndef	TYPEDEF_U8
#define	TYPEDEF_U8	1

typedef unsigned char	u8 ;

#endif /* TYPEDEF_U8 */

#ifndef	TYPEDEF_U16
#define	TYPEDEF_U16	1

typedef unsigned short	u16 ;

#endif

#ifndef	TYPEDEF_U32
#define	TYPEDEF_U32	1

typedef unsigned int	u32 ;

#endif


/*  The structure for key information */
typedef struct {
	int	decrypt ;
	int	Nr ;		/* key-length-dependent number of rounds */
	u32	ek[4*(RIJNDAEL_MAXNR + 1)] ;	/* encrypt key schedule */
	u32	dk[4*(RIJNDAEL_MAXNR + 1)] ;	/* decrypt key schedule */
} rijndael_ctx ;



#ifdef	__cplusplus
extern "C" {
#endif

extern int	rijndael_set_key(rijndael_ctx *, u_char *, int, int) ;
extern int	rijndael_decrypt(rijndael_ctx *, u_char *, u_char *) ;
extern int	rijndael_encrypt(rijndael_ctx *, u_char *, u_char *) ;

#ifdef	__cplusplus
}
#endif



#endif /* __RIJNDAEL_H */


