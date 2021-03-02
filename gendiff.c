/* CHK=0xD619 */
#define USE_FCRC
char *revision = "x1.33";

/*+-------------------------------------------------------------------------
	gendiff.c - generate diff/patch files
	...!gatech!kd4nc!n4hgf!wht

  Usage: cd <moddir>
         gendiff -o <olddir> [-n <prefix>] [-a] <filelist>

  where <moddir> and <olddir> are two directories. <moddir> contains
  a newer version of a software set, while <origdir> contains an older
  version.  <filelist> is a list of files in the software set for
  which a set of patches are to be created.  <filelist> may contain
  new filenames found in <moddir> but not in <origdir>.

  <prefix> is a filename prefix used by gendiff to choose filenames
  for it's output.  The default for <prefix> is '/tmp/diff'.  Thus,
  if -n is not specified, gendiff will output files named /tmp/diff.01,
  /tmp/diff.02, etc.  In addition, a file containing the list of files
  generated will be placed in /tmp/diff.fls.

  If one of two conditions is met, gendiff will not generate diffs
  for a file found in <filelist>.  If the file is not found in <origdir>
  or if the diffs between the older and newer file are larger than
  the newer file, no diff is generated.  Rather, the filename is
  placed in the /tmp/diff.fls (or whatever per -n), the use of which
  is made apparent below. -f forces diffs to be generated.

  The number of /tmp/diff.xx files generated depends on the size of
  the patches.  gendiff files a diff.xx file with patches until it is
  28k in size or larger.  Then it increments the numeric file suffix
  and places additional patches there.

  The -a switch causes gendiff to produce shell scripts containing
  embedded patches.  For example, to do the patch, you type
         sh /tmp/diff.01
  Omitting -a causes gendiff to produce patch input files.  For
  example, to do the patch, you type
         patch -p < /tmp/diff.01
  I haven't figured out why (being a green shell programmer [after
  five years of UNIX :-/]), but -a files screw up when lines in
  the patch contain trailing backslashes.

  Consider an example software project:

  /version1    /version2    what happened
  Makefile     Makefile     added new object module
  widget.c     widget.c     modified slightly to use new whizbang.c
  util.c       util.c       modified extensively to accommodate whizbang.c
  widget.h     widget.h     no changes made
   ---         whizbang.c   new module

  Running gendiff:
  cd /version2
  gendiff -o /version1 -n PATCH.1 Makefile widget.c util.c widget.h whizbang.c

  causes:
  PATCH.1.01 through PATCH.?? to contain patches (lets say for example
             only one PATCH.1.xx files are created)
  PATCH.1.fls contains:
      PATCH.1.01           the patch file (patches to Makefile and widget.c)
      util.c               the file was changed so much, the diffs were larger
      whizbang.c           the file is new.

  You make a quick file called APPLY_PATCH.1
      #!/bin/sh
      echo applying PATCH 1 (see README.P1)
      patch -p < PATCH.1.01

  create README.P1

  edit PATCH.1.fls to include:
      README.P01
      APPLY_PATCH.01
      PATCH.1.01
      util.c
      whizbang.c

  and then, using shar"3",
      shar -nwidget/patch1 -l40 -o /tmp/widgetp1 `cat PATCH.1.fls`

  and your semi-automated patch production process is complete.
  clear as mud?

  note:  this works for me on System V.  I dont THINK there
  is anything to keep it from working on BSD as well.  Just
  set CONTEXT_DIFF below to something reasonable and
     cc -o gendiff gendiff.c

--------------------------------------------------------------------------*/
/*+:EDITS:*/
/*:04-26-2000-11:15-wht@bob-RELEASE 4.42 */
/*:01-24-1997-02:37-wht@yuriatin-SOURCE RELEASE 4.00 */
/*:09-11-1996-20:00-wht@yuriatin-3.48-major telnet,curses,structural overhaul */
/*:11-23-1995-11:20-wht@kepler-source control 3.37 for tsx-11 */
/*:11-14-1995-10:23-wht@kepler-3.37.80-source control point: SOCKETS */
/*:01-12-1995-15:20-wht@n4hgf-apply Andrew Chernov 8-bit clean+FreeBSD patch */
/*:05-04-1994-04:39-wht@n4hgf-ECU release 3.30 */
/*:12-16-1993-03:39-wht@n4hgf-add cond. compilation for fcntl vs sys/time */
/*:12-16-1993-03:39-wht@n4hgf-add cond. compilation for CONTEXT_DIFF */
/*:03-17-1993-15:46-wht@n4hgf-first diff shorter to make room for verbiage */
/*:03-17-1993-15:14-wht@n4hgf-use 4 lines of context */
/*:04-24-1990-01:57-wht@n4hgf-document for shar-list */
/*:04-14-1990-15:16-wht-creation */

