/*+-------------------------------------------------------------------------
	pcmdserve.c - ecu socket server procedure commands
	wht@wht.net

  Defined functions:
	pcmd_serve(param)
	pcmd_servewire(param)

	ptrace on
	dial 'ttyS42'
AGAIN
	hangup
	serve 44553 # may set $i0 - $i6 and $s0
	ifnz $i0
	   echo 'serve error '+%itos($i0)+': '+%errstr($i0) ; return
	# if we get this far,
	# $s0 is set to IP address of calling host as a string
	# $i1,$i2,$i3,$i4 are set to each individual byte of the IP address
	# $i5 is the calling port number
	# $i6 is the ecu file number for the socket
	echo 'Connect from host '+$s0+' port '+%itos($i5)
	echo -n 'Integer variable address breakout: '
	echo %itos($i1)+'.'+%itos($i2)+'.'+%itos($i3)+'.'+%itos($i4)+'.'
	#set $i0,$s0,$i1,$i2,$i3,$i4,$i5,$i6

	servewire $i6
	ifnz $i0
		echo 'servewire error '+%itos($i0)+': '+%errstr($i0) ; return
	goto AGAIN

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:04-02-1998-19:04-wht@kepler-increase tty read size */
/*:03-31-1998-18:33-wht@kepler-creation */

#include "ecu.h"

#if defined(CFG_TelnetServer)
#include "ecuerror.h"
#include "termecu.h"
#include "ecukey.h"
#include "esd.h"
#include "var.h"
#include "procedure.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <setjmp.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>

char *timeofday_text();
char *inet_utoa();


/*+-------------------------------------------------------------------------
	pcmd_serve(param)
--------------------------------------------------------------------------*/
int
pcmd_serve(param)
ESD *param;
{
	int erc = 0;
	char switches[8];
	int itmp;
	long ltmp;
	UINT16 serve_port;
	int listen_sockfd = -1;
	int sockfd = -1;
	struct sockaddr_in sockaddr,*sa = &sockaddr;
	struct timeval tval,*tv = &tval;
    UINT32 peer_addr;
    UINT16 peer_port;
	int old_ttymode = get_ttymode(); /* save original tty mode */
 
#ifdef CFG_HasFdSet
	CFG_FDSET fdset;
#else
	int fdset;
#endif

	get_switches(param, switches, sizeof(switches));

	if(strchr(switches,'R'))
	{
	}

	if (erc = gint(param, &ltmp))
		return (erc);
	serve_port = ltmp;

	if (proc_trace)
	{
		pprintf("%s: ",timeofday_text(3,0));
		pprintf("serve on port %d\n",serve_port);
	}

	iv[0] = 0;

	listen_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(listen_sockfd < 0)
	{
		iv[0] = errno;
		pperror("socket");
		goto FUNC_EXIT;
	}

	itmp = 1;
	if(setsockopt(listen_sockfd,SOL_SOCKET,SO_REUSEADDR,&itmp,sizeof(itmp)) < 0)
	{
		iv[0] = errno;
		pperror("setsockopt SO_REUSEADDR");
		goto FUNC_EXIT;
	}

	memset((char *)sa,0,sizeof(*sa));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(serve_port);

	itmp = bind(listen_sockfd,(struct sockaddr *)sa,sizeof(*sa));
	if(itmp < 0)
	{
		iv[0] = errno;
		pperror("bind");
		goto FUNC_EXIT;
	}

	itmp = 1;
	if(setsockopt(listen_sockfd,SOL_SOCKET,SO_REUSEADDR,&itmp,sizeof(itmp)) < 0)
	{
		iv[0] = errno;
		pperror("setsockopt SO_REUSEADDR");
		goto FUNC_EXIT;
	}

	if(listen(listen_sockfd,1))
	{
		iv[0] = errno;
		pperror("listen");
		goto FUNC_EXIT;
	}

	if(proc_trace)
	{
		pprintf("%s: ",timeofday_text(3, 0));
		pprintf("serve listen on TCP port %d started (sockfd=%d)\n",
			serve_port,listen_sockfd);
	}

	ttymode(3);
	while(1)
	{
#ifdef CFG_HasFdSet
		FD_ZERO(&fdset);
		FD_SET(listen_sockfd, &fdset);
#else
		fdset = 1 << listen_sockfd;
#endif
		tv->tv_sec = 10;
		tv->tv_usec = 0;
		if ((itmp = select(listen_sockfd + 1, &fdset, 0, 0, tv)) < 1)
		{
			if(itmp == 0)
				continue;
			else if ((errno == EINTR) && !ck_sigint())
				continue;
			iv[0] = errno;
			pperror("listen select");
			goto FUNC_EXIT;
		}
		break;
	}

    itmp = sizeof(*sa);
	if((sockfd = accept(listen_sockfd,(struct sockaddr *)sa,&itmp)) < 0)
	{
		iv[0] = errno;
		pperror("accept");
		goto FUNC_EXIT;
	}

    itmp = sizeof(*sa);
    if(itmp = getpeername(sockfd,(struct sockaddr *)sa,&itmp))
    {
		iv[0] = errno;
        pperror("getpeername");
		goto FUNC_EXIT;
    }
    peer_addr = ntohl(sa->sin_addr.s_addr);
    peer_port = ntohs(sa->sin_port);
	if (proc_trace)
	{
		pprintf("%s: ",timeofday_text(3,0));
		pprintf("connect from %s:%d\n", inet_utoa(peer_addr),peer_port);
	}
	
	if(erc = socket_fdopen(sockfd,&itmp))
	{
		pputs("socket_fdopen: ");
		proc_error(erc);
		erc = 0;
		iv[0] = EPERM;
		goto FUNC_EXIT;
    }

	esdzero(sv[0]);
	esdstrcat(sv[0], inet_utoa(peer_addr));
	iv[1] = (peer_addr >> 24) & 0xFF;
	iv[2] = (peer_addr >> 16) & 0xFF;
	iv[3] = (peer_addr >> 8) & 0xFF;
	iv[4] = (peer_addr >> 0) & 0xFF;
	iv[5] = peer_port;
	iv[6] = itmp;

FUNC_EXIT:
	if(listen_sockfd > -1)
		close(listen_sockfd);
	ttymode(old_ttymode);

	if(!iv[0])
		return(0);

	if(sockfd >= 0)
		close(sockfd);
	return (erc);

}							 /* end of pcmd_serve */

