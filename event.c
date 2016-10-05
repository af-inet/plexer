
#include "event.h"

/* Unlike deletion, adding is safe to do even
 * while we're iterating through the array, since
 * elements are always added to the end.
 */
struct plxr_ev_t *plxr_ev_add(
    struct plxr_ev_t    *ev,
    struct plxr_evset_t *set,
    int fd,
    int events,
    void (*cb)(struct plxr_ev_t *ev, int event)
){
    if ( set->len >= PLXREV_MAX ) {
        return NULL;
    }

    /* put this `ev` on the end of the array */
    ev->index = set->len;

    /* save a reference to this `ev` */
    set->ev_set[ ev->index ] = ev;

    /* pfd configuration */
    ev->pfd = & set->pfd_set[ ev->index ];
    ev->pfd->fd = fd;
    ev->pfd->events = 0;

    /* poll flags, see `man poll` */
    if ( events & PLXREV_READ )
        ev->pfd->events |= POLLIN;

    if ( events & PLXREV_WRITE )
        ev->pfd->events |= POLLOUT;
 
    /* ev configuration */
    ev->fd = fd;
    ev->cb = cb;
    ev->evset = set;
    ev->flag_delete = 0;

    set->len++;

    return ev;
}

/* Modifying an array while you're iterating through is is tricky business.
 *
 * So instead of deleting right away we mark it to be deleted at a later, safer time -
 * this prevents edge cases which cause undesired behavior... (such as events which trigger other events to be deleted)
 */
void plxr_ev_del(
    struct plxr_ev_t *ev
){
    ev->flag_delete = 1;
    /* this `ev` is being deleted, so lets not actually poll it */
    ev->pfd->fd = -1;
}

void plxr_ev_del_(
    struct plxr_ev_t *ev
){
    struct plxr_ev_t *last_ev;
    struct plxr_evset_t *set;

    set = ev->evset;

    /* grab the last `ev` */
    last_ev = set->ev_set[ set->len - 1 ];

    /* overwrite `ev` with `last_ev` */
    set->ev_set[  ev->index ] = last_ev;
    set->pfd_set[ ev->index ] = *last_ev->pfd;

    /* reassign it's index */
    last_ev->index = ev->index;

    /* reassign it's pfd */
    last_ev->pfd = & set->pfd_set[ ev->index ];

    set->len--;
}

#define POLL_ERROR   (-1)
#define POLL_TIMEOUT  (0)
/* from <poll.h>
 * POLLERR
 * POLLHUP
 * POLLIN
 * POLLNVAL
 * POLLOUT
 * POLLPRI
 */
int plxr_ev_poll(struct plxr_evset_t *set, int timeout) {
    struct plxr_ev_t *ev;
    int events;
    int res;
    size_t i;

    if (set->len == 0) {
        /* there weren't any events to process */
        return 0;
    }

    res = poll(set->pfd_set, set->len, timeout);

    switch (res) {
        case POLL_ERROR:
            return -1;
        case POLL_TIMEOUT:
            return 0;
        default:
            /* Success! Lets go handle those events. */
            break;
    }

    for( i=0; i<set->len; i++ ) {

        ev = set->ev_set[i];

        // Before we process this ev, lets check if it should be deleted...
        if (ev->flag_delete) {
            plxr_ev_del_(ev);
            /* Deleting the `i`th element actually overwrites `i` with the last element,
             * so after we delete this we want to back up and process the new element at this index.
             */
            i -= 1;
            // This `ev` was deleted, so lets skip triggering event callbacks.
            continue;
        }

        events = ev->pfd->revents;

        if ( events & POLLIN )
            ev->cb( ev, PLXREV_READ );

        if ( events & POLLOUT )
            ev->cb( ev, PLXREV_WRITE );

        if ( events & POLLHUP )
            ev->cb( ev, PLXREV_CLOSE );

        if ( events & POLLNVAL )
            ev->cb( ev, PLXREV_ERROR );

        if ( events & POLLERR )
            ev->cb( ev, PLXREV_ERROR );
    }

    return res;
}

