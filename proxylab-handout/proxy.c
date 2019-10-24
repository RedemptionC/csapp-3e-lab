#include <stdio.h>
#include "csapp.h"
#include "cache.h"

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3";
void doit(int fd);
void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rp,char *host,char *port);
void forward(int clientfd,char *host,char *port,char *uri);
void * threaddoit(void *argp);

int main(int argc, char **argv) 
{
    cache_init();

    Signal(SIGPIPE,SIG_IGN);
    pthread_t tid;
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char *host,*query;
    /* Check command line args */
    if (argc != 2) {
	fprintf(stderr, "usage: %s <port>\n", argv[0]);
	exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) 
    {
        clientlen = sizeof(clientaddr);
        int *connfd=(int *)malloc(sizeof(int)); 
        *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
        Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                        port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        pthread_create(&tid,NULL,threaddoit,connfd);
       // doit(connfd);                                            //line:netp:tiny:doit
       // Close(connfd);                                            //line:netp:tiny:close
        // cache_dealloc(); 好像放在这里会直接被执行，而不是所有线程结束再执行
    }
}

void * threaddoit(void *argp)
{
    int fd=*((int *)argp);
    pthread_detach(pthread_self());
    free(argp);
    doit(fd);
    Close(fd);
    return NULL;
}

void doit(int fd) 
{
    char hostname[MAXBUF];
    char port[10];
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);       //line:netp:doit:parserequest
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(fd, method, "501", "Not Implemented",
                    "Tiny does not implement this method");
        return;
    }                                                    //line:netp:doit:endrequesterr
    read_requesthdrs(&rio,hostname,port);                              //line:netp:doit:readrequesthdrs
    /*注意uri里可能包括host和port,还可能有http:// */
    char *real_uri=&uri[0];
    if(strstr(uri,"http"))
    {
        char *p=index(uri,'/');
        real_uri=index(p+2,'/');
    }
    cacheobj *t=reader(hostname,port,real_uri);
    printf("before forward,host :%s,port:%s,uri:%s\n",hostname,port,uri);
    /*forward request to real server,and response to client*/
    if(t==NULL)
        forward(fd,hostname,port,real_uri);
    else
    {
        /*cache_response*/
        cache_response(fd,t);
    }
}

void forward(int clientfd,char *host,char *port,char *uri)
{
    rio_t rio;
    int serverfd=Open_clientfd(host,port);
    char buf[MAXBUF];
    sprintf(buf,"GET %s HTTP/1.0\r\n",uri);
    sprintf(buf,"%sHost: %s\r\n",buf,host);
    sprintf(buf,"%sUser-Agent: %s\r\n",buf,user_agent_hdr);
    sprintf(buf,"%sConnection: close\r\n",buf);
    sprintf(buf,"%sProxy-Connection: close\r\n\r\n",buf);
    Rio_writen(serverfd,buf,strlen(buf));
    memset(buf,0,MAXBUF);

    Rio_readinitb(&rio,serverfd);

    int len;
    /*注意，无论response是否已被缓存，都要将其转发
    给client，因为此时如果先缓存再用缓存的转发也没有意义
    此时唯一的改动是要将其构建成cacheobj,然后writer

    另：读取response当然可以都用readn,但是为了更好地解析response
    先用readline读取头部，根据\r\n判断结束，再用readn保存response body
    不过，还有一个问题，readn的时候，不要每次都用buf作为参数，读进来一个长度的
    body后，要让buf加上相应的位移？
    */
   /*这里有个bug，因为结构体里的变量是指针，而这里只是指向了局部变量
     没有开辟真正的内存
   */
    cacheobj *t=(cacheobj *)malloc(sizeof(cacheobj));
    build_obj(t,host,port,uri);
    /*read response header using readline*/
    Rio_readlineb(&rio,buf,MAXBUF);
    while(strcmp(buf,"\r\n"))
    {
        char *p;
        if((p=strstr(buf,"length"))!=NULL)
        {   
            t->size=atoi(p+7);
        }
        Rio_writen(clientfd,buf,strlen(buf));
        if((p=strstr(buf,"-type"))!=NULL)
        {   /*这里要编辑字符串，所以先把他写回*/
            *(index(p,'\r'))='\0';
            t->filetype=(p+7);
        }
        Rio_readlineb(&rio,buf,MAXBUF);
    }
    Rio_writen(clientfd,buf,strlen(buf));
    /*read response body using readn*/
    // memset(buf,0,MAXBUF);
    /*直接往cacheobj里写啊，*/
    t->content=(char *)malloc(t->size*(sizeof(char)));
    char *bufp=t->content;
    while((len=Rio_readnb(&rio,bufp,MAXBUF))>0)
    {
        Rio_writen(clientfd,bufp,len);
        bufp+=len;

    }
    if(t->size<=MAX_OBJECT_SIZE)
        writer(t);
    Close(serverfd);
}
void read_requesthdrs(rio_t *rp,char *host,char *port)
{
    char buf[MAXBUF];
    Rio_readlineb(rp,buf,MAXBUF);
    printf("request header :%s\n",buf);
    while(strcmp(buf,"\r\n"))
    {
        if(strcasestr(buf,"host")) /*如果这一行包含host*/
        {

            *(index(buf,'\r'))='\0';
            char * p=index(buf,':');//定位到host前的：
            char * p2=index(p+1,':');//是否含有port，如果没有，默认80
            if(p2==NULL)
            {
                sprintf(port,"80");
                strcpy(host,p+1);
            }
            else
            {
                *p2='\0';
                strcpy(host,p+2);
                strcpy(port,p2+1);// 这里仅考虑了port后面没有路径的，好像有的情况下会有
            }
            break;/*只利用host那一行，获取host和port*/
        }
        Rio_readlineb(rp,buf,MAXBUF);
    }

}

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
