#define ENTRY ">entry"
#define MAXCAL 10
#define MAXENTRIES 200
#define TITLELEN 50
#define MAXEDIT 30


struct cal {
	int no;
	long location;
	long date;
	int stime;
	int etime;
	int format;
	char title[TITLELEN];
	} ;
