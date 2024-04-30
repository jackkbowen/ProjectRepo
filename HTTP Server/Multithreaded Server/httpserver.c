// Asgn 2: A simple HTTP server.
// By: Eugene Chou
//     Andrew Quinn
//     Brian Zhao

#include "asgn2_helper_funcs.h"
#include "connection.h"
#include "debug.h"
#include "response.h"
#include "request.h"
#include "queue.h"
#include "rwlock.h"
#include "linkedlist.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <bits/getopt_core.h>

void handle_connection(int);

void handle_get(conn_t *);
void handle_put(conn_t *);
void handle_unsupported(conn_t *);
pthread_mutex_t mutex;
queue_t *q = NULL;
List *map = NULL;
rwlock_t *rwlock = NULL;
uint32_t activeWriters;

void auditLog(char *oper, char *uri, int statusCode, char *requestID);

// Credit: Gary from discord
// Avoiding segfaults on pop / popping too much

int worker_threads() {
    int id;
    while (1) {
        if (!queue_pop(q, (void *) &id)) {
            fprintf(stderr, "pop error");
            return EXIT_FAILURE;
        }
        handle_connection(id);
        close(id);
    }
    return 0;
}

int main(int argc, char **argv) {
    int opt;
    int numThreads = 4;
    int rc = pthread_mutex_init(&mutex, NULL);
    assert(rc == 0);

    if (argc < 2) {
        warnx("wrong arguments: %s port_num", argv[0]);
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Used chatGPT to learn how to use getopt with switch statements
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't': numThreads = atoi(optarg); break;
        case '?': fprintf(stderr, "Unknown option: -%c\n", optopt); return -1;
        default: fprintf(stderr, "Unexpected case in switch\n"); return -1;
        }
    }

    printf("Num Threads: %d", numThreads);
    char *endptr = NULL;
    size_t port = (size_t) strtoull(argv[argc - 1], &endptr, 10);
    if (endptr && *endptr != '\0') {
        warnx("invalid port number: %s", argv[argc - 1]);
        return EXIT_FAILURE;
    }

    signal(SIGPIPE, SIG_IGN);
    Listener_Socket sock;
    listener_init(&sock, port);

    // Threadsafe Queue from asgn3
    q = queue_new(numThreads);

    // Linked Map for checking URI
    map = makelist();

    // rwlock with N_WAY priority
    rwlock = rwlock_new(N_WAY, 1);

    //Making Server - Creating worker threads

    pthread_t worker_t[numThreads];
    for (int i = 0; i < numThreads; i++) {
        // Dispatcher pushing requests into a threadsafe queue
        pthread_create(&worker_t[i], NULL, (void *(*) (void *) ) worker_threads, NULL);
    }

    // Dispatcher
    while (1) {
        // New socket connection, listening for client
        // worker thread is idle
        // Queue_push from asgn3 and is sleeping waiting on a lock/cv
        int connfd = listener_accept(&sock);
        if (!queue_push(q, (void *) &connfd)) { //block if full
            fprintf(stderr, "push error\n");
            return 1;
        }
        handle_connection(connfd);
        close(connfd);
    }
    destroy(map);
    return EXIT_SUCCESS;
}

// Worker Thread wait->recv->send
void handle_connection(int connfd) {

    conn_t *conn = conn_new(connfd);

    const Response_t *res = conn_parse(conn);

    if (res != NULL) {
        conn_send_response(conn, res);
    } else {
        //debug("%s", conn_str(conn));
        const Request_t *req = conn_get_request(conn);
        if (req == &REQUEST_GET) {
            handle_get(conn);
        } else if (req == &REQUEST_PUT) {
            handle_put(conn);
        } else {
            handle_unsupported(conn);
        }
    }

    conn_delete(&conn);
}

