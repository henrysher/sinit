/* See LICENSE file for copyright and license details. */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "util.h"

static void sigpoweroff(void);
static void sigreap(void);
static void sigreboot(void);
static void spawn(char *const []);

static struct {
	int sig;
	void (*handler)(void);
} sigmap[] = {
	{ SIGUSR1, sigpoweroff },
	{ SIGCHLD, sigreap     },
	{ SIGINT,  sigreboot   },
};

#include "config.h"

static sigset_t set;

int
main(void)
{
	int sig;
	int i;

	if (getpid() != 1)
		return EXIT_FAILURE;
	setsid();

	sigemptyset(&set);
	for (i = 0; i < LEN(sigmap); i++)
		sigaddset(&set, sigmap[i].sig);
	sigprocmask(SIG_BLOCK, &set, NULL);

	spawn(rcinitcmd);

	while (1) {
		sigwait(&set, &sig);
		for (i = 0; i < LEN(sigmap); i++)
			if (sigmap[i].sig == sig)
				sigmap[i].handler();
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
		sigprocmask(SIG_UNBLOCK, &set, NULL);
		setsid();
		setpgid(0, 0);
		execvp(argv[0], argv);
		weprintf("sinit: execvp %s:", argv[0]);
		_exit(errno == ENOENT ? 127 : 126);
	}
}
