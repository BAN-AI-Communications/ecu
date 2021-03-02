/*+-------------------------------------------------------------------------
	qmake.c - quad-make for WHT development
	wht@wht.net

Make up to 4 objects at a time ( for yuriatin quad-m88k)

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:16-wht@bob-RELEASE 4.42 */
/*:09-02-1998-13:59-wht@menlo-make 8 at a time */
/*:12-15-1997-20:32-wht@fep-fix pause() unreliability */
/*:01-24-1997-02:38-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-30-1996-00:52-wht@yuriatin-creation */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <fcntl.h>
#include <errno.h>
#include "ecu_config.h"

#include "ecu_types.h"
#include "ecu_time.h"
#include "ecu_stat.h"
#include <sys/wait.h>
#include <time.h>

int keep_all_qfiles = 0;
time_t start_epoch;
int next = 0;

typedef struct target
{
	char *ofn;
	char *qfn;
	int status;
	int exit_status;
	pid_t pid;
	time_t start_epoch;
	time_t end_epoch;
}
T;

#define STATUS_NOTSTARTED 0
#define STATUS_RUNNING 1
#define STATUS_DONE 2

#define TMAX 1024
#define MMAX 8				 /* max makes outstanding */

T   t[TMAX];
int tq = 0;
int targets_to_make;
int running;
int in_progress;

#define MAX_EXEC_ARG 10

char *str_token_static = (char *)0;

/*+-------------------------------------------------------------------------
	status_text(status)
--------------------------------------------------------------------------*/
char *
status_text(status)
int status;
{
	switch (status)
	{
		case STATUS_NOTSTARTED:
			return ("NOTSTARTED");
		case STATUS_RUNNING:
			return ("RUNNING");
		case STATUS_DONE:
			return ("DONE");
	}
	return ("???");
}							 /* end of status_text */

/*+-----------------------------------------------------------------------
	arg_token(parsestr,termchars)

Get next token from string parsestr ((char *)0 on 2nd, 3rd, etc.
calls), where tokens are nonempty strings separated by runs of chars
from termchars.  Writes nulls into parsestr to end tokens.
termchars need not remain constant from call to call.

Treats multiple occurrences of a termchar as one delimiter (does not
allow null fields).
------------------------------------------------------------------------*/
char *
arg_token(parsestr, termchars)
char *parsestr;
char *termchars;
{
	char *parseptr;
	char *token;

	if (!parsestr && !str_token_static)
		return ((char *)0);

	if (parsestr)
	{
		str_token_static = (char *)0;
		parseptr = parsestr;
	}
	else
		parseptr = str_token_static;

	while (*parseptr)
	{
		if (!strchr(termchars, *parseptr))
			break;
		parseptr++;
	}

	if (!*parseptr)
	{
		str_token_static = (char *)0;
		return ((char *)0);
	}

	token = parseptr;

	/*
	 * tokens beginning with apostrophe or quotes kept together
	 */
	if (*token == '\'')
	{
		token++;
		parseptr++;
		while (*parseptr)
		{
			if (*parseptr == '\'')
			{
				str_token_static = parseptr + 1;
				*parseptr = 0;
				return (token);
			}
			parseptr++;
		}
		str_token_static = (char *)0;
		return (token);
	}
	else if (*token == '"')
	{
		token++;
		parseptr++;
		while (*parseptr)
		{
			if (*parseptr == '"')
			{
				str_token_static = parseptr + 1;
				*parseptr = 0;
				return (token);
			}
			parseptr++;
		}
		str_token_static = (char *)0;
		return (token);
	}

	while (*parseptr)
	{
		if (strchr(termchars, *parseptr))
		{
			*parseptr = 0;
			str_token_static = parseptr + 1;
			while (*str_token_static)
			{
				if (!strchr(termchars, *str_token_static))
					break;
				str_token_static++;
			}
			return (token);
		}
		parseptr++;
	}
	str_token_static = (char *)0;
	return (token);
}							 /* end of arg_token */

/*+-------------------------------------------------------------------------
	build_arg_array(cmd,arg,arg_max_quan,&narg)
--------------------------------------------------------------------------*/
void
build_arg_array(cmd, arg, arg_max_quan, narg_rtn)
char *cmd;
char **arg;
int arg_max_quan;
int *narg_rtn;
{
	int narg;

	str_token_static = (char *)0;
	memset((char *)arg, 0, sizeof(char *) * arg_max_quan);

	if (!(arg[0] = arg_token(cmd, " \t\r\n")))
	{
		*narg_rtn = 0;
		return;
	}

	for (narg = 1; narg < arg_max_quan; ++narg)
	{
		if (!(arg[narg] = arg_token((char *)0, " \t\r\n")))
			break;
	}

	*narg_rtn = narg;

}							 /* end of build_arg_array */

