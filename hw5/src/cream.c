#include "cream.h"
#include "utils.h"
#include "csapp.h"
#include "queue.h"
#include "hashmap.h"
#include "debug.h"

queue_t *queue;
hashmap_t *hashmap;

void map_free_function(map_key_t key, map_val_t val) {
    free(val.val_base);
    free(key.key_base);
}

void *thread(void *vargp) {
    Pthread_detach(pthread_self());
    while (1){
        int* connfd_ptr = (int*)dequeue(queue);
        int connfd = *connfd_ptr;
        request_header_t request_header;
        read(connfd, &request_header, sizeof(request_header));
        uint8_t code = request_header.request_code;

        if (code == 0x01) { // PUT
            debug("put");
            if (request_header.key_size >= MIN_KEY_SIZE && request_header.key_size <= MAX_KEY_SIZE && request_header.value_size >= MIN_VALUE_SIZE && request_header.value_size <= MAX_VALUE_SIZE) {
                map_key_t key;
                map_val_t val;
                void *key_ptr = malloc(sizeof(request_header.key_size));
                void *val_ptr = malloc(sizeof(request_header.value_size));
                read(connfd, key_ptr, request_header.key_size);
                read(connfd, val_ptr, request_header.value_size);
                key.key_base = key_ptr;
                key.key_len =request_header.key_size;
                val.val_base = val_ptr;
                val.val_len =request_header.value_size;
                int success = put(hashmap, key, val, true);
                if (success == 1) {
                    response_header_t response_header = {OK, 0};
                    write(connfd, &response_header, sizeof(response_header));
                } else {
                    response_header_t response_header = {BAD_REQUEST, 0};
                    write(connfd, &response_header, sizeof(response_header));
                }
            } else {
                response_header_t response_header = {BAD_REQUEST, 0};
                write(connfd, &response_header, sizeof(response_header));
            }
        }

        else if (code == 0x02) { // GET
            debug("get");
            if (request_header.key_size >= MIN_KEY_SIZE && request_header.key_size <= MAX_KEY_SIZE) {
                map_key_t key;
                void *key_ptr = malloc(sizeof(request_header.key_size));
                read(connfd, key_ptr, request_header.key_size);
                key.key_base = key_ptr;
                key.key_len = request_header.key_size;
                map_val_t val = get(hashmap, key);
                if (val.val_base == NULL && val.val_len == 0) {
                    response_header_t response_header = {NOT_FOUND, 0};
                    write(connfd, &response_header, sizeof(response_header));
                } else {
                    response_header_t response_header = {OK, val.val_len};
                    write(connfd, &response_header, sizeof(response_header));
                    write(connfd, val.val_base, val.val_len);
                }
            } else {
                response_header_t response_header = {BAD_REQUEST, 0};
                write(connfd, &response_header, sizeof(response_header));
            }
        }

        else if (code == 0x04) { // EVICT
            debug("evict");
            if (request_header.key_size >= MIN_KEY_SIZE && request_header.key_size <= MAX_KEY_SIZE) {
                map_key_t key;
                void *key_ptr = malloc(sizeof(request_header.key_size));
                read(connfd, key_ptr, request_header.key_size);
                key.key_base = key_ptr;
                key.key_len = request_header.key_size;
                delete(hashmap, key);

                response_header_t response_header = {OK, 0};
                write(connfd, &response_header, sizeof(response_header));
            } else {
                response_header_t response_header = {BAD_REQUEST, 0};
                write(connfd, &response_header, sizeof(response_header));
            }
        }

        else if (code == 0x08) { // CLEAR
            debug("clear");
            clear_map(hashmap);
            response_header_t response_header = {OK, 0};
            write(connfd, &response_header, sizeof(response_header));
        }

        else { //invalid request
            debug("invalid request\n");
            response_header_t response_header = {UNSUPPORTED, 0};
            write(connfd, &response_header, sizeof(response_header));
        }

        Close(connfd);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("invalid args: too few args\n");
        errno = EINVAL;
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "-h")  == 0) {
        exit(EXIT_SUCCESS);
    }
    if (argc > 4) {
        printf("invalid args: too many args\n");
        errno = EINVAL;
        exit(EXIT_FAILURE);
    }

// argv[1] = NUM_WORKERS    The number of worker threads used to service requests.
// argv[2] = PORT_NUMBER    Port number to listen on for incoming connections.
// argv[3] = MAX_ENTRIES    The maximum number of entries that can be stored in `cream`'s underlying data store.

    Signal(SIGPIPE, SIG_IGN);

    int i, listenfd, *connfd;
    connfd = malloc(sizeof(connfd));
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = open_listenfd(atoi(argv[2]));
    queue = create_queue();
    hashmap = create_map(atoi(argv[3]), jenkins_one_at_a_time_hash, map_free_function);

    for (i = 0; i < atoi(argv[1]); i++) {
            Pthread_create(&tid, NULL, thread, NULL);
    }

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        *connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
        enqueue(queue, connfd);
    }
}

