#include <time.h>
#include <stdlib.h>

/* timegm -- based on Linux timegm man page */

time_t
timegm(struct tm *tm)
{
	time_t ret;
	char *tz = getenv("TZ");
	const char *tzreq = "UTC+0";	/* more portable than ""? */

	if (tz)
		tz = estrdup(tz, strlen(tz));
	if (setenv("TZ", tzreq, 1) < 0) {
		warning(_("setenv(TZ, %s) failed (%s)"), tzreq, strerror(errno));
		return -1;
	}
	tzset();
	ret = mktime(tm);
	if (tz) {
		if (setenv("TZ", tz, 1) < 0)
			fatal(_("setenv(TZ, %s) restoration failed (%s)"), tz, strerror(errno));
		free(tz);
	} else {
		if (unsetenv("TZ") < 0)
			fatal(_("unsetenv(TZ) failed (%s)"), strerror(errno));
	}
	tzset();
	return ret;
}
