/*+-------------------------------------------------------------------------
	sockutil.c

  Defined functions:
	gethostaddr(namep)
	resolve_name(namep, canonp, canonl)
	socket_local_text(fd)
	socket_peer_text(fd)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:08-20-1996-15:13-root@fep-change include order */
/*:02-09-1994-17:42-wht@gyro-creation */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

extern int verbose;

/*+-------------------------------------------------------------------------
	socket_local_text(fd)
--------------------------------------------------------------------------*/
char *
socket_local_text(fd)
int fd;
{
	int length, err;
	int port;

	char *host;
	struct sockaddr_in address;
	static char sc256[256];

	sc256[0] = 0;

	length = sizeof(address);
	err = getsockname(fd, (struct sockaddr *)&address, &length);
	if (err < 0)
		return ("[??]:?");

	host = inet_ntoa(address.sin_addr);
	port = ntohs(address.sin_port);
	sprintf(sc256, "[%s]:%d", host, port);
	return (sc256);

}							 /* end of socket_local_text */

/*+-------------------------------------------------------------------------
	socket_peer_text(fd)
--------------------------------------------------------------------------*/
char *
socket_peer_text(fd)
int fd;
{
	int length, err;
	int port;
	char *host;
	struct sockaddr_in address;
	static char sc256[256];

	sc256[0] = 0;

	length = sizeof(address);
	err = getpeername(fd, (struct sockaddr *)&address, &length);
	if (err < 0)
		return ("[??]:?");

	host = inet_ntoa(address.sin_addr);
	port = ntohs(address.sin_port);
	sprintf(sc256, "[%s]:%d", host, port);
	return (sc256);

}							 /* end of socket_peer_text */

/*+-------------------------------------------------------------------------
	resolve_name(namep,canonp,canonl) - host name lookup

Resolve host name using domain resolver or whatever, and copy canonical
host name into canonp[canonl].  But if argument is dotted-decimal string
(first char is digit), just convert it without lookup (we don't want to
fail just because the INADDR database is not complete).

returns 0xFFFFFFFF if error

Thanks for this from ping.c (but some mods)
--------------------------------------------------------------------------*/
u_long
resolve_name(namep, canonp, canonl)
char *namep;
char *canonp;
int canonl;
{
	struct hostent *hp;
	u_long inetaddr;
	int n;

	if (isdigit(*namep))
	{
		/* Assume dotted-decimal */
		inetaddr = (u_long) inet_addr(namep);
		if (canonp)
			*canonp = '\0';	 /* No canonical name */
	}
	else
	{
		if (!(hp = gethostbyname(namep)))
		{
			printf("*  %6d: gethostbyname failed\n", getpid());
			if (canonp)
				*canonp = '\0';	/* No canonical name */
			return (0xFFFFFFFF);
		}
		n = ((n = strlen(hp->h_name)) >= canonl) ? canonl - 1 : n;
		if (canonp)
		{
			memcpy(canonp, hp->h_name, n);
			*(canonp + n) = '\0';
		}
		inetaddr = ntohl(*((long *)hp->h_addr));
	}
	if (verbose > 3)
		printf("resolve_name returning %s\n", inet_utoa(inetaddr));
	return (inetaddr);
}							 /* end of resolve_name */

/*+-------------------------------------------------------------------------
	gethostaddr(namep)
--------------------------------------------------------------------------*/
u_long
gethostaddr(namep)
char *namep;
{
	u_long inetaddr = resolve_name(namep, (char *)0, 0);

	return (inetaddr);

}							 /* end of gethostaddr */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of sockutil.c */
