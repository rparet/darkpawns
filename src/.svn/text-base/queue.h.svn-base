/* ************************************************************************
*  File: queue.h                                                          *
*                                                                         *
*  Usage: structures and prototypes for queues                            *
*                                                                         *
*  Written by Eric Green (ejg3@cornell.edu)                               *
*                                                                         *
*  Changes:                                                               *
*      3/6/98 ejg:  Moved defines and structs to queue.c.                 *
************************************************************************ */


/* function protos need by other modules */
struct queue *queue_init(void);
struct q_element *queue_enq(struct queue *q, void *data, long key);
void queue_deq(struct queue *q, struct q_element *qe);
void *queue_head(struct queue *q);
long queue_key(struct queue *q);
long queue_elmt_key(struct q_element *qe);
void queue_free(struct queue *q);


