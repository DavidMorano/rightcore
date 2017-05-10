
#include <assert.h>

// FIXME on weird sizeof(int)!=4 machines
typedef unsigned int DWORD;

#define SWB_MODULUS (-5)
// really 2^32 - 5 = 4294967291.  This is to get gcc to stop complaining.
#define SWB_LEN 43
#define SWB_TAP 22


/* Adapted July 28, 2001 by Jeff Henrikson from the 
   source code of MIT Scheme 7.5.16.  As a consequence of it's GPL, this 
   file must also be distributed under the GNU General Public License.  
   Bugs/comments: jehenrik@alum.mit.edu.

   Excerpt from src/lib/options/random.scm:

;;; A "subtract-with-borrow" RNG, based on the algorithm from "A New
;;; Class of Random Number Generators", George Marsaglia and Arif
;;; Zaman, The Annals of Applied Probability, Vol. 1, No. 3, 1991.

;;; The basic algorithm produces a sequence of values x[n] by
;;;      let t = (x[n-s] - x[n-r] - borrow[n-1])
;;;      if (t >= 0)
;;;        then x[n] = t, borrow[n] = 0
;;;        else x[n] = t + b, borrow[n] = 1
;;; where the constants R, S, and B are chosen according to some
;;; constraints that are explained in the article.  Finding
;;; appropriate constants is compute-intensive; the constants used
;;; here are taken from the article and are claimed to represent
;;; "hundreds of hours" of compute time.  The period of this generator
;;; is (- (EXPT B R) (EXPT B S)), which is approximately (EXPT 10 414).
*/

class SWBrandom {
 private:
  DWORD buf[SWB_LEN];
  int head;
  bool borrow;

 public:
  SWBrandom() {
    head=0;
    borrow=false;  // needs frobbing to fit test data
  }

  /** warning, the bounds of elements should be in [0,2^32-5) */
  void seed(DWORD x) {
    buf[head++]=x;
    if(!(head<SWB_LEN)) {head -= SWB_LEN; assert(head==0);}
  }

  /** the value returned satisfies is in [0,2^32-5).  Not [0,2^32)! */
  DWORD get() {
    DWORD ret;
    int tap = head-SWB_TAP;
    if(tap<0) tap += SWB_LEN;
    DWORD tmp = buf[tap] - (borrow? 1: 0);
    if(borrow=(tmp<buf[head]))
      ret = tmp - buf[head] + SWB_MODULUS;
    else
      ret = tmp - buf[head];
    buf[head++]=ret;
    if(!(head<SWB_LEN)) {head -= SWB_LEN; assert(head==0);}
    return ret;
  }
};



