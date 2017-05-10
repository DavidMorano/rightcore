/* main */



#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/lihdr.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/netdb.h>
// ethernet address size
#define MACSIZE 6
#include <sys/arp.h>
#include <string.h>
#include <memory.h>
#include <osfcn.h>
#include <libc.h>
#include <iostream.h>
#include <fstream.h>
#include <strstream.h>

// NI device filename
#define NIDEV		"/dev/ni/clone0"

// minimum packet size
const int MIN_ETHER_PKT = 60;

// IP address size
const int IP_ADDR_SIZE = 4;

// protocol type for ARP
#define ARP_TYPE        0x0806          
// protocol type for RARP
#define RARP_TYPE       0x8035          

// struct ether_arp.arp_op values for RARP
#define RARP_REQUEST 3
#define RARP_REPLY 4

class ArpMap {
private:
	char am_ipaddr[IP_ADDR_SIZE];
	unsigned char am_etheraddr[MACSIZE];
	int etherparse(char* asciiaddr, unsigned char* etheraddr);
public:
	ArpMap(char* asciiaddr, const char* hostname);
	const char* ipaddr()	{ return am_ipaddr; }
	const char* etheraddr()	{ return (const char*)am_etheraddr; }
};

char* Progname;

int main(int, char* argv[])
{
	struct strbuf sctl;	// send stream control structure
	struct strbuf rctl;	// recv stream control structure
	char sbuf[128];		// send control data buffer
	char rbuf[128];		// recv control data buffer
	union DL_primitives* llcp;

	Progname = argv[0];

	// open ethers file
	ifstream ethers("/etc/ethers");
	if (!ethers) {
		cerr << Progname << ": cannot open /etc/ethers" << endl;
		exit(2);
	}

	// create table of hosts/ethernet addresses
	const int arpmapmax = 256;
	ArpMap* arptable[arpmapmax];
	char asciiaddr[20];
	char hostname[20];
	ethers.width(20);
	unsigned char etheraddr[MACSIZE];
	for (ArpMap** arpentry = arptable; ethers >> asciiaddr; ++arpentry) {

		if (! (ethers >> hostname)) {
			cerr << Progname << ": cannot read hostname for " <<
			asciiaddr << " in /etc/ethers" << endl;
			exit(3);
		}		

		*arpentry = new ArpMap(asciiaddr, hostname);
	}
	// terminate arptable list
	*arpentry = 0;

#ifdef DEBUG
	cout << Progname << ": ethers file read" << endl;
#endif

	// save local host IP address
	const char* getlocalhostname();
	struct hostent* localhostent = gethostbyname(getlocalhostname());
	if (! localhostent) {
		cerr << Progname << ": cannot get local host name" << endl;
		exit(3);
	}
	unsigned char localipaddr[IP_ADDR_SIZE];
	memcpy(localipaddr, localhostent->h_addr, IP_ADDR_SIZE);

	// Open network device driver
	int nifd;	// network interface file descriptor
	if ((nifd = open(NIDEV, O_RDWR)) == -1) {
		cerr << Progname << ": open(" << NIDEV <<
		 ", O_RDWR) failed:" << endl;
		perror("open(NIDEV)");
		exit(4);
	}

	// send DL_INFO_REQ message to driver
	sctl.len = DL_INFO_REQ_SIZE;
	sctl.buf = sbuf;
	llcp = (union DL_primitives*)sbuf;
	llcp->prim_type = DL_INFO_REQ;
	if (putmsg(nifd, &sctl, NULL, RS_HIPRI) != 0) {
		cerr << Progname << ": putmsg(DL_INFO_REQ) failed:" << endl;
		perror("putmsg(DL_INFO_REQ)");
		exit(5);
	}

	// read DL_INFO_ACK message from driver
	rctl.maxlen = sizeof (rbuf);
	rctl.len = 0;
	rctl.buf = rbuf;
	int flags = RS_HIPRI;
	if (getmsg(nifd, &rctl, NULL, &flags) != 0) {
		cerr << Progname << ": getmsg() failed:" << endl;
		perror("getmsg()");
		exit(6);
	}

	// check for DL_INFO_ACK message
	llcp = (union DL_primitives*)rbuf;
	if (llcp->prim_type != DL_INFO_ACK) {
		cerr << Progname << ": getmsg() failed:" << endl;
		perror("getmsg()");
		exit(7);
	}

	// save LSAP address length from message
	long subnet_type = llcp->info_ack.SUBNET_type;
	long ADDR_length = llcp->info_ack.ADDR_length;

	// send DL_BIND_REQ message to driver
	sctl.len = DL_BIND_REQ_SIZE;
	sctl.buf = sbuf;
	llcp = (union DL_primitives*)sbuf;
	llcp->prim_type = DL_BIND_REQ;
	llcp->bind_req.LLC_sap = RARP_TYPE;
	llcp->bind_req.GROWTH_field[0] = NULL;
	llcp->bind_req.GROWTH_field[1] = NULL;
	if (putmsg(nifd, &sctl, NULL, 0) != 0) {
		cerr << Progname << ": putmsg(DL_BIND_REQ) failed:" << endl;
		perror("(putmsg(DL_BIND_REQ)");
		(void)close(nifd);
		exit(9);
	}

	// read DL_BIND_ACK from driver
	rctl.maxlen = sizeof (rbuf);
	rctl.len = 0;
	rctl.buf = rbuf;
	flags = RS_HIPRI;
	if (getmsg(nifd, &rctl, NULL, &flags) != 0) {
		cerr << Progname << ": getmsg(DL_BIND_ACK) failed" << endl;
		perror("getmsg(DL_BIND_ACK)");
		(void)close(nifd);
		exit(10);
	}

	// check for DL_BIND_ACK message
	llcp = (union DL_primitives*)rbuf;
	if (llcp->prim_type != DL_BIND_ACK) {
		cerr << Progname << ": getmsg(DL_BIND_ACK) failed" << endl;
		perror("getmsg(DL_BIND_ACK)");
		(void)close(nifd);
		exit(11);
	}

	// save local host ethernet address
	unsigned char localetheraddr[MACSIZE];
	memcpy(localetheraddr,
	 (unsigned char*)rbuf + llcp->bind_ack.ADDR_offset, MACSIZE);

	while (1) {

		// receive RARP packets
		char packetbuf[1500];	// ethernet packet buffer

		// initialize packet receive structures
		struct strbuf rdata;
		rdata.maxlen = sizeof (packetbuf);
		rdata.len = 0;
		rdata.buf = packetbuf;

		rctl.maxlen = sizeof (rbuf);
		rctl.len = 0;
		rctl.buf = rbuf;
		flags = 0;

		llcp = (union DL_primitives*)rbuf;
		int ret;
		if ((ret = getmsg(nifd, &rctl, &rdata, &flags)) != 0) {
			cerr << Progname <<
			 ": getmsg(DL_UNITDATA_IND) failed: " << ret << endl;
			perror("getmsg(DL_UNITDATA_IND");
			break;
		}
		// check for correct primitive type.
		if (llcp->prim_type != DL_UNITDATA_IND) {
			cerr << Progname <<
			 ": getmsg() != DL_UNITDATA_IND:" << endl;
			perror("getmsg(DL_UNITDATA_IND");
			sleep(2);
			continue;
		}


		// see if requested address is in arptable
		struct ether_arp* arppkt = (struct ether_arp*)packetbuf;

#ifdef DEBUG
		cout << Progname << ": received rarp request from ";
		void printetheraddr(unsigned char*);
		printetheraddr(arppkt->arp_sha);
		cout << endl;
#endif

		for (ArpMap** arpentry = arptable; *arpentry; ++arpentry) {
			if (! memcmp(arppkt->arp_sha,
			 (*arpentry)->etheraddr(), MACSIZE)) {
				break;
			}
		}
		if (! *arpentry)
			continue;

		// reply to RARP request

		// make rarp reply packet
		arppkt->arp_op = RARP_REPLY;
		memcpy(arppkt->arp_sha, localetheraddr, MACSIZE);
		memcpy(arppkt->arp_spa, localipaddr, IP_ADDR_SIZE);
		memcpy(arppkt->arp_tpa, (*arpentry)->ipaddr(),
		 IP_ADDR_SIZE);
		
		// initialize packet transmit structures
		struct strbuf sdata;
		sdata.buf = packetbuf;
		sdata.len =
		 (rdata.len > MIN_ETHER_PKT ? rdata.len : MIN_ETHER_PKT);
		sctl.len = (int)(DL_UNITDATA_REQ_SIZE + ADDR_length + 2);
		sctl.buf = sbuf;
		llcp = (union DL_primitives*)sbuf;
		llcp->prim_type = DL_UNITDATA_REQ;
		llcp->data_req.RA_length = ADDR_length;
		llcp->data_req.RA_offset = DL_UNITDATA_REQ_SIZE;
		llcp->data_req.SERV_class = DL_NOSERV;
		llcp->data_req.FILLER_field = 0;
		memcpy(sbuf + DL_UNITDATA_REQ_SIZE, (*arpentry)->etheraddr(),
		 MACSIZE);
		*(short*)(sbuf + DL_UNITDATA_REQ_SIZE + MACSIZE)
		 = RARP_TYPE;

		for (int replies = 3; replies; --replies) {

			// transmit RARP_REPLY
			if (putmsg(nifd, &sctl, &sdata, 0) != 0) {
				cerr << Progname <<
				 ": putmsg(DL_UNITDATA_REQ) failed" << endl;
				perror("putmsg(DL_UNITDATA_REQ)");
				(void)close(nifd);
				exit(12);
			}

#ifdef DEBUG
			cout << Progname << ": replied to ";
			printetheraddr(arppkt->arp_tha);
			cout << " @ ";
			void printipaddr(unsigned char*);
			printipaddr(arppkt->arp_tpa);
			cout << endl;
#endif

			sleep(2);
		}
	}

	// terminate stream to data link driver

	// send DL_UNBIND_REQ message to driver
	sctl.len = DL_UNBIND_REQ_SIZE;
	sctl.buf = sbuf;
	llcp = (union DL_primitives *)sbuf;
	llcp->prim_type = DL_UNBIND_REQ;
	if (putmsg(nifd, &sctl, NULL, 0) != 0) {
		cerr << Progname << ": putmsg(DL_UNBIND_REQ) failed" << endl;
		perror("putmsg(DL_UNBIND_REQ)");
		(void)close(nifd);
		exit(13);
	}

	// read (receive) DL_OK_ACK message from driver
	rctl.maxlen = sizeof (rbuf);
	rctl.len = 0;
	rctl.buf = rbuf;
	flags = RS_HIPRI;
	if (getmsg(nifd, &rctl, NULL, &flags) != 0) {
		cerr << Progname << ": getmsg(DL_OK_ACK) failed" << endl;
		perror("getmsg(DL_OK_ACK)");
		(void)close(nifd);
		exit(14);
	}

	// check for DL_OK_ACK message received
	llcp = (union DL_primitives *)rbuf;
	if (llcp->prim_type != DL_OK_ACK) {
		cerr << Progname << ": getmsg(DL_OK_ACK) failed" << endl;
		(void)close(nifd);
		exit(15);
	}

	// close network device driver
	if (close(nifd) == -1) {
		cerr << Progname << ": close(\"" << NIDEV << "\")" << endl;
		perror("close(NI)");
		exit(16);
	}

	exit(0);
}

