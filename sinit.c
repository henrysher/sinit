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

volatile sig_atomic_t signum;

typedef struct {
	int sig;
	void (*func)(void);
} Sigmap;

static void sigpoweroff(void);
static void sigreap(void);
static void sigreboot(void);
static void spawn(const Arg *);

static Sigmap dispatchsig[] = {
	{ SIGUSR1, sigpoweroff },
	{ SIGCHLD, sigreap     },
	{ SIGINT,  sigreboot   },
};

static int sigfd = -1;

#include "config.h"

int
main(void)
{
	struct signalfd_siginfo siginfo;
	sigset_t sigset;
	int i;
	int ret;
	ssize_t n;
	fd_set rfds;

	if (getpid() != 1)
		return EXIT_FAILURE;
	setsid();

	if (sigemptyset(&sigset) < 0)
		eprintf("sinit: sigemptyset:");

	for (i = 0; i < LEN(dispatchsig); i++)
		if (sigaddset(&sigset, dispatchsig[i].sig) < 0)
			eprintf("sinit: sigaddset:");

	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0)
		eprintf("sinit: sigprocmask:");

	sigfd = signalfd(-1, &sigset, 0);
	if (sigfd < 0)
		eprintf("sinit: signalfd:");

	spawn(&(Arg){ .v = rcinitcmd });

	while (1) {
		FD_ZERO(&rfds);
		FD_SET(sigfd, &rfds);
		ret = select(sigfd + 1, &rfds, NULL, NULL, NULL);
		if (ret < 0)
			eprintf("sinit: select:");
		if (ret == 0)
			continue;
		if (FD_ISSET(sigfd, &rfds)) {
			n = read(sigfd, &siginfo, sizeof(siginfo));
			if (n < 0)
				eprintf("sinit: read:");
			if (n != sizeof(siginfo))
				continue;
			for (i = 0; i < LEN(dispatchsig); i++)
				if (dispatchsig[i].sig == siginfo.ssi_signo)
					dispatchsig[i].func();
		}
	}

	return EXIT_SUCCESS;
}

static void
sigpoweroff(void)
{
	spawn(&(Arg){ .v = rcpoweroffcmd });
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
	pid_t pid;
	char *const *p = arg->v;

	pid = fork();
	if (pid < 0) {
		weprintf("sinit: fork:");
	} else if (pid == 0) {
		setsid();
		setpgid(0, 0);
		execvp(*p, p);
		weprintf("sinit: execvp %s:", p);
		_exit(errno == ENOENT ? 127 : 126);
	}
}
