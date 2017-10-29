/*
 *	Copyright (c) 1997, by Sun Microsystems Inc.
 *	All Rights Reserved
 */
#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <stdio.h>
#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


void usage(char *name)
{
	char *program;

	if ((program = strrchr(name, '/')) == NULL)
		program = name;
	else
		++program;

	fprintf(stderr, "Usage: %s -h host -p printer files...\n", program);
	exit(1);
}


int response(int fd)
{
        char    c;

        if ((read(fd, &c, 1) != 1) || c) {
                errno = EIO;
                return (c);
        }
        return (0);
}

int send_message(int fd, char *fmt, ...)
{
        char    buf[BUFSIZ];
        va_list ap;

        va_begin(ap, fmt);
        vsprintf(buf, fmt, ap);
        va_end(ap);

        if (write(fd, buf, (int)((strlen(buf) != 0) ? strlen(buf) : 1)) < 0)
                return (-1);
        return (response(fd));
}

int send_file(int fd, char *name, int type, int len, char *ptr)
{
	if (send_message(fd, "%c%d %s\n", type, len, name) != 0) {
		fprintf(stderr, "not accepting file %s\n", name);
		return (-1);
	}

	while (len > 0) {
		int rc = write(fd, ptr, len);

		if (rc < 0) {
			fprintf(stderr, "error sending data %s\n", name);
			return (-1);
		}
		len -= rc;
		ptr += rc;
	}

	if (send_message(fd, "", NULL) != 0) {
		fprintf(stderr, "%s communication error\n");
		return (-1);
	}
	return (0);
}
               		 

/*
 * This is an example piece of source to help people communicate with
 * network attached printers using the LPD protocol (RFC-1179)
 */
main(int ac, char *av[])
{
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int c;
	int ofd;
	int tmp = 1023;
	char *host = NULL,
	     *printer = NULL;
	char buf[BUFSIZ];


	while ((c = getopt(ac, av, "h:p:")) != EOF)
		switch (c) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			printer = optarg;
			break;
		default:
			usage(av[0]);
		}

	if ((optind >= ac) || (host == NULL) || (printer == NULL))
		usage(av[0]);

	memset(&sin, NULL, sizeof (sin));
	if ((hp = gethostbyname(host)) == NULL) {
                fprintf(stderr, "unknown host %s", host);
                exit(1);
        }
        memcpy((caddr_t)&sin.sin_addr, hp->h_addr, hp->h_length);
        sin.sin_family = hp->h_addrtype;

        if ((sp = getservbyname("printer", "tcp")) == NULL) {
                fprintf(stderr, "printer/tcp: unknown service");
                exit(1);
        }
        sin.sin_port = sp->s_port;


	/* connect to the printer */	
	if ((ofd = rresvport(&tmp)) < 0) {
		perror("rresvport()");
		exit(1);
	}

	if (connect(ofd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("connect()");
		exit(1);
	}

	/* request to send a job */
	if (send_message(ofd, "%c%s\n", 2, printer) != 0) {
		fprintf(stderr, "%s not accepting data\n", printer);
		exit(1);
	}

	/* build the control file */
	sprintf(buf, "Hhost\nPuser\n");
	for (tmp = 0; tmp < (ac - optind); tmp++) {
		char c = 'A' + tmp,
		     file[BUFSIZ];

		sprintf(file, "fdf%c000host\nUdf%c000host\nNdf%c000host\n", c,
			c, c);
		strcat(buf, file);
	}

	/* send the control file */
	if (send_file(ofd, "dfA000host", 2, strlen(buf), buf) < 0) {
		fprintf(stderr, "failed to send control file\n");
		exit(1);
	}
	

	/* send the data files */
	for (tmp = optind; tmp < ac; tmp++) {
		struct ustat st;
		int fd;
		int	mflags = 0 ;
		char *ptr;

		if ((fd = open(av[tmp], O_RDONLY)) < 0) {
			fprintf(stderr, "couldn't open %s\n", av[tmp]);
			continue;
		}

		if (fstat(fd, &st) < 0) {
			fprintf(stderr, "couldn't stat %s\n", av[tmp]);
			close(fd);
			continue;
		}
		
		mflags |= MAP_PRIVATE ;
#if	defined(MAPNORESERVE)
		mflags |= MAP_NORESERVE ;
#endif
		if ((ptr = mmap(NULL, st.st_size, PROT_READ,mflags,
				fd, 0)) == MAP_FAILED) {
			fprintf(stderr, "couldn't mmap %s\n", av[tmp]);
			close(fd);
			continue;
		}
		close(fd);

		sprintf(buf, "%c%d df%c000host\n", 3, st.st_size,
			('A' + tmp - optind));
		if (send_file(ofd, buf, 3, st.st_size, ptr) < 0) {
			fprintf(stderr, "error sending data file\n");
			exit(1);
		}

		munmap(ptr, st.st_size);
	}	
	
	close(ofd);
	exit(0);
}
