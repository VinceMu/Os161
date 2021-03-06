#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "bar.h"
#include "bar_driver.h"



/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */

/* Declare any globals you need here (e.g. locks, etc...) */

#define BUFFER_SIZE 10
#define TRUE 1
#define FALSE 0

static struct semaphore *orderQueue,*emptyCount, *fullCount;
static const char* drinks[10] = {
                                    "beer",
                                    "vodka",
                                    "rum",
                                    "gin",
                                    "tequila",
                                    "brandy",
                                    "whisky",
                                    "bourbon",
                                    "triple_sec",
                                    "orange_juice"
                                };

static struct lock *drink_lockArray[10];
static struct barorder* myBottles[BUFFER_SIZE];

int firstGlobalIndex;
int lastGlobalIndex; 

/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */

/*
 * order_drink()
 *
 * Takes one argument referring to the order to be filled. The
 * function makes the order available to staff threads and then blocks
 * until a bartender has filled the glass with the appropriate drinks.
 */

void order_drink(struct barorder *order)
{
    P(emptyCount);
    P(orderQueue);
    
    struct barorder* bOrder = kmalloc (sizeof (struct barorder));
    memcpy (bOrder, order, sizeof (struct barorder));
    myBottles[firstGlobalIndex] = bOrder;

    if(order->go_home_flag == TRUE) 
        bOrder->lock = NULL;
    if(order->go_home_flag == FALSE) 
        bOrder->lock = sem_create ("lock",0);

    firstGlobalIndex = (firstGlobalIndex + 1) % BUFFER_SIZE;

    V(orderQueue);
    V(fullCount);

    if((*order).go_home_flag == TRUE) return;
    P(bOrder->lock);
    sem_destroy (bOrder->lock); 
    memcpy (&order->glass, &bOrder->glass, sizeof (struct glass));
    kfree (bOrder);
}



/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY BARTENDER THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it returns a bointer to the order.
 *
 */

struct barorder *take_order(void)
{
    P(fullCount);
    P(orderQueue);

    struct barorder *ret = myBottles[lastGlobalIndex];
    lastGlobalIndex = (lastGlobalIndex + 1) % BUFFER_SIZE;

    V(orderQueue);
    V(emptyCount);

    return ret;
}


/*
 * fill_order()
 *
 * This function takes an order provided by take_order and fills the
 * order using the mix() function to mix the drink.
 *
 * NOTE: IT NEEDS TO ENSURE THAT MIX HAS EXCLUSIVE ACCESS TO THE
 * REQUIRED BOTTLES (AND, IDEALLY, ONLY THE BOTTLES) IT NEEDS TO USE TO
 * FILL THE ORDER.
 */

void fill_order(struct barorder *order)
{

        unsigned *list = order->requested_bottles;
        int curr, i, j;
        bool swap = FALSE;
        
        for(i = 0; i < DRINK_COMPLEXITY-1; i++) 
        { 
            swap = FALSE;
            for(j = 0; j < DRINK_COMPLEXITY-1-i; j++) {           
                if(list[j] > list[j+1]) {
                    curr = list[j];
                    list[j] = list[j+1];
                    list[j+1] = curr;
                    swap = TRUE;
                }    
            }
            if(!swap) break;
        }

        for (int i = 0; i < DRINK_COMPLEXITY; ++i)
        {
            if (list[i] != 0) 
                lock_acquire(drink_lockArray[list[i]-1]);
        }

        mix(order);
        for (int i = 0; i < DRINK_COMPLEXITY; ++i)
        {
            if (list[i] != 0) 
                lock_release(drink_lockArray[list[i]-1]);
        }

}


/*
 * serve_order()
 *
 * Takes a filled order and makes it available to (unblocks) the
 * waiting customer.
 */

void serve_order(struct barorder *order)
{
    if((*order).go_home_flag == TRUE) return;
    V(order->lock);
}



/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * bar_open()
 *
 * Perform any initialisation you need prior to opening the bar to
 * bartenders and customers. Typically, allocation and initialisation of
 * synch primitive and variable.
 */

void bar_open(void)
{
    orderQueue = sem_create("orderQueue", 1);
    emptyCount = sem_create("emptyCount", BUFFER_SIZE);
    fullCount = sem_create("fullCount", 0);
    for (int i = 0; i < 10; ++i) 
        drink_lockArray[i] = lock_create(drinks[i]);
}

/*
 * bar_close()
 *
 * Perform any cleanup after the bar has closed and everybody
 * has gone home.
 */

void bar_close(void)
{
    memset (&myBottles, 0, sizeof (myBottles));
    sem_destroy(orderQueue);
    sem_destroy(emptyCount);
    sem_destroy(fullCount);
    for (int i = 0; i < 10; ++i)
        lock_destroy(drink_lockArray[i]);
}
