/*
条件编译还是不懂
*/
#include "csapp.h"



/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cacheobj
{
    char *port;/*指针还是数组？*/
    char *host;
    char *uri;
    unsigned int size;
    unsigned int lru;
    struct cacheobj *next;
    char *content;
    char *filetype;
    // struct cacheobj *prev;   /*没必要双端，头插就行了*/
};

typedef struct cacheobj cacheobj;

int obj_match(cacheobj * co,char *host,char *port,char *uri);

cacheobj * reader(char *host,char *port,char *uri);

void writer(cacheobj *obj);

void insert(cacheobj *obj);

cacheobj * get_lru_obj();

void delete(cacheobj *obj);

void cache_init();

void cache_dealloc();

void cache_response(int fd,cacheobj *obj);

void build_obj(cacheobj *obj,char *host,char *port,char *uri);