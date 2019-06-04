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

int distribution_init_test(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n);

double normal_cdf(double x, double E, double D)
 {return 0.5*(1.+erf((x-E)/sqrt(2.*D)));}

/*
 *
 *
 *
 *
 *
 *
 *
 */
int main(int argc,char* argv[])
 {
//1. трансформация процесса в демона

//2. fork, создание особых потомков для генерации различных
//   независимых выборок

 pid_t pid;
 struct timespec comt1,comt2;
 int fd;
 char fname[20];
 unsigned cnt,n,lcnt,cnt2;
 float rtime,ptime,ipc;
 long_long ins;

 if(argc==6)
  {
  strcpy(fname,argv[1]);
  cnt=atoi(argv[2]);
  n=atoi(argv[3]);
  lcnt=atoi(argv[4]);
  cnt2=atoi(argv[5]);
  }
 else
  {
  printf("Input file name: ");
  scanf("%s",fname);
  printf("Sample length for init: ");
  scanf("%i",&cnt);
  printf("Sum length: ");
  scanf("%i",&n);
  printf("Load counter: ");
  scanf("%i",&lcnt);
  printf("Gen sample length: ");
  scanf("%i",&cnt2);
  }
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
 double meanX,meanS;
 distribution_init_test(&meanX,&meanS,&varS,cnt,lcnt,n);

// printf("\nES=%lli, DS=%e",meanS,varS);
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

// pipe(pfd);
 pid=fork();
 if(pid==0)
  {
  close(3);

//  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t1);
  //... какая-нибудь нагрузка
  for(unsigned i=0;i<=lcnt;)
   i++;
//  clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&t2);
//  sdel=t2.tv_sec-t1.tv_sec;
//  if(t2.tv_nsec-t1.tv_nsec<0)
//   nsdel=1000000000+t2.tv_nsec-t1.tv_nsec;
//  else
//   nsdel=t2.tv_nsec-t1.tv_nsec;

//  close(1);
//  dup(4);
//  printf("%1li.%.9li",sdel,nsdel);
  exit(1);
  }
// close(4);

 wait3(stat,0,&info);
// read(3,msg,11);
// msg[11]=0;
// close(3);
 return info.ru_stime.tv_sec+info.ru_utime.tv_sec+(info.ru_stime.tv_usec+info.ru_utime.tv_usec)*0.000001;
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

int distribution_init_test(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n)
 {
 double _meanX=0,_meanS=0,tmp=0;
 double _varS;
 const unsigned m=cnt/n;
 double S[m],X[cnt],rnd;
 for(unsigned i=0;i<m;++i)
  S[i]=0;
 for(unsigned i=0;i<m;++i)
  {
  for(unsigned j=0;j<n;++j)
   {
//   kek:
   rnd=get_rand(lcnt);
   //МОЙ ФИЛЬТР
   //Квантиль 0.999 - 13950; 0.99 - 1735
//   if(rnd==0||rnd>13950)
//    goto kek;
   X[i*n+j]=rnd;
   S[i]+=rnd;
   }
  tmp+=S[i];
  fprintf(stderr,"\r%i/%i",i,m);
  }

 _meanX=tmp/cnt;

 for(unsigned k=0;k<m;++k)
  S[k]-=_meanX*n;

 //Небольшая погрешность(хотя кто знает) в обмен на производительность
 _meanS=tmp/m-_meanX*n;

//вычисление дисперсии
 for(unsigned i=0;i<m;++i)
  _varS+=pow(S[i]-_meanS,2);
 _varS/=m;

//Вывод сумм S
 for(unsigned i=0;i<m;++i)
  {
  printf("%f\n",S[i]); //normal_cdf(S[i],_meanS,_varS) - как аргумент
  fflush(stdout);
  }

//Вывод значений X
 close(1);
 creat("X",0664);
 for(unsigned i=0;i<cnt;++i)
  {
  printf("%f\n",X[i]);
  fflush(stdout);
  }

 *meanX=_meanX;
 *meanS=_meanS;
 *varS=_varS;

 }
