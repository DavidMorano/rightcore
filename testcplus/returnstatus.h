/* return-status */



class returnstatus {
	int		rs ;
public:
	returnstatus(int a) : rs(a) { } ;
	int get() const { return rs ; } ;
	void set(int a) { rs = a ; } ;
} ;


