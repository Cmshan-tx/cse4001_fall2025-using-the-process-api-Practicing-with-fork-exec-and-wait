#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

static void die(const char *msg) {perror(msg); exit(1); }

int main(int argc, char *argv[]) {
	const char *variant = (argc >= 2) ? argv[1] : "execvp";

	char *ls_argv[] = { "ls", "-l", NULL };
	char *newenv[] = {
		"LANG=C"
		"PATH=/bin:/usr/bin",
		"Q4_VARIANT=custom_env",
		NULL
		};
	setbuf(stdout, NULL);
	printf("[parent] spawning child using %s...\n", variant);

	pid_t rc = fork();
	if (rc < 0) die("fork");

	if (rc == 0) {
		if (strcmp(variant, "execl") == 0) {
			execl("/bin/ls", "ls", "-l", (char*)0);
		} else if (strcmp(variant, "execv") == 0) {
			execv("/bin/ls", ls_argv);
		} else if (strcmp(variant, "execlp") == 0) {
			execlp("ls", "ls", "-l", (char*)0);
		} else if (strcmp(variant, "execvp") == 0) {
			execvp("ls", ls_argv);
		} else if (strcmp(variant, "execle") == 0) {
			execle("/bin/ls", "ls", "-l", (char*)0, newenv);
#ifdef __DLIBC__
		} else if (strcmp(variant, "execvpe") == 0) {
			extern int execvpe(const char *, char *const[], char *const[]);
			execvpe("ls", ls_argv, newenv);
#endif  
		} else {
			fprintf(stderr, "Unknown variant: %s\n", variant);
			_exit(2);
		}
		die("exec*");
	}
	int status;
	if (waitpid(rc, &status, 0) < 0) die("waitpid");
	if (WIFEXITED(status)) {
		printf("[parent] child exited with %d\n", WEXITSTATUS(status));
	} else if (WIFSIGNALED(status)) {
		printf("[parent] child killed by signal %d\n", WTERMSIG(status));
	}
	return 0;
}
