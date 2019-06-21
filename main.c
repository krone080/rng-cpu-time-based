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
#include <unistd.h>
#include <math.h>
#include <string.h>

double get_rand(const unsigned lcnt, unsigned *freq);

double get_rand_devi(const double meanX, const unsigned lcnt,unsigned *freq)
 {return get_rand(lcnt,freq)-meanX;}

double get_rand_devi_sum(const double meanX, const unsigned lcnt, const unsigned n);

void distribution_init(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n);

void distribution_init_test(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n, char *fname);

void middleman_generating();

//не доделана
void freq_splitting_mode();

double normal_cdf(double x, double E, double D)
 {return 0.5*(1.+erf((x-E)/sqrt(2.*D)));}

void get_bit_sequence(double num, char* seq, unsigned split_len);

pid_t pfd_to[2],pfd_from[2];

int main(int argc,char* argv[])
 {
 middleman_generating();

 char fname[20],choice;
 unsigned cnt,n,lcnt,left_bound=0,right_bound=0,split_num=1;
 FILE *ffile;

 if(argc==6)
  {
  strcpy(fname,argv[1]);
  cnt=atoi(argv[2]);
  n=atoi(argv[3]);
  lcnt=atoi(argv[4]);
  }
 else
  {
  printf("Mode (Usual or Freq_splitting) (U/F): ");
  scanf("%c",&choice);
  printf("Input file name: ");
  scanf("%s",fname);
  printf("Sample length for init: ");
  scanf("%i",&cnt);
  printf("Sum length: ");
  scanf("%i",&n);
  printf("Load counter: ");
  scanf("%i",&lcnt);

  if(choice=='F')
   {
   printf("\nLeft bound: ");
   scanf("%i",&left_bound);
   printf("Right bound: ");
   scanf("%i",&right_bound);
   printf("Spliting num: ");
   scanf("%i",&split_num);
   }
  }

 double varS;
 double meanX,meanS;
 switch(choice)
  {
  case 'U':
   distribution_init_test(&meanX,&meanS,&varS,cnt,lcnt,n,fname);
   break;
  case 'F':
   freq_splitting_mode();
   break;
  default:
   break;
  }


 return 0;
 }

double get_rand(const unsigned lcnt, unsigned *freq)
 {
 struct rusage info;
 ssize_t a;
 a=write(pfd_to[1],&lcnt,sizeof(lcnt));
 a=read(pfd_from[0],&info,sizeof(info));

 unsigned freq_cpu[4],cpu_core;
 FILE *ftmp;

 ftmp=fopen("tmp","r");
 *freq=0;
 for(unsigned i=0;i<lcnt;++i)
  {
  fscanf(ftmp,"%i|%i|%i|%i|  %i",&freq_cpu[0],&freq_cpu[1],&freq_cpu[2],&freq_cpu[3],&cpu_core);
  *freq+=freq_cpu[cpu_core];
  }
 close(3);
 system("> tmp");
 return info.ru_stime.tv_sec+info.ru_utime.tv_sec+(info.ru_stime.tv_usec+info.ru_utime.tv_usec)*0.000001;
 }

double get_rand_devi_sum(const double meanX, const unsigned lcnt, const unsigned n)
 {
 double sum=0.;
 unsigned freq;
 for(unsigned i=0;i<n;++i)
  sum+=get_rand_devi(meanX,lcnt,&freq);
 return sum;
 }

