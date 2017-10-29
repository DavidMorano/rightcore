

struct anas2_item {
	int	i = 0 ; /* index */
	int	w = 0 ; /* which-type */
	anas2_item() { } ;
	anas2_item(int _i,int _w) : i(_i), w(_w) { 
	} ;
	anas2_item &operator = (const anas2_item &wi) = default ;
	/* do not need 'move' constructor or assignment (too simple) */
	void setindex(int ai) {
	    i = ai ;
	} ;
	void setwhich(int aw) {
	    w = aw ;
	} ;
} ;

struct anas2_char {
	int		ch ;
	int		max = 1 ;
	vector<int>	counts ;
	anas_char(char _ch,int n) : ch(_ch), counts(n,0) { 
	} ;
} ;

typedef vector<anas2_char>	anas2map_t ;
typedef stack<anas2_item>	anas2stack_t ;

struct anas2 {
	anas2stack_t	work ;
	anas2map_t	m ;
    	anas2_item	wi ;
	string		src ;
	string		tmp ;
	res_t		*resp = NULL ;
	int		total = 0 ;
	int		n = 0 ;
	anas() = default ;
	anas(res_t *_rp,cchar *sp) : src(sp), resp(_rp) {
	    n = src.length() ;
	    tmp.resize(n,'-') ;
	    for (auto ch : src) {
		loadchar(ch) ;
	    } /* end for */
	} ;
	void loadchar(char ch) {
	    const int	wn = m.size() ;
	    bool	f = false ;
	    int		w ;
	    for (w = 0 ; w < wn ; w += 1) {
		f = (m[w].ch == ch) ;
		if (f) break ;
	    }
	    if (f) {
		m[w].max += 1 ;
	    } else {
		struct anas_char	item(ch,n) ;
		m.push_back(item) ;
	    }
	} ;
	bool canadd(int i,int w) {
	    bool	f = (m[w].counts[i] < m[w].max) ;
	    return f ;
	} ;
	void inc(int i,int w) {
	    m[w].counts[i] += 1 ;
	} ;
	void enter(int i) {
	    const int	n = m.size() ;
	    if (i > 0) {
		for (int w = 0 ; w < n ; w += 1) {
		    m[w].counts[i] = m[w].counts[i-1] ;
		}
	    } else {
		for (int w = 0 ; w < n ; w += 1) {
		    m[w].counts[i] = 0 ;
		}
	    }
	} ;
	int mkstr() {
	    anas2_item	wi ;
	    int		i, w ;
    	    while (! work.empty()) {
      		wi = work.top() ;
      		work.pop() ;
		i = wi.i ;
		w = wi.w ;
	        if (i < n) {
	            enter(i) ;
		    if (canadd(i,w)) {	/* restriction */
		        tmp[i] = m[w].ch ;
		        inc(i,w) ;
		        if ((i+1) == n) {
#if	CF_DEBUGS
			    debugprintf("mainanas/anas2::mkstr: es=%s\n",
				tmp.c_str()) ;
#endif
	    	            resp->insert(tmp) ;
			    total += 1 ;
		        } else {
	                    pushall(i+1) ;
		        }
		    } /* end if (hit) */
	    } /* end if (possible) */
	    return total ;
	} ; /* end method (mkstr) */
	int num() const {
	    return total ;
	} ;
	void push(int i,int w) {
	    anas2_item	wi(i,w) ;
	    work.push(wi) ;
	} ;
	void pushall(int i) {
	    int		n = m.size() ;
	    for (int w = 0 ; w < n ; w += 1) {
    	        push(i,w) ;
	    }
	} ;
	int mkstrs() {
	    pushall(0) ;
	    return mkstr() ;
	} ;
} ;

static int anas2(res_t *resp,cchar *sc)
{
	const int	n = strlen(sc) ;
	int		rs = SR_OK ;
	if (n > 0) {
	    anas2	worker(resp,sc) ;
	    rs = worker.mkstrs() ;
	    cout << "num=" << worker.num() << endl ;
  	} /* end if (valid) */
	return rs ;
}
/* end subroutine (anas2) */

