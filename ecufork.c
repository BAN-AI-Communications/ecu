#if 0
#define FORK_DEBUG
#endif
/*+-------------------------------------------------------------------------
	ecufork.c -- ecu spawning ground
	wht@wht.net

  Defined functions:
	exec_cmd(cmdstr)
	expand_wildcard_list(wild, expcmd)
	extract_stdin_stdout_stderr(cmdargv,pfd_stdin,pfd_stdout,pfd_stderr)
	find_executable(progname)
	fork_cmd(cmdstr)
	is_executable(progname)
	shell(shellcmd)
	smart_fork()

  This boy is no longer a boy.  Now he is a man.  He is a small
  boy, but his heart is big.  His name shall be Little Big Man.
  -- Chief Lodge Skins

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:11-03-1997-02:42-wht@kepler-4.08a-no more fork debug for wht */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:10-16-1996-20:13-wht@yuriatin-redirection took a while */
/*:10-16-1996-19:05-wht@yuriatin-support redirect on fork */
/*:10-16-1996-03:30-wht@yuriatin-add fork_cmd */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:08-11-1996-02:10-wht@kepler-rename ecu_log_event to logevent */
/*:02-28-1996-21:30-wht@kepler-must use /bin/sh for ">" commands */
/*:01-27-1996-21:04-wht@n4hgf-drop using custom SHELL - use csh or bash */
/*:01-01-1996-19:38-wht@kepler-no endwin in shell unless needed */
/*:12-11-1995-17:41-wht@kepler-any child of ecu gets pid saved in child-pid */
/*:12-10-1995-17:43-wht@kepler-use $SHELL, pwd.pw_shell, then default */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:19-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:38-wht@n4hgf-ECU release 3.30 */
/*:01-06-1994-05:52-wht@n4hgf-clean up LINUX port */
/*:12-12-1993-13:27-wht@n4hgf-differentiate MAX_EXEC_ARG only for i286 */
/*:11-12-1993-11:00-wht@n4hgf-Linux changes by bob@vancouver.zadall.com */
/*:09-10-1992-13:58-wht@n4hgf-ECU release 3.20 */
/*:08-22-1992-15:38-wht@n4hgf-ECU release 3.20 BETA */
/*:04-29-1992-13:29-wht@n4hgf-ignore SIGQUIT when in executing a child */
/*:09-25-1991-18:02-wht@n4hgf2-find_executable flunks directories now */
/*:09-06-1991-04:20-wht@n4hgf2-expand_wildcard_list minor bug */
/*:08-29-1991-01:56-wht@n4hgf2-use max esd size instead of 5120 */
/*:07-25-1991-12:55-wht@n4hgf-ECU release 3.10 */
/*:07-17-1991-07:04-wht@n4hgf-avoid SCO UNIX nap bug */
/*:09-19-1990-19:36-wht@n4hgf-logevent now gets pid for log from caller */
/*:08-14-1990-20:40-wht@n4hgf-ecu3.00-flush old edit history */

#include "ecu.h"
#include "ecu_pwd.h"
#include "esd.h"
#include "ecufork.h"

extern int last_child_wait_status;
extern int last_child_wait_pid;

CFG_PidType child_pid = -1;

#ifdef M_I286
#define MAX_EXEC_ARG 512
#else
#define MAX_EXEC_ARG 2048
#endif /* M_I286 */

#ifdef linux
char *default_shell_path = "/bin/bash";	/* probably moot since sh==bash */
#else
char *default_shell_path = "/bin/csh";

#endif

/*+-------------------------------------------------------------------------
	smart_fork()
--------------------------------------------------------------------------*/
int
smart_fork()
{
	int count = 5;
	int pid;

	while (count--)
	{
		if ((pid = fork()) >= 0)
			return (pid);
		if (count)
			Nap(40L);
	}
	return (-1);
}							 /* end of smart_fork */

/*+-----------------------------------------------------------------------
	shell(shellcmd)

  param 'shellcmd' is a shell command prefixed with either
  a '!', '$', '>' character.

  '!' causes the command to run as a normal subshell of a process.
  '$' causes the communications line to be stdin and stdout
      for the spawned shell
  '>' causes spawned shell to receive exactly sames files as ecu
------------------------------------------------------------------------*/
void
shell(shellcmd)
char *shellcmd;
{
	int itmp;
	char *shell_path;

#if defined(FORK_DEBUG)
	char s80[80];

#endif
	int wait_status;
	int restart_rcvr = need_rcvr_restart();
	char *getenv();

	kill_rcvr_process(SIGUSR1);	/* stop receiver process gracefully */

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGCLD, SIG_DFL);

	ttymode(0);				 /* set canonical tty mode */
	if ((child_pid = smart_fork()) < 0)
	{
		ff(se, "Cannot fork\r\n");
		if (restart_rcvr)
			start_rcvr_process(1);
		ttymode(1);			 /* control tty back to raw mode */
		xmtr_signals();
		return;
	}

