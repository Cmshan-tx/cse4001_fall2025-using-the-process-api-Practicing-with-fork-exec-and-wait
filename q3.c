#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void die(const char *msg) { perror(msg); exit(1); }

int main(void) {
	//stdout unbuff so prints immediately
	setbuf(stdout, NULL);

	int p[2];
	if (pipe(p) < 0) die ("pipe");

	pid_t rc = fork();
	if (rc < 0) die("fork");

	if (rc == 0) {
		// CHILD 
		close(p[0]); //closes read
		printf("hello\n");
		
		if (write(p[1], "x", 1) != 1) die("write");
		close(p[1]);
		_exit(0);
	} else {
		// PARENT
		close(p[1]);
		char token;
                if (read(p[0], &token, 1) != 1) die("read");
		close(p[0]);
		printf("goodbye\n");
		return 0;
	}
}
