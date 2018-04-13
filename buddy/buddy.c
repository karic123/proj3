/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;

	/*DONE!   TODO: DECLARE NECESSARY MEMBER VARIABLES */

	int Pageindex; //index of current page
	char blockAddress; //address of current block
	int blockSize; //order size of current block


} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* TODO: INITIALIZE PAGE STRUCTURES */
		INIT_LIST_HEAD(&g_pages[i].list); //init page list

		g_pages[i].Pageindex = i; //sets page index to i
		g_pages[i].blockAddress = PAGE_TO_ADDR(i); //sets block address to page index to address
		g_pages[i].blockSize = -1; //initialize to -1
	}

	//starts as one block
	g_pages[0].blockSize = MAX_ORDER;

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */

	//out of bounds??
	if (size > (1 << MAX_ORDER) || size<=0)
	{
		printf("ERROR: invalid size \n");
		return NULL;
	}

	int size_order = MIN_ORDER; //what order do we need to alloc this mem??
	int i;

	while(size_order <= MAX_ORDER && (1 << size_order)<size)
	{
		size_order++;
	}

	//printf("size requested: %i, order of %i\n", size, size_order); //DEBUGGING STUFF

	for( i = size_order; i<=MAX_ORDER; i++)//find available memory
	{
		if(!list_empty(&free_area[i]))	//empty block has been found
		{
			page_t *left;
			page_t *right;
			int requestIndex;
			void *requestAddress;
			//I think the variable names are self explanatory

			if(i==size_order)//block has been partitioned WOO!!
			{
				left = list_entry(free_area[i].next, page_t, list);
				list_del(&(left->list)); //delete entry from list because it has been partitioned
			}

			else//break block down and put half in free_area
			{
				left = &g_pages[ADDR_TO_PAGE(buddy_alloc(1<<size_order))]; //move left
				requestIndex = left->Pageindex + (1<<size_order) / PAGE_SIZE; //change request index to new left
				right = &g_pages[requestIndex];//set right to be to the right of left

				list_add(&(right->list), &free_area[size_order]);
			}
			left->blockSize = size_order; //update left block
			requestAddress = PAGE_TO_ADDR(left->Pageindex); //find correct address
			return(requestAddress); //return the address that was requested by the page and check everything again :)

		}
	}

	//cannot find big enough blocks for this request :(
	return NULL;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
