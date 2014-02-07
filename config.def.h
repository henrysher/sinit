/* See LICENSE file for copyright and license details. */

static const char *rcinitcmd[] = { "/bin/rc.init", NULL };
static Arg rcinitarg = { .v = rcinitcmd };

static const char *rcrebootcmd[] = { "/bin/rc.shutdown", "reboot", NULL };
static const char *rcpoweroffcmd[] = { "/bin/rc.shutdown", "poweroff", NULL };

/* if you make this path NULL, then sinit will not use a FIFO */
static const char *fifopath = "/var/run/sinit.fifo";
static Command commands[] = {
	{ "poweroff",        spawn,        { .v = rcpoweroffcmd } },
	{ "reboot",          spawn,        { .v = rcrebootcmd   } },
};
