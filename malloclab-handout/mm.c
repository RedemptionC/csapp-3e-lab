/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "red",
    /* First member's full name */
    "red",
    /* First member's email address */
    "M201976393@hust.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


/* $begin mallocmacros */
/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ //line:vm:mm:beginconst
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  //line:vm:mm:endconst 

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) //line:vm:mm:pack

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            //line:vm:mm:get
#define PUT(p, val)  (*(unsigned int *)(p) = (val))    //line:vm:mm:put

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   //line:vm:mm:getsize
#define GET_ALLOC(p) (GET(p) & 0x1)                    //line:vm:mm:getalloc

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)                      //line:vm:mm:hdrp
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE) //line:vm:mm:ftrp

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE))) //line:vm:mm:nextblkp
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE))) //line:vm:mm:prevblkp

/*explicit :bp is a node in free list,compute address of next and previous node to bp*/
#define NEXT_NODE(bp)   GET((char *)bp+WSIZE)
#define PREV_NODE(bp)   GET(bp)
/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */  
static char *free_head=NULL;

/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void  place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void checkheap(int verbose);
static void checkblock(void *bp);

/* 
 * mm_init - Initialize the memory manager 
 */
/* $begin mminit */
int mm_init(void) 
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) //line:vm:mm:begininit
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += (2*WSIZE);                     //line:vm:mm:endinit  
    free_head=NULL;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) 
        return -1;
    return 0;
}
/* $end mminit */

/*
    修改前驱后继的指针，将bp从free list中删除
    在两种情况下被调用：
    1.空闲块合并
    2.place，如果剩下的块不够split，就把一整个空闲块
    用来放置，因此需要将这整个块从free list中删除
*/
void freelist_delete(void * bp)
{
    /*
    如果bp在链表中间，那么头指针不用改变
    如果bp就是头节点，那么头指针指向bp的前驱
    */
//    mm_checkheap(1); //不能在这里检查frelist里是否有没有标记为free的，因此这个函数就是用来删除free block的
    int fhead=bp==free_head?1:0;
    char *nextnode=NEXT_NODE(bp);
    char *prevnode=PREV_NODE(bp);
    // printf("bp : %p,prevnode :%p,nextnode :%p\n",bp,prevnode,nextnode);
    if(nextnode!=NULL)
        PUT(nextnode,prevnode);
    if(prevnode!=NULL)
    {
        // printf("PUT(%p,%p)\n",prevnode+4,nextnode);
        PUT(prevnode+WSIZE,nextnode);
    }
    PUT(bp,NULL);/*删除节点后，要把原来存的指针删掉，他已经 不属于free list了*/
    PUT((char *)bp+WSIZE,NULL);
    if(fhead)
    {
        free_head=prevnode;
    }
    // mm_checkheap(1);
}

/*
    将新的空闲块插在表头
*/
void freelist_inserth(void *bp)
{
    if(free_head==NULL)
    {
        /*
        第一个空闲块，除了要让头指针指向它，还要设置它的前驱后继为NULL        */
        free_head=bp;
        PUT(free_head,NULL);
        PUT((char *)free_head+WSIZE,NULL);
        return;
    }
    PUT(bp,free_head);
    PUT((char *)bp+WSIZE,NULL);/*设置前驱为原head,后继为NULL*/
    PUT(free_head+WSIZE,bp);
    free_head=bp;
}
/* 
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
/* $begin mmmalloc */
void *mm_malloc(size_t size) 
{
    size_t asize;      /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;      

    /* $end mmmalloc */
    if (heap_listp == 0){
        mm_init();
    }
    /* $begin mmmalloc */
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)                                          //line:vm:mm:sizeadjust1
        asize = 2*DSIZE;                                        //line:vm:mm:sizeadjust2
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE); //line:vm:mm:sizeadjust3

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {  //line:vm:mm:findfitcall
        place(bp, asize);                  /*问题在这，我在place里已经修改bp了，为什么返回后又没了？？*/
        // mm_checkheap(1);
        return bp;
        // return coalesce(bp);//bp指向分配好的块，所以不应该coalesce
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);                 //line:vm:mm:growheap1
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)  
        return NULL;                                  //line:vm:mm:growheap2
    place(bp, asize);                                 //line:vm:mm:growheap3
    // mm_checkheap(1);
    return bp;
} 
/* $end mmmalloc */

/* 
 * mm_free - Free a block 
 */
/* $begin mmfree */
void mm_free(void *bp)
{
    /* $end mmfree */
    if (bp == 0) 
        return;

    /* $begin mmfree */
    size_t size = GET_SIZE(HDRP(bp));
    /* $end mmfree */
    if (heap_listp == 0){
        mm_init();
    }
    /* $begin mmfree */

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    freelist_inserth(bp);
    coalesce(bp);
// mm_checkheap(1);
}

