#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <papi.h>


int main()
 {
//1. трансформация процесса в демона

//2. fork, создание особых потомков для генерации различных
//   независимых выборок

 pid_t pid;
 void* stat=NULL;
 struct rusage info;
 struct timespec t1,t2,comt1,comt2;
 int fd;
 char fname[20];
 long sdel,nsdel;
 unsigned cnt, lcnt;
//PAPI
 const int num_hwcntrs=PAPI_num_counters();
 int events[11];
 float rtime,ptime,ipc;
 long_long ins;

 PAPI_multiplex_init();
 printf("The number of available hardware counters: %i\nInput file name: ",num_hwcntrs);
 scanf("%s",fname);
 printf("And counter: ");
 scanf("%i",&cnt);
 printf("And load counter: ");
 scanf("%i",&lcnt);
 close(1);
 fd=creat(fname,0664);
 if(fd<0)
  {
  perror("creat()");
  return -1;
  }

 printf("Total inst;inst/s;CPUget_s;CPUget_ns;CPUwait_s"
        ";CPUwait_mcs;REALget_s;REALget_ns\n");
// Параметры цикла (счётчик) будут, скорее всего, изменяемыми и будут
// зависеть от оценок, посчитанных заранее
 int err;
 err=PAPI_library_init(PAPI_VER_CURRENT);
 if (err!=PAPI_VER_CURRENT)
  {
  printf("PAPI library init error!\n");
  exit(1);
  }
 fprintf(stderr,"PAPI_library_init(): %s\n",PAPI_strerror(err));
 int EventSet=PAPI_NULL;
 err=PAPI_create_eventset(&EventSet);
 fprintf(stderr,"PAPI_create_eventset(): %s\n",PAPI_strerror(err));
 int EventCodes[2]={
  PAPI_TOT_INS,
  PAPI_TOT_CYC
};
 err=PAPI_add_events(EventSet,EventCodes,2);
 fprintf(stderr,"PAPI_add_events(): %s\n",PAPI_strerror(err));
// err=PAPI_start(EventSet);
// fprintf(stderr,"PAPI_start(): %s\n",PAPI_strerror(err));
// PAPI_ipc(&rtime,&ptime,&ins,&ipc);
// clock_gettime(CLOCK_REALTIME,&comt1);
// PAPI_ipc(&rtime,&ptime,&ins,&ipc);
 long_long values[6];
// err=PAPI_stop(EventSet,values);
// fprintf(stderr,"PAPI_stop(): %s\n",PAPI_strerror(err));
 for(unsigned k=0;k<cnt;++k)
  {
//  pid=fork();
  pid=0;
  if(pid==0)
   {
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t1);
//   err=PAPI_ipc(&rtime,&ptime,&ins,&ipc);
//   fprintf(stderr,"PAPI_ipc(): %s\n",PAPI_strerror(err));
   err=PAPI_start(EventSet);
   fprintf(stderr,"PAPI_start(): %s\n",PAPI_strerror(err));
   //... какая-нибудь нагрузка
   for(unsigned i=0;i<=lcnt;)
    i++;
   err=PAPI_stop(EventSet,values);
   fprintf(stderr,"PAPI_stop(): %s\n",PAPI_strerror(err));
//   err=PAPI_ipc(&rtime,&ptime,&ins,&ipc);
//   fprintf(stderr,"PAPI_ipc(): %s\n",PAPI_strerror(err));
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t2);
   sdel=t2.tv_sec-t1.tv_sec;
   if(t2.tv_nsec-t1.tv_nsec<0)
    nsdel=1000000000+t2.tv_nsec-t1.tv_nsec;
   else
    nsdel=t2.tv_nsec-t1.tv_nsec;

   printf("%lli;%lli;%f;%f;%li;%li;",values[0],values[1],/*ptime,rtime,*/sdel,nsdel);
//   exit(1);
   }

//3. wait, получение времени (входного значения)
// СРАВНЕНИЕ ТОЧНОСТИ И ОБЛАСТИ ЗНАЧЕНИЙ ТАЙМЕРОВ

  clock_gettime(CLOCK_REALTIME,&t1);
//  wait3(stat,0,&info);
  clock_gettime(CLOCK_REALTIME,&t2);

  sdel=t2.tv_sec-t1.tv_sec;
  if(t2.tv_nsec-t1.tv_nsec<0)
   nsdel=1000000000+t2.tv_nsec-t1.tv_nsec;
  else
   nsdel=t2.tv_nsec-t1.tv_nsec;
//         s  mcs  s   ns
  printf("%li;%li;%li;%li\n",
         info.ru_utime.tv_sec+info.ru_stime.tv_sec,
         info.ru_utime.tv_usec+info.ru_stime.tv_usec,
         sdel,nsdel);
//  sleep(1);
  }
 clock_gettime(CLOCK_REALTIME,&comt2);

 long csdel=comt2.tv_sec-comt1.tv_sec,cnsdel;
 if(comt2.tv_nsec-comt1.tv_nsec<0)
  cnsdel=1000000000+comt2.tv_nsec-comt1.tv_nsec;
 else
  cnsdel=comt2.tv_nsec-comt1.tv_nsec;
 printf("\n%li %li",csdel,cnsdel);
//4. преобразование

//5. предоставление числа/последовательности на выход

 return 0;
 }
