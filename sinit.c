/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "util.h"

typedef union {
        const void *v;
} Arg;

typedef struct {
        const char *name;
        void (*func)(const Arg *arg);
        const Arg arg;
} Command;

static void dispatchcmd(int);
static void spawn(const Arg *);

#include "config.h"

int
main(void)
{
	sigset_t set;
	pid_t pid;
	fd_set rfds;
	int status, fd, n;

	if (getpid() != 1)
		return EXIT_FAILURE;
	setsid();

	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, 0);

	pid = fork();
	if (pid < 0)
		return EXIT_FAILURE;
	if (pid > 0)
		for (;;)
			wait(&status);

	sigprocmask(SIG_UNBLOCK, &set, 0);

	spawn(&rcinitarg);

	if (!fifopath)
		return EXIT_SUCCESS;

	unlink(fifopath);
	umask(0);
	if (mkfifo(fifopath, 0600) < 0)
		weprintf("sinit: mkfifo %s:", fifopath);

	fd = open(fifopath, O_RDWR | O_NONBLOCK);
	if (fd < 0)
		weprintf("sinit: open %s:", fifopath);
	if (fd >= 0) {
		while (1) {
			FD_ZERO(&rfds);
			FD_SET(fd, &rfds);
			n = select(fd + 1, &rfds, NULL, NULL, NULL);
			if (n < 0)
				eprintf("sinit: select:");
			if (FD_ISSET(fd, &rfds))
				dispatchcmd(fd);
		}
	}

	return EXIT_SUCCESS;
}

static void
dispatchcmd(int fd)
{
	int i;
	char buf[BUFSIZ], *p;
	ssize_t n;

	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		weprintf("sinit: read:");
	buf[n] = '\0';
	p = strchr(buf, '\n');
	if (p)
		*p = '\0';
	for (i = 0; i < LEN(commands); i++) {
		if (strcmp(commands[i].name, buf) == 0) {
			commands[i].func(&commands[i].arg);
			break;
		}
	}
}

static void
spawn(const Arg *arg)
{
	int status;
	pid_t pid;
	char *const *p = arg->v;

	pid = fork();
	if (pid < 0) {
		weprintf("sinit: fork:");
	} else if (pid == 0) {
		pid = fork();
		if (pid < 0)
			weprintf("sinit: fork:");
		else if (pid > 0)
			exit(0);
		setsid();
		setpgid(0, 0);
		execvp(*p, p);
		weprintf("sinit: execvp %s:", p);
		_exit(errno == ENOENT ? 127 : 126);
	}
	waitpid(pid, &status, 0);
}
