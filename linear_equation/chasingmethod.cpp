#include"cahsingmethod.cpp"
#include<stdlib>

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