/*+-------------------------------------------------------------------------
	pcmd_servewire(param)
--------------------------------------------------------------------------*/
int
pcmd_servewire(param)
ESD *param;
{
	int erc = 0;
	int log_traffic = 0;
	int itmp;
	char switches[8];
	char banner[1024];
	char myhostnm[256];
	int filenum = -1;
	int fdmax;
	int sockfd;
	int ttyfd;
	struct timeval tval,*tv = &tval;
	char buf[1024];
	int old_ttymode = get_ttymode(); /* save original tty mode */
	char *timeofday_text();

#ifdef CFG_HasFdSet
	CFG_FDSET fdset;
#else
	int fdset;
#endif

	if (shm->Liofd < 0)
		return(eNoLineAttached);

	get_switches(param, switches, sizeof(switches));
	log_traffic = !!strchr(switches,'l');

	if (erc = get_filenum(param, &filenum))
		return (erc);
	sockfd = ecufileno(filenum);
	if(sockfd < 0)
	{
		iv[0] = EBADF;
		pperror("ecu file number illegal or not open");
		goto FUNC_EXIT;
	}

	fdmax = sockfd;
	ttyfd = shm->Liofd;

	if(fdmax < ttyfd)
		fdmax = ttyfd;
	fdmax++;

	if(log_traffic || proc_trace)
	{
		pprintf("%s: ",timeofday_text(3, 0));
		pputs("servewire entering service\n");
	}

	iv[0] = 0;
	ttymode(3);
	lflush(2);			/* flush line input and output */
	shm->Ltelnet_raw = 1;
	
	myhostnm[0] = 0;
	gethostname(myhostnm,sizeof(myhostnm));
	if(!myhostnm[0])
		strcpy(myhostnm,"noname");
	sprintf(banner,"%s:%s at %s\r\n",myhostnm, shm->Lline, timeofday_text(4,0));
	write(sockfd,banner,strlen(banner));
	while(1)
	{
#ifdef CFG_HasFdSet
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		FD_SET(ttyfd, &fdset);
#else
		fdset = 1 << sockfd;
		fdset |= 1 << ttyfd;
#endif
		tv->tv_sec = 10;
		tv->tv_usec = 0;
		if ((itmp = select(fdmax, &fdset, 0, 0, tv)) < 1)
		{
			if(itmp == 0)
				continue;
			iv[0] = errno;
			pperror("connection select");
			goto FUNC_EXIT;
		}
		if(FD_ISSET(sockfd,&fdset))
		{
			errno = 0;
			/*
			 * shorter read length for socket than tty
			 */
			if((itmp = read(sockfd,buf,sizeof(buf) / 4)) < 0)
			{
				iv[0] = errno;
				pperror("socket read");
				goto FUNC_EXIT;
			}
			if(!itmp)
			{
				if(log_traffic || proc_trace)
				{
					pprintf("%s: ",timeofday_text(3, 0));
					pputs("socket closed\n");
				}
				break;
			}
			if(log_traffic)
				hex_dump(buf,itmp,"socket read",1);
			write(ttyfd,buf,itmp);
		}
		if(FD_ISSET(ttyfd,&fdset))
		{
			errno = 0;
			/*
			 * big gulps from short tty buffers
			 */
			if((itmp = read(ttyfd,buf,sizeof(buf))) < 0)
			{
				iv[0] = errno;
				pperror("tty read");
				goto FUNC_EXIT;
			}
			if(log_traffic)
				hex_dump(buf,itmp,"tty read",1);
			write(sockfd,buf,itmp);
		}
	}


FUNC_EXIT:
	if(log_traffic || proc_trace)
	{
		pprintf("%s: ",timeofday_text(3, 0));
		pputs("servewire exiting\n");
	}

	shm->Ltelnet_raw = 0;
	ttymode(old_ttymode);
	close_ecu_file(filenum);
	return(0);

}	/* end of pcmd_servewire */

#endif /* CFG_TelnetServer */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of pcmdserve.c */
