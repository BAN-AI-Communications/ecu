/* vi: set tabstop=4 shiftwidth=4: */
/*+-----------------------------------------------------------------------
  MACsetclk.c -- get time from MAC, set system time,
                 update cmos clock (/dev/cmos or /dev/rtc)
	...!gatech!kd4nc!n4hgf!wht

  The output format of the MAC is
  
  000000000011111111112222
  012345678901234567890123
  
  09:12:52.3-AM-06/23/87^M
            ^^^^
            spaces if 24hr mode

  Note: must be root to execute

  Exit status codes for the program:
  1-n  signal values if program killed
  200  illogical cmos device behavior (when reading clock) (/dev/cmos only)
  249  stime() call failed
  250  could not open communications line
  251  /dev/{cmos,rtc} read&write access denied
  252  user is not root
  253  usage
  254  couldn't get time from line (after 40 retries! BAD line conditions)
  255  could not establish a connection or line error

  Use -DUTC_CLK if you wish your cmos clock kept in UTC, else
  time will be kept in local time.  It is recommended that the clock be
  kept UTC under UNIX since (I think) standard UNIX utilities expect
  it that way.

------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:01-28-1990-04:55-wht-working on cmostime version 4 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termio.h>

#ifndef UINT16
#define UINT16	unsigned short
#endif
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef UINT
#define UINT	unsigned int
#endif
#ifndef UINT32
#define UINT32	unsigned long
#endif

void usage(void);
void hangup(int );
int make_lock_name(char *,char *);
int create_lock_file(char *);
char *other_lock_name(char *);
int lock_tty(void );
void unlock_tty(void );
int valid_baud_rate(unsigned int );
int lset_baud_rate(int );
void lset_parity(int );
void lgetc(char *);
void lputc(char );
void lputs_paced(int ,char *);
char *lgets_timeout(struct lrwt *);
int lgetc_timeout(unsigned long );
void lkill_buf(void );
int lopen(void );
void lclose(void );
int hayes_send_cmd(char *);
int hayes_dial(void );

UINT16	geteuid();
UINT16	getuid();
long	nap(long);
long time(long *);
char *ctime();

typedef struct lrwt	/* param to lgets_timeout in eculine.c */
{
	UINT32 to1;		/* timeout for 1st character (granularity 20) */
	UINT32 to2;		/* timeout for each next char (granularity 20) */
	int raw_flag;	/* !=0, rtn full buffer, ==0, rtn filtered hayes result */
	char *buffer;	/* buffer to fill */
	int bufsize;	/* size of buffer */
	int count;		/* from proc, count rcvd */
}	LRWT;

#define	EPOCH		40587					/* UNIX starts JD 2440587, */
#define	leap(y,m)	((y+m-1 - 70%m) / m)	/* also known as 1/1/70 */
#define	NBSTONE		'*'						/* time "hack" */
#define	NBSFMT		"%05ld %03d %02d%02d%02d UTC"

#define pf	printf
#define	ff	fprintf
#define se	stderr
#define so	stdout

/* lopen() and related routines error codes */
#define LOPEN_INVALID	-1		/* for invalid tty name */
#define LOPEN_UNKPID	-2		/* unknown pid using line */
#define LOPEN_LCKERR	-3		/* lock file open error */
#define LOPEN_NODEV		-4		/* device does not exist */
#define LOPEN_OPNFAIL	-5		/* count not open line */
#define LOPEN_ALREADY	-6		/* line already open */

extern char *revision;		/* ecurev.c temp file from buildrev */
extern char *numeric_revision;	/*ecunumrev.c */

char LLCKname[128];		/* lock file name */
int Lmodem_already_init = 0;
char Ltelno[64];		/* telephone number for remote or null */
char Lline[64];			/* line name */
int Liofd;				/* file descriptor for line */
int Lparity;			/* 0==NONE, 'e' == even, 'o' == odd */
struct termio	Ltermio;	/* attributes for the line to remote */
UINT Lbaud;				/* baud rate */
UINT16 euid;
UINT16 uid;
char *cmos = "/dev/cmos";