/*+-------------------------------------------------------------------------
	is_executable(progname)
--------------------------------------------------------------------------*/
int
is_executable(progname)
char *progname;
{
	struct stat ss;

	if (stat(progname, &ss) < 0)	/* if cannot stat, flunk */
		return (0);
	if ((ss.st_mode & 0111) == 0)	/* if no --x--x--x, flunk */
		return (0);
	if ((ss.st_mode & S_IFMT) != S_IFREG)	/* if no --x--x--x, flunk */
		return (0);
	return (1);				 /* whew, this OUGHT to work */

}							 /* end of is_executable */

/*+-------------------------------------------------------------------------
	find_executable(progname)
PATH=':/usr/wht/bin:/bin:/usr/bin:/usr/wht/bin:/etc/tuckerware' len=56
--------------------------------------------------------------------------*/
char *
find_executable(progname)
char *progname;
{
	int itmp;
	static char *path_buf = (char *)0;

#define PATHNAME_QUAN 64
	static char *path_name[PATHNAME_QUAN + 1];
	static char rtn_path[256];
	static int path_count = 0;
	char *cp;
	char *getenv();

	if (path_buf == (char *)0)
	{
		if ((cp = getenv("PATH")) == (char *)0)
			return (cp);
		if (!(path_buf = malloc(strlen(cp) + 1)))
			return ((char *)0);
		strcpy(path_buf, cp);
		path_name[PATHNAME_QUAN] = (char *)0;
		cp = path_buf;
		for (path_count = 0; path_count < PATHNAME_QUAN; path_count++)
		{
			if (*cp == 0)
				break;
			path_name[path_count] = cp;
			while ((*cp != ':') && (*cp != 0))
				cp++;
			if (*cp == ':')
				*cp++ = 0;
		}
	}						 /* end of get and process path env variable */

/* look for executable */
	for (itmp = 0; itmp < path_count; itmp++)
	{
		if (*path_name[itmp] == 0)	/* if null path (./) */
			strcpy(rtn_path, "./");
		else
			sprintf(rtn_path, "%s/", path_name[itmp]);
		strcat(rtn_path, progname);
		if (is_executable(rtn_path))
			return (rtn_path);
	}
	return ((char *)0);
}							 /* end of find_executable */
/*+-------------------------------------------------------------------------
	execute(cmdstr) - execute an arbitrary program with arguments
kills rcvr process if alive and restarts it when done if was alive
--------------------------------------------------------------------------*/
int
execute(cmdstr)
char *cmdstr;
{
	char *cmdpath;
	char *cmdargv[MAX_EXEC_ARG];
	int itmp;
	pid_t child_pid;

	build_arg_array(cmdstr, cmdargv, MAX_EXEC_ARG, &itmp);
	if (itmp == MAX_EXEC_ARG)
	{
		puts("Too many arguments to command\n");
		return (-1);
	}
	else if (!itmp)
	{
		puts("null command\n");
		return (-1);
	}

	if (*cmdargv[0] == '/')
	{
		cmdpath = cmdargv[0];
		cmdargv[0] = strrchr(cmdargv[0], '/') + 1;
	}
	else
	{
		if ((cmdpath = find_executable(cmdargv[0])) == (char *)0)
		{
			printf("Cannot find %s\n", cmdargv[0]);
			return (-1);
		}
	}

	if ((child_pid = fork()) < 0)
	{
		puts("Cannot fork\n");
		return (-1);
	}

	if (child_pid == 0)		 /* we are the spawned (going to call exec) */
	{
		sleep(1);			/* make sure fast systems let parent run again */
		execv(cmdpath, cmdargv);
		perror(cmdpath);
		_exit(255);			 /* end of spawned process */
	}						 /* end of if child process */

	return (child_pid);

}							 /* end of execute */

/*+-------------------------------------------------------------------------
	start_make(cmd)
--------------------------------------------------------------------------*/
pid_t
start_make(cmd)
char *cmd;
{
	return (execute(cmd));
}							 /* end of start_make */

/*+-------------------------------------------------------------------------
	estr()
--------------------------------------------------------------------------*/
char *
estr()
{
	static char s64[64];
	time_t now = time(0);

	sprintf(s64, "%10ld", (long)now - (long)start_epoch);
	return (s64);
}							 /* end of estr */

/*+-------------------------------------------------------------------------
	die()
--------------------------------------------------------------------------*/
void
die()
{
	T  *tp;

	for (tp = t; tp < (t + tq); tp++)
	{
		printf("%s %s (%s) %ld secs\n",
			tp->ofn,
			status_text(tp->status),
			(tp->exit_status) ? "FAILED" : "succeeded",
			(long)(tp->end_epoch) - (long)(tp->start_epoch));
	}
	exit(1);

}							 /* end of die */

