#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char **argv) {
#define BUFFER_SIZE 4096
    char filename[BUFFER_SIZE + 4] = "";
    char oper[BUFFER_SIZE];
    (void) argc;
    (void) argv;
    const char s[2] = "\n";
    int opSize = 0;
    int filenameSize = 0;
    int readSize = 0;
    int writeSize = 0;
    char buffer[BUFFER_SIZE];
    char *token;
    char *CLtoken;
    char *contentToken;
    unsigned long long int contentLength;

    opSize = read(STDIN_FILENO, oper, 4);
    token = strtok(oper, s);

    if (opSize < 0) {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    if (strcmp(token, "get") == 0) {

        filenameSize = read(STDIN_FILENO, filename + 4, BUFFER_SIZE);

        if ((strlen(filename + 4) <= 2) || (opSize < 0) || (filenameSize < 0)) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        if (filenameSize < 0) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        } else if (filenameSize > 0) {
            unsigned long readLength = strlen(filename + 4);
            token = strtok(filename + 4, s);

            if ((readLength - 1) != strlen(token)) {
                fprintf(stderr, "Invalid Command\n");
                return 1;
            }

            int fd = open(token, O_RDONLY);
            if (fd < 0) {
                fprintf(stderr, "Invalid Command\n");
                return 1;
            }

            do {
                readSize = read(fd, buffer, BUFFER_SIZE);

                if (readSize < 0) {
                    fprintf(stderr, "Invalid Command\n");
                    return 1;
                }

                else if (readSize > 0) {
                    do {
                        int bytes = write(STDOUT_FILENO, buffer + writeSize, readSize - writeSize);

                        if (bytes <= 0) {
                            fprintf(stderr, "Invalid Command\n");
                            return 1;
                        }
                        writeSize += bytes;
                    } while (writeSize < readSize);
                }
            } while (readSize > 0);
            close(fd);
        }
    }

    else if (strcmp(oper, "set") == 0) {
        filenameSize = read(STDIN_FILENO, filename + 4, BUFFER_SIZE);

        if ((strlen(filename + 4) <= 2) || (opSize < 0) || (filenameSize < 0)) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        if (filenameSize < 0) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        } else if (filenameSize > 0) {

            token = strtok(filename + 4, s);
            if (strcmp(token + (strlen(token) - 3), "dat")) {
                int fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd < 0) {
                    fprintf(stderr, "Invalid Command\n");
                    return 1;
                }
            }

            int fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                fprintf(stderr, "Invalid Command\n");
                return 1;
            }
            CLtoken = strtok(0, s);
            contentLength = strtoull(CLtoken, NULL, 10);

            contentToken = strtok(0, "\0");

            if (sizeof(contentToken) < contentLength) {
                writeSize = write(fd, contentToken, sizeof(contentToken));
                if (writeSize == -1) {
                    fprintf(stderr, "Operation Failed\n");
                    return 1;
                }
            } else {
                writeSize = write(fd, contentToken, contentLength);
                if (writeSize == -1) {
                    fprintf(stderr, "Operation Failed\n");
                    return 1;
                }
            }

            while ((readSize = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
                writeSize = write(fd, buffer, BUFFER_SIZE);
                if (writeSize == -1) {
                    fprintf(stderr, "Operation Failed\n");
                    return 1;
                }
            }

            write(STDOUT_FILENO, "OK\n", 3);
            close(fd);
            return 0;
        }

    } else {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }
    return 0;
}