/*+-------------------------------------------------------------------------
	usage()
--------------------------------------------------------------------------*/
void
usage()
{
	ff(se,"Usage: NBSsetclk [-][-e][-o][-n][-b#][-t#][-l<name>]\n");
	ff(se,"Defaults 1200-N %s %s\n",Ltelno,Lline);
	ff(se," -        use defaults\n");
	ff(se," -e       even parity\n");
	ff(se," -o       odd parity\n");
	ff(se," -n       no parity\n");
	ff(se," -b#      baud rate\n");
	ff(se," -t#      telephone number\n");
	ff(se," -l<name> line (e.g., /dev/tty2a)\n");
	exit(253);

}	/* end of usage */

/*+-----------------------------------------------------------------------
	hangup(sig) -- terminate program (with comm line cleanup)
------------------------------------------------------------------------*/
void
hangup(sig)
int sig;
{
void	lclose();

	ff(se,"\nhangup %d (see code)\n",sig);
	if(Liofd != -1)
		lclose();			/* close line */
	exit(sig);
}	/* end of hangup */

/*+-------------------------------------------------------------------------
	make_lock_name(ttyname,lock_file_name)
--------------------------------------------------------------------------*/
make_lock_name(ttyname,lock_file_name)
register char *ttyname;
char *lock_file_name;
{
	if(strncmp(ttyname,"/dev/tty",8))
		return(LOPEN_INVALID);

	strcpy(lock_file_name,"/usr/spool/uucp/LCK..");
	strcat(lock_file_name,ttyname + 8);
	return(0);

}	/* end of make_lock_name */

/*+-----------------------------------------------------------------------
	create_lock_file()

  Returns 0 if lock file created, else error codes:
	LOPEN_ if error
    else pid of process currently busy on device
  Everybody's uucp is diffeent - hack this if you like
------------------------------------------------------------------------*/
create_lock_file(lock_file_name)
char *lock_file_name;
{
register int fdlock;
int pid;
int old_umask;

	old_umask = umask(0);

	if((fdlock = open(lock_file_name,O_CREAT | O_EXCL | O_RDWR,0666)) < 0)
	{		/* file already exists */
		if((fdlock = open(lock_file_name,O_RDWR,0666)) < 0)
		{
			umask(old_umask);
			return(LOPEN_LCKERR);
		}

		if(read(fdlock,(char *)&pid,sizeof(pid)) > 0)
		{
			if(kill(pid,0) && (errno == ESRCH))	/* is owner dead? */
			{
				pid = getpid();
				lseek(fdlock,0L,0);
				write(fdlock,(char *)&pid,sizeof(pid));
				close(fdlock);
				umask(old_umask);
				return(0);
			} 
			/* owner pid still active with lock */
			close(fdlock);
			umask(old_umask);
			return(pid);
		}
		close(fdlock);
		umask(old_umask);
		return(LOPEN_UNKPID);
	} 
	pid = getpid();
	write(fdlock,(char *)&pid,sizeof(pid));
	close(fdlock);
	chmod(lock_file_name,0666);
	umask(old_umask);
	return(0);

}	/* end of create_lock_file */

/*+-------------------------------------------------------------------------
	other_lock_name(first_lock_name)
--------------------------------------------------------------------------*/
char *
other_lock_name(first_lock_name)
char *first_lock_name;
{
register int itmp;
static char other_lock_name[64];

	strcpy(other_lock_name,first_lock_name);
	itmp = strlen(other_lock_name) - 1;
	if(islower(other_lock_name[itmp]))
		other_lock_name[itmp] = toupper(other_lock_name[itmp]);
	else if(isupper(other_lock_name[itmp]))
		other_lock_name[itmp] = tolower(other_lock_name[itmp]);

	return(other_lock_name);
		
}	/* end of other_lock_name */