/*+-------------------------------------------------------------------------
	report()
--------------------------------------------------------------------------*/
void
report()
{
	T  *tp;
	int exit_status = 0;

	for (tp = t; tp < (t + tq); tp++)
	{
		if (tp->exit_status)
		{
			printf("%s failed exit status %d\n",
				tp->ofn, tp->exit_status);
			exit_status++;
		}
	}
	printf("qmake: exit status %d\n",exit_status);
	exit(exit_status);
}							 /* end of report */

/*+-------------------------------------------------------------------------
	sigint(sig)
--------------------------------------------------------------------------*/
void
sigint(sig)
int sig;
{
#if 0
	printf("qmake: signal %d\n",sig);
	exit(100+sig);
#else
	T  *tp;
	int exit_status = 100+sig;

	signal(SIGINT,SIG_DFL);
	signal(SIGQUIT,SIG_DFL);

	printf("\ninterrupted\n");

	for (tp = t; tp < (t + tq); tp++)
	{
		char *status = status_text(tp->status);
		switch(tp->status)
		{
			case STATUS_RUNNING:
				break;
			case STATUS_DONE:
			default:
				if(sig == SIGINT)
					continue;
				break;
		}
		printf("%s %s %d\n", tp->ofn, status, tp->pid);
	}

	printf("qmake: exit status %d\n",exit_status);
	exit(exit_status);
#endif
}	/* end of sigint */

/*+-------------------------------------------------------------------------
	reap(sig)
--------------------------------------------------------------------------*/
void
reap(sig)
int sig;
{
	int stat_loc;
	pid_t pid;
	int found = 0;
	T  *tp;

	if ((pid = wait(&stat_loc)) < 0)
	{
		switch (errno)
		{
			case ECHILD:
				printf("!! reap() found ECHILD\n");
				die();
			default:
				perror("wait");
				die();
		}
	}
	found = 0;
	for (tp = t; tp < (t + next); tp++)
	{
		if (tp->pid == pid)
		{
			char s128[128];

			tp->end_epoch = time(0);
			tp->exit_status = WEXITSTATUS(stat_loc);
			tp->status = STATUS_DONE;
			printf("%s     %s done (%s) %ld secs\n",
				estr(), tp->ofn,
				(tp->exit_status) ? "FAILED" : "succeeded",
				(long)(tp->end_epoch) - (long)(tp->start_epoch));
			if(!tp->exit_status && !keep_all_qfiles)
				unlink(tp->qfn);
			found = 1;
			tp->status = STATUS_DONE;
			running--;
			targets_to_make--;
			break;
		}
	}
	if (!found)
	{
		printf("did not match terminated pid %d\n", pid);
		die();
	}
	signal(SIGCLD, reap);
}							 /* end of reap */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{

	int i;
	char s256[256];
	int errflg = 0;
	extern char *optarg;
	extern int optind;

	setbuf(stdout,0);
	setbuf(stderr,0);

	while ((i = getopt(argc, argv, "k")) != -1)
	{
		switch (i)
		{
			case 'k':
				keep_all_qfiles = 1;
				break;

			case '?':
				errflg++;
				break;
		}
	}
	if (errflg || (optind == argc))
	{
		fprintf(stderr, "qmake [-k] o-files...\n");
		exit(1);
	}

	while (optind < argc)
	{
		t[tq].ofn = strdup(argv[optind]);
		sprintf(s256,"%s.q",t[tq].ofn);
		t[tq].qfn = strdup(s256);
		t[tq].status = 0;
		tq++, optind++;
	}

	targets_to_make = tq;
	running = 0;
	start_epoch = time(0);
	signal(SIGCLD, reap);
	signal(SIGINT, sigint);
	signal(SIGQUIT, sigint);

	while (targets_to_make)
	{
		struct timeval tv;

		/*
		 * spawn up to MMAX makes
		 */
		while ((running < MMAX) && (next < tq))
		{

			sprintf(s256, "sh -c 'time make %s > %s 2>&1'",
				t[next].ofn, t[next].qfn);
			t[next].start_epoch = time(0);
			t[next].pid = start_make(s256);
			t[next].status = STATUS_RUNNING;
			printf("%s %s started pid=%d\n", estr(), t[next].ofn, t[next].pid);
			running++;
			next++;
		}

		/*
		 * now wait on outcomes; pause() seems unreliable
		 */

		memset((char *)&tv,0,sizeof(tv));
		tv.tv_usec = 500*1000;
		select(0,0,0,0,&tv);
		if (!running)
			break;
	}

	report();

}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of qmake.c */
