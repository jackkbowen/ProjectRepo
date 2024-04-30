#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include <unistd.h>
#include "asgn2_helper_funcs.h"
#define BUFF_SIZE 50000

int defaultResponse(int socketFD, int response) {
    char msg200[] = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n";
    char msg201[] = "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n";
    char msg400[] = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
    char msg403[] = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
    char msg404[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n";
    char msg500[]
        = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server Error\n";
    char msg501[] = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
    char msg505[]
        = "HTTP/1.1 505 Version Not Supported\r\nContent-Length: 22\r\n\r\nVersion Not Supported\n";

    switch (response) {
    case 200: write_n_bytes(socketFD, msg200, strlen(msg200)); break;

    case 201: write_n_bytes(socketFD, msg201, strlen(msg201)); break;

    case 400: write_n_bytes(socketFD, msg400, strlen(msg400)); break;

    case 403: write_n_bytes(socketFD, msg403, strlen(msg403)); break;

    case 404: write_n_bytes(socketFD, msg404, strlen(msg404)); break;

    case 500: write_n_bytes(socketFD, msg500, strlen(msg500)); break;

    case 501: write_n_bytes(socketFD, msg501, strlen(msg501)); break;

    case 505: write_n_bytes(socketFD, msg505, strlen(msg505)); break;
    }
    return 0;
}

int returnBadPort() {
    fprintf(stderr, "Invalid Port\n");
    return 1;
}

ssize_t socketRead(int fd, char buf[], char entireBuf[], ssize_t nbytes) {
    ssize_t totalBytesRead = 0;
    regex_t regex;
    regmatch_t pmatch[2];
    int found;
    char *pat = "(\r\n\r\n)";

    if ((found = regcomp(&regex, pat, REG_EXTENDED)) != 0)
        return -1;
    //printf("Found = %i\n", found);

    while (totalBytesRead < nbytes) {
        //printf("Before Recv\n");
        ssize_t bytesRead = recv(fd, buf, nbytes - totalBytesRead, 0);
        if (bytesRead == -1) // encountered an error
            return -1;
        else if (bytesRead == 0) {
            //printf("Bytes 0");
            return totalBytesRead;
        } else {
            //printf("In else\n");
            strcat(entireBuf, buf);
            totalBytesRead += bytesRead;
            buf[totalBytesRead] = '\0';
            int match = regexec(&regex, buf, 2, pmatch, 0);
            //printf("After exec\n");
            if (match != 0)
                return 1;

            //printf("After Match\n");
            char *matching = buf + pmatch[1].rm_so;
            buf[pmatch[1].rm_eo] = '\0';
            (void) matching;
            //printf("match: %s\n", method);
            return totalBytesRead;
        }
    }
    regfree(&regex);
    return nbytes;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "No port number\n");
        return 1;
    }
    char buf[BUFF_SIZE];
    char entireBuf[BUFF_SIZE];
    int port = atoi(argv[1]);

    // Creates a Socket for the listener
    Listener_Socket socket;

    //Checks if port is a valid port number
    listener_init(&socket, port);
    if ((socket.fd == -1) || ((1023 > port) || (port > 65535))) {
        return returnBadPort();
    }

    while (1) {
        // connection link for the socket
        socket.fd = listener_accept(&socket);
        ssize_t bytesWritten = 0;
        ssize_t bytesRead = socketRead(socket.fd, buf, entireBuf, BUFF_SIZE);
        if (bytesRead == -1) {
            return 1;
        }

        struct stat statDir;
        //printf("Bytes Read: %zi\n", bytesRead);
        //printf("Entire Buffer: %s\n", entireBuf+1);
        int readFrom = strlen(buf);

        //printf("Buffer: %s\n", buf);
        //printf("Size of Buffer: %lu\n", strlen(buf));
        char testDir[BUFF_SIZE];
        strcpy(testDir, buf);
        //printf("Buf to get filename: %s\n", testDir+4);
        char *token;
        token = strtok(testDir + 4, " /");
        stat(token, &statDir);
        //printf("Token: %s\n", token);

        if (S_ISDIR(statDir.st_mode) == 1) {
            defaultResponse(socket.fd, 403);
            return 1;
        }

        //printf("After tok: %s\n", token);
        //printf("Buf to get filename: %s\n", buf);

        (void) bytesRead;
        (void) bytesWritten;
        struct stat st;
        int fileSize;
        //printf("Before regex\n");
        regex_t regex;
        char *pat
            = "^([A-Za-z]+{3}) (/[A-Za-z]+\\.[A-Za-z][A-Za-z][A-Za-z]) (HTTP/[0-9]*\\.[0-9]+\r\n)";
        regmatch_t pmatch[5];
        regcomp(&regex, pat, REG_EXTENDED);

        //printf("Before Match\n");
        int match = regexec(&regex, buf, 5, pmatch, 0);
        //printf("Match: %i\n", match);
        if (match != 0) {
            defaultResponse(socket.fd, 400);
            return 1;
        }

        char *method = buf + pmatch[1].rm_so;
        buf[pmatch[1].rm_eo] = '\0';
        //printf("Method: %s\n", method);

        char *URI = buf + pmatch[2].rm_so;
        buf[pmatch[2].rm_eo] = '\0';

        char *version = buf + pmatch[3].rm_so;
        buf[pmatch[3].rm_eo] = '\0';

        char CLBuf[6];

        memcpy(CLBuf, entireBuf + readFrom - 5, 4);
        //printf("Content Lenght: %s\n", CLBuf);
        int contentLenght = atoi(CLBuf);
        //printf("After Match\n");
        regfree(&regex);
        memmove(URI, URI + 1, strlen(URI));

        if (strlen(version) != 10) {
            defaultResponse(socket.fd, 400);
            return 1;
        } else if (strcmp(version, "HTTP/1.1\r\n") != 0) {
            defaultResponse(socket.fd, 505);
            return 1;
        } else if ((strcmp(method, "GET")) != 0 && strcmp(method, "PUT")) {
            defaultResponse(socket.fd, 501);
            return 1;
        }

        if (strcmp(method, "GET") == 0) {
            char cl[BUFF_SIZE];
            //char toSend[BUFF_SIZE];
            //int readSize = 0;
            //ssize_t writeSize = 0;
            //ssize_t contentLenght = 0;
            //printf("In Get\n");
            int fd = open(URI, O_RDONLY);
            //printf("URI: %s\n", URI);
            //printf("FD: %i\n", fd);
            if (fd == -1) {
                defaultResponse(socket.fd, 404);
                return 1;
            }
            //printf("Before fstat");
            fstat(fd, &st);
            fileSize = st.st_size;
            //char getBuf[fileSize];

            sprintf(cl, "HTTP/1.1 200 OK\r\nContent-Length: %i\r\n\r\n", fileSize);
            send(socket.fd, cl, strlen(cl), 0);
            //printf("Write Size: %zi", writeSize);
            pass_n_bytes(fd, socket.fd, fileSize);

            close(fd);
            return 0;
        }

        else if (strcmp(method, "PUT") == 0) {
            //printf("In Put\n");
            //printf("Method: %s\nURI: %s\nVersion %s\n", method, URI, version);
            //printf("Entire Buffer: %s\n", entireBuf+1);

            if (access(URI, F_OK) == 0)
                defaultResponse(socket.fd, 200);
            else
                defaultResponse(socket.fd, 201);

            int fd = open(URI, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd < 0) {
                defaultResponse(socket.fd, 400);
                return 1;
            }

            //printf("Content Lenght: %i\n", contentLenght);
            //printf("File Contents: %s\n", entireBuf+readFrom);
            write_n_bytes(fd, entireBuf + readFrom, contentLenght);
            close(fd);
        }
        regfree(&regex);
    }
    close(socket.fd);
    return 0;
}
