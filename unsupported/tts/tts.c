char *version = "@(#) tts 0.4";

#if defined(M_SYSV) || defined (i386) || defined(linux)
#undef SYSV
#define SYSV 1
#endif

/*+-------------------------------------------------------------------------
	tts.c - tucker's time server

  Defined functions:
	datagram_received(sockfd)
	demon_death_signal_to_parent(sig)
	inet_utoa(addr)
	main(argc, argv)
	send_datagram(sockfd, whereto, msg, msglen)
	start_datagram_listen(port)
	xmit_PLEASE()
	xmit_time()

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-03-1995-16:49-wht@kepler-add -R */
/*:10-03-1995-16:42-wht@kepler-PLEASE */
/*:10-03-1995-16:32-wht@kepler-every 30 min */
/*:05-30-1994-20:49-wht@n4hgf-once + signal protection */
/*:05-11-1994-01:54-root@n4hgf-SCO master needs alternate bcst addr */
/*:02-10-1994-13:02-wht@gyro-dont time out select if not master */
/*:02-10-1994-12:55-wht@gyro-use 0 for bcst + lengthen interval */
/*:02-09-1994-17:04-wht@gyro-creation */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/socket.h>

#if defined(SYSV)
#include <fcntl.h>
#else
#include <sys/ioctl.h>
#endif

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <setjmp.h>

#define TTS_PORT 10999
#define TTS_XMIT_INTERVAL 60

#ifdef M_SYSV
#define PID_T pid_t
#else
#define PID_T int
#endif

char *tts_lockf = "/etc/tts.pid";
char *tts_dir = "/tmp";

fd_set datagram_listen_sockets;

int dgram_sockfd;

char *service_mnemonic = "TTS";

int verbose = 0;
int mypid;
int parent_still_alive;
int master = 0;
int standalone = 0;

char my_host_name[MAXHOSTNAMELEN];
u_long my_host_addr;
u_long my_loopback_addr;
int once = 0;

long time();
char *inet_utoa();

/*+-------------------------------------------------------------------------
	send_datagram(sockfd,whereto,msg,msglen)
--------------------------------------------------------------------------*/
int
send_datagram(sockfd, whereto, msg, msglen)
int sockfd;
struct sockaddr_in *whereto;
char *msg;
int msglen;
{
	int itmp;

	if (verbose > 1)
	{
		char *addrstr = inet_utoa(ntohl(whereto->sin_addr.s_addr));
		int port = ntohs(whereto->sin_port);

		printf("TTS: send datagram to [%s]:%d\n", addrstr, port);
		if(verbose > 2)
			hex_dump(msg,-msglen,"DATAGRAM SENT",1);
	}

	itmp = sendto(sockfd, msg, msglen, 0, whereto, sizeof(*whereto));
	if ((itmp < 0) || (itmp != msglen))
	{
		if (itmp < 0)
			perror("TTS: sendto");
		else
			printf("TTS: sent %d chars, ret=%d\n", msglen, itmp);
	}
	return (itmp);

}							 /* end of send_datagram */

/*+-------------------------------------------------------------------------
	xmit_time()
--------------------------------------------------------------------------*/
void
xmit_time()
{
	struct sockaddr_in whereto;
	char msg[64];
	long now;

	memset((char *)&whereto, 0, sizeof(whereto));
	whereto.sin_addr.s_addr = htonl((my_host_addr & 0xFFFFFF00) | 0);
	whereto.sin_port = htons(TTS_PORT);
	whereto.sin_family = AF_INET;

	time(&now);
	sprintf(msg, "tts %ld\n", now);
	send_datagram(dgram_sockfd, &whereto, msg, strlen(msg));

}							 /* end of xmit_time */

/*+-------------------------------------------------------------------------
	xmit_PLEASE()
--------------------------------------------------------------------------*/
void
xmit_PLEASE()
{
	struct sockaddr_in whereto;
	char *msg = "tts PLEASE\n";

	memset((char *)&whereto, 0, sizeof(whereto));
	whereto.sin_addr.s_addr = htonl((my_host_addr & 0xFFFFFF00) | 0);
	whereto.sin_port = htons(TTS_PORT);
	whereto.sin_family = AF_INET;

	send_datagram(dgram_sockfd, &whereto, msg, strlen(msg));

}							 /* end of xmit_PLEASE */

