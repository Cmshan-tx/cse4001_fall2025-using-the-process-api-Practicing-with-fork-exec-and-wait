#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

static void die(const char *msg) { perror(msg); exit(1); }

int main(void) {
	setbuf(stdout, NULL);
	pid_t rc = fork();
	if (rc < 0) die("fork");
	if (rc == 0) {
		// CHILD
		printf("[child ] pid=%d: trying wait() (should fail: no children\n", getpid());
		errno = 0;
		pid_t w = wait(NULL);
		if (w == -1) {
			printf("[child ] wait() returned -1, errno=%d (%s)\n", errno, strerror(errno));
			//expected ECHILD
		} else {
			printf("[child ] wait() unexpectedly returned pid=%d\n", w);
		}

		// Exit for PARENT
		_exit(42);
	} else {
		//PARENT
		int status = 0;
		printf("[parent] pid=%d: waiting for child pid=%d...\n", getpid(), rc);

		pid_t w = wait(&status);
		if (w == -1) die("wait(parent)");

		printf("[parent] wait() returned pid=%d\n", w);
		if (WIFEXITED(status)) {
			printf("[parent] child exited normally with status =%d\n", WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			printf("[parent] child was terminated by signal %d\n", WTERMSIG(status));
		} else {
			printf("[parent] child ended (other condition)\n");
		}
		return 0;
	}
}