/*+-------------------------------------------------------------------------
	lock_tty()
--------------------------------------------------------------------------*/
lock_tty()
{
register int itmp;
struct stat ttystat;

	if(itmp = make_lock_name(Lline,LLCKname))
		return(itmp);

	if(stat(Lline,&ttystat) < 0)
		return(LOPEN_NODEV);

	if(itmp = create_lock_file(LLCKname))
		return(itmp);

	if(itmp = create_lock_file(other_lock_name(LLCKname)))
	{
		unlink(LLCKname);
		LLCKname[0] = 0;
		return(itmp);
	}

}	/* end of lock_tty */

/*+-----------------------------------------------------------------------
	void unlock_tty()
------------------------------------------------------------------------*/
void
unlock_tty()
{

	if(LLCKname[0] == 0)
		return;
	unlink(LLCKname);
	unlink(other_lock_name(LLCKname));
	LLCKname[0] = 0;
}	/* end of unlock_tty */

/*+-------------------------------------------------------------------------
	valid_baud_rate(baud) -- returns (positive) baud rate selector
or -1 if invalid baud rate
--------------------------------------------------------------------------*/
valid_baud_rate(baud)
UINT	baud;
{
	switch(baud)
	{
		case 110: return(B110);
		case 300: return(B300);
		case 600: return(B600);
		case 1200: return(B1200);
		case 2400: return(B2400);
		case 4800: return(B4800);
		case 9600: return(B9600);
		case 19200: return(EXTA);
		case 38400: return(EXTB);
		default: return(-1);
	}
}	/* end of valid_baud_rate */

/*+-----------------------------------------------------------------------
	lset_baud_rate(ioctl_flag)

  If 'ioctl_flag' is set,then ioctl(Liofd,TCSETA,&Ltermio)
  is executed after setting baud rate
------------------------------------------------------------------------*/
lset_baud_rate(ioctl_flag)
int ioctl_flag;
{
int baud_selector = valid_baud_rate(Lbaud);

	if(baud_selector < 0)
	{
		ff(se,"invalid baud rate: %u\n",Lbaud);
		ff(se,"valid rates: 110,300,600,1200,2400,4800,9600,19200\n");
		return(1);
	}
	Ltermio.c_cflag &= ~CBAUD;
	Ltermio.c_cflag |= baud_selector;

	if(ioctl_flag)
		 ioctl(Liofd,(int)TCSETA,(char *)&Ltermio);
	return(1);

}	/* end of lset_baud_rate */

/*+-----------------------------------------------------------------------
	lset_parity(ioctl_flag)

  If 'ioctl_flag' is set,then ioctl(Liofd,TCSETA,&Ltermio)
  is executed after setting parity
------------------------------------------------------------------------*/
void
lset_parity(ioctl_flag)
int ioctl_flag;
{
	Ltermio.c_cflag &= ~(CS8 | PARENB | PARODD);
	switch(Lparity)
	{
		case 'e':
			Ltermio.c_cflag |= CS7 | PARENB;
			Ltermio.c_iflag |= ISTRIP;
			break;
		case 'o':
			Ltermio.c_cflag |= PARODD | CS7 | PARENB;
			Ltermio.c_iflag |= ISTRIP;
			break;
		default:
			ff(se,"invalid parity: %c ... defaulting to no parity\n");
		case 0:
		case 'n':
			Ltermio.c_cflag |= CS8;
			Ltermio.c_iflag &= ~(ISTRIP);
			Lparity = 0;
			break;
	}			

	if(ioctl_flag)
		 ioctl(Liofd,(int)TCSETA,(char *)&Ltermio);

}	/* end of lset_parity */

/*+-------------------------------------------------------------------------
	lgetc(char_rtnd)
--------------------------------------------------------------------------*/
void
lgetc(char_rtnd)
char *char_rtnd;
{
	if(read(Liofd,char_rtnd,1) < 1)
	{
		perror("line read error");
		hangup(255);
	}
}	/* end of lgetc */

