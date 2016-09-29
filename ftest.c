#include <stdio.h>
#include <sys/time.h>

#include "fifostats.h"

void cb(int fd)
{
	struct timeval tv;
	static char time_str[128];

	gettimeofday(&tv, NULL);
	strftime(time_str, sizeof(time_str), "%F %T", localtime(&tv.tv_sec));
	write(fd, time_str, strlen(time_str));
}

int main(void)
{
	fifostats f = {
		.cb = cb,
		.path = "ftest.fifo",
		.delay = 1000,
	};

	fifostats_init(&f);

	sleep(20);

	fifostats_destroy(&f);
	return 0;
}