#if defined(linux) || defined(__FreeBSD__)
	windows_end(0);
#endif

	if (!child_pid)			 /* we are the spawned (going to call shell) */
	{
		if (*shellcmd != '>')/* '>' prefix means leave fd's alone! */
		{

			/*
			 * Hook-up our "standard output" to either the tty or the line
			 * as appropriate for '!' or '$'
			 */
			close(TTYOUT);
			fcntl(((*shellcmd == '$') ? shm->Liofd : TTYERR), F_DUPFD, TTYOUT);
			if (*shellcmd == '$')
			{
				close(TTYIN);
				fcntl(shm->Liofd, F_DUPFD, TTYIN);
			}
			close(shm->Liofd);
		}

		child_signals();	 /* signals for child */

		shell_path = 0;
		if (*shellcmd == '>')
			shell_path = "/bin/sh";
		else if (*shellcmd == '!')
		{
			if (!(shell_path = getenv("SHELL")))
			{
				struct passwd *pw;

				if (pw = getpwuid(getuid()))
					shell_path = pw->pw_shell;
				endpwent();
			}
		}
		if (!shell_path)
			shell_path = default_shell_path;

		shellcmd++;
		child_signals();
		if (!*shellcmd)
			execl(shell_path, shell_path, (char *)0);
		else
			execl(shell_path, shell_path, "-c", shellcmd, (char *)0);
		ff(se, "cannot execute %s\r\n", shell_path);	/* should not get here */
		_exit(255);			 /* end of spawned process */
	}						 /* end of if child process */

#if defined(FORK_DEBUG)
	sprintf(s80, "DEBUG fork shell pid %d", child_pid);
	logevent(getpid(), s80); /* shell */
#endif

	while (((itmp = wait(&wait_status)) != child_pid) && (itmp != -1))
		;
	last_child_wait_status = wait_status;
	last_child_wait_pid = child_pid;
	child_pid = -1;

	xmtr_signals();			 /* restore standard xmtr signals */
	ttymode(1);				 /* control tty back to raw mode */

/* any comm program will probably doodle with the line characteristics. */
/* we want to make sure they are restored to normal */
	lreset_ksr();			 /* restore comm line params */

	if (restart_rcvr)
		start_rcvr_process(1);

}							 /* end of shell */

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

#define PATHNAME_QUAN 32
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
	extract_stdin_stdout_stderr(cmdargv,pfd_stdin,pfd_stdout,pfd_stderr)
--------------------------------------------------------------------------*/
int
extract_stdin_stdout_stderr(cmdargv, pfd_stdin, pfd_stdout, pfd_stderr)
char **cmdargv;
int *pfd_stdin;
int *pfd_stdout;
int *pfd_stderr;
{
	int rtn = 0;
	char *arg;

	for (; *cmdargv; cmdargv++)
	{
		arg = *cmdargv;

		/*
		 * if argument is a redirection directive
		 */
		if (strchr("><", *arg))
		{
			char *path;
			int append_flag; /* true if append to stdout/err */
			int stderr_flag; /* true if stderr as well as stdout */
			char **cmdsrc;	 /* squash pointer temp */
			char **cmddst;	 /* squash pointer temp */

			if (!(path = *(cmdargv + 1)))	/* if nothing after redirect */
			{
				pprintf("no path after '%s'\n", arg);
				return (-1);
			}
			switch (*arg)
			{
				case '<':	 /* stdin */
					if ((*pfd_stdin = open(path, O_RDONLY, 0)) < 0)
					{
						pputs("could not open for stdin: ");
						pperror(path);
						rtn = -1;
						break;
					}
					if (proc_trace)
						pprintf("opened '%s' for stdin\n", path);
					break;

				case '>':	 /* stdout */
					append_flag = !!strchr(arg + 1, '>');	/* append? */
					stderr_flag = !!strchr(arg, '&');	/* stderr too? */
					if ((*pfd_stdout = open(path, O_WRONLY | O_CREAT |
						((append_flag) ? O_APPEND : O_TRUNC), 0644)) < 0)
					{
						pputs("could not open for stdout: ");
						pperror(path);
						rtn = -1;
						break;
					}
					if (stderr_flag)
						*pfd_stderr = *pfd_stdout;
					if (proc_trace)
					{
						pprintf("opened '%s' for stdout", path);
						if (stderr_flag)
							pputs(" and stderr");
						pprintf(" (%s)\n",
							(append_flag) ? "append" : "scratch");
					}
					break;
			}

			/*
			 * squash redirect and path from arguments
			 */
			cmdsrc = cmdargv + 2;
			cmddst = cmdargv;
			while (1)
			{
				*cmddst = *cmdsrc;
				if (!*cmddst)
					break;
				cmdsrc++, cmddst++;
			}
			cmdargv--;
		}
	}
	if (rtn)
	{
		close(*pfd_stdin);
		*pfd_stdin = -1;
		close(*pfd_stdout);
		*pfd_stdout = -1;
		close(*pfd_stderr);
		*pfd_stderr = -1;
	}
	return (rtn);

}							 /* end of extract_stdin_stdout_stderr */

