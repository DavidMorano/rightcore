/*
 * Definitions for NNTP client routines.
 *
 * @(#)$Id: nntpclnt.h,v 1.6 1992/08/03 04:51:45 sob Exp sob $
 */

#ifdef __STDC__
extern	char	*getserverbyfile(char *s);
extern	int	server_init(char *s);
extern	void	put_server(char *s);
extern	int	get_server(char *s, int sz);
extern	void	close_server(void);
extern	int	handle_server_response(int response, char *servername);
#else
extern	char	*getserverbyfile();
extern	int	server_init();
extern	void	put_server();
extern	int	get_server();
extern	void	close_server();
extern	int	handle_server_response();
#endif

