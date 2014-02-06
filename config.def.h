/* See LICENSE file for copyright and license details. */
static const char *fifopath = "/var/run/sinit.fifo";
static Command commands[] = {
	{ "poweroff",   cmdpoweroff,   {0} },
	{ "reboot",     cmdreboot,     {0} },
};