const char*
getlocalhostname()
{
	int gethostname(char*, int);
	char namebuf[512];
	if (gethostname(namebuf, sizeof (namebuf)) < 0) {
		perror("gethostname()");
		exit(17);
	}

	char* hostname = new char[strlen(namebuf) + 1];
	strcpy(hostname, namebuf);
	return hostname;
}

int
gethostname(char* namebuf, int namebufsize)
{
	struct utsname hostnames;

	if (uname(&hostnames) < 0) {
		namebuf[0] = '\0';
		return -1;
	}

	strncpy(namebuf, hostnames.nodename, namebufsize);
	return 0;
}

ArpMap::ArpMap(char* asciiaddr, const char *hostname)
{
	// parse given ascii ethernet address into etheraddr
	if (! etherparse(asciiaddr, am_etheraddr)) {
		cerr << Progname <<
		 ": bad Ethernet address format in /etc/ethers: " <<
		 asciiaddr << endl;
		exit(100);
	}

	// get IP address for given host
	struct hostent* hostent = gethostbyname(hostname);
	if (! hostent) {
		cerr << Progname << ": unknown host: " << hostname << endl;
		exit(1);
	}
	memcpy(am_ipaddr, hostent->h_addr, IP_ADDR_SIZE);
#ifdef DEBUG
	cout << asciiaddr << "\t" << hostname << endl;
#endif
}

int
ArpMap::etherparse(char* asciiaddr, unsigned char* etheraddr)
{
	unsigned int octetval;
	istrstream ethstream(asciiaddr);
	ethstream >> hex;

	for (int i = 0; i < MACSIZE; ++i) {
		ethstream >> octetval;
		if (octetval > 255)
			return 0;
		if (i < (MACSIZE - 1))
			if (ethstream.get() != ':')
				return 0;
		*etheraddr++ = octetval;
	}
	return 1;
}

#ifdef DEBUG

void
printetheraddr(unsigned char* addr)
{
	cout << hex;
	cout.width(2);

	for (int i = 0; i < MACSIZE; ++i) {
		cout.fill('0');
		cout << (int)addr[i];
		if (i < (MACSIZE - 1))
			cout << ':';
	}
}

void
printipaddr(unsigned char* addr)
{
	cout << dec;
	cout.width();
	cout.fill();

	for (int i = 0; i < IP_ADDR_SIZE; ++i) {
		cout << (int)addr[i];
		if (i < (IP_ADDR_SIZE - 1))
			cout << '.';
	}
}

#endif



