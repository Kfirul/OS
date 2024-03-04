#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Function to execute GPG command for decrypting a file
void execute_gpg(char *input_file, char *passphrase) {
    int gpg_pipe[2];
    pipe(gpg_pipe); // Create a pipe for interprocess communication

    pid_t gpg_pid = fork(); // Fork a new process
    if (gpg_pid == -1) {
        perror("fork"); // Print error if fork fails
        exit(EXIT_FAILURE);
    } else if (gpg_pid == 0) {
        // Child process: executes the GPG command
        close(gpg_pipe[0]); // Close read end of gpg pipe
        dup2(gpg_pipe[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(gpg_pipe[1]); // Close write end of gpg pipe

        // Execute the GPG command for decryption
        execlp("gpg", "gpg", "--decrypt", "--passphrase", passphrase, "-o", "-", input_file, NULL);
        perror("exec gpg"); // Print error if exec fails
        exit(EXIT_FAILURE);
    }

    // Parent process continues here
    close(gpg_pipe[1]); // Close write end of gpg pipe
    dup2(gpg_pipe[0], STDIN_FILENO); // Redirect stdin to read from gpg pipe
    close(gpg_pipe[0]); // Close read end of gpg pipe
}

// Function to execute gunzip command
void execute_gunzip() {
    int gunzip_pipe[2];
    pipe(gunzip_pipe); // Create a pipe for interprocess communication

    pid_t gunzip_pid = fork(); // Fork a new process
    if (gunzip_pid == -1) {
        perror("fork"); // Print error if fork fails
        exit(EXIT_FAILURE);
    } else if (gunzip_pid == 0) {
        // Child process: executes the gunzip command
        close(gunzip_pipe[0]); // Close read end of gunzip pipe
        dup2(gunzip_pipe[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(gunzip_pipe[1]); // Close write end of gunzip pipe

        // Execute the gunzip command
        execlp("gunzip", "gunzip", "-", NULL);
        perror("exec gunzip"); // Print error if exec fails
        exit(EXIT_FAILURE);
    }

    // Parent process continues here
    close(gunzip_pipe[1]); // Close write end of gunzip pipe
    dup2(gunzip_pipe[0], STDIN_FILENO); // Redirect stdin to read from gunzip pipe
    close(gunzip_pipe[0]); // Close read end of gunzip pipe
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <file.gpg> <passphrase>\n", argv[0]); // Print usage instructions if incorrect arguments
        exit(EXIT_FAILURE);
    }

    // Execute GPG to decrypt the file
    execute_gpg(argv[1], argv[2]);
    
    // Execute gunzip to decompress the file
    execute_gunzip();

    // Execute tar to extract the file
    execlp("tar", "tar", "xvf", "-", NULL);
    perror("exec tar"); // Print error if exec fails
    exit(EXIT_FAILURE);
}
