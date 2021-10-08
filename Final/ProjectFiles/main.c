/*********************************************************************
 * File: main.c
 * 
 * Author: Alexandros Petridis
 * Date: September 2021
 *
 * Description: main file
 ********************************************************************/
#include "covidTrace.h"

int main(void) {
  srand(time(NULL));

  double CPU_time = 0.0;
  clock_t begin, end;
  time_t begin_time, end_time;
  time_t total_time = 0;
  begin_time = time(NULL);
  begin = clock();
  
  near_contacts_file = fopen("nearContacts.txt", "w");
  if (!near_contacts_file) {
    printf("Unable to open file nearContacts.bin\n");
    return 1;
  }
  log_file = fopen("log_file.txt", "w");
  if (!log_file) {
    printf("Unable to open file log_file.bin\n");
    return 2;
  }
//  BTnearMe_call = fopen("BTnearme_call.bin", "wb");
  BTnearMe_call = fopen("BTnearme_call.csv", "w");
  if (!BTnearMe_call) {
    printf("Unable to open file BTnearme_call.bin\n");
    return 3;
  }

  pthread_t search_and_push_t, update_all_contacts_t, update_near_contacts_t, test_covid_t;

  // Thread for new search and push into lists
  if (pthread_create(&search_and_push_t, NULL, &search_and_push, NULL)) {
    printf("No new search and push recorded!\n");
    return 4;
  }
  // Thread for updating list of all contacts
  if (pthread_create(&update_all_contacts_t, NULL, &update_all_contacts, NULL)) {
    printf("All contacts list didn't update!\n");
    return 5;
  }
  // Thread for updating list of near contacts
  if (pthread_create(&update_near_contacts_t, NULL, &update_near_contacts, NULL)) {
    printf("Near contacts list didn't update!\n");
    return 6;
  }
  // Thread for uploading list of near contacts in case of positive COVID test
  if (pthread_create(&test_covid_t, NULL, &test_covid, NULL)) {
    printf("Covid test wasn't made!\n");
    return 7;
  }
  
  msleep(30*24*60*60*SECOND*1000);
  
  end = clock();
  end_time = time(NULL);
  CPU_time += (double)(end-begin) / CLOCKS_PER_SEC;
  total_time += (end_time - begin_time);
  printf("CPU time was %f seconds\n", CPU_time);
  printf("Total time was %d seconds\n", (int) total_time);
  printf("CPU was inactive for %f %%\n", 100*((int) total_time - CPU_time)/((int) total_time)); 

  fclose(near_contacts_file);
  fclose(log_file);
  fclose(BTnearMe_call);

  return 0;
}
