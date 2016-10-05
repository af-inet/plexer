#ifndef PLXR_EVENT_H
#define PLXR_EVENT_H

#include <stdlib.h>
#include <poll.h>
#include <errno.h>

#define PLXREV_MAX (256)

#define PLXREV_READ   (1 << 1)
#define PLXREV_WRITE  (1 << 2)
#define PLXREV_POLL   (1 << 3)
#define PLXREV_CLOSE  (1 << 4)
#define PLXREV_ERROR  (1 << 5)

/* Wraps a `fd` monitored for I/O events.
 * `data` may be used for any purpose (will be left untouched by plxr_* functions).
 */
struct plxr_ev_t {

    int fd;

    struct pollfd *pfd;         /* will point to a struct within evset->pfd_set */

    struct plxr_evset_t *evset; /* the set this event belongs to */

    size_t index;               /* index within the evset */

    void (*cb)(struct plxr_ev_t *ev, int event); /* callback for event handling */
 
    /* flag used to mark this event for deletion
     * NOTE: will refactor to a `flags` field if more flags become neccesary
     **/
    char flag_delete;

    void *data; /* user data */

};

/* Polling is optimized by keeping all `struct pollfd`s stored in a continous array.
 */
struct plxr_evset_t {

    struct pollfd    pfd_set[PLXREV_MAX];
    struct plxr_ev_t *ev_set[PLXREV_MAX];
    
    size_t len; /* current number of `pfd`s */

};

/* Initializes a new `ev` adding it to `set`.
 * 
 * Return Value:
 *  on success, `ev` is returned.
 *  on error, NULL is returned.
 *      (If the length of the set was >= PLXREV_MAX.)
 */
struct plxr_ev_t *plxr_ev_add(
    struct plxr_ev_t    *ev,
    struct plxr_evset_t *set,
    int fd,
    int events,
    void (*cb)(struct plxr_ev_t *ev, int event)
);

/* Removes `ev` from it's `pfd_set`.
 */
void plxr_ev_del(struct plxr_ev_t *ev);

/* Polls the `set` of events, triggering callbacks as neccesary.
 *
 * `timeout` (milliseconds) maximum amount of time to wait for events to be ready.
 *
 * Return Value:
 *   on success, the number of events that were ready is returned 
 *      (which is zero on timeout)
 *   on error, -1 is returned.
 */
int plxr_ev_poll(struct plxr_evset_t *set, int timeout);

#endif /* PLXR_EVENT_H */
