/* See LICENSE file for copyright and license details. */
#include <err.h>
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

static void cmdpoweroff(const Arg *);
static void cmdreboot(const Arg *);
static void dispatchcmd(int);
static void spawn(const char *, char *const []);

#include "config.h"

int
main(void)
{
	sigset_t set;
	pid_t pid;
	fd_set rfds;
	int c, fd, n;

	if (getpid() != 1)
		return EXIT_FAILURE;

	sigfillset(&set);
	sigprocmask(SIG_BLOCK, &set, 0);

	pid = fork();
	if (pid < 0)
		return EXIT_FAILURE;
	if (pid > 0)
		for (;;)
			wait(&c);

	sigprocmask(SIG_UNBLOCK, &set, 0);

	spawn("/bin/rc", (char *[]){ "rc", NULL });

	unlink(fifopath);
	umask(0);
	if (mkfifo(fifopath, 0600) < 0)
		eprintf("mkfifo %s:");

	fd = open(fifopath, O_RDWR | O_NONBLOCK);
	if (fd < 0)
		eprintf("open %s:", fifopath);
	while (1) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		n = select(fd + 1, &rfds, NULL, NULL, NULL);
		if (n < 0)
			eprintf("select:");
		if (FD_ISSET(fd, &rfds))
			dispatchcmd(fd);
	}

	return EXIT_SUCCESS;
}

static void
cmdpoweroff(const Arg *arg)
{
	spawn("/bin/rc.shutdown", (char *[]) { "rc", "poweroff", NULL });
}

static void
cmdreboot(const Arg *arg)
{
	spawn("/bin/rc.shutdown", (char *[]) { "rc", "reboot", NULL });
}

static void
dispatchcmd(int fd)
{
	int i;
	char buf[BUFSIZ], *p;
	ssize_t n;

	n = read(fd, buf, sizeof(buf) - 1);
	if (n < 0)
		eprintf("read:");
	if (n == 0)
		return;
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
spawn(const char *file, char *const argv[])
{
	pid_t pid;

	pid = fork();
	if (pid < 0)
		eprintf("fork:");
	if (pid == 0) {
		setsid();
		setpgid(0, 0);
		execvp(file, argv);
		_exit(errno == ENOENT ? 127 : 126);
	}
}
