#ifndef ESP_EV_COMPAT_H
#define ESP_EV_COMPAT_H

#include "esp_event.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define MAX_IO_WATCHERS 16
#define MAX_TIMER_WATCHERS 16

// Basic types to match libev
typedef struct ev_loop ev_loop;
typedef struct ev_io ev_io;
typedef struct ev_timer ev_timer;
typedef float ev_tstamp;

// Event data structures
typedef struct {
    ev_io *watcher;
    int revents;
} io_event_data;

typedef struct {
    ev_timer *watcher;
    int revents;
} timer_event_data;

// Event types
#define EV_READ  1
#define EV_WRITE 2
#define EV_TIMER 4

// Default event loop instance
extern ev_loop *EV_DEFAULT;

// Function prototypes
void ev_io_init(ev_io *watcher, void (*cb)(ev_loop *loop, ev_io *w, int revents), int fd, int events);
void ev_io_start(ev_loop *loop, ev_io *watcher);
void ev_io_stop(ev_loop *loop, ev_io *watcher);

void ev_timer_init(ev_timer *watcher, void (*cb)(ev_loop *loop, ev_timer *w, int revents), ev_tstamp after, ev_tstamp repeat);
void ev_timer_again(ev_loop *loop, ev_timer *watcher);
void ev_timer_stop(ev_loop *loop, ev_timer *watcher);

void ev_run(ev_loop *loop, int flags);
void ev_break(ev_loop *loop, int how);
void ev_default_loop_init(void);

#define EVBREAK_ALL 0

// Implementation-specific structures
struct ev_loop {
    bool running;
    
    // ESP event loop handle
    esp_event_loop_handle_t esp_event_loop;
    
    // IO watchers
    ev_io *io_watchers[MAX_IO_WATCHERS];
    int io_count;
    
    // Mutex for thread-safe watcher management
    SemaphoreHandle_t io_mutex;
    
    // Task handle for IO monitoring
    TaskHandle_t io_task_handle;
};

struct ev_io {
    void (*cb)(ev_loop *loop, ev_io *w, int revents);
    int fd;
    int events;
    int active;
    void *data;
};

struct ev_timer {
    void (*cb)(ev_loop *loop, ev_timer *w, int revents);
    ev_tstamp after;
    ev_tstamp repeat;
    int active;
    void *data;
    
    // ESP-specific fields
    esp_timer_handle_t esp_timer_handle;
    ev_loop *loop;
};


#endif // ESP_EV_COMPAT_H