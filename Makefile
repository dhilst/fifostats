CFLAGS += -Wall -Wextra -I.

ftest: ftest.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o ftest ftest.c -lpthread

clean:
	rm ftest
