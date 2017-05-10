#ifdef	COMMENT

static int srvreg_bufinit(op)
SRVREG		*op ;
{
	int	rs ;
	int	len ;


	len = MIN(op->pagesize,SRVREG_TOPSIZE) ;

	rs = buffer_start(&op->top,len) ;

	if (rs < 0)
		goto bad0 ;

	rs = buffer_start(&op->run,SRVREG_RUNSIZE) ;

	if (rs < 0)
		goto bad1 ;

	return rs ;

/* bad stuff */
bad1:
	buffer_finish(&op->top) ;

bad0:
	return rs ;
}


static int srvreg_bufread(op,start,len,rpp)
SRVREG		*op ;
uint		start ;
uint		len ;
char		**rpp ;
{
	int	rs ;
	int	lev1, lev2 ;


	lev1 = buffer_contain(&op->top,start,len,rpp) ;

	if (lev1 >= 2)
		return len ;

	lev2 = buffer_contain(&op->run,start,len,rpp) ;

	if (lev2 >= 2)
		return len ;

/* OK, we put it in the running buffer */



}
/* end subroutine (srvreg_readbuf) */


static int srvreg_buffree(op)
SRVREG		*op ;
{


	buffer_finish(&op->top) ;

	buffer_finish(&op->run) ;

	return SR_OK ;
}


static int buffer_start(bp,size)
struct srvreg_buffer	*bp ;
int			size ;
{
	int	rs ;


	memset(bp,0,sizeof(struct srvreg_buffer)) ;

	rs = uc_malloc(size,&bp->buf) ;

	if (rs >= 0)
		bp->bufsize = size ;

	return rs ;
}


static int buffer_contain(bp,start,len,rpp)
struct srvreg_buffer	*bp ;
uint			start, len ;
char			*rpp ;
{
	int	rs = 0 ;


	if ((bp->buf == NULL) || (bp->bufsize == 0))
		return 0 ;

	if (start < bp->foff)
		return 0 ;

	rs = 2 ;
	if ((start + len) > (bp->foff + bp->buflen))
		rs = 1 ;

	if (rpp != NULL)
		*rpp = bp->buf + bp->bufstart + (start - bp->foff) ;

	return rs ;
}


static int bufferfree(bp)
struct srvreg_buffer	*bp ;
{


	if (bp->buf != NULL) {

		free(bp->buf) ;

		bp->buf = NULL ;
	}

	bp->bufsize = 0 ;
	bp->buflen = 0 ;
	return SR_OK ;
}

#endif /* COMMENT */