/*+-----------------------------------------------------------------------
	lputc(lchar) -- write lchar to comm line
------------------------------------------------------------------------*/
void
lputc(lchar)
char lchar;
{
register int itmp;

	if((itmp = write(Liofd,&lchar,1)) < 1)
	{
		if(itmp < 0)
			perror("\nline write error");
		else
			fputs("\nline write error\n",se);
		hangup(255);
	}
}	/* end of lputc */

/*+-----------------------------------------------------------------------
	lputs_paced(pace_msec,string) -- write string to comm line
  with time between each character 
------------------------------------------------------------------------*/
void
lputs_paced(pace_msec,string)
register int pace_msec;
register char *string;
{
register long msec = (pace_msec) ? (long)pace_msec : 20L;

	while(*string)
	{
		lputc(*string++);
		nap(msec);
	}

}	/* end of lputs_paced */

/*+-------------------------------------------------------------------------
	char *lgets_timeout(LRWT *)

Refer to LWRT structure.

to1 and to2 are unsigned long values in milliseconds (not currently
supported well under BSD4); to1 is the time to wait for the first
character, to2 the time to wait for subsequent characters.

if raw_flag 0,     non-printables are stripped from beginning
                   and end of received characters (i.e., modem
                   response reads); NULs discarded, parity stripped
if raw_flag 1,     full raw read buffer returned
if raw_flag 2,     full buffer, NULs discarded, parity stripped

buffer is address to read chars into

bufsize is buffer max size (allowing room for terminating null) which
should be at least 2 if raw_size includes 0x80 bit, else at least 12
characters if 0x80 omitted.

count is a int which, at return, receives the actual count read

--------------------------------------------------------------------------*/
char *
lgets_timeout(lrwt)
LRWT	*lrwt;
{
register int actual_count = 0;
register char *cptr = lrwt->buffer;
int max_count = lrwt->bufsize;
char *rtn_val;
int timeout_counter;
int qc1,qc2;
long quantum,ltmp;

/* minimum wait is 60 msec */
	if(Lbaud < 300)
		if(lrwt->to2 < 300L) lrwt->to2 = 300L;
	if(Lbaud < 1200)
		if(lrwt->to2 < 200L) lrwt->to2 = 200L;
	else
		if(lrwt->to2 < 60L) lrwt->to2 = 60L;

/* shortest interval */
	ltmp = (lrwt->to1 < lrwt->to2) ? lrwt->to1 : lrwt->to2;

/* calculate wait quantum */
	quantum = ltmp / 10L;				/* try for ten ticks */
	if(quantum < 20L)
		quantum = 20L;
	qc1 = lrwt->to1 / quantum;
	if(!qc1) qc1 = 1L;
	qc2 = lrwt->to2 / quantum;
	if(!qc2) qc2 = 1L;

/* perform the lrtw function
   input: qc1 is first nap count (for first charcters) 
          qc2 is 2nd nap count (for subsequent characters) 
          quantum is the nap period in milliseconds
          cptr is char* to receive read string
          max_count is max number of characters incl null
          lrwt->raw_flag as described above

  output: lrwt->count is actual count of return result
          lrwt->buffer is return read buffer
*/
	max_count--;				/* leave room for null */

	lrwt->raw_flag &= 0x0F;		/* get rid of 0xF0 flags */
	timeout_counter = qc1;		/* first timeout */ 
	*cptr = 0;					/* init result string */
	while(timeout_counter--)
	{
		nap(quantum);
		while(Rdchk(Liofd))
		{
			lgetc(cptr);
			if(lrwt->raw_flag != 1)
			{
				*cptr &= 0x7F;
				if(*cptr == 0)
					continue;
			}

			*++cptr = 0;
			actual_count++;
			if(--max_count == 0)
				goto READ_LINE_POST_PROCESS;
			timeout_counter = qc2;
		}
	}

READ_LINE_POST_PROCESS:
	if(lrwt->raw_flag)
	{
		lrwt->count = actual_count;
		return(lrwt->buffer);
	}
	cptr = lrwt->buffer;
	while(((*cptr >0) && (*cptr < 0x20)) || (*cptr >= 0x7F))
		cptr++;
	rtn_val = cptr;
	actual_count = 0;
	while(((*cptr &= 0x7F) >= 0x20) && (*cptr <= 0x7E))
	{
		cptr++;
		actual_count++;
	}
	*cptr = 0;
	strcpy(lrwt->buffer,rtn_val);
	lrwt->count = actual_count;
	return(lrwt->buffer);
}	/* end of lgets_timeout */

