//#include "stdafx.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

// s.235
// m=log2(n) - n=number of bytes to process
void fft(float *re, float *im, int m)
{
   int i=1;
   int n=2<<m;
   int nm1=n-1;
   int nd2=n>>1;
   int j=nd2;
   int k,l=1;
   int le;
   int le2;
   int jm1;
   int ip;

   float tr,ti;
   float ur,ui;
   float sr,si;

   for(;i<nm1;i++) {

      if(i>=j) goto dont_rotate;

      tr=re[j];
      ti=im[j];
      re[j]=re[i];
      im[j]=im[i];
      re[i]=tr;
      im[i]=ti;

dont_rotate:

      k=nd2;

loop:
      if(k>j) {
         j=j+k;
         continue;
      }
      j=j-k;
      k=k>>1;
      goto loop;
   }

   for(;l<m+2;l++) {
      le=2<<(l-1);
      le2=le>>1;
      ur=1;
      ui=0;
      sr=cos(M_PI/le2);
      si=-sin(M_PI/le2);

      for(j=1;j<le2+1;j++) {
         jm1=j-1;

         for(i=jm1;i<nm1+1;i+=le) {
            ip=i+le2;
            tr=re[ip]*ur-im[ip]*ui;
            ti=re[ip]*ui+im[ip]*ur;
            re[ip]=re[i]-tr;
            im[ip]=im[i]-ti;
            re[i]=re[i]+tr;
            im[i]=im[i]+ti;
         }

         tr=ur;
         ur=tr*sr-ui*si;
         ui=tr*si+ui*sr;
      }
   }
}

// s.236
// m=log2(n) - n=number of bytes to process
void inverse_fft(float *re, float *im, int m)
{
   int n=2<<m;
   float fn=(float)n;
   int i=0;
   for(;i<n;i++) im[i]=-im[i];

   fft(re,im,m);

   for(i=0;i<n;i++) {
      re[i]=re[i]/fn;
      im[i]=-im[i]/fn;
   }
}

// s.242
// m=log2(n) - n=number of bytes to process
void real_fft(float *re, float *im, int m)
{
   int n=2<<m;
   int i=0;
   int nm1=n-1;
   int nd2=n>>1;
   int n4=(n>>2)-1;

   int nh=n/2-1;

   int l;
   int le;
   int le2;
   float fle2;
   float ur;
   float ui;
   float sr;
   float si;

   int j,jm1,ip;

   float tr,ti;

   for(;i<nh+1;i++) {
      re[i]=re[2*i];
      im[i]=re[2*i+1];
   }

   m--;
   fft(re,im,m);
   m++;

   for(i=1;i<n4;i++) {
      int im2=nd2-i;
      int ip2=i+nd2;
      int ipm=im2+nd2;

      re[ip2]=(im[i]+im[im2])/2.f;
      re[ipm]=re[ip2];
      im[ip2]=-(re[i]-re[im2])/2.f;
      im[ipm]=-im[ip2];
      re[i]=(re[i]+re[im2])/2.f;
      re[im2]=re[i];
      im[i]=(im[i]-im[im2])/2.f;
      im[im2]=-im[i];
   }
   re[n*3/4]=im[n/4];
   re[nd2]=im[0];
   im[n*3/4]=0;
   im[nd2]=0;
   im[n/4]=0;
   im[0]=0;

   l=m+1;
   le=2<<(l-1);
   le2=le/2;
   fle2=le2;
   ur=1;
   ui=0;
   sr=cos(M_PI/fle2);
   si=-sin(M_PI/fle2);

   for(j=1;j<le2;j++) {
      jm1=j-1;
      for(i=jm1;i<nm1+1;i+=le) {
         ip=i+le2;
         tr=re[ip]*ur-im[ip]*ui;
         ti=re[ip]*ui+im[ip]*ur;
         re[ip]=re[i]-tr;
         im[ip]=im[i]-ti;
         re[i]=re[i]+tr;
         im[i]=im[i]+ti;
      }
      tr=ur;
      ur=tr*sr-ui*si;
      ui=tr*si+ui*sr;
   }
}


// s.239
// m=log2(n) - n=number of bytes to process
void inverse_real_fft(float *re, float *im, int m)
{
   int n=2<<m;
   int k;
   float fn=n;

   for(k=(n>>1)+1;k<n;k++) {
      re[k]=re[n-k];
      im[k]=-im[n-k];
   }

   for(k=0;k<n;k++) {
      re[k]=re[k]+im[k];
   }

   real_fft(re,im,m);

   for(k=0;k<n;k++) {
      re[k]=(re[k]+im[k])/fn;
      im[k]=0;
   }
}
