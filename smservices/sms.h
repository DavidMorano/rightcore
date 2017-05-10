/* sms */

/* sms operations */


#ifndef	SMS_INCLUDE
#define	SMS_INCLUDE	1



#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"plainq.h"



/* object defines */

#define	SMS		struct sms_head

/* other defines */

#define	SMS_SVCCODE	1		/* used when over UDPMUX */




struct sms_bufdesc {
	char	*buf ;
	int	buflen ;
} ;

struct sms_head {
	unsigned long	magic ;
	PLAINQ	tq ;
	time_t	timeout ;
	uint	svc ;		/* numeric service */
	int	fd ;		/* socket file descriptor */
	int	pid ;		/* daemon PID */
} ;




#if	(! defined(SMS_MASTER)) || (SMS_MASTER == 0)

extern int sms_open(SMS *,char *,char *,char *,char *,int) ;
extern int sms_write(SMS *,char *,int) ;
extern int sms_close(SMS *) ;

extern SMS	obj_sms(char *) ;
extern SMS	*new_sms(char *) ;
extern void	free_sms(SMS *) ;

#endif /* SMS_MASTER */


#endif /* SMS_INCLUDE */



