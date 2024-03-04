#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void sigint_handler() {
    // Ignoring SIGINT signal in the parent process
    printf("Child process interrupted with SIGINT signal.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory/file> <passphrase>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Register SIGINT handler to ignore the signal in the parent process
    signal(SIGINT, sigint_handler);

    // Create pipes
    int pipefd_tar[2];
    int pipefd_gzip[2];
    pipe(pipefd_tar);
    pipe(pipefd_gzip);

    pid_t pid;

    // Fork for tar process
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: tar
        close(pipefd_tar[0]); // Close read end of tar pipe
        dup2(pipefd_tar[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd_tar[1]); // Close unused write end of pipe

        // Close file descriptors before executing tar
        close(STDIN_FILENO);
        close(pipefd_gzip[0]);
        close(pipefd_gzip[1]);

        execlp("tar", "tar", "cf", "-", argv[1], NULL); // Execute tar command
        perror("exec tar");
        exit(EXIT_FAILURE);
    }

    // Fork for gzip process
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: gzip
        close(pipefd_tar[1]); // Close write end of tar pipe
        dup2(pipefd_tar[0], STDIN_FILENO); // Redirect stdin to pipe

        close(pipefd_gzip[0]); // Close read end of gzip pipe
        dup2(pipefd_gzip[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(pipefd_gzip[1]); // Close unused write end of pipe

        // Close file descriptors before executing gzip
        close(pipefd_tar[0]);

        execlp("gzip", "gzip", "-c", NULL); // Execute gzip command
        perror("exec gzip");
        exit(EXIT_FAILURE);
    }

    // Fork for gpg process
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: gpg
        close(pipefd_gzip[1]); // Close write end of gzip pipe
        dup2(pipefd_gzip[0], STDIN_FILENO); // Redirect stdin to pipe
        close(pipefd_gzip[0]); // Close unused read end of pipe

        // Close file descriptors before executing gpg
        close(pipefd_tar[0]);
        close(pipefd_tar[1]);

        execlp("gpg", "gpg", "--symmetric", "--passphrase", argv[2], "-o", "output.gpg", "-", NULL); // Execute gpg command with passphrase
        perror("exec gpg");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(pipefd_tar[0]); // Close read end of tar pipe
    close(pipefd_tar[1]); // Close write end of tar pipe
    close(pipefd_gzip[0]); // Close read end of gzip pipe
    close(pipefd_gzip[1]); // Close write end of gzip pipe

    // Wait for all child processes to finish
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    printf("myzip completed successfully.\n");
    exit(EXIT_SUCCESS);
}