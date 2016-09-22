//g++ -shared -fPIC rbd_list_watchers.cc -o librbd_list_watchers.so 
//g++ -shared -fPIC rbd_list_watchers.cc -c && ar rcs librbd_list_watchers.a rbd_list_watchers.o 
#include <rados/librados.hpp>
#include <rbd/librbd.hpp>
#include "rbd_list_watchers.h"

#define RBD_HEADER_PREFIX      "rbd_header."
#define RBD_OBJECT_MAP_PREFIX  "rbd_object_map."
#define RBD_DATA_PREFIX        "rbd_data."
#define RBD_ID_PREFIX          "rbd_id."

#define RBD_SUFFIX  ".rbd"

extern "C" int rbd_list_watchers(rados_ioctx_t io, rbd_image_t image, const char *name, struct obj_watch_list **watch_list)
{
    uint8_t old;
    int retval = rbd_get_old_format(image, &old);
    if(retval < 0){
        return retval;
    }
    std::string header_oid;
    if (old) {
        header_oid = name;
        header_oid += RBD_SUFFIX;
    } else {
        rbd_image_info_t info;
        retval = rbd_stat(image, &info, sizeof(info));
        if (retval < 0){
            return retval;
        }

        char prefix[RBD_MAX_BLOCK_NAME_SIZE + 1];
        strncpy(prefix, info.block_name_prefix, RBD_MAX_BLOCK_NAME_SIZE);
        prefix[RBD_MAX_BLOCK_NAME_SIZE] = '\0';

        header_oid = RBD_HEADER_PREFIX;
        header_oid.append(prefix + strlen(RBD_DATA_PREFIX));
    }
    librados::IoCtx io_ctx;
    librados::IoCtx::from_rados_ioctx_t(io, io_ctx);
    std::list<obj_watch_t> watchers;
    io_ctx.list_watchers(header_oid, &watchers);
    for (std::list<obj_watch_t>::iterator i = watchers.begin(); i != watchers.end(); ++i) {
        if(*watch_list == NULL){
            *watch_list = new struct obj_watch_list;
            (*watch_list)->next = NULL;
        }
        memcpy(&(*watch_list)->watcher, &*i, sizeof(obj_watch_t));
        watch_list = &(*watch_list)->next;
    }
    rbd_watch_list_free(*watch_list);
    *watch_list = NULL;
    return retval;
}

extern "C" void rbd_watch_list_free(struct obj_watch_list *watch_list){
    if(watch_list == NULL)
        return;
    rbd_watch_list_free(watch_list->next);
    delete watch_list;
}
