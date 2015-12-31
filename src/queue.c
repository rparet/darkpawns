/* ************************************************************************
*  File: queue.c                                                          *
*                                                                         *
*  Usage: generic queue functions for building and using a priority queue *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Moved defines and structs from queue.h.               *
************************************************************************ */

#include "config.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "queue.h"


/* number of queues to use (reduces enqueue cost) */
#define NUM_EVENT_QUEUES    10

struct queue {
  struct q_element *head[NUM_EVENT_QUEUES], *tail[NUM_EVENT_QUEUES];
};

struct q_element {
  void *data;
  long key;
  struct q_element *prev, *next;
};


/* external variables */
extern unsigned long pulse;


/* returns a new, initialized queue */
struct queue *queue_init(void)
{
  struct queue *q;

  CREATE(q, struct queue, 1);

  return q;
}


/* add data into the priority queue q with key */
struct q_element *queue_enq(struct queue *q, void *data, long key)
{
  struct q_element *qe, *i;
  int bucket;

  CREATE(qe, struct q_element, 1);
  qe->data = data;
  qe->key = key;

  bucket = key % NUM_EVENT_QUEUES;   /* which queue does this go in */

  if (!q->head[bucket]) { /* queue is empty */
    q->head[bucket] = qe;
    q->tail[bucket] = qe;
  }

  else {
    for (i = q->tail[bucket]; i; i = i->prev) {

      if (i->key < key) { /* found insertion point */
    if (i == q->tail[bucket])
      q->tail[bucket] = qe;
    else {
      qe->next = i->next;
      i->next->prev = qe;
    }

    qe->prev = i;
    i->next = qe;
    break;
      }
    }

    if (i == NULL) { /* insertion point is front of list */
      qe->next = q->head[bucket];
      q->head[bucket] = qe;
      qe->next->prev = qe;
    }
  }

  return qe;
}


/* remove queue element qe from the priority queue q */
void queue_deq(struct queue *q, struct q_element *qe)
{
  int i;

  assert(qe);

  i = qe->key % NUM_EVENT_QUEUES;

  if (qe->prev == NULL)
    q->head[i] = qe->next;
  else
    qe->prev->next = qe->next;

  if (qe->next == NULL)
    q->tail[i] = qe->prev;
  else
    qe->next->prev = qe->prev;

  FREE(qe);
}


/*
 * removes and returns the data of the
 * first element of the priority queue q
 */
void *queue_head(struct queue *q)
{
  void *data;
  int i;

  i = pulse % NUM_EVENT_QUEUES;

  if (!q->head[i])
    return NULL;

  data = q->head[i]->data;
  queue_deq(q, q->head[i]);
  return data;
}


/*
 * returns the key of the head element of the priority queue
 * if q is NULL, then return the largest unsigned number
 */
long queue_key(struct queue *q)
{
  int i;

  i = pulse % NUM_EVENT_QUEUES;

  if (q->head[i])
    return q->head[i]->key;
  else
    return LONG_MAX;
}


/* returns the key of queue element qe */
long queue_elmt_key(struct q_element *qe)
{
  return qe->key;
}


/* free q and contents */
void queue_free(struct queue *q)
{
  int i;
  struct q_element *qe, *next_qe;

  for (i = 0; i < NUM_EVENT_QUEUES; i++)
    for (qe = q->head[i]; qe; qe = next_qe) {
      next_qe = qe->next;
      FREE(qe);
    }

  FREE(q);
}
