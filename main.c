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
#include <unistd.h>
#include <math.h>

//Получение параметров нормального распределения в текущий момент
//времени
double get_rand(const unsigned lcnt);

double get_rand_devi(const double meanX, const unsigned lcnt)
 {return get_rand(lcnt)-meanX;}

double get_rand_devi_sum(const double meanX, const unsigned lcnt, const unsigned n);

int distribution_init(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n);

int distribution_init_test(long long *meanX, long long *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n);
/*
 *
 *
 *
 *
 *
 *
 *
 */
int main()
 {
//1. трансформация процесса в демона

//2. fork, создание особых потомков для генерации различных
//   независимых выборок

 pid_t pid;
 struct timespec comt1,comt2;
 int fd;
 char fname[20];
 unsigned cnt,n,lcnt,cnt2;
//PAPI
 const int num_hwcntrs=PAPI_num_counters();
 int events[11];
 float rtime,ptime,ipc;
 long_long ins;

 printf("The number of available hardware counters: %i\nInput file name: ",num_hwcntrs);
 scanf("%s",fname);
 printf("Sample length for init: ");
 scanf("%i",&cnt);
 printf("Sum length: ");
 scanf("%i",&n);
 printf("Load counter: ");
 scanf("%i",&lcnt);
 printf("Gen sample length: ");
 scanf("%i",&cnt2);
 close(1);
 fd=creat(fname,0664);
 if(fd<0)
  {
  perror("creat()");
  return -1;
  }

// Параметры цикла (счётчик) будут, скорее всего, изменяемыми и будут
// зависеть от оценок, посчитанных заранее

 double varS;
 long long meanX,meanS;
 distribution_init_test(&meanX,&meanS,&varS,cnt,lcnt,n);

 printf("\nES=%lli, DS=%e",meanS,varS);
// for(unsigned i=0;i<cnt2;++i)
//  {
//  printf("%.9f ",get_rand_devi_sum(meanX,lcnt,n));
//  fflush(stdout);
//  }
//4. преобразование

//5. предоставление числа/последовательности на выход

 return 0;
 }
/*
 *
 *
 *
 *
 *
 *
 *
 *
 */
double get_rand(const unsigned lcnt)
 {
 void* stat=NULL;
 struct rusage info;
 struct timespec t1,t2;
 int pfd[2],pid;
 long sdel,nsdel;
 char msg[12];

 pipe(pfd);
 pid=fork();
 if(pid==0)
  {
  close(3);

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t1);
  //... какая-нибудь нагрузка
  for(unsigned i=0;i<=lcnt;)
   i++;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t2);
  sdel=t2.tv_sec-t1.tv_sec;
  if(t2.tv_nsec-t1.tv_nsec<0)
   nsdel=1000000000+t2.tv_nsec-t1.tv_nsec;
  else
   nsdel=t2.tv_nsec-t1.tv_nsec;

  close(1);
  dup(4);
  printf("%1li.%.9li",sdel,nsdel);
  exit(1);
  }
 close(4);

 wait3(stat,0,&info);
 read(3,msg,11);
 msg[11]=0;
 close(3);
 return atof(msg);
 }

double get_rand_devi_sum(const double meanX, const unsigned lcnt, const unsigned n)
 {
 double sum=0.;
 for(unsigned i=0;i<n;++i)
  sum+=get_rand_devi(meanX,lcnt);
 return sum;
 }

int distribution_init(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n)
 {
 double sample[cnt],_meanX=0.,_meanS=0.,_varS;

 for(unsigned k=0;k<cnt;++k)
  {
  sample[k]=get_rand(lcnt);
  _meanX+=sample[k];
  }
 _meanX/=cnt;
 for(unsigned k=0;k<cnt;++k)
  sample[k]-=_meanX;

 const unsigned m=cnt/n;
 double S[m];
 for(unsigned i=0;i<m;++i)
  S[i]=0.;

//вычисление мат. ожидания
 for(unsigned i=0;i<m;++i)
  {
  for(unsigned j=0;j<n;++j)
   S[i]+=sample[i*n+j];
  _meanS+=S[i];
  }
 _meanS/=m;

//вычисление дисперсии
 for(unsigned i=0;i<m;++i)
  {
  S[i]=pow(S[i]-_meanS,2.);
  _varS+=S[i];
  }
 _varS/=m;

 *meanX=_meanX;
 *meanS=_meanS;
 *varS=_varS;
 }

int distribution_init_test(long long *meanX, long long *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n)
 {
 long long _meanX=0,_meanS=0,tmp=0;
 double _varS;
 const unsigned m=cnt/n;
 long S[m];
 for(unsigned i=0;i<m;++i)
  S[i]=0;
 for(unsigned i=0;i<m;++i)
  {
  for(unsigned j=0;j<n;++j)
//   S[i]+=get_rand(lcnt)*1000000000;
   {
   S[0]=get_rand(lcnt)*1000000000;
   printf("%li\n",S[0]);
   fflush(stdout);
   }
//  tmp+=S[i];
  fprintf(stderr,"\r%i/%i",i,m);
  }
//ЗАВЕРШАЕМ ПРОГРАММУ
 exit(0);
 _meanX=(long long)(tmp/cnt);

 for(unsigned k=0;k<m;++k)
  S[k]-=_meanX*n;

 //Небольшая погрешность(хотя кто знает) в обмен на производительность
 _meanS=((long long)(tmp/m))-_meanX*n;

 //Используем уже выработанные данные
 for(unsigned i=0;i<m;++i)
  {
  printf("%li ",S[i]);
  fflush(stdout);
  }

//вычисление дисперсии
 for(unsigned i=0;i<m;++i)
  _varS+=pow(S[i]-_meanS,2);
 _varS/=m;

 *meanX=_meanX;
 *meanS=_meanS;
 *varS=_varS;

 }