/*+-------------------------------------------------------------------------
	lgetc_timeout(timeout_msec)

 reads one character from line unless timeout_msec passes with no receipt.
 timeout_msec < 20 msec becomes 20 msec
 return char if received, else -1 if timeout
--------------------------------------------------------------------------*/
int
lgetc_timeout(timeout_msec)
UINT32	timeout_msec;
{
LRWT	lr;
char getc_buf[2];		/* room for one char + null */

	lr.to1 = timeout_msec;
	lr.to2 = timeout_msec;
	lr.raw_flag = 1;		/* full raw read */
	lr.buffer = getc_buf;
	lr.bufsize = sizeof(getc_buf);
	lgets_timeout(&lr);
	return(  (lr.count == 1) ? (int)getc_buf[0] : -1 );

}	/* end of lgetc_timeout */

/*+-------------------------------------------------------------------------
	lkill_buf()
--------------------------------------------------------------------------*/
void
lkill_buf()
{
	ioctl(Liofd,(int)TCFLSH,(char *)2); /* flush input and output */
}	/* end of lkill_buf */

/*+----------------------------------------------------------------------
	lopen()
returns negative LOPEN_ codes if failure else positive pid using line
else 0 if successful open
------------------------------------------------------------------------*/
int
lopen()
{
register int	itmp;

	if(Liofd >= 0)
		return(LOPEN_ALREADY);
	if(itmp = lock_tty())		/* get lock file */
		return(itmp);
	Liofd = open(Lline,O_RDWR | O_NDELAY,0777);
	if(Liofd < 0)
		return(LOPEN_OPNFAIL);
	else
	{
		ioctl(Liofd,(int)TCGETA,(char *)&Ltermio);
		Ltermio.c_iflag = (IGNPAR | IGNBRK | IXOFF );
		Ltermio.c_cflag |= (CREAD | HUPCL);
		Ltermio.c_lflag = 0;

		Ltermio.c_cc[VMIN]   = 1;
		Ltermio.c_cc[VTIME]  = 1;

		lset_baud_rate(0);		/* do not perform ioctl */
		lset_parity(1);			/* do perform ioctl */
	}

	return(0);

}	/* end of lopen */

/*+-----------------------------------------------------------------------
	lclose()
------------------------------------------------------------------------*/
void
lclose()
{
	if(Liofd < 0)
		return;
	ioctl(Liofd,(int)TCGETA,(char *)&Ltermio); /* save initial state */
	Ltermio.c_cflag |= HUPCL;
	ioctl(Liofd,(int)TCSETA,(char *)&Ltermio);
	close(Liofd);
	Liofd = -1;
	unlock_tty();		/* kill lock file */

}	/* end of lclose */

/*+-------------------------------------------------------------------------
	ltoggle_dtr()
--------------------------------------------------------------------------*/
void
ltoggle_dtr()
{
	lclose();
	nap(300L);
	lopen();
	ioctl(Liofd,TCSETA,(char *)&Ltermio);
	nap(100L);
}	/* end of ltoggle_dtr */

