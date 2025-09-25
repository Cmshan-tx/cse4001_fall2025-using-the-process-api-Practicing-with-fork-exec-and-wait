#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
 static void die (const char *msg) {
	 perror(msg);
	 exit(1);
 }

int main(void)
{
	int fd = open ("out.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (fd < 0) die("open");

	pid_t rc = fork();
	if (rc < 0) die("fork");

	//stdout unbuffered for clearer terminal ordering 
	setbuf(stdout, NULL);


	char buf[128];
	if (rc == 0) {
		// Child write 5 lines
	        for (int i = 1; i <= 5; i++) {
			int n = snprintf(buf, sizeof(buf), "child (pid=%d) line %d\n", getpid(), i);
			if (write(fd, buf, n) != n) die("write(child)");
		}
		// showing the FD used successfully 
		printf("[child ] done writing\n");

		//exit
		if (close(fd) < 0) die("close(child)");
		_exit(0);
	} else {
		for (int i = 1; i <= 5; i++) {
			int n = snprintf(buf, sizeof(buf), "parent (pid=%d) line %d\n", getpid(), i);
			if (write(fd, buf, n) != n) die("write(parent)");
		}
		printf("[parent] done writing (child pid=%d)\n", rc);
		int status;
		(void)waitpid(rc, &status, 0);

		if (close(fd) < 0) die("close(parent)");
		return 0;
	}

}
