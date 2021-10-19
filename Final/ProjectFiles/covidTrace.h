/*********************************************************************
 * File: covidTrace.h
 * 
 * Author: Alexandros Petridis
 * Date: September 2021
 *
 * Description: Header file for structs, thread functions and helpers
 ********************************************************************/
#ifndef __COVIDTRACE_H__
#define __COVIDTRACE_H__

#include <stdio.h>
#include <time.h> 
#include <errno.h>  	
#include <stdint.h> 
#include <inttypes.h>  
#include <stdlib.h>	
#include <stdbool.h> 	
#include <pthread.h>	

#define SECOND 0.01
#define POPULATION 100000


/****************** GLOBAL VARIABLES AND STRUCTS **********************/
struct timespec ts;

pthread_mutex_t mutex;

FILE* near_contacts_file;
FILE* log_file;
FILE* BTnearMe_call;

unsigned char buff1[100];
unsigned char buff2[200];

// 48-bit value
typedef struct { 
  uint64_t v: 48;
} uint48;

typedef struct {
  uint48 macValue;
  time_t timeCaptured;
  int key;
} macaddress;

struct node {
  macaddress value;
  struct node* next;
};
typedef struct node node_t;

node_t* allContactsHead = NULL;
node_t* nearContactsHead = NULL;

/************************* NODE HELPERS *****************************/
/*********************************************************************
 * Function: search
 * Description:
 *   This function search a macaddress type of data from a Linked list
 * Parameters:
 *   node_t* head = pointer to head of list
 *   int key = key of data to be searched from list
 * Return:
 *   (struct timespec) The time that specific macaddress was captured
 ********************************************************************/
time_t search(node_t* head, int key) {
  node_t* current = head;
  while (current != NULL) {
    if (current->value.key == key)
      return current->value.timeCaptured;
    current = current->next;
  }
  return 0;
}

/*********************************************************************
 * Function: getCount
 * Description:
 *   This function counts the length of a Linked list
 * Parameters:
 *   node_t* head = pointer to head of list
 * Return:
 *   (int) Length of list 
 ********************************************************************/
int getCount(node_t* head) {
  if (head == NULL)
    return 0;
  else
    return 1 + getCount(head->next);
}

/*********************************************************************
 * Function: push
 * Description:
 *   This function pushes a macaddress type of data into a Linked list
 * Parameters:
 *   node_t** head_ref = double pointer to head of list
 *   macaddress new_mac = data to be pushed into list
 * Return:
 *   Nothing
 ********************************************************************/
void push(node_t** head_ref, macaddress new_mac) {
  node_t* new_node = malloc(sizeof(node_t));
  new_node->value = new_mac;
  new_node->next = (*head_ref);
  (*head_ref) = new_node;
  fprintf(log_file, "Pushed Mac address: %" PRIu64 ", Time scanned:%d\n", new_mac.macValue, (int) new_mac.timeCaptured);
  fflush(log_file);
}

/*********************************************************************
 * Function: deleteNode
 * Description:
 *   This function deletes a macaddress type of data from a Linked list
 * Parameters:
 *   node_t** head_ref = double pointer to head of list
 *   macaddress new_mac = data to be pushed into list
 * Return:
 *   Nothing
 ********************************************************************/
void deleteNode(node_t** head_ref, macaddress new_mac) {
  node_t* temp = *head_ref, *prev;
  if (temp != NULL && temp->value.key == new_mac.key) {
    *head_ref = temp->next;
    free(temp);
    return;
  }
  while (temp != NULL && temp->value.key != new_mac.key) {
    prev = temp;
    temp = temp->next;
  }
  if (temp == NULL)
    return;
  prev->next = temp->next;
  free(temp);
  fprintf(log_file, "Delete Mac address: %" PRIu64 ", Time scanned:%d\n", new_mac.macValue, (int) new_mac.timeCaptured);
  fflush(log_file);
} 



/*********************** FUNCTIONS HELPERS **************************/
/*********************************************************************
 * Function: BTnearMe
 * Description:
 *   This function creates a macaddress every 10 seconds
 * Parameters:
 *   No parameters
 * Return:
 *   (macaddress) A new random macaddress
 ********************************************************************/
macaddress BTnearMe() {
  macaddress randMac;
  randMac.timeCaptured = time(NULL);
  timespec_get(&ts, TIME_UTC);
  strftime(buff1, sizeof(buff1), "%T", gmtime(&ts.tv_sec));
  fprintf(BTnearMe_call, "%s.%09ld\n", buff1, ts.tv_nsec);
  fflush(BTnearMe_call);
//  sprintf(buff2, "%s.%09ld\n", buff1, ts.tv_nsec);
//  fwrite(buff2, sizeof(buff2), 1, BTnearMe_call);
  randMac.key = (rand() % POPULATION) + 1;
  randMac.macValue.v = (uint64_t) randMac.key; 
  return randMac;
}


/*********************************************************************
 * Function: push_contact
 * Description:
 *   This function makes the choise of all contacts vs near contacts
 * Parameters:
 *  (macaddress) randMac = The MAC address to be pushed
 * Return:
 *   Nothing
 ********************************************************************/
void push_contact(macaddress randMac) {
  if (search(allContactsHead, randMac.key) == 0) {
    push(&allContactsHead, randMac);
    return;
  }
  else if (search(nearContactsHead, randMac.key) == 0) {
    time_t time_limit = randMac.timeCaptured - search(allContactsHead, randMac.key);
    fprintf(log_file, "Seen %d seconds before. Mac address: %" PRIu64 ", Time scanned: %d\n", (int) time_limit, randMac.macValue, (int) randMac.timeCaptured);
    fflush(log_file);
    if ((time_limit >= 4*60*SECOND) && (time_limit <= 20*60*SECOND)) {
      push(&nearContactsHead, randMac);
      return;
    }
    return;
  }
}

