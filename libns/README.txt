LIBNS


#define	NS		struct ns_head


struct ns_head {
} ;


typedef struct ns_head	ns ;


int ns_open(nsp,nsroot)
NS	*nsp ;
char	nsroot[] ;
{



}


int ns_close(nsp)
NS	*nsp ;
{


}


int ns_getaddr(nsp,name,service,vep)
NS		*nsp ;
char		name[], service[] ;
VECELEM		*vep ;
{




}