/* $end mmfree */
/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin mmfree */
static void *coalesce(void *bp) 
{
    char * prevblock=PREV_BLKP(bp);
    char * nextblock=NEXT_BLKP(bp);
    size_t prev_alloc = GET_ALLOC(FTRP(prevblock));
    size_t next_alloc = GET_ALLOC(HDRP(nextblock));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
        freelist_delete(nextblock);
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));/*设置合并之后的header和footer*/
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        freelist_delete(bp);
        bp = PREV_BLKP(bp);//？
        /*
        合并之后，对应的指针要删掉
        */
    }

    else {                                     /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        // bp = PREV_BLKP(bp);
        freelist_delete(bp);
        freelist_delete(nextblock);
        bp=prevblock;
    }

    return bp;
}
/* $end mmfree */

/*
 * mm_realloc - Naive implementation of realloc
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    /* Free the old block. */
    mm_free(ptr);

    return newptr;
}

/* 
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose)  
{ 
    checkheap(verbose);
}

/* 
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; //line:vm:mm:beginextend
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        //line:vm:mm:endextend

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   //line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   //line:vm:mm:freeblockftr
    freelist_inserth(bp);
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ //line:vm:mm:newepihdr

    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          //line:vm:mm:returnblock
}
/* $end mmextendheap */

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    size_t csize = GET_SIZE(HDRP(bp));   

    if ((csize - asize) >= (2*DSIZE)) { 
        char *nextnode=NEXT_NODE(bp);
        char *prevnode=PREV_NODE(bp);/*保存原来块在freelist中的前驱后继*/
        PUT(HDRP(bp), PACK(asize, 1));/*大空闲块中用来分配的，设置head和footer*/
        PUT(FTRP(bp), PACK(asize, 1));
        int fhead=bp==free_head?1:0;/*bp是表头吗*/
        bp = NEXT_BLKP(bp);/*bp指向remaing free block*/
        /*
            now bp points to the remaining block
            修改前驱后继以及remaining block的指针，使remaining block
            处于原来的位置
        */
        if(nextnode!=NULL)
            PUT(nextnode,bp);/*debug:对于第一个chunk，也是第一个空闲块，他的前驱后继为NULL，所以这里是向NULL写*/
        if(prevnode!=NULL)
            PUT(prevnode+WSIZE,bp);
        PUT(bp,prevnode);   /*设置剩下自由块的前驱后继指针（与原自由块相同）*/
        PUT((char *)bp+WSIZE,nextnode);
        if(fhead)
        {
            free_head=bp;/*如果bp原来是表头，现在头指针仍指向它*/
        }

        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));/*设置剩下自由块的header和footer*/
    }
    else { 
        PUT(HDRP(bp), PACK(csize, 1));
        /*
            delete from free list
            modify next and previous node's pointers to it
            修改后继的前驱为自己的前驱，前驱的后继为自己的后继
        */
        PUT(FTRP(bp), PACK(csize, 1));
        freelist_delete(bp);
    }
    // return bp;
}
/* $end mmplace */

/* 
 * find_fit - Find a fit for a block with asize bytes 
 */

static void *find_fit(size_t asize)
{
    /*no free block*/
    if(free_head==NULL)
        return NULL;
    /* First-fit search */
    void *bp;

    for (bp = free_head; bp!=NULL; bp = PREV_NODE(bp)) {
        if (asize <= GET_SIZE(HDRP(bp)))
        {
            return bp;
        }
    }
    return NULL; /* No fit */

}
/* $end mmfirstfit */

static void printblock(void *bp) 
{
    size_t hsize, halloc, fsize, falloc;

    checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));  
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));  

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp, 
           hsize, (halloc ? 'a' : 'f'), 
           fsize, (falloc ? 'a' : 'f')); 
}

static void checkblock(void *bp) 
{
    if ((size_t)bp % 8)
        printf("Error: %p is not doubleword aligned\n", bp);
    if (GET(HDRP(bp)) != GET(FTRP(bp)))
    {
        printf("Error: header does not match footer\n");
        exit(1);
    }
}

/* 
 * checkheap - Minimal check of the heap for consistency 
 */
void checkheap(int verbose) 
{
    char *bp = heap_listp;

    if (verbose)
        printf("Heap (%p):\n", heap_listp);

    /*检查prologue的header size是否为8，是否为allocated*/
    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
    {
        printf("Bad prologue header\n");
        exit(1);
    }
    checkblock(heap_listp);

    /*检查每个块是否对齐，是否header和footer里的size相等*/
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose) 
            printblock(bp);
        checkblock(bp);
    }

    if (verbose)
        printblock(bp);
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");

    /*检查free list*/
    void * t=free_head;
    for(;t!=NULL;t=PREV_NODE(t))
    {
        if(verbose)
        {
            printf("free node *** prevnode %p nextnode %p ",PREV_NODE(t),NEXT_NODE(t));
            printblock(t);
        }
        if(GET_ALLOC(HDRP(t))!=0)
        {
            printf("%p free node not marked as free\n",t);
            exit(1);
        }

    }
}
