#include "cache.h"

int readcount;
sem_t w,mutex;
cacheobj *head=NULL;

/*这种实现暗藏了一个问题，就是如果指定url的内容改变了，他仍然会返回缓存*/
/*only if host,port,uri match ,return 1,otherwise return 0*/
int obj_match(cacheobj * co,char *host,char *port,char *uri)
{
    if(strcmp(co->host,host))
        return 0;
    if(strcmp(co->port,port))
        return 0;
    if(strcmp(co->uri,uri))
        return 0;
    return 1;
}

/*
    if find cached obj,read it from cache,then response from proxy
    while traversing,increment every obj's lru
*/
cacheobj * reader(char *host,char *port,char *uri)
{
    /*现在的问题是读者也要写（修改lru），分的不是很开，暂时先用个大锁吧*/
    // P(&mutex)
    // readcount++;
    // if(readcount==1)
    //     P(&w);
    
    cacheobj *t=head->next;
    int match=0;
    while(t!=NULL)
    {
        match=obj_match(t,host,port,uri);
        P(&mutex);   
        t->lru++;
        V(&mutex);

        if(match==1)
        {
            P(&mutex);
            /*match，也就代表要读它，将其lru设为0，即最近使用过的*/
            t->lru=0;
            V(&mutex);
            break;
        }
        
        t=t->next;
    }
    /*跳出循环，要么t==null,要么match==1,此时t!=null,总之，直接返回t即可*/
    return t;
}

cacheobj * get_lru_obj()
{
    cacheobj *t=head->next,*lru_obj=head->next;
    while(t!=NULL)
    {
        if(t->lru>lru_obj->lru)
        {
            lru_obj=t;
        }
        t=t->next;
    }
    return lru_obj;
}

/*find previous node,and delete it*/
/*并且从head->size减去obj->size*/

void delete(cacheobj *obj)
{
    cacheobj *t=head;
    while(t->next!=obj)
    {
        t=t->next;
    }
    P(&mutex);
    t->next=t->next->next;
    head->size-=obj->size;
    V(&mutex);
}

void insert(cacheobj *obj)
{
    while(head->size+obj->size>MAX_CACHE_SIZE)
    {
        cacheobj *dobj = get_lru_obj(head);
        delete(dobj);
    }
    /*now we have enough space,insert it into the list*/
    P(&mutex);
    obj->next=head->next;
    head->next=obj;
    V(&mutex);
}

/*
    在proxy里判断了不超过MAX_OBJ再执行writer
*/
void writer(cacheobj *obj)
{
    cacheobj *t=head->next,*lru_obj=head->next;
    int match=0,sum_size=0;
    while(t!=NULL)
    {
        P(&mutex);
        t->lru++;
        head->size+=t->size;
        V(&mutex);
        match=obj_match(t,obj->host,obj->port,obj->uri);
        
        if(match==1)
            break;

        t=t->next;
    }
    /*如果尚未缓存，那么缓存，调用insert函数*/
    if(match==0)
    {
        insert(obj);
    }
    else
    {
        /*已经缓存，此时不需要写cache了*/
        return;
    }
    
}

/*init cache head,we only use head's size and next*/
void cache_init()
{
    head=(cacheobj *)malloc(sizeof(cacheobj));
    head->content=NULL;
    head->host=NULL;
    head->lru=0;
    head->next=NULL;
    head->size=0;
    head->uri=NULL;
    head->filetype=NULL;
    readcount=0;
    Sem_init(&mutex,0,1);
    Sem_init(&w,0,1);
}

/*before it teminates,free all cache obj*/
void cache_dealloc()
{
    while(head!=NULL)
    {
        cacheobj *t=head;
        head=head->next;
        free(t);
    }
}

void cache_response(int fd,cacheobj *obj)
{
    /*response header*/
    char buf[MAXBUF];
    sprintf(buf, "HTTP/1.0 200 OK\r\n");    //line:netp:servestatic:beginserve
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, obj->size);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, obj->filetype);
    Rio_writen(fd, buf, strlen(buf));

    /*response body*/
    Rio_writen(fd,obj->content,strlen(obj->content));
}

void build_obj(cacheobj *obj,char *host,char *port,char *uri)
{   
    obj->host=(char *)malloc(sizeof(char)*strlen(host));
    strcpy(obj->host,host);
    obj->port=(char *)malloc(sizeof(char)*strlen(port));
    strcpy(obj->port,port);
    obj->uri=(char *)malloc(sizeof(char)*strlen(uri));
    strcpy(obj->uri,uri);
    obj->lru=0;
    obj->next=NULL;
    obj->size=0;
    obj->filetype=NULL;
    obj->content=NULL;
}