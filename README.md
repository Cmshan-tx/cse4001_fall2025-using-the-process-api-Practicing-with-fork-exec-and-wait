# Assignment: Practicing the Process API
Practicing with fork, exec, wait. 

### Overview

In this assignment, you will practice using the Process API to create processes and run programs under Linux. The goal is to gain hands-on experience with system calls related to process management. Specifically, you will practice using the unix process API functions 'fork()', 'exec()', 'wait()', and 'exit()'. 

⚠️ Note: This is not an OS/161 assignment. You will complete it directly on Linux. 

Use the Linux in your CSE4001 container. If you are using macOS, you may use the Terminal (you may need to install development tools with C/C++ compilers). 

**Reference Reading**: Arpaci-Dusseau, *Operating Systems: Three Easy Pieces*, Chapter 5 (Process API Basics)
 👉 [Chapter 5 PDF](http://pages.cs.wisc.edu/~remzi/OSTEP/cpu-api.pdf)

---

### **Steps to Complete the Assignment**

1. **Accept the GitHub Classroom Invitation**
    [GitHub Link](https://classroom.github.com/a/FZh4BrQG)
2. **Set up your Repository**
   - Clone the assignment repository.
3. **Study the Reference Materials**
   - Read **Chapter 5**.
   - Download and explore the sample programs from the textbook repository:
      [OSTEP CPU API Code](https://github.com/remzi-arpacidusseau/ostep-code/tree/master/cpu-api).
4. **Write Your Programs**
   - Adapt the provided example code to answer the assignment questions.
   - Each program should be clear, well-commented, and compile/run correctly.
   - Add your solution source code to the repository.

5. **Prepare Your Report**
   - Answer the questions in the README.md file. You must edit the README.md file and not create another file with the answers. 
   - For each question:
     - Include your **code**.
     - Provide your **answer/explanation**.
6. **Submit Your Work via GitHub**
   - Push both your **program code** to your assignment repository.
   - This push will serve as your submission.
   - Make sure all files, answers, and screenshots are uploaded and rendered properly.








---
### Questions
1. Write a program that calls `fork()`. Before calling `fork()`, have the main process access a variable (e.g., x) and set its value to something (e.g., 100). What value is the variable in the child process? What happens to the variable when both the child and parent change the value of x?


```cpp
<img width="1061" height="114" alt="Screenshot 2025-09-24 at 9 30 54 PM" src="https://github.com/user-attachments/assets/a1c7e89e-42d0-4522-99d6-19f3ba7c05a4" />

// Add your code or answer here. You can also add screenshots showing your program's execution.
Before the fork() call both Parent and Child utilize the same value of x, or instance. So the below changes allow both Parent and Child to rely on two different copies of x so that each can function independently from eachother.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
        int x = 100;

        printf("[before fork] pid=%d, x=%d\n", getpid(), x);
        pid_t rc = fork();
        if (rc < 0) {
                // fork failure
                perror("fork");
                exit(1);

        } else if (rc == 0) {
                // child (new process)
                x += 1;
                printf("[child ] pid=%d, x=%d\n", getpid(), x);
        } else {
                // parent goes down this path (og)
                x -= 1;
                printf("[parent] pid=%d, x=%d (child pid=%d)\n", getpid(), x, rc);
        }
        printf("[final ] pid=%d, x=%d\n", getpid(), x);
        return 0;
}
```


2. Write a program that opens a file (with the `open()` system call) and then calls `fork()` to create a new process. Can both the child and parent access the file descriptor returned by `open()`? What happens when they are writing to the file concurrently, i.e., at the same time?

```cpp
<img width="815" height="76" alt="Screenshot 2025-09-24 at 9 35 53 PM" src="https://github.com/user-attachments/assets/9969b083-bb4e-4c8e-a757-a67693c01bda" />
yes because it establishes a shared offset based on the scheduling, and both can write at the same time.
// Add your code or answer here. You can also add screenshots showing your program's execution.
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
```

3. Write another program using `fork()`.The child process should print “hello”; the parent process should print “goodbye”. You should try to ensure that the child process always prints first; can you do this without calling `wait()` in the parent?

```cpp
<img width="1057" height="134" alt="Screenshot 2025-09-24 at 9 44 48 PM" src="https://github.com/user-attachments/assets/6c676837-7ff6-4afd-a6aa-ea75c1f98af7" />

Yes you can do this without calling 'wait()' in the parent through using a pipe as a barrier. The child closes the read end and prints hello, then the parent closes the write end until a byte from the child arrives and then prints goodbye.
// Add your code or answer here. You can also add screenshots showing your program's execution.
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
```


4. Write a program that calls `fork()` and then calls some form of `exec()` to run the program `/bin/ls`. See if you can try all of the variants of `exec()`, including (on Linux) `execl()`, `execle()`, `execlp()`, `execv()`, `execvp()`, and `execvpe()`. Why do you think there are so many variants of the same basic call?

```cpp
<img width="1057" height="1054" alt="Screenshot 2025-09-24 at 9 45 54 PM" src="https://github.com/user-attachments/assets/dc4e7a2c-1cf5-458a-b110-d964a99e09aa" />
Honestly i am not sure at this point
// Add your code or answer here. You can also add screenshots showing your program's execution.
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
```

5. Now write a program that uses `wait()` to wait for the child process to finish in the parent. What does `wait()` return? What happens if you use `wait()` in the child?

```cpp
<img width="1061" height="111" alt="Screenshot 2025-09-24 at 9 48 12 PM" src="https://github.com/user-attachments/assets/c079434a-e148-43cd-8b01-9c39ea1e118b" />

// Add your code or answer here. You can also add screenshots showing your program's execution.
Using wait() in the child results in failure.
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
```

6. Write a slight modification of the previous program, this time using `waitpid()` instead of `wait()`. When would `waitpid()` be useful?

```cpp
<img width="1057" height="77" alt="Screenshot 2025-09-24 at 9 50 40 PM" src="https://github.com/user-attachments/assets/6488c48c-6d52-4b24-ac24-f1a374673388" />

// Add your code or answer here. You can also add screenshots showing your program's execution.

Waitpid() would be useful when you have multiple children and need to wait on one of them.

  include <stdio.h>
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
```

7. Write a program that creates a child process, and then in the child closes standard output (`STDOUT FILENO`). What happens if the child calls `printf()` to print some output after closing the descriptor?

```cpp
<img width="1055" height="87" alt="Screenshot 2025-09-24 at 9 52 48 PM" src="https://github.com/user-attachments/assets/58e2061d-eeab-4e6b-acff-bfe7a7cfa27e" />

// Add your code or answer here. You can also add screenshots showing your program's execution.
Those prinf produce no output.
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
```

