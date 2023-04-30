#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  i32 start_time;
  u32 end_time;
  u32 time_run; 
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);
  u32 quantum_length = next_int_from_c_str(argv[2]);
  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */



  //list is the head
  //data is an array of all the processes to be added
  u32 time = 0; 
  u32 tasks_finished = 0;

  /*
  assuming that data is orderd and the first task will start at time one
  */

  /*we set the start time and response time to -1 for all to check if they have started or not*/
  for(int i = 0; i < size; i++){
    data[i].start_time = -1;
    data[i].time_run = 0;
  }
  
  /*add first process*/  
  TAILQ_INSERT_TAIL(&list, &data[0], pointers);
  data[0].start_time = 0;

  /*while loop to act out rr*/
  struct process *curr_proc;
  while (tasks_finished != size){

    /*run curr_proc for quantum_length quantums or until finish*/
    curr_proc = TAILQ_FIRST(&list);
    for(int i = 0; i < quantum_length; i++){
      time++;

      
      /*look for tasks to add to the queue*/
      for(int i = 0; i < size + 1; i++){
        if(data[i].start_time == -1 && data[i].arrival_time <= time){
          TAILQ_INSERT_TAIL(&list, &data[i], pointers);
          data[i].start_time = -2;
          // printf("added proc %d, arrived, %d tasks added %d\n", data[i].pid,data[i].arrival_time, tasks_finished);
        }
      }

      
      /*if there is no process to run but stuff still to do move forward in time*/
      if(curr_proc == NULL){
        break;
      }

      
      /*if this is the first time its run, modify start time*/
      if(curr_proc->start_time == -2){
        curr_proc->start_time = time-1;
        // printf("pid: %d, %d, %d\n",curr_proc->pid, curr_proc->start_time, curr_proc->arrival_time);
      } 
      // printf("%d: %d, time run%d\n",time, curr_proc->pid, curr_proc->burst_time);
      

      /*check if it has run to completion, if so break out of loop*/
      // printf("pid: %d --%d, %d\n",curr_proc->pid, (curr_proc->time_run+1), curr_proc->burst_time);
      if(++curr_proc->time_run == curr_proc->burst_time){
        curr_proc->end_time = time;
        tasks_finished ++; 
        // printf("break timerun = bursttime\n");
        break;
      } 
    }

    /*now we move the just finished process to end of list or remove all together*/
    TAILQ_REMOVE(&list, curr_proc, pointers);

    if(curr_proc->time_run != curr_proc->burst_time){
      // printf("readded %d\n",curr_proc->pid);
      TAILQ_INSERT_TAIL(&list, curr_proc, pointers);
    }

    curr_proc = NULL;
  }
  
  // printf("out of while\n");

  /* add up the wait times*/
  for(int i = 0; i < size; i++){
    // int r = data[i].end_time-data[i].arrival_time - data[i].burst_time;
    // int w = data[i].start_time - data[i].arrival_time;
    // printf("%d,  %d,  %d     ",  data[i].end_time,data[i].arrival_time , data[i].burst_time);
    // printf("%d,%d\n",r,w);
    total_waiting_time += (data[i].end_time-data[i].arrival_time - data[i].burst_time);
    total_response_time += (data[i].start_time - data[i].arrival_time);
  }


  // if(TAILQ_EMPTY(&list))

  // TAILQ_INSERT_TAIL(&list, &data[0], pointers);

  // struct process *iproc;
  //  TAILQ_FOREACH(iproc, &list, pointers)
  //    printf("start %d, pid %d\n",iproc->start_time, iproc->pid);
  // free(iproc);



  /* End of "Your code here" */



  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
