#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

static void die(const char *msg) { perror(msg); exit(1); }

int main(void) {
	setbuf(stdout, NULL);
	pid_t a = fork();
	if (a < 0) die("fork");
	if (a == 0) { sleep(2); _exit(7);}

	pid_t b = fork();
	if (b < 0) die("fork B");
	if (b == 0) { sleep(1); _exit(9); }
        // PARENT
	int statusA = 0;
	int statusB = 0;
	int gotA = 0;
	printf("[parent] polling for A=%d with WNOHANG: ", a);
	for (int i = 0; i < 20; i++) {
		pid_t w = waitpid(a, &statusA, WNOHANG);
		if (w == -1) die("waitpid(A, WNOHANG)");
		if (w == 0) {
			printf("."); fflush(stdout);
			usleep(200000); //  200ms
		} else if (w == a) {
			gotA = 1;
			printf(" done\n");
			break;
		}
	}

	if (!gotA) {
		if (waitpid(a, &statusA, 0) == -1) die("waitpid(A, 0)");
		printf("\n[parent] A finished (blocking wait)\n");
	}
	if (WIFEXITED(statusA))
		printf("[parent] A exited with status=%d\n", WEXITSTATUS(statusA));
	else if (WIFSIGNALED(statusA))
		printf("[parent] A killed by signal %d\n", WTERMSIG(statusA));

	pid_t wB = waitpid(b, &statusB, 0);
	if (wB == -1) die("waitpid(B)");
	if (WIFEXITED(statusB))
		printf("[parent] child pid=%d exited with status=%d\n", wB, WEXITSTATUS(statusB));
	else if (WIFSIGNALED(statusB))
		printf("[parent] child pid=%d killed by signal %d\n", wB, WTERMSIG(statusB));
	
	return 0;

		
}
