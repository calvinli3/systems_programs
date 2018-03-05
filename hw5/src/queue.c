#include "queue.h"
#include "debug.h"

queue_t *create_queue(void) {
    queue_t* new_queue = (queue_t*)calloc(1, sizeof(queue_t));
    if (new_queue == NULL) {
        return NULL;
    }
    sem_t items_sem;
    if (sem_init(&items_sem, 0, 0) != 0) {
        return NULL;
    }
    pthread_mutex_t lock_mtx;
    if (pthread_mutex_init(&lock_mtx, NULL) != 0) {
        abort();
    }
    new_queue->items = items_sem;
    new_queue->lock = lock_mtx;

    new_queue->front = NULL;
    new_queue->rear = NULL;
    return new_queue;

}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    if(pthread_mutex_lock(&(self->lock)) != 0) {
        abort();
    }
    //if any pointer is null, or if queue_t pointer is invalidated, return false
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        if(pthread_mutex_unlock(&(self->lock)) != 0) {
            abort();
        }
        return false;
    }

    //loop thru queue and call destroy_function on each item in queue
    // and free the queue_node_t instances
    queue_node_t* current = self->front;

    while (current != NULL) {
        queue_node_t* temp = current->next;
        destroy_function(current->item);
        free(current);
        current = temp;
    }
    self->invalid = true;
    if(pthread_mutex_unlock(&(self->lock)) != 0) {
        abort();
    }
    return true;
}

bool enqueue(queue_t *self, void *item) {
    if(pthread_mutex_lock(&(self->lock)) != 0) {
        abort();
    }
    //if any pointer is null, or if queue_t pointer is invalidated, return false
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        if(pthread_mutex_unlock(&(self->lock)) != 0) {
            abort();
        }
        return false;
    }

    // calloc a new queue_node_t to add to queue
    queue_node_t* node = (queue_node_t*)calloc(1, sizeof(queue_node_t));

    if (self->front == NULL) {
        self->front = node;
        self->rear = node;
    } else {
        self->rear->next = node;
        self->rear = node;
    }

    if(sem_post(&(self->items)) != 0) {
        if(pthread_mutex_unlock(&(self->lock)) != 0) {
            abort();
        }
        return false;
    }
    // new queue_node_t should have pointer to item
    node->item = item;
    if(pthread_mutex_unlock(&(self->lock)) != 0) {
        abort();
    }

    return true;
}

void *dequeue(queue_t *self) {
    if(sem_wait(&(self->items)) != 0) {
        return false;
    }
    if(pthread_mutex_lock(&(self->lock)) != 0) {
        abort();
    }

     //if any pointer is null, or if queue_t pointer is invalidated, return false
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        if(pthread_mutex_unlock(&(self->lock)) != 0) {
            abort();
        }
        return false;
    }

    void* temp_item = self->front->item;
    queue_node_t* temp_head = self->front;
    if (self->front->next == NULL) {
        self->front = NULL;
    } else {
        self->front = self->front->next;
    }

    free(temp_head);

    if(pthread_mutex_unlock(&(self->lock)) != 0) {
        abort();
    }

    return temp_item;
}
