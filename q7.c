#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/wait.h>
#include <errno.h>

int 
main(void)
{
	setbuf(stdout, NULL);

	int rc = fork();
	if (rc < 0) {
		//fork failure or F.F. -> exit
		fprintf(stderr, "fork failed\n");
		exit(1);
	} else if (rc == 0) {
		// CHILD
		if (close(STDOUT_FILENO) == -1) {
			perror("close(stdout)");
			_exit(1);
		}

		int r = printf("[child] you should not see this on stdout\n");
		if (r < 0) {
			fprintf(stderr, "[child:stderr] printf failed: errno=%d (%s)\n", errno, strerror(errno));
		}
		ssize_t w = write(STDOUT_FILENO, "child direct write\n", 19);
		if (w == -1) {
			fprintf(stderr, "[child:stderr] write(fd=1) failed: errno=%d (%s)\n",
					 errno, strerror(errno));
		}
		_exit(0);
	} else {
		//PARENT
		printf("[parent] this should appear on stdout (fd is open)\n");

		int status = 0;
		int wc = wait(&status);
		assert(wc >= 0);

		if (WIFEXITED(status)) {
			printf("[parent] child exited with status=%d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("[parent] child killed by signal %d\n", WTERMSIG(status));
		} 
		return 0;
	}
}

