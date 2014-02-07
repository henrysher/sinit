/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/signalfd.h>
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

volatile sig_atomic_t signum;

typedef struct {
	int sig;
	void (*func)(void);
} Sigmap;

static void dispatchcmd(int);
static void sigfifo(void);
static void sigreap(void);
static void sigreboot(void);
static void spawn(const Arg *);

static Sigmap dispatchsig[] = {
	{ SIGHUP,  sigfifo   },
	{ SIGCHLD, sigreap   },
	{ SIGINT,  sigreboot },
};

static int sigfd = -1;
static int fifofd = -1;

#include "config.h"

int
main(void)
{
	struct signalfd_siginfo siginfo;
	sigset_t sigset;
	int maxfd, i, ret;
	ssize_t n;
	fd_set rfds;

	if (getpid() != 1)
		return EXIT_FAILURE;
	setsid();

	sigemptyset(&sigset);
	for (i = 0; i < LEN(dispatchsig); i++)
		sigaddset(&sigset, dispatchsig[i].sig);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	sigfd = signalfd(-1, &sigset, 0);
	if (sigfd < 0)
		eprintf("sinit: signalfd:");

	spawn(&(Arg){ .v = rcinitcmd });

	while (1) {
		FD_ZERO(&rfds);
		FD_SET(sigfd, &rfds);
		maxfd = sigfd;
		if (fifofd != -1) {
			FD_SET(fifofd, &rfds);
			if (fifofd > maxfd)
				maxfd = fifofd;
		}
		ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);
		if (ret < 0)
			eprintf("sinit: select:");
		if (ret > 0) {
			if (FD_ISSET(sigfd, &rfds)) {
				n = read(sigfd, &siginfo, sizeof(siginfo));
				if (n <= 0)
					continue;
				for (i = 0; i < LEN(dispatchsig); i++)
					if (dispatchsig[i].sig == siginfo.ssi_signo)
						dispatchsig[i].func();
			}
			if (fifofd != -1)
				if (FD_ISSET(fifofd, &rfds))
					dispatchcmd(fifofd);
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
sigfifo(void)
{
	if (!fifopath)
		return;
	unlink(fifopath);
	umask(0);
	if (mkfifo(fifopath, 0600) < 0)
		weprintf("sinit: mkfifo %s:", fifopath);
	fifofd = open(fifopath, O_RDWR | O_NONBLOCK);
	if (fifofd < 0)
		weprintf("sinit: open %s:", fifopath);
}

static void
sigreap(void)
{
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

static void
sigreboot(void)
{
	spawn(&(Arg){ .v = rcrebootcmd });
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
