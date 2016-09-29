# Fifo Stats. A simple tool to provide runtime data through name pipes with Apache 2.0 License.

Some times it good to have a way to see what is going on from inside our programs. The Linux
has procfs that does that, providing runtime data trough filesystem. This header/library
does the same, but using named pipes.

# Core concepts

Before start let's name some concepts. This tool is about proving data. The user consuming
that data will be called "consumer" while the user providing data will be called "provider".
The central part of the tool is a callback implemented by "provider". That callback
is called everytime that "consumer" asks for it by reading a named pipe. The name of
the named pipe is provided by "provider".

# Usage

The first thing to do is to define the callback.
``` c
void callback(int fd)
{
	write(fd, "something", strlen("something"));
}
```

The example should be self-explanatory. The sole argument
is an opened file descriptor. The data write to it is readed by "consumer".
That callback will be called everytime that the "consumer" read from
named fifo. But wait, how that fifo is created?! Let's see...

There is one type defined struct called `fifostats`. That struct hold
all that is needed to this library work. The second step is allocate
and initialize that type/struct:
```
	fifostas f = {
		.cb = callback, /* our callback previously defined */
		.path = "somepath", /* the path where create the named pipe */
		.delay = 1000, /* This is used to avoid "consumer's" DoS */
	};
```

That structure has some undescore prefixed members that are private
and should not be touched. The other ones have been exapleined at above
comments. After that the user can start the fifostats by passing it
to `fifostats_init`;

```c
   fifostats_init(&f);
```

Okay, this will create the name pipe if it not exists. It returns an integer
where 0 is success and -1 is some error. The errors are logged to syslog. After
that call everything is left to callback. When "consumer" try to read the named
pipe the callback is called as we seem before.

When the "producer" wants to finish its execution it must call
`fifostats_destroy(fifostats *f)` passing the same argument that was passed to
`fifostats_init()`. This will join the thread created by `fifostats_init` and
remove the named pipe. After `fifostats_destroy` returns the callback will not
be called anymore. Keep in mind that blocking the callback's return will block
`fifostats_destroy()` from returning.

# Why a header with all that static inlines???

This is a simple library. So simple that is used by only one file at my libraries.
Making it a header simplify the compilation. Okay, I must confess, I am lazy. If
you are really concerned about this, remember that this is open/free software. Fell
free to improve it! :)
