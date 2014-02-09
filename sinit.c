/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "util.h"

typedef struct {
	int sig;
	void (*func)(void);
} Sigmap;

static void sigpoweroff(void);
static void sigreap(void);
static void sigreboot(void);
static void spawn(char *const []);

static Sigmap dispatchsig[] = {
	{ SIGUSR1, sigpoweroff },
	{ SIGCHLD, sigreap     },
	{ SIGINT,  sigreboot   },
};

#include "config.h"

int
main(void)
{
	struct signalfd_siginfo si;
	sigset_t set;
	int fd;
	int i;
	ssize_t n;

	if (getpid() != 1)
		return EXIT_FAILURE;
	setsid();

	if (sigemptyset(&set) < 0)
		eprintf("sinit: sigemptyset:");

	for (i = 0; i < LEN(dispatchsig); i++)
		if (sigaddset(&set, dispatchsig[i].sig) < 0)
			eprintf("sinit: sigaddset:");

	if (sigprocmask(SIG_BLOCK, &set, NULL) < 0)
		eprintf("sinit: sigprocmask:");

	fd = signalfd(-1, &set, SFD_CLOEXEC);
	if (fd < 0)
		eprintf("sinit: signalfd:");

	spawn(rcinitcmd);

	while (1) {
		n = read(fd, &si, sizeof(si));
		if (n < 0)
			eprintf("sinit: read:");
		if (n != sizeof(si))
			continue;
		for (i = 0; i < LEN(dispatchsig); i++)
			if (dispatchsig[i].sig == si.ssi_signo)
				dispatchsig[i].func();
	}

	/* not reachable */
	return EXIT_SUCCESS;
}

static void
sigpoweroff(void)
{
	spawn(rcpoweroffcmd);
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
	spawn(rcrebootcmd);
}

static void
spawn(char *const argv[])
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		weprintf("sinit: fork:");
	} else if (pid == 0) {
		setsid();
		setpgid(0, 0);
		execvp(argv[0], argv);
		weprintf("sinit: execvp %s:", argv[0]);
		_exit(errno == ENOENT ? 127 : 126);
	}
}