void handle_get(conn_t *conn) {
    char *uri = conn_get_uri(conn);
    struct stat getSize;
    struct stat getIfDir;
    uint32_t fileSize;

    const Response_t *res = NULL;
    int statusCode = 500;
    char *requestID = NULL;
    requestID = conn_get_header(conn, "Request-Id");

    //debug("handling get request for %s", uri);
    // What are the steps in here?

    // 1. Open the file.
    // If  open it returns < 0, then use the result appropriately
    //   a. Cannot access -- use RESPONSE_FORBIDDEN
    //   b. Cannot find the file -- use RESPONSE_NOT_FOUND
    //   c. other error? -- use RESPONSE_INTERNAL_SERVER_ERROR
    // (hint: check errno for these cases)!

    // Errno Cases:
    // a = EACCES - Permission Denied
    // b = ENOENT - No such file or directo

    pthread_mutex_lock(&mutex);

    if ((compareURI(map, uri) == 0) && (activeWriters > 0)) {
        // URI is being accessed and its  a writer
        reader_lock(rwlock);
    }

    add(uri, rwlock, map);

    //debug("GET -- Mutex Lock");

    // ---------------------------------------------------------------
    // -- Critical Section
    // ---------------------------------------------------------------

    // Check if file already exists before opening it.

    // Moved sending the response and printing the audit log to insidethe critical section.
    // Using goto isnt coherant?

    int fd = open(uri, O_RDONLY);

    reader_lock(rwlock);

    if (fd < 0) {
        if (errno == EACCES) {
            // record a 403 status code in the audit
            res = &RESPONSE_FORBIDDEN;
            statusCode = 403;
            conn_send_response(conn, res);
            auditLog("GET", conn_get_uri(conn), statusCode, requestID);
        }

        else if (errno == ENOENT) {
            // record a 404 status code in the audit
            res = &RESPONSE_NOT_FOUND;
            statusCode = 404;
            conn_send_response(conn, res);
            auditLog("GET", conn_get_uri(conn), statusCode, requestID);
        }

        else {
            // record a 500 status code in the audit
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, res);
            auditLog("GET", conn_get_uri(conn), statusCode, requestID);
        }
        delete (uri, map);

        pthread_mutex_unlock(&mutex);
        reader_unlock(rwlock);
        //debug("GET -- Mutex Unlock2");
        close(fd);
        return;
    }

    // 2. Get the size of the file.
    // (hint: checkout the function fstat)!
    // Get the size of the file.

    fstat(fd, &getSize);
    fileSize = getSize.st_size;

    // 3. Check if the file is a directory, because directories *will*
    // open, but are not valid.
    // (hint: checkout the macro "S_IFDIR", which you can use after you call fstat!)
    if (stat(uri, &getIfDir) == 0) {
        if (getIfDir.st_mode & __S_IFDIR) {
            // record a 403 error in the audit
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            statusCode = 403;
            conn_send_response(conn, res);
            auditLog("GET", conn_get_uri(conn), statusCode, requestID);
            delete (uri, map);

            pthread_mutex_unlock(&mutex);
            reader_unlock(rwlock);

            close(fd);
            return;
        }
    }
    delete (uri, map);

    pthread_mutex_unlock(&mutex);
    reader_unlock(rwlock);
    //debug("GET -- Mutex Unlock1");

    // ---------------------------------------------------------------
    // -- End of Critical Section
    // ---------------------------------------------------------------

    // 4. Send the file
    // (hint: checkout the conn_send_file function!)
    // record a 200 status code in the audit
    conn_send_file(conn, fd, fileSize);
    //res = &RESPONSE_OK;
    statusCode = 200;
    auditLog("GET", conn_get_uri(conn), statusCode, requestID);

    close(fd);
}

void handle_unsupported(conn_t *conn) {
    //debug("handling unsupported request");

    // send responses
    conn_send_response(conn, &RESPONSE_NOT_IMPLEMENTED);
}

void handle_put(conn_t *conn) {

    char *uri = conn_get_uri(conn);
    const Response_t *res = NULL;
    int statusCode = 500;
    char *requestID = NULL;
    requestID = conn_get_header(conn, "Request-Id");

    //debug("handling put request for %s", uri);

    // ---------------------------------------------------------------
    // -- Critical Section
    // ---------------------------------------------------------------

    // Moved sending the response and printing the audit log to insidethe critical section.
    // Passing a lot more mixed cases with this change
    // Maybe sending the reponses with goto printed the audits non-coherently

    pthread_mutex_lock(&mutex);
    //debug("PUT -- Mutex Lock");
    activeWriters = activeWriters + 1;

    // Check if file already exists before opening it.
    bool existed = access(uri, F_OK) == 0;
    //debug("%s existed? %d", uri, existed);

    // Open the file..
    int fd = open(uri, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    writer_lock(rwlock);
    if (fd < 0) {
        //debug("%s: %d", uri, errno);
        if (errno == EACCES || errno == EISDIR || errno == ENOENT) {
            res = &RESPONSE_FORBIDDEN;
            statusCode = 403;
            conn_send_response(conn, res);
            auditLog("PUT", conn_get_uri(conn), statusCode, requestID);
            close(fd);
        } else {
            res = &RESPONSE_INTERNAL_SERVER_ERROR;
            conn_send_response(conn, res);
            auditLog("PUT", conn_get_uri(conn), statusCode, requestID);
            close(fd);
        }
    }
    activeWriters = activeWriters - 1;

    pthread_mutex_unlock(&mutex);
    writer_unlock(rwlock);
    //debug("PUT -- Mutex Unlock");

    // ---------------------------------------------------------------
    // -- End of Critical Section
    // ---------------------------------------------------------------

    res = conn_recv_file(conn, fd);

    if (res == NULL && existed) {
        res = &RESPONSE_OK;
        statusCode = 200;
        conn_send_response(conn, res);
        auditLog("PUT", conn_get_uri(conn), statusCode, requestID);
        close(fd);
    } else if (res == NULL && !existed) {
        res = &RESPONSE_CREATED;
        statusCode = 201;
        conn_send_response(conn, res);
        auditLog("PUT", conn_get_uri(conn), statusCode, requestID);
        close(fd);
    }
}

void auditLog(char *oper, char *uri, int statusCode, char *requestID) {
    // GET (OPER) ,/a.txt (URI) ,200 (STATUS CODE),1 (REQUEST ID)
    fprintf(stderr, "%s,/%s,%d,%s\n", oper, uri, statusCode, requestID);
}