/*+-------------------------------------------------------------------------
	fork_cmd(cmdstr) - execute an arbitrary program with arguments
--------------------------------------------------------------------------*/
int
fork_cmd(cmdstr)
char *cmdstr;
{
	char *cmdpath;
	char *cmdargv[MAX_EXEC_ARG];
	int itmp;
	int fd_stdin;
	int fd_stdout;
	int fd_stderr;

#if defined(FORK_DEBUG)
	char s256[256];

	strcpy(s256, "DEBUG exec ");
	strncat(s256, cmdstr, sizeof(s256) - 12);
	s256[sizeof(s256) - 12] = 0;
	logevent(getpid(), s256);
#endif

	build_arg_array(cmdstr, cmdargv, MAX_EXEC_ARG, &itmp);
	if (itmp == MAX_EXEC_ARG)
	{
		pputs("Too many arguments to command\n");
		return (-1);
	}
	else if (!itmp)
	{
		pputs("null command\n");
		return (-1);
	}

	/*
	 * peel redirection out of arguments
	 */
	fd_stdin = -1;
	fd_stdout = -1;
	fd_stderr = -1;
	itmp = extract_stdin_stdout_stderr(cmdargv,
		&fd_stdin, &fd_stdout, &fd_stderr);
	if(itmp)
		return(-1);

	if (*cmdargv[0] == '/')
	{
		cmdpath = cmdargv[0];
		cmdargv[0] = strrchr(cmdargv[0], '/') + 1;
	}
	else
	{
		if ((cmdpath = find_executable(cmdargv[0])) == (char *)0)
		{
			pputs("Cannot find %s\n", cmdargv[0]);
			return (-1);
		}
	}

	if ((child_pid = smart_fork()) < 0)
	{
		pputs("Cannot fork\n");
		return (-1);
	}

	if (child_pid == 0)		 /* we are the spawned (going to call exec) */
	{
		int fd;

		child_signals();

		/*
		 * redirection
		 */
		if (fd_stdin > -1)
		{
			close(TTYIN);
			dup2(fd_stdin, TTYIN);
		}
		if (fd_stdout > -1)
		{
			close(TTYOUT);
			dup2(fd_stdout, TTYOUT);
		}
		if (fd_stderr > -1)
		{
			close(TTYERR);
			dup2(fd_stderr, TTYERR);
		}

		/*
		 * close all files except stdin/out/err and comm line
		 */
		for (fd = TTYERR + 1; fd < 100; fd++)
		{
			if (fd != shm->Liofd)
				close(fd);
		}
		execv(cmdpath, cmdargv);
		perror(cmdpath);
		_exit(255);			 /* end of spawned process */
	}						 /* end of if child process */

	return (child_pid);

}							 /* end of fork_cmd */

/*+-------------------------------------------------------------------------
	exec_cmd(cmdstr) - execute an arbitrary program with arguments
kills rcvr process if alive and restarts it when done if was alive
--------------------------------------------------------------------------*/
int
exec_cmd(cmdstr)
char *cmdstr;
{
	char *cmdpath;
	char *cmdargv[MAX_EXEC_ARG];
	int itmp;
	int restart_rcvr = need_rcvr_restart();
	int old_ttymode = get_ttymode();
	int wait_status = 0;

#if defined(FORK_DEBUG)
	char s256[256];

	strcpy(s256, "DEBUG exec ");
	strncat(s256, cmdstr, sizeof(s256) - 12);
	s256[sizeof(s256) - 12] = 0;
	logevent(getpid(), s256);
#endif

	build_arg_array(cmdstr, cmdargv, MAX_EXEC_ARG, &itmp);
	if (itmp == MAX_EXEC_ARG)
	{
		pputs("Too many arguments to command\n");
		return (-1);
	}
	else if (!itmp)
	{
		pputs("null command\n");
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
			pputs("Cannot find %s\n", cmdargv[0]);
			return (-1);
		}
	}

	kill_rcvr_process(SIGUSR1);	/* stop receiver process gracefully */

	logevent(getpid(), cmdpath);

