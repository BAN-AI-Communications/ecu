#if defined(CFG_TelnetOption)
/*+-------------------------------------------------------------------------
	sockserv.c -- TCP server handler
	I ain't redy yit fo' IPv6

  Defined functions:

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:01-01-2000-20:46-wht@WIN32-more work */
/*:11-10-1999-00:16-wht@menlo-creation as shaped charge for ECU */

#include "ecu.h"
#include "ecuerror.h"
#include "esd.h"
#include "var.h"

#include "sockserv.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <setjmp.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>

static fd_set _listen_sockets;
static char _exception_label[64];

/*+-------------------------------------------------------------------------
	listenTCPWithTimeout(interfaceIP,tcpPortNum,timeoutMsecs,fdReturned)
--------------------------------------------------------------------------*/
int
listenTCPWithTimeout(interfaceIP,tcpPortNum,timeoutMsecs,fdReturned)
UINT32 interfaceIP;
UINT16 tcpPortNum;
UINT32 timeoutMsecs;
int *fdReturned;
{
	int iv0Set = 0;
	int i;
	int fd = -1;
	int erc = 0;
	struct stat _st, *st = &_st;
	fd_set selectFDSET;
	struct timeval timer;
	struct sockaddr_in _myAddr,*myAddr = &_myAddr;
	struct sockaddr_in _hisAddr,*hisAddr = &_hisAddr;

	memset((char *)&_listen_sockets,0,sizeof(_listen_sockets));

	errno = 0;
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		pperror("\nsocket creation");
		erc = eConnectFailed;
		goto EXIT;
	}

	i = 1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i)) < 0)
	{
		pperror("sockserv: setsockopt SO_REUSEADDR");
		erc = eConnectFailed;
		goto EXIT;
	}

	memset((char *)myAddr, 0, sizeof(*myAddr));
	myAddr->sin_family = AF_INET;
	myAddr->sin_addr.s_addr = htonl(interfaceIP);
	myAddr->sin_port = htons(tcpPortNum);
	i = bind(fd,myAddr,sizeof(*myAddr));
	if(i < 0)
	{
		char s128[128];
		sprintf(s128,"sockserv: bind error on port %d",tcpPortNum);
		pperror(s128);
		erc = eConnectFailed;
		goto EXIT;
	}

	i = 1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i)) < 0)
	{
		pperror("sockserv: setsockopt SO_REUSEADDR");
		erc = eConnectFailed;
		goto EXIT;
	}

	if(listen(fd,5))
	{
		pperror("sockserv: start_stream_listen listen");
		erc = eConnectFailed;
		goto EXIT;
	}

	if(proc_trace)
	{
		pprintf("sockserv: stream socket established port %d (fd=%d)\n",
			tcpPortNum,fd);
	}

	FD_SET(fd,&_listen_sockets);

	memcpy((char *)&selectFDSET,(char *)&_listen_sockets,sizeof(selectFDSET));
	timer.tv_sec = timeoutMsecs / 1000;
	timer.tv_usec = (timeoutMsecs % 1000) * 1000;
	errno = 0;
	if((i = select(fd + 1,&selectFDSET,0,0,&timer)) < 0)
	{
		if(proc_trace)
		{
			pprintf("sockserv: errno=%d", errno);
			pperror("");
		}
		
		/* interrupt or error */
		iv[0] = 2;
		iv0Set = 1;
	}
	else if(!i)
	{
		/* timeout */
		iv[0] = 1;
		iv0Set = 1;
	}
	else
	{
		/* we got one */
		int accept_fd = fd;
		i = sizeof(*hisAddr);
		if(proc_trace)
			pprintf("listenTCPWithTimeout accepting on fd=%d\n",fd);
		if((fd = accept(accept_fd, (struct sockaddr *)hisAddr, &i)) < 0)
		{
			close(accept_fd);
			pputs("accept() error\n");
			erc = eConnectFailed;
			iv[0] = 40;
			iv0Set = 1;
			goto EXIT;
		}
		if(fd != accept_fd)
			close(accept_fd);

		/*
		 * see if we really have a connection
		 */
		if (fstat(fd, st))
		{
			pputs("connection closed by remote\n");
			erc = eConnectFailed;
			iv[0] = 41;
			iv0Set = 1;
			goto EXIT;
		}
		if(proc_trace)
		{
			char s32[32];
			mode_map(st->st_mode, s32);
			pprintf("socket fd=%d mode=%s\n",fd,s32);
		}
		fchmod(fd,0666);
		if(proc_trace)
		{
			char s32[32];
			fstat(fd, st);
			mode_map(st->st_mode, s32);
			pprintf("socket fd=%d mode=%s\n",fd,s32);
		}
		iv[0] = 0;
		iv0Set = 1;
	}

