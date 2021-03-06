#include<stdlib.h>
#include"numericalanalysis.h"

bool ZTChasingMethod(double *a, double *b, double *c, double *f, double *x, int num)
{
  int i;
  double *beta = (double *)malloc(sizeof(double)*num);
  double *y    = (double *)malloc(sizeof(double)*num);

  beta[0] = c[0]/b[0];
  for(i=1; i<num-1; i++)
    {
      beta[i] = c[i]/(b[i]-a[i]*beta[i-1]);
    }

  y[0] = f[0]/b[0];
  for(i=1; i<num; i++)
    {
      y[i] = (f[i]-a[i]*y[i-1])/(b[i]-a[i]*beta[i-1]);
    }

  x[num-1] = y[num-1];
  for(i=num-2; i>=0; i--)
    {
      x[i] = y[i] - beta[i]*x[i+1];
    }

  free(beta);
  free(y);

  return true;
}

bool FanShengjin(double a,double b,double c,double d,double *X)
{
  double A,B,C,delta,Y1,Y2,T,theta,mid;
  int i,j;
  A=b*b-3*a*c;
  B=b*c-9*a*d;
  C=c*c-3*b*d;
  delta=B*B-4*A*C;
  if(A==0&&B==0)
    {
      X[0]=-b/3.0/a;
      X[3]=1;
    }
  else if (delta>0)
    {
      Y1=A*b+3*a*(-B+sqrt(delta))/2.0;
      Y2=A*b+3*a*(-B-sqrt(delta))/2.0;
      X[0]=(-b-fabs(Y1)/Y1*pow(fabs(Y1),1.0/3.0)-fabs(Y2)/Y2*pow(fabs(Y2),1.0/3.0))/3.0/a;
      X[3]=1;
    }
  else if(delta==0)
    {
      X[0]=-b/a+B/A;
      X[1]=-B/A/2.0;
      if(X[0]<X[1])
	{
	  mid=X[0];
	  X[0]=X[1];
	  X[1]=mid;
	}
      X[3]=2;
    }
  else if(delta<0)
    {
      T=(2*A*b-3*a*B)/2.0/pow(A,1.5);
      theta=acos(T);
      X[0]=(-b-2*sqrt(A)*cos(theta/3.0))/3.0/a;
      X[1]=(-b+sqrt(A)*(cos(theta/3.0)+sqrt(3.0)*sin(theta/3.0)))/3.0/a;
      X[2]=(-b+sqrt(A)*(cos(theta/3.0)-sqrt(3.0)*sin(theta/3.0)))/3.0/a;
      for(i=0;i<3;i++)
	{
	  for(j=0;j<3;j++)
	    {
	      if(X[j]<X[j+1])
		{
		  mid=X[j];
		  X[j]=X[j+1];
		  X[j+1]=mid;
		}
	    }
	}
      X[3]=3;
    }
}

double length(double a,double b,double c,double d)
{
  double ch;
  ch=sqrt((a-b)*(a-b)+(c-d)*(c-d));
  return ch;
}