/*********************************************************************
 * Function: testCOVID
 * Description:
 *   This function simulates the result of a COVID test
 * Parameters:
 *   No parameters
 * Return:
 *   boolean of true or false depends from COVID test result
 ********************************************************************/
bool testCOVID() {
  if (rand() % 100 > 93.2)
    return 0;
  else
    return 1;
}

/*********************************************************************
 * Function: print_near_contacts
 * Description:
 *   This function prints into a file all macaddresses of near contacts
 * list 
 * Parameters:
 *   Nothing
 * Return:
 *   Nothing
 ********************************************************************/
void print_near_contacts() {
  node_t* head = nearContactsHead;
  timespec_get(&ts, TIME_UTC);
  strftime(buff2, sizeof(buff2), "%D %T", gmtime(&ts.tv_sec));
  fprintf(near_contacts_file, "%d Macaddreses were uploaded on %s.%09ld\n", getCount(nearContactsHead), buff2, ts.tv_nsec);
  fflush(near_contacts_file);
  while (head != NULL) {
    fprintf(near_contacts_file, "%" PRIu64 "\n",head->value.macValue);
    fflush(near_contacts_file);
    head = head->next;
  }
  fprintf(near_contacts_file, "\n");
  fflush(near_contacts_file);
  fprintf(log_file, "nearContacts.txt updated!\n");
  fflush(log_file);
}

/*********************************************************************
 * Function: msleep
 * Description:
 *   This function sleeps for msec given in miliseconds
 * Parameters:
 *   (long) msec = miliseconds to sleep
 * Return:
 *   (int) -1 for error, positive for success
 * Source: https://stackoverflow.com/a/1157217
 ********************************************************************/
int msleep(long msec) {
    struct timespec ts;
    int res;
    if (msec < 0){
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


/*********************** THREAD FUNCTIONS **************************/
/*********************************************************************
 * Function: search_and_push
 * Description:
 *   This function creates a MAC address every 10 seconds and pushes 
 * it into all contacts list or near contacts list
 * Parameters:
 *   Nothing
 * Return:
 *   Nothing
 ********************************************************************/
void* search_and_push() {
  while(1) {
    msleep(10*SECOND*1000);
    pthread_mutex_lock(&mutex);
    macaddress randMac = BTnearMe();
    push_contact(randMac);
    pthread_mutex_unlock(&mutex);
  }
}

/*********************************************************************
 * Function: update_all_contacts
 * Description:
 *   This function deletes a macaddress from all contacts list that 
 * has been inserted in 20 minutes ago.
 * Parameters:
 *   Nothing
 * Return:
 *   Nothing
 ********************************************************************/
void* update_all_contacts() {
  node_t* temp;
  time_t time_lim;
  int len;
  msleep(20*60*SECOND*1000);
  while(1) {
    msleep(60*SECOND*1000);
    pthread_mutex_lock(&mutex);
    temp = allContactsHead;
    len = getCount(allContactsHead);
    for (int i=0; i<len; i++) {
      time_lim = time(NULL) - temp->value.timeCaptured; 
      if (time_lim > (20*60*SECOND)) {      
        deleteNode(&allContactsHead, temp->value);
        temp = temp->next;
      }
      else
        temp = temp->next;
    } 
    pthread_mutex_unlock(&mutex);
  }
}

/*********************************************************************
 * Function: update_near_contacts
 * Description:
 *   This function deletes a macaddress from near contacts list that 
 * has been inserted in 14 days ago.
 * Parameters:
 *   Nothing
 * Return:
 *   Nothing
 ********************************************************************/
void* update_near_contacts() {
  node_t* temp;
  time_t time_lim;
  int len;
  msleep(14*24*60*60*SECOND*1000);
  while(1) {
    msleep(24*60*60*SECOND*1000);
    pthread_mutex_lock(&mutex);
    temp = nearContactsHead;
    len = getCount(nearContactsHead);
    for (int i=0; i<len; i++) {
      time_lim = time(NULL) - temp->value.timeCaptured;
      pthread_mutex_lock(&mutex);
      if (time_lim > (14*24*60*60*SECOND)) {
        deleteNode(&nearContactsHead, temp->value);
        temp = temp->next;
      }
      else
        temp = temp->next;
    } 
    pthread_mutex_unlock(&mutex);
  }
}

/*********************************************************************
 * Function: test_covid
 * Description:
 *   This function tests for COVID and if it's TRUE it is uploading  
 * the near contacts list
 * Parameters:
 *   Nothing
 * Return:
 *   Nothing
 ********************************************************************/
void* test_covid()  {
  while(1) {
    msleep(4*60*60*SECOND*1000);
    if (testCOVID()) {
      pthread_mutex_lock(&mutex);
      print_near_contacts();
      timespec_get(&ts, TIME_UTC);
      strftime(buff2, sizeof(buff2), "%D %T", gmtime(&ts.tv_sec));
      fprintf(log_file, "Upload %d near contacts after possitive COVID test\nLocal date & time = %s.%09ld\n", getCount(nearContactsHead), buff2, ts.tv_nsec);
      fflush(log_file);
      pthread_mutex_unlock(&mutex);
    }

  }
}

#endif /* __COVIDTRACE_H__ */
