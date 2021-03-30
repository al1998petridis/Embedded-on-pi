#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define QUEUESIZE 50
#define LOOP 100000
#define PRO_COUNT 1
#define CON_COUNT 33

void *producer (void *args);
void *consumer (void *args);

typedef struct {
  void * (*work)(void *);
  void * arg;
  double wait_time;
} workFunction;

void *showMe(void *i){
  int s = *(int *)i;
  printf("Just Display %d\n",s);
}

struct timeval start_time, end_time;
int counter = 0; //counter for how many items get into queue
double long waits[LOOP*PRO_COUNT];

typedef struct {
  workFunction buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

queue *queueInit (void);
void queueDelete (queue *q);
void queueAdd (queue *q, workFunction *in);
void queueDel (queue *q, workFunction *out);
void csvfile (double long waits[], char *file);

int main ()
{
  double long aggreg_time = 0;
  double long mean_time = 0;
  gettimeofday(&start_time, NULL);
  queue *fifo;
  pthread_t pro[PRO_COUNT], con[CON_COUNT];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  fifo = queueInit ();
  if (fifo ==  NULL) {
    fprintf (stderr, "main: Queue Init failed.\n");
    exit (1);
  }

  for (int i=0; i<PRO_COUNT; i++)
    pthread_create (&pro[i], NULL, producer, fifo);
  for (int i=0; i<CON_COUNT; i++)
    pthread_create (&con[i], NULL, consumer, fifo);
  for (int i=0; i<PRO_COUNT; i++)
    pthread_join (pro[i], NULL);
  for (int i=0; i<CON_COUNT; i++)
    pthread_join (con[i], NULL);

  queueDelete (fifo);
  char file[18] = "for_plots";
  csvfile(waits, file);
  gettimeofday(&end_time, NULL);
  aggreg_time = (end_time.tv_sec - start_time.tv_sec)*1000 + (end_time.tv_usec - start_time.tv_usec)/1000;
  for (int i=0; i<LOOP*PRO_COUNT; i++) {
    mean_time += waits[i];
  }
  mean_time = mean_time/(LOOP*PRO_COUNT);
  printf("Aggregation time of program: %f ms\n", aggreg_time);
  printf("Mean value of waiting time: %f us\n", mean_time);
  return 0;
}

void *producer (void *q)
{
  struct timeval tstart;
  int j, random;
  queue *fifo;
  fifo = (queue *)q;

  for (j = 0; j < LOOP; j++) {
    random = rand()%10;
    workFunction *p1;
    int* argum = (int*)malloc(sizeof(int));
    p1 = (workFunction *)malloc(sizeof(workFunction));
    argum = &random;
    p1->arg = (void *) argum;
    p1->work = &showMe;

    pthread_mutex_lock (fifo->mut);
    while (fifo->full) {
      printf ("producer: queue FULL.\n");
      pthread_cond_wait (fifo->notFull, fifo->mut);
    }
    gettimeofday(&tstart, NULL);
    p1->wait_time = tstart.tv_sec*1000000 + tstart.tv_usec;
    queueAdd (fifo, p1);
    pthread_mutex_unlock (fifo->mut);
    pthread_cond_signal (fifo->notEmpty);
  }
  pthread_cond_broadcast (fifo->notEmpty);
  return (NULL);
}

void *consumer (void *q)
{
  struct timeval tfinish;
  workFunction d;

  queue *fifo;
  fifo = (queue *)q;

  while (1) {
    pthread_mutex_lock (fifo->mut);
    if (!fifo->empty && counter<LOOP*PRO_COUNT) {
      queueDel (fifo, &d);
      gettimeofday(&tfinish,NULL);
      pthread_cond_signal (fifo->notFull);
      waits[counter] = tfinish.tv_sec*1000000 + tfinish.tv_usec - d.wait_time;
      printf("Waiting time was %Lf us\n",waits[counter]);
	    counter++;
      d.work(d.arg);
    }
    else if (counter == LOOP*PRO_COUNT) {
      pthread_mutex_unlock(fifo -> mut);
      break;
    }
    else if (fifo->empty) {
      printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }
/*    while (fifo->empty) {
      printf ("consumer: queue EMPTY.\n");
      pthread_cond_wait (fifo->notEmpty, fifo->mut);
    }
    queueDel (fifo, &d);
    gettimeofday(&tfinish,NULL);
    pthread_cond_signal (fifo->notFull);
    waits[counter] = tfinish.tv_sec*1000000 + tfinish.tv_usec - d.wait_time;
    printf("Waiting time was %lf seconds\n",waits[counter]);
	  counter++;
    d.work(d.arg);
    pthread_cond_signal (fifo->notFull);
    printf ("consumer: recieved\n");*/
    pthread_mutex_unlock (fifo->mut);
  }
  return (NULL);
}



queue *queueInit (void)
{
  queue *q;

  q = (queue *)malloc (sizeof (queue));
  if (q == NULL) return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
  pthread_mutex_init (q->mut, NULL);
  q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
  pthread_cond_init (q->notEmpty, NULL);
	
  return (q);
}

void queueDelete (queue *q)
{
  pthread_mutex_destroy (q->mut);
  free (q->mut);	
  pthread_cond_destroy (q->notFull);
  free (q->notFull);
  pthread_cond_destroy (q->notEmpty);
  free (q->notEmpty);
  free (q);
}

void queueAdd (queue *q, workFunction *in)
{
  q->buf[q->tail] = *in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel (queue *q, workFunction *out)
{
  *out = q->buf[q->head];

  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}

void csvfile (double long waits[], char *file)
{
  FILE *f;
  file = strcat(file, ".csv");
  f = fopen(file, "w");
  for (int i=0; i<LOOP*PRO_COUNT; i++)
    fprintf(f, "%d , %Lf", i,waits[i]);
  fclose(f);
}