/*+-------------------------------------------------------------------------
	start_datagram_listen(port)

Creates an internet socket, binds it to an address,

Input: port number desired, or 0 for a random one
Output: file descriptor of socket, or a negative error
--------------------------------------------------------------------------*/
int
start_datagram_listen(port)
int port;
{
	int sockfd;
	int itmp;
	struct sockaddr_in tts;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("TTS: dgram socket");
		return (-1);
	}

	itmp = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &itmp, sizeof(itmp)) < 0)
	{
		perror("TTS: setsockopt SO_REUSEADDR");
		return (-1);
	}

	itmp = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &itmp, sizeof(itmp)) < 0)
	{
		perror("TTS: setsockopt SO_BROADCAST");
		return (-1);
	}

	memset(&tts, 0, sizeof(tts));
	tts.sin_family = AF_INET;
	tts.sin_addr.s_addr = INADDR_ANY;
	tts.sin_port = htons(port);

	itmp = bind(sockfd, &tts, sizeof(tts));
	if (itmp < 0)
	{
		printf("TTS: datagram socket port %d ", port);
		perror("bind");
		close(sockfd);
		return (-1);
	}

	itmp = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &itmp, sizeof(itmp)) < 0)
	{
		perror("TTS: setsockopt SO_REUSEADDR");
		return (-1);
	}

	if (verbose)
	{
		printf("TTS: datagram socket established port %d (sockfd=%d)\n",
			port, sockfd);
	}

	FD_SET(sockfd, &datagram_listen_sockets);
	return (sockfd);

}							 /* end of start_datagram_listen */

/*+-------------------------------------------------------------------------
	datagram_received(sockfd)
--------------------------------------------------------------------------*/
void
datagram_received(sockfd)
int sockfd;
{
	int itmp;
	int count;
	char buf[2048];
	struct sockaddr_in from;
	char *addrstr;
	u_long from_addr;
	int port;

	itmp = sizeof(from);
	memset((char *)&from, 0, sizeof(from));
	count = recvfrom(sockfd, buf, sizeof(buf), 0, &from, &itmp);
	if (itmp == 4)
		from.sin_addr.s_addr = htonl(my_host_addr);

	if (count < 0)
	{
		sprintf(buf, "TTS: datagram_received on sockfd %d", sockfd);
		perror(buf);
		return;
	}

	if (!count)
	{
		printf("TTS: datagram_received zero length read\n");
		return;
	}

	if (verbose > 2)
	{
		addrstr = inet_utoa(from.sin_addr.s_addr);
		port = ntohs(from.sin_port);
		printf("TTS: datagram from [%s]:%d has %d bytes\n",
			addrstr, port, count);
		hex_dump(buf, -count, "DATAGRAM RCVD", 1);
	}

	if ((from_addr = ntohl(from.sin_addr.s_addr)) == my_host_addr)
		return;

	buf[count] = 0;

	if (!strncmp(buf, "tts ", 4))
	{
		long now;
		long to_be;

#ifndef SYSV
		struct timeval to_be_tv;

#endif
		/*
		 * if time request, master sends time
		 */
		if(master && !strncmp(buf + 4,"PLEASE",6))
		{
			xmit_time();
			return;
		}

		/*
		 * otherwise assume incoming time set
		 */
		time(&now);
		sscanf(buf + 4, "%ld", &to_be);
		if (verbose > 2)
		{
			printf("TTS: master setting time to %s", ctime(&to_be));
			printf("                        was %s\n", ctime(&now));
		}
#ifdef SYSV
		if (stime(&to_be))
			perror("stime");
#else
		to_be_tv.tv_sec = to_be;
		to_be_tv.tv_usec = 0;
		if (settimeofday(&to_be_tv, (struct timezone *)0))
			perror("settimeofday");
#endif

		if (once)
			exit(0);

	}

}							 /* end of datagram_received */

