/*
    This program compresses a directory or file using tar, gzip, and gpg,
    and then encrypts it with a passphrase provided by the user.

    Usage: ./program_name <directory/file> <passphrase>
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// Function to handle SIGINT signal
void ignore_sigint() {
    // Ignore SIGINT signal in the parent process
    printf("Child process interrupted with SIGINT signal.\n");
}

int main(int argc, char *argv[]) {
    // Check for correct number of command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory/file> <passphrase>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Register SIGINT handler to ignore the signal in the parent process
    signal(SIGINT, ignore_sigint);

    // Create pipes for communication between processes
    int tar_pipe[2]; // Pipe for tar process
    int gzip_pipe[2]; // Pipe for gzip process
    pipe(tar_pipe);
    pipe(gzip_pipe);

    pid_t pid; // Process ID variable

    // Fork for tar process
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: tar
        close(tar_pipe[0]); // Close read end of tar pipe
        dup2(tar_pipe[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(tar_pipe[1]); // Close unused write end of pipe

        // Close file descriptors before executing tar
        close(STDIN_FILENO);
        close(gzip_pipe[0]);
        close(gzip_pipe[1]);

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
        close(tar_pipe[1]); // Close write end of tar pipe
        dup2(tar_pipe[0], STDIN_FILENO); // Redirect stdin to pipe

        close(gzip_pipe[0]); // Close read end of gzip pipe
        dup2(gzip_pipe[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(gzip_pipe[1]); // Close unused write end of pipe

        // Close file descriptors before executing gzip
        close(tar_pipe[0]);

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
        close(gzip_pipe[1]); // Close write end of gzip pipe
        dup2(gzip_pipe[0], STDIN_FILENO); // Redirect stdin to pipe
        close(gzip_pipe[0]); // Close unused read end of pipe

        // Close file descriptors before executing gpg
        close(tar_pipe[0]);
        close(tar_pipe[1]);

        execlp("gpg", "gpg", "--symmetric", "--passphrase", argv[2], "-o", "output.gpg", "-", NULL); // Execute gpg command with passphrase
        perror("exec gpg");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(tar_pipe[0]); // Close read end of tar pipe
    close(tar_pipe[1]); // Close write end of tar pipe
    close(gzip_pipe[0]); // Close read end of gzip pipe
    close(gzip_pipe[1]); // Close write end of gzip pipe

    // Wait for all child processes to finish
    for (int i = 0; i < 3; i++) {
        wait(NULL);
    }

    printf("myzip was successful.\n");
    exit(EXIT_SUCCESS);
}