/* this code executed by the father (forking) process */
/* wait on death of child (morbid in life, but ok here) */

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGCLD, SIG_DFL);

	if ((child_pid = smart_fork()) < 0)
	{
		pputs("Cannot fork\n");
		if (restart_rcvr)
			start_rcvr_process(1);
		xmtr_signals();
		return (-1);
	}

	if (child_pid == 0)		 /* we are the spawned (going to call exec) */
	{
		ttymode(0);			 /* set canonical tty mode */
		child_signals();
		execv(cmdpath, cmdargv);
		perror(cmdpath);
		_exit(255);			 /* end of spawned process */
	}						 /* end of if child process */

	wait_status = 0;
	while (((itmp = wait(&wait_status)) != child_pid) && (itmp != -1))
		;
	last_child_wait_status = wait_status;
	last_child_wait_pid = child_pid;
	child_pid = -1;

/* resume our normally scheduled program */
	lreset_ksr();			 /* restore comm line params */
	ttymode(old_ttymode);	 /* control tty back to original */
	if (restart_rcvr)
		start_rcvr_process(1);
	xmtr_signals();
	if (last_child_wait_status == -1)
		pprintf("last child exited with status -1\n");
	return (last_child_wait_status);

}							 /* end of exec_cmd */

/*+-------------------------------------------------------------------------
	expand_wildcard_list(wild,&expcmd)

called with 'foo <wildcardlist>' for command expansion prior to exec()
         or '<wildcardlist>' to expand a list of files.

If called with 'foo'-style wild, anything you want to protect from csh
globbing or other interpretation must be properly protected (quoted) --
AND quoting will be removed one level by the csh.

if return 0, wild has been expanded, expcmd must be free()'d when done
if return -1, error, expcmd has error message (static message: DO NOT FREE)
--------------------------------------------------------------------------*/
int
expand_wildcard_list(wild, expcmd)
char *wild;
char **expcmd;
{
	char *cp;

#define P_READ 0
#define P_WRITE 1
	int stdout_pipe[2];
	int stderr_pipe[2];
	int count;
	int expcmd_size = 0;
	int itmp;
	int wait_status;
	int restart_rcvr = need_rcvr_restart();
	char *shell_path;
	FILE *fp_pipe = (FILE *) 0;
	char *echo_cmd;
	static char static_s256[256];	/* MUST BE STATIC */
	static char *pipe_err_msg = "system error: no pipe";
	static char *mem_err_msg = "system error: no memory";

	if (strchr(wild, '<') || strchr(wild, '>') || strchr(wild, '&'))
	{
		*expcmd = "illegal characters: '<', '>' or '&'";
		return (-1);
	}

	if (pipe(stdout_pipe) < 0)
	{
		*expcmd = pipe_err_msg;
		return (-1);
	}
	if (pipe(stderr_pipe) < 0)
	{
		close(stdout_pipe[P_READ]);
		close(stdout_pipe[P_WRITE]);
		*expcmd = pipe_err_msg;
		return (-1);
	}
	if (!(echo_cmd = malloc(strlen(wild) + 10)))
	{
		close(stdout_pipe[P_READ]);
		close(stdout_pipe[P_WRITE]);
		close(stderr_pipe[P_READ]);
		close(stderr_pipe[P_WRITE]);
		*expcmd = mem_err_msg;
		return (-1);
	}

	strcpy(echo_cmd, "echo ");
	strcat(echo_cmd, wild);

	kill_rcvr_process(SIGUSR1);	/* stop receiver process gracefully */

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGCLD, SIG_DFL);

#if 0

	/*
	 * does not work for /bin/sh, so kill this code until brighter days
	 */
	shell_path = 0;
	if (!(shell_path = getenv("SHELL")))
	{
		struct passwd *pw;

		if (pw = getpwuid(getuid()))
			shell_path = pw->pw_shell;
		endpwent();
	}
	if (!shell_path)
		shell_path = default_shell_path;
#else
	shell_path = default_shell_path;
#endif