/*+-------------------------------------------------------------------------
	demon_death_signal_to_parent(sig)
--------------------------------------------------------------------------*/
demon_death_signal_to_parent(sig)
int sig;
{
	sleep(2);
	exit(0);				 /* parent exit */
}							 /* end of demon_death_signal_to_parent */

/*+-------------------------------------------------------------------------
    inet_utoa(addr) - unsigned long to dot-formatted text
--------------------------------------------------------------------------*/
char *
inet_utoa(addr)
u_long addr;
{
	static char txt[40];

	sprintf(txt, "%u.%u.%u.%u",
		(addr >> 24) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 8) & 0xFF,
		(addr) & 0xFF);
	return (txt);

}							 /* end of inet_utoa */
/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int sockfd;
	int fdcount;
	fd_set read_fd_set;
	fd_set excep_fd_set;
	struct timeval timer;
	struct sockaddr_in RnsClient_addr;
	struct tm *lt;
	FILE *fppid;
	int errflg = 0;
	int killflg = 0;
	int rm_lockf = 0;
	struct rlimit rl, *rlp = &rl;
	long now;
	long last = 0;
	extern char *optarg;
	extern int optind;

	/*
	 * go to working directory
	 */
	if (chdir(tts_dir))
	{
		printf("TTS: CANNOT CHANGE DIRECTORY TO ");
		perror(tts_dir);
		exit(1);
	}

	while ((itmp = getopt(argc, argv, "RKkmsvo")) != -1)
	{
		switch (itmp)
		{
			case 'k':
				killflg = SIGTERM;
				break;
			case 'K':
				killflg = SIGQUIT;
				break;
			case 's':
				killflg = SIGUSR1;
				break;
			case 'v':
				verbose++;
				break;
			case 'm':
				master = 1;
				break;
			case 'o':
				once = 1;
				break;
			case 'R':
				rm_lockf = 1;
				break;
			case '?':
				errflg++;
				break;
		}
	}

	printf("%s\n",version + 5);
	if(errflg || (optind != argc))
	{
		fprintf(stderr,"Usage: tts [-R] [-K] [-k] [-m] [-s] [-v] [-o]\n");
		exit(1);
	}
	

	/*
	 * make stderr and stdout the same and unbufferred
	 */
	close(2);
	dup2(1, 2);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	mypid = getpid();		 /* we use mypid a great deal */

	if (gethostname(my_host_name, sizeof(my_host_name)))
	{
		printf("tts ");
		perror("gethostname");
		exit(1);
	}

	if ((my_host_addr = gethostaddr(my_host_name)) == 0xFFFFFFFF)
	{
		printf("tts cannot get address for '%s'\n", my_host_name);
		exit(1);
	}

	if ((my_loopback_addr = gethostaddr("localhost")) == 0xFFFFFFFF)
	{
		printf("tts cannot get address for 'localhost'\n");
		exit(1);
	}

	if (master && once)
	{
		int itmp;

		puts("One shot time broadcast");
		dgram_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (dgram_sockfd < 0)
		{
			perror("TTS: once socket");
			exit(1);
		}

		itmp = 1;
		if (setsockopt(dgram_sockfd, SOL_SOCKET, SO_BROADCAST, &itmp,
			sizeof(itmp)) < 0)
		{
			perror("TTS: setsockopt SO_BROADCAST");
			return (-1);
		}

		xmit_time();
		exit(0);
	}

	/*
	 * remove lock file?
	 */
	if(rm_lockf)
		unlink(tts_lockf);

	/*
	 * check to see if tts already active
	 */
	if (fppid = fopen(tts_lockf, "r"))
	{
		PID_T hispid;

		if (fscanf(fppid, "%d", &hispid))
		{
			if (!kill(hispid, 0) || (errno != ESRCH))	/* if process exists */
			{
				if (killflg)
				{
					printf("sending kill(%d,%d)\n", hispid, killflg);
					if (kill(hispid, killflg))
						perror("kill");
					else
						printf("successful\n");
				}
				else
					printf("TTS: IS ALREADY RUNNING\n");
				exit(1);
			}
		}
		fclose(fppid);
	}
	if (killflg)
	{
		printf("TTS is not running\n");
		exit(0);
	}

	/*
	 * barring a race, we seem to be it; fork and disassociate like a good
	 * demon
	 */

	parent_still_alive = 1;
	if (itmp = fork())
	{
		if (itmp < 0)
		{
			perror("TTS: fork");
			exit(1);
		}
		exit(0);
	}

#ifdef TIOCNOTTY
	if (standalone)
	{
		for (itmp = 0; itmp < FD_SETSIZE; itmp++)
			close(itmp);
		open("/", O_RDONLY);
		dup2(0, 1);
		dup2(0, 2);
		itmp = open("/dev/tty", O_RDWR);
		if (itmp >= 0)
		{
			ioctl(itmp, TIOCNOTTY, (char *)0);
			close(itmp);
		}
	}
#endif /* TIOCNOTTY */

	/*
	 * publish our existence
	 */
	mypid = getpid();
	if (!(fppid = fopen(tts_lockf, "w")))
	{
		perror(tts_lockf);
		exit(1);
	}
	fprintf(fppid, "%d\n", mypid);
	fclose(fppid);
	if (verbose)
	{
		sleep(2);
		printf("\n");
		printf("<<< TTS %s >>>   starting (pid %d,verbose=%d)\n",
			version, mypid, verbose);
	}

	/*
	 * call the phone company and get local and 800 number installed
	 */
	if ((dgram_sockfd = start_datagram_listen(TTS_PORT)) < 0)
	{
		printf("TTS: COULD NOT ESTABLISH INET UDP SOCKET\n");
		exit(1);
	}

	for (itmp = 1; itmp < NSIG; itmp++)
		signal(itmp, SIG_IGN);
	signal(SIGTERM, SIG_DFL);

#ifdef RLIMIT_NOFILE
	rlp->rlim_cur = FD_SETSIZE;	/* current (soft) limit */
	rlp->rlim_max = FD_SETSIZE;	/* hard limit */
	(void)setrlimit(RLIMIT_NOFILE, rlp);
#endif

	/*
	 * start chinese fireworks
	 */
	if (master)
		xmit_time();
	else
		xmit_PLEASE();

	while (1)
	{
	  SELECT:
		read_fd_set = datagram_listen_sockets;
		excep_fd_set = datagram_listen_sockets;
		if (verbose > 8)
		{
			printf("TTS: SELECT on fds ");
			for (sockfd = 0; sockfd < FD_SETSIZE; sockfd++)
			{
				if (FD_ISSET(sockfd, &read_fd_set))
					printf("%d ", sockfd);
			}
			printf("\n");
		}
		errno = 0;
		timer.tv_sec = 10;
		timer.tv_usec = 0;
		fdcount = select(FD_SETSIZE,
			&read_fd_set,
			(fd_set *) 0,
			&excep_fd_set,
			(master) ? &timer : 0);

		if (fdcount < 0)
		{
			if (errno == EINTR)
				goto SELECT;
			perror("TTS main select");
			exit(1);
		}

		if (verbose > 3)
			printf("TTS: tts select found %d read(s)\n", fdcount);

		for (sockfd = 0; (sockfd < FD_SETSIZE) && fdcount; sockfd++)
		{
			if (FD_ISSET(sockfd, &excep_fd_set))
				printf("TTS: EXCEPTION on sockfd %d\n", sockfd);

			if (FD_ISSET(sockfd, &read_fd_set))
			{
				fdcount--;
				if (FD_ISSET(sockfd, &datagram_listen_sockets))
				{
					if (verbose)
					{
						printf("TTS: datagram received (sockfd=%d)\n",
							sockfd);
					}
					datagram_received(sockfd);
				}
				else
				{
					if (verbose)
						printf("TTS: unknown data on sockfd %d\n", sockfd);
					close(sockfd);
					FD_CLR(sockfd, &read_fd_set);
				}
			}
		}
		time(&now);
		lt = localtime(&now);
		if (master && ((now - last) > 1800))
		{
			last = now;
			xmit_time();
		}
	}						 /* end of select loop */

}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of tts.c */