/*+-------------------------------------------------------------------------
	hayes_modem_init(cmd)
--------------------------------------------------------------------------*/
hayes_modem_init(cmd)
char *cmd;
{
register char *cptr;
int retry_count = 0;
int itmp;
char s32[32];

LRWT	lr;

	while(!Lmodem_already_init)
	{
		lkill_buf();
		lputs_paced(20,"ATQ0V1E1\r");
		itmp = 0;
		while(itmp != '\r')
		{
			if((itmp = lgetc_timeout(300L)) < 0)
			{
				if(retry_count > 3)
					return(-1);
				retry_count++;
				ltoggle_dtr();
				break;
			}
		}
		if(itmp < 0)
			continue;

		lr.to1 = 300L;
		lr.to2 = 100L;
		lr.raw_flag = 0;
		lr.buffer = s32;
		lr.bufsize = sizeof(s32);
		fflush(se);
		lgets_timeout(&lr);
		if(strcmp(s32,"OK"))
		{
			if(retry_count > 3)
				return(-1);
			retry_count++;
				ltoggle_dtr();
			continue;
		}
		nap(300L);
		Lmodem_already_init = 1;
	}

	return(0);

}	/* end of hayes_modem_init */

/*+-------------------------------------------------------------------------
	hayes_send_cmd(cmd)
--------------------------------------------------------------------------*/
hayes_send_cmd(cmd)
char *cmd;
{
register char *cptr;
int itmp;
char s32[32];

	hayes_modem_init();
	cptr = cmd;
	while(*cptr)
	{
		lputc(*cptr++);
		if(lgetc_timeout(500L) < 0)
			return(-1);
	}
	lputc('\r');
	if(lgetc_timeout(500L) < 0)
		return(-1);
	return(0);

}	/* end of hayes_send_cmd */

/*+-----------------------------------------------------------------------
	hayes_dial()
returns 1 on success (CONNECT), 
		0 if failure to connect
		-1 if cannot talk to modem
------------------------------------------------------------------------*/
int
hayes_dial()
{
register int itmp;
char s128[128];
int rtn_code = -1;	/* assume fail, CONNECT will chg to zero */
int s7;
LRWT	lr;

	s7 = 30;
	strcpy(s128,"ATV1E1S11=45DT" );
	strcat(s128,Ltelno);

	if(itmp = hayes_send_cmd(s128))
		return(itmp);

/* some modems (ahem, the older Hayes 2400) do not accurately honor S7 */
	lr.to1 = s7 * 2 * 1000L;
	lr.to2 = 100L;
	lr.raw_flag = 0;
	lr.buffer = s128;
	lr.bufsize = sizeof(s128);
	ff(se,"dialing %s ... INT to abort ... ",Ltelno);
	fflush(se);
	lgets_timeout(&lr);
	if(lr.count)
		ff(se,"%s\n",s128);
	if(strncmp(s128,"CONNECT",7) == 0)
		return(1);
	return(0);

}	/* end of hayes_dial */

