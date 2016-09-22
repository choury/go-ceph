#ifndef RBD_LIST_WATCHERS__
#define RBD_LIST_WATCHERS__
#include <rados/rados_types.h>
#include <rbd/librbd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct obj_watch_list{
    struct obj_watch_t watcher;
    struct obj_watch_list *next;
}obj_watch_list_t;


int rbd_list_watchers(rados_ioctx_t io, rbd_image_t image, const char *name, struct obj_watch_list **watch_list);

void rbd_watch_list_free(struct obj_watch_list *watch_list);

#ifdef __cplusplus
}
#endif

#endif