#if defined(SVR4) || defined(linux) || defined(sun) || defined(BSD) || defined(__FreeBSD__)
#define CONTEXT_DIFF "diff -c"
#else /* count on GNU diff */
#define CONTEXT_DIFF "gnudiff -d -C 4"
#endif

#include <stdio.h>
#include <signal.h>
#include <time.h>

#ifdef BSD
#include <sys/file.h>
#else
#include <fcntl.h>
#endif

#include <sys/errno.h>
#include <sys/types.h>
#include <sys/stat.h>

FILE *popen();
long time();
struct tm *gmtime();

int auto_patch = 0;
int force_diff = 0;
FILE *fpout = (FILE *) 0;
FILE *fpfls = (FILE *) 0;
char flsname[128];
char *diffname = (char *)0;
int diffnum = 1;
char namebuf[256];
long ltmp;
char *subject = (char *)0;
char *month_text[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
	"Oct", "Nov", "Dec"
};

long size_limit = 35000L;
long size_limit2 = 45000L;

/*+-------------------------------------------------------------------------
	gendiff(old,new)
--------------------------------------------------------------------------*/
void
gendiff(old, new, reason)
char *old;
char *new;
char *reason;
{
	char buf[512];
	char diffbuf[1024];
	FILE *fppipe;
	FILE *fpintermediate;
	long now;
	struct tm *utc;
	struct stat statbuf_new;

	sprintf(buf, "%s %s %s", CONTEXT_DIFF, old, new);
	if (!(fppipe = popen(buf, "r")))
	{
		perror(buf);
		exit(1);
	}
	sprintf(buf, "/tmp/gend.%05d\n", getpid());
	if (!(fpintermediate = fopen(buf, "w+")))
	{
		perror(buf);
		exit(1);
	}
	unlink(buf);
	while (fgets(diffbuf, sizeof(diffbuf), fppipe))
		fputs(diffbuf, fpintermediate);
	pclose(fppipe);
	if (stat(new, &statbuf_new))
	{
		perror(new);
		exit(1);
	}
	if (!force_diff && (ftell(fpintermediate) > statbuf_new.st_size))
	{
		printf("--> %s: diffs larger than file (%s)\n", new, reason);
		fprintf(fpfls, "%s\n", new);
		fclose(fpintermediate);
		return;
	}
	rewind(fpintermediate);
	while (fgets(diffbuf, sizeof(diffbuf), fpintermediate))
	{
	  OPEN_DIFF_OUT:
		if (!fpout)
		{
			sprintf(namebuf, "%s.%02d", diffname, diffnum++);
			if (!(fpout = fopen(namebuf, "w")))
			{
				perror(namebuf);
				exit(1);
			}
			printf("routing diffs to %s\n", namebuf);
			fputs(":\n", fpout);
			fputs("#-------------------------------------------------------\n",
				fpout);
			fprintf(fpout, "# %s\n", namebuf);
			if (subject)
				fprintf(fpout, "# %s\n", subject);
			time(&now);
			utc = gmtime(&now);
			fprintf(fpout,
				"# created by gendiff %s on %02d %s %04d %02d:%02d UTC\n",
				revision, utc->tm_mday, month_text[utc->tm_mon],
				utc->tm_year + 1900, utc->tm_hour, utc->tm_min);
			fputs("#-------------------------------------------------------\n",
				fpout);
			if (auto_patch)
				fputs("sed -e 's/^P//' << DIFF_EOF | patch -p\n", fpout);
		}
		if (auto_patch)
			fputc('P', fpout);
		fputs(diffbuf, fpout);
	}
	fclose(fpintermediate);
	if ((ltmp = ftell(fpout)) > size_limit)
	{
		size_limit = size_limit2;
		if (auto_patch)
			fprintf(fpout, "DIFF_EOF\nexit 0\n#end of %s\n", namebuf);
		ltmp = ftell(fpout);
		fclose(fpout);
		fpout = (FILE *) 0;
		if (auto_patch)
			chmod(namebuf, 0755);
		printf("# diff file %s received %ld bytes\n", namebuf, ltmp);
		fprintf(fpfls, "%s\n", namebuf);
	}
	printf("%s: diffs generated (%s)\n", new, reason);
}							 /* end of gendiff */

