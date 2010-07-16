/*
 * usleep - round microseconds up to an integral number of seconds.
 *
 * The real usleep() doesn't work this way; this is a hack for systems
 * that don't have usleep().
 */

int
usleep(unsigned int usec)
{
	unsigned int seconds = usec / 1000000;

	/* Round up: */
	seconds += (usec % 1000000 > 0);	/* 1 or 0 */

	return sleep(seconds);
}