void distribution_init(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n)
 {
 double sample[cnt],_meanX=0.,_meanS=0.,_varS=0.;
 unsigned freq; //временно так

 for(unsigned k=0;k<cnt;++k)
  {
  sample[k]=get_rand(lcnt,&freq);
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

void distribution_init_test(double *meanX, double *meanS, double *varS, const unsigned cnt,
                      const unsigned lcnt, const unsigned n, char *fname)
 {
 double _meanX=0,_meanS=0,tmp=0;
 double _varS=0.;
 const unsigned m=cnt/n;
 double S[m],X[cnt],rnd;
 unsigned freq[cnt];
 system("> tmp");
 for(unsigned i=0;i<m;++i)
  S[i]=0;
 for(unsigned i=0;i<m;++i)
  {
  for(unsigned j=0;j<n;++j)
   {
   rnd=get_rand(lcnt,&freq[i*n+j]);
   X[i*n+j]=rnd;
   S[i]+=rnd;
   }
  tmp+=S[i];
  printf("\r%i/%i",i,m);
  fflush(stdout);
  }

 _meanX=tmp/cnt;

 for(unsigned k=0;k<m;++k)
  S[k]-=_meanX*n;

//Небольшая погрешность
 _meanS=tmp/m-_meanX*n;

//вычисление дисперсии
 for(unsigned i=0;i<m;++i)
  _varS+=pow(S[i]-_meanS,2);
 _varS/=m;

//Вывод сумм S
 creat(fname,0664);
 FILE *ffile=fopen(fname,"w");
 for(unsigned i=0;i<m;++i)
  {
  fprintf(ffile,"%f\n",S[i]); //normal_cdf(S[i],_meanS,_varS) - как аргумент
  fflush(ffile);
  }

//Вывод значений X
 creat("X",0664);
 FILE* fXfile=fopen("X","w");
 for(unsigned i=0;i<cnt;++i)
  {
  fprintf(fXfile,"%f\n",X[i]);
  fflush(fXfile);
  }
//Вывод значений Xfreq
 creat("Xfreq",0664);
 FILE* fXfreqfile=fopen("Xfreq","w");
 for(unsigned i=0;i<cnt;++i)
  {
  fprintf(fXfreqfile,"%i\n",freq[i]);
  fflush(fXfreqfile);
  }

 *meanX=_meanX;
 *meanS=_meanS;
 *varS=_varS;
 }

void middleman_generating()
 {
 pipe(pfd_to);
 pipe(pfd_from);

//Процесс <Посредник>
 if(fork()==0)
  {
  close(pfd_to[1]);
  close(pfd_from[0]);

  unsigned lcnt;
  while((read(pfd_to[0],&lcnt,sizeof(lcnt))>0)&&!(lcnt==0))
   {
   void* stat=NULL;
   struct rusage info;
   char s[150];
   memset(s,0,150);

//Генерируемый процесс <Задание>
   if(fork()==0)
    {
    sprintf(s,"cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq | tr  '\n' '|' >> tmp\n"
              "ps --no-headers -o psr -p %i >> tmp",getpid());
    for(unsigned i=0;i<lcnt;++i)
     {
     system(s);
     }
    exit(1);
    }
   wait3(stat,0,&info);
   write(pfd_from[1],&info,sizeof(info));
   }
  exit(1);
  }
 close(pfd_to[0]);
 close(pfd_from[1]);
 }

void freq_splitting_mode(const unsigned cnt,const unsigned lcnt, const unsigned n, char *fname,
                         const unsigned left_bound,const unsigned right_bound, const unsigned split_num)
 {
 double _meanX[split_num],_meanS[split_num],_varS[split_num];
 const unsigned m=cnt/n;
 double S[split_num],X[cnt],rnd;
 unsigned Xfill[split_num],Sfill[split_num],freq;
 system("> tmp");

 for(unsigned i=0;i<split_num;++i)
  {
  S[i]=0;
  Xfill[i]=0;
  _meanX[i]=0;
  _meanS[i]=0;
  _varS[i]=0;
  }

 //начальное заполнение
 unsigned i;
 while(1)
  {
  rnd=get_rand(lcnt,&freq);
  i=split_num-1;
 //позже можно сделать дерево бинарного поиска
  for(i;i>=0;++i)
   {
   if((freq>=left_bound+i*(left_bound+right_bound)/split_num)&&
      (freq<left_bound+(i+1)*(left_bound+right_bound)/split_num))
    {
    _meanX[i]+=rnd;
    ++Xfill[split_num];
    break;
    }
   }
  if(Xfill[i]==n)
   break;
  }
 _meanX[i]/=n;

 while(1)
  {
  rnd=get_rand(lcnt,&freq);
  i=split_num-1;
//позже можно сделать дерево бинарного поиска
  for(i;i>=0;++i)
   {
   if((freq>=left_bound+i*(left_bound+right_bound)/split_num)&&
      (freq<left_bound+(i+1)*(left_bound+right_bound)/split_num))
    {
//  n можно будет потом заменить
    if(Xfill[i]<n)
     {
     _meanX[i]+=rnd;
     ++Xfill[i];
     break;
     }
    if(Sfill[i]<n)
     {
     S[i]+=rnd;
     ++Xfill[i];
     break;
      }
    }
   }
  }
 }

void get_bit_sequence(double num, char* seq, unsigned split_len)
 {
 memset(seq,0,split_len+1);
 unsigned shift=2;
 double position=0.5;
 for(unsigned i=0;i<split_len;i++)
  {
  if(num<position)
   {
   seq[i]='0';
   position-=1/shift;
   }
  else
   {
   seq[i]='1';
   position+=1/shift;
   }
  shift<<=1;
  }
 }
