/*
 * Copyright 2016 Daniel Hilst Selli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef FIFOSTAT_H
#define FIFOSTAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#define error(fmt, args...) \
	do { \
		openlog("fifostat", LOG_PERROR | LOG_PID, LOG_USER); \
		syslog(LOG_MAKEPRI(LOG_USER, LOG_ERR), "fifostat ERROR: " fmt,  ##args); \
		closelog(); \
	} while (0)

typedef void (*fifostats_cb)(int fd);
typedef struct {
	fifostats_cb cb;
	const char *path;
	unsigned int delay;
	/* private members */
	bool      _stop;
	pthread_t _id;
} fifostats;

static void *fifo_task(void *arg)
{
	int fd;
	fifostats *f = arg;

        while (!f->_stop) {
                fd = open(f->path, O_WRONLY); /* Should block here, until
                                                 * other end is open too */
                if (fd < 0) {
                        error("Can't open %s: %s", f->path, strerror(errno));
			if (f->delay)
				usleep(f->delay * 1000);
			else
				sleep(3);
                        continue;
                }
               	
		f->cb(fd);
                close(fd);
		if (f->delay > 0)
			usleep(f->delay * 1000);      /* Don't let user flood us */
        }
	return NULL;
}

static inline int fifostats_init(fifostats *f)
{
        struct stat s;

	assert(f->cb);
	assert(f->path);

        if (!(stat(f->path, &s))) { /* File exists */
                if ((s.st_mode & S_IFMT) != S_IFIFO) {
                        error("File %s exists but is not a FIFO", f->path);
                        return -1;
                }
        } else {
                if (mkfifo(f->path, O_RDWR)) {
                        error("Can't create %s, error: %s", f->path, strerror(errno));
                        return -1;
                }
        }

        pthread_create(&f->_id, NULL, fifo_task, f);
        return 0;
}

static inline void fifostats_destroy(fifostats *f)
{
	f->_stop = true;
	pthread_join(f->_id, NULL);
	unlink(f->path);
}

#ifdef __cplusplus
}
#endif

#endif