/*+-------------------------------------------------------------------------
	main(argc,argv,envp)

  main() program forks to create rcvr process; then main()
  becomes the xmtr process
------------------------------------------------------------------------*/
main(argc,argv,envp)
int argc;
char **argv;
char **envp;
{
char *cptr;
int iargv;
int swchar;
int itmp;
LRWT lr;
char rd_buf[64];
long now;
long julian;
long time_at_connect;
int day_of_year,year,month,day,hour,min,sec;
int have_time = 0;
int max_tries;
struct tm *lt;
struct tm *gmtime();
struct tm *localtime();
extern char *optarg;

	setbuf(stderr,NULL);
	setbuf(stdout,NULL);

/* init line variables */
	strcpy(Lline,DEFTTY);
	strcpy(Ltelno,"1(202)653-0351");
	Liofd = -1;
	Lbaud = 1200;
	Lparity = 0;

	if(argc < 2)
		usage();

	if((argc == 2) && (!strcmp(argv[1],"-")))
		;
	else
	{
		while((swchar = getopt(argc,argv,"eonb:t:l:")) != -1)
		{
			switch(swchar)
			{
				case 'e': Lparity = 'e'; break;
				case 'o': Lparity = 'o'; break;
				case 'n': Lparity =  0 ; break;
				case 'b': Lbaud = atoi(optarg); break;
				case 't': strcpy(Ltelno,optarg); break;
				case 'l': strcpy(Lline,optarg); break;
				default:  usage();
			}
		}
	}

	uid = getuid();
	euid = geteuid();
	if((euid == 0) || (uid == 0))	/* if root running or prog text ... */
		nice(-40);
	else
	{
		ff(se,"must be root\n");
		exit(252);
	}

/* before we make a long distance call, make sure we can access clock */
	get_clock(&year,&month,&day,&hour,&min,&sec);

	signal(SIGHUP,hangup);
	signal(SIGQUIT,hangup);
	signal(SIGINT,hangup);
	signal(SIGTERM,hangup);

	ff(se,"using %s, ",Lline);
	if(itmp = lopen())
	{
		switch(itmp)
		{
			case LOPEN_INVALID:
				ff(se,"invalid line name\n"); break;
			case LOPEN_UNKPID:
				ff(se,"unknown pid is using line\n"); break;
			case LOPEN_LCKERR:
				ff(se,"lock file error\n"); break;
			case LOPEN_NODEV:
				ff(se,"line does not exist\n"); break;
			case LOPEN_ALREADY:
				ff(se,"line already open\n"); break;
			case LOPEN_OPNFAIL:
				ff(se,"line open error\n"); break;
			default:
				ff(se,"pid %d using line\n",itmp); break;
		}
		exit(250);
	}

	lr.to1 = 1100L;
	lr.to2 =  100L;
	lr.raw_flag = 0;	/* full raw read */
	lr.buffer = rd_buf;
	lr.bufsize = sizeof(rd_buf);

/* spend money on long distance call ... only time freaks can understand */
	if(!hayes_dial())
	{
		fputs("\ndial failed\n",se);
		hangup(255);
	}

	time_at_connect = time((long *)0);

	max_tries = 40;		/* 40 sec max connect time (charged for 1 min anyway) */
	while(!have_time)
	{
		for(itmp = 0; itmp < 30; itmp++)		/* look for asterisk */
		{
			if(lgetc_timeout(500L) == NBSTONE)
				break;
		}

		lgets_timeout(&lr);		/* get date/time string */

		if(sscanf(lr.buffer,NBSFMT,&julian,&day_of_year,&hour,&min,&sec) != 5)
		{
			ff(se,"garbled result: '%s'  ",lr.buffer);
			if(--max_tries)
			{
				puts("... retrying");
				continue;
			}
			puts("... aborting");
			break;		/* too many retries ... BAAAAD line condx */
		}
		have_time = 1;
	}

	lclose();

/* no longer spending money */

	fprintf(stdout,"Connect time %ld second(s)\n",
		time((long *)0) - time_at_connect);

	if(have_time)
	{
		now = (((julian - EPOCH) * 24 + hour) * 60 + min) * 60 + sec;
		now--; /* off by one */
		if(stime(&now) < 0)
		{
			perror("stime");
			exit(249);
		}
		fputs("Time retrieved from standard: ",stdout);
		fputs(ctime(&now),stdout);
		time(&now);
		get_clock(&year,&month,&day,&hour,&min,&sec);
		printf("cmos time before setting: %02d/%02d/%04d %02d:%02d:%02d %s\n",
			month+1,day+1,year,hour,min,sec,
#if defined(UTC_CLK)
			"UTC"
#else
			"local"
#endif
			);
		fputs("           after setting: ",stdout);
#if defined(UTC_CLK)
		lt = gmtime(&now);
#else
		lt = localtime(&now);
#endif
		set_clock(lt->tm_year,lt->tm_mon,lt->tm_mday,
				lt->tm_hour,lt->tm_min,lt->tm_sec);
		get_clock(fdcmos,&year,&month,&day,&hour,&min,&sec);
		printf("%02d/%02d/%04d %02d:%02d:%02d %s\n",
			month+1,day+1,year,hour,min,sec,
#if defined(M_UNIX)
			"UTC"
#else
			"local"
#endif
			);
		exit(0);
	}
	else
	{
		puts("Did not get time ... sorry");
		exit(254);
	}

}	/* end of main */
/* end of NBSsetclk.c */