/*+-------------------------------------------------------------------------
	fcrc_different(old,new)

Perform some kind of checkum check on the file, using wht@n4hgf's fcrc
or BSD/SysV sum
--------------------------------------------------------------------------*/
fcrc_different(old, new)
char *old;
char *new;
{
	char buf[512];
	char obuf[128];
	char nbuf[128];
	FILE *fppipe;

#ifdef USE_FCRC
	sprintf(buf, "fcrc %s", old);
#else
	sprintf(buf, "sum %s", old);
#endif
	if (!(fppipe = popen(buf, "r")))
	{
		perror(buf);
		exit(2);
	}
	if (!fgets(obuf, sizeof(obuf), fppipe))
	{
		fprintf(stderr, "read error on old file checksum read\n");
		exit(3);
	}
	pclose(fppipe);

#ifdef USE_FCRC
	sprintf(buf, "fcrc %s", new);
#else
	sprintf(buf, "sum %s", new);
#endif
	if (!(fppipe = popen(buf, "r")))
	{
		perror(buf);
		exit(2);
	}
	if (!fgets(nbuf, sizeof(nbuf), fppipe))
	{
		fprintf(stderr, "read error on new file checksum read\n");
		exit(3);
	}
	pclose(fppipe);

#ifdef USE_FCRC
	obuf[5 + 4] = 0;
	nbuf[5 + 4] = 0;
	return (!!strncmp(obuf + 5, nbuf + 5, 4));
#else
	return (!!strcmp(obuf, nbuf));
#endif

}							 /* end of fcrc_different */

/*+-------------------------------------------------------------------------
	main(argc,argv)
--------------------------------------------------------------------------*/
main(argc, argv)
int argc;
char **argv;
{
	int itmp;
	int errflg = 0;
	char *olddir = (char *)0;
	char oldfile[256];
	char *newfile;
	struct stat ostat;
	struct stat nstat;
	FILE *fpdiffc;
	extern char *optarg;
	extern int optind;

	close(2);
	dup(1);
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	printf("gendiff revision %s\n", revision);

	while ((itmp = getopt(argc, argv, "afo:n:s:")) != -1)
	{
		switch (itmp)
		{
			case 'a':
				auto_patch = 1;
				break;
			case 'f':
				force_diff = 1;
				break;
			case 'o':
				olddir = optarg;
				break;
			case 'n':
				diffname = optarg;
				break;
			case 's':
				subject = optarg;
				break;
			case '?':
				errflg++;
		}
	}
	if (errflg || !olddir)
	{
		fprintf(stderr, "\
usage: gendiff -af -o <olddir> -n <outnamebase> -s <subj> <files>\n");
		fprintf(stderr, "-a auto-apply     -f force diffs despite size\n");
		exit(1);
	}

	if (!diffname)
	{
		diffname = "/tmp/diff";
		printf("default ");
	}
	printf("base diff name is %s\n", diffname);
	sprintf(flsname, "%s.fls", diffname);
	printf("diff distribution manifest name is %s\n", flsname);
	if (!(fpfls = fopen(flsname, "w")))
	{
		perror(flsname);
		exit(1);
	}

	while (optind < argc)
	{
		strcpy(oldfile, olddir);
		strcat(oldfile, "/");
		strcat(oldfile, newfile = argv[optind++]);
		if (stat(oldfile, &ostat))
		{
			printf("Warning: %s: not in old distribution\n", newfile);
			fprintf(fpfls, "%s\n", newfile);
			continue;
		}
		if (stat(newfile, &nstat))
		{
			fprintf(stderr, "======> not found in new distribution ");
			perror(newfile);
			exit(1);
		}
		if (ostat.st_size != nstat.st_size)
		{
			gendiff(oldfile, newfile, "size");
			continue;
		}
		if (fcrc_different(oldfile, newfile))
		{
			gendiff(oldfile, newfile, "checksum");
			continue;
		}
	}

	if (fpout)
	{
		if (auto_patch)
			fprintf(fpout, "DIFF_EOF\nexit 0\n#end of %s\n", namebuf);
		ltmp = ftell(fpout);
		fclose(fpout);
		fpout = (FILE *) 0;
		if (auto_patch)
			chmod(namebuf, 0755);
		printf("# diff file %s received %ld bytes\n", namebuf, ltmp);
		fprintf(fpfls, "%s\n", namebuf);
	}

	exit(0);
}							 /* end of main */

/* vi: set tabstop=4 shiftwidth=4: */
/* end of gendiff.c */