#if defined(FORK_DEBUG)
	sprintf(static_s256, "DEBUG expand sh=%s `%.128s'", shell_path, echo_cmd);
	logevent(getpid(), static_s256);	/* expand_wildcard_list */
#endif

	if ((child_pid = smart_fork()) == 0)
	{
		int null = open("/dev/null", O_WRONLY, 0);

		close(stdout_pipe[P_READ]);
		close(TTYOUT);
		dup(stdout_pipe[P_WRITE]);
		close(stdout_pipe[P_WRITE]);
		close(TTYERR);
		dup(stderr_pipe[P_WRITE]);
		close(stderr_pipe[P_WRITE]);
		close(null);
		child_signals();
		execl(shell_path, shell_path,
			"-e", "-c", echo_cmd, (char *)0);
		_exit(255);
	}

#if defined(FORK_DEBUG)
	sprintf(static_s256, "DEBUG expand pid %d", child_pid);
	logevent(getpid(), static_s256);	/* expand_wildcard_list */
#endif

	free(echo_cmd);

	close(stdout_pipe[P_WRITE]);
	close(stderr_pipe[P_WRITE]);
	if (child_pid == -1)
	{
		close(stdout_pipe[P_READ]);
		close(stderr_pipe[P_READ]);
		*expcmd = "could not fork";
		if (restart_rcvr)
			start_rcvr_process(0);
		xmtr_signals();
		return (-1);
	}

	if (!(*expcmd = malloc(expcmd_size = ESD_MAXSZ)))
	{
		close(stdout_pipe[P_READ]);
		close(stderr_pipe[P_READ]);
		kill(child_pid, SIGKILL);
		*expcmd = mem_err_msg;
		if (restart_rcvr)
			start_rcvr_process(0);
		xmtr_signals();
		return (-1);
	}

	if (!(fp_pipe = fdopen(stdout_pipe[P_READ], "r")) ||
		((count = fread(*expcmd, 1, expcmd_size, fp_pipe)) < 0))
	{
		free(*expcmd);
		kill(child_pid, SIGKILL);
		close(stdout_pipe[P_READ]);
		close(stderr_pipe[P_READ]);
		*expcmd = "error reading wild list expansion";
		if (restart_rcvr)
			start_rcvr_process(0);
		xmtr_signals();
		return (-1);
	}

	/*
	 * make sure stdout is closed
	 */
	if (fp_pipe)
		fclose(fp_pipe);
	close(stdout_pipe[P_READ]);

	/*
	 * place trailing null kill trailing new line
	 */
	if (count)
	{
		cp = (*expcmd) + count;
		*cp-- = 0;
		if (*cp == '\n')
		{
			*cp = 0;
			count--;
		}
	}

	/*
	 * if no expansion, read stderr to find out why
	 */
	if (!count)
	{
		free(*expcmd);
		count = read(stderr_pipe[P_READ], static_s256, sizeof(static_s256) - 1);
		if (count < 0)
			strcpy(static_s256, strerror(errno));
		else
			static_s256[count] = 0;
		if (static_s256[count - 1] == '\n')
			static_s256[count - 1] = 0;
		close(stderr_pipe[P_READ]);
		if (strncmp(static_s256, "echo: ", 6))
			*expcmd = static_s256;
		else
			*expcmd = static_s256 + 6;
		if (restart_rcvr)
			start_rcvr_process(0);
		return (-1);
	}

	/*
	 * clean up zombie
	 */
	wait_status = 0;
	while (((itmp = wait(&wait_status)) != child_pid) && (itmp != -1))
		;
	child_pid = -1;

	xmtr_signals();

	/*
	 * if bad termination status, read stderr
	 */
	if (wait_status)
	{
		free(*expcmd);
		count = read(stderr_pipe[P_READ], static_s256, sizeof(static_s256) - 1);
		if (count < 0)
			strcpy(static_s256, strerror(errno));
		else
			static_s256[count] = 0;
		if (static_s256[count - 1] == '\n')
			static_s256[count - 1] = 0;
		close(stderr_pipe[P_READ]);
		if (strncmp(static_s256, "echo: ", 6))
			*expcmd = static_s256;
		else
			*expcmd = static_s256 + 6;
		if (restart_rcvr)
			start_rcvr_process(0);
		return (-1);
	}
	close(stderr_pipe[P_READ]);

	/*
	 * whew: we have (I think) a file list expansion
	 */
	if (restart_rcvr)
		start_rcvr_process(0);

	return (0);
}							 /* end of expand_wildcard_list */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of ecufork.c */