EXIT:
	if(proc_trace && iv0Set)
		pprintf("listenTCPWithTimeout set $i0 to %d\n",(int)iv[0]);
	*fdReturned = fd;
	if(!erc)
		return(0);

	close(fd);
	return (0);

}	/* end of listenTCPWithTimeout */

/*+-------------------------------------------------------------------------
	sockserveException() -- I/O error
--------------------------------------------------------------------------*/
void
sockserveException()
{
	ESD *e = esdalloc(256); /* trusting about memory errors in old age :/ */
	shm->Lsockserve = 0;
	shm->Ltelnet = 0;
	sprintf(e->pb,"goto %s",_exception_label);
	e->cb = strlen(e->pb);
	if(proc_trace)
		pprintf("SOCKET EXCEPTION - forcing %s\n",e->pb);
	execute_esd(e);
	esdfree(e);
}	/* end of sockserveException */

/*+-------------------------------------------------------------------------
	pcmd_sockserv(param)
--------------------------------------------------------------------------*/
int
pcmd_sockserve(param)
ESD *param;
{
	char interfaceIpAddrString[32];
	ulong ipaddr;
	long portnum;
	int fd;
	int i;
	int erc;
	struct sockaddr_in _hisAddr,*hisAddr = &_hisAddr;
	char switches[32];

	strcpy(interfaceIpAddrString,"0.0.0.0");

	get_switches(param, switches, sizeof(switches));
	if(strchr(switches,'i'))
	{
		if (erc = get_ipaddr_zstr(param, interfaceIpAddrString,
			sizeof(interfaceIpAddrString)))
		return (erc);
	}

	if (erc = gint(param, &portnum))
		return (erc);

	if (erc = get_alphanum_zstr(param, _exception_label, sizeof(_exception_label)))
		return (erc);
	if (!find_cproc_labelled_lcb(_exception_label))
	{
		pprintf("goto/gosub label not found: %s\n", _exception_label);
		return (eFATAL_ALREADY);
	}

	if(interfaceIpAddrString[0] == '0')
		ipaddr = INADDR_ANY;
	else
		ipaddr = inet_atou(interfaceIpAddrString);
	if (proc_trace)
		pprintf("sockserv interface = %s\n",inet_utoa(ipaddr));

	if(erc = listenTCPWithTimeout(ipaddr,(UINT16)portnum,240 * 1000,&fd))
	{
		pprintf("listenTCPWithTimeout erc = %04X (%s)\n",erc,
			erc_text(erc));
		goto EXIT;
	}

	shm->Liofd = fd;
	shm->Ltelnet = 1;
	shm->Lconnected = 1;
	shm->Lsockserve = 1;

	if (proc_trace)
		pprintf("sockserv wire fd= %d\n",fd);

	i = sizeof(*hisAddr);
	getpeername(fd, (struct sockaddr *)hisAddr, &i);
	sprintf(shm->Lipaddr_str, "%s:%d",
		inet_utoa(htonl(hisAddr->sin_addr.s_addr)),
		htons(hisAddr->sin_port));
	if (proc_trace)
		pprintf("sockserv connected to %s\n", shm->Lipaddr_str);
	iv[0] = 0;

EXIT:;
	if(proc_trace)
		pprintf("sockserv exiting with $i0 to %d\n",(int)iv[0]);
	return(0);
	
}	/* end of pcmd_sockserv */

/*+-------------------------------------------------------------------------
	pcmd_sockclose(param)
--------------------------------------------------------------------------*/
int
pcmd_sockclose(param)
ESD *param;
{
	if (!shm->Lsockserve)
	{
		pprintf("line not opened by sockserve\n");
		return (eFATAL_ALREADY);
	}

	close(shm->Liofd);
	shm->Ltelnet = 0;
	shm->Lconnected = 0;
	shm->Lsockserve = 0;

	if (proc_trace)
		pprintf("sockclose closed fd= %d\n",shm->Liofd);

	return(0);
	
}	/* end of pcmd_sockclose */


#endif /* CFG_TelnetOption */
