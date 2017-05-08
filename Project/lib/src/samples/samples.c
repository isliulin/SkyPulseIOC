#include		<math.h>
#include		<stdlib.h>
#include		"samples.h"
#define M_PI	3.141592653589793
/*..............................................................*/
/*   Izracun determinante matrike reda n x n, p=element (0,0)	*/
/*..............................................................*/
double	det(double *p, int x,int y, int n)
{
	int		i,j,sign=1;
	double 	D=0;

	for(i=1,j=0;j<n;++j,i*= 2)
	{
		if(!(i & y))
		{
			if(x < n-1)
			{
				D  +=  p[n*j+x]*det(p, x+1, y+i, n)*sign;
				sign *= -1;
			}
			else
				D=p[n*j+x];
		}
	}
	return(D);
}
/*..............................................................*/
/* inicializacija vzorcev pointer p, stopnja n
*/
samples	*init_samples(samples *p, int n)
{
	int	i,j;
	if(!p)
	{
		p=malloc(sizeof(samples));
		p->tp=malloc(4*n*n *sizeof(double));
		p->fp=malloc(2*n*sizeof(double));
		p->rp=malloc(2*n*sizeof(double));
	}
	p->n = n;
	for(i=0; i<n; ++i)
	{
		p->fp[i]=0;
		for(j=0; j<n; ++j)
			p->tp[n*i + j]=0;
	}
	return(p);
}
/*..............................................................*/
samples *freesamples(samples *p)
{
	if(p)
	{
		free(p->tp);
		free(p->rp);
		free(p->fp);
		free(p);
	}
	return(NULL);
}
/*..............................................................*/
/* vpis vzorca f(t) v stack p
*/
void	add_sample(samples *p, double t, double f)
{
	int	i,j;
	for(i=0; i<p->n; ++i)
	{
		for(j=0; j<p->n; ++j)
			if(i+j)
				p->tp[i*(p->n)+j]  +=  pow(t,i+j);
			else
				p->tp[i*(p->n)+j]  +=  1.0;
		if(i)
			p->fp[i]  +=  f*pow(t,i);
		else
			p->fp[i]  +=  f;
	}
}
/*..............................................................*/
/* vpis periodicnega vzorca f(t) v stack p
*/
void	add_sample_t(samples *p, double t, double f, double per)
{
	int	i,j;
	double	*q = malloc(2*(p->n) * sizeof(double));

	p->per=per;
	for(i=0; i < (p->n)/2+1; ++i)
		q[i]=cos(2.0*M_PI*t/per*i);
	for(i=1; i < (p->n)/2+1; ++i)
		q[i+(p->n)/2]=sin(2.0*M_PI*t/per*i);
	for(i=0; i < p->n; ++i)
	{
		for(j=0; j < p->n ; ++j)
			p->tp[(p->n)*i+j]  +=  q[j]*q[i];
		p->fp[i]  +=  q[i]*f;
	}
	free(q);
}
/*..............................................................*/
/* vpis eksponencialnega ènega vzorca f(t) v stack p
*/
void	add_sample_e(samples *p, double t, double f)
{
	int	i,j;
	double	*q = malloc(2*(p->n) * sizeof(double));
	q[0]=t;
	for(i=1; i < p->n; ++i)
		q[i]=exp(pow(t,i));
	for(i=0; i < p->n; ++i)
	{
		for(j=0; j < p->n ; ++j)
			p->tp[(p->n)*i+j]  +=  q[i]*q[j];
		p->fp[i]  +=  q[i]*f;
	}
	free(q);
}
/*..............................................................*/
/* Resitev lin. enacbe reda n-1, p= matrika koef. reda n x n	*/
/* a[0] + a[1]*p + a[2]*p^2 + ... a[n-1]*p^n-1 = q				*/
/* r = vektor resitev											*/
/* ce je DET(p)  ==  0, ni resitve, funkcija vrne NULL			*/
/*..............................................................*/
double	*solve(samples *p)
{
	int		i,j;
	double	*c,d;
	d=det(p->tp,0,0,p->n);
	if(!d)
		return(NULL);

	c=malloc(p->n * p->n * sizeof(double));
	for(i=0; i<p->n; ++i)
	{
		for(j=0;j< p->n * p->n; ++j)
			if(j % p->n  ==  i)
				c[j]=p->fp[j / p->n];
			else
				c[j]=p->tp[j];
		p->rp[i]=det(c,0,0,p->n)/d;
	}
	free(c);
	return(p->rp);
}
/*..............................................................*/
/* Izracun polinoma a0 + a1*k + a2*k^2 + ...+ an*k^n			*/
/*..............................................................*/
double	polyp(samples *p, double t)
{
	int		i;
	double	ft;
	for(i=1, ft=p->rp[0]; i<p->n; ++i)
		ft  +=  p->rp[i]*pow(t,i);
	return(ft);
}
/*..............................................................*/
/* Izracun polinoma a0*k + a1*e^k + a2*e^(k^2) +...+ an*e^k(k^n)*/
/*..............................................................*/
double	polye(samples *p, double t)
{
	int		i;
	double	ft;
	for(i=1, ft=p->rp[0]*t; i<p->n; ++i)
		ft  +=  p->rp[i]*exp(pow(t,i));
	return(ft);
}
/*..............................................................*/
/* Izracun trig. polinoma										*/
/* a0 + a1*cos(k/per) + a2(cos(2*k/per) +...+					*/
/*      a(n/2)*sin(k/per) + a(n/2+1)(sin(2*k/per)				*/
/*..............................................................*/
double	polyt(samples *p, double t)
{
	int		i;
	double	ft;
	for(i=1, ft=p->rp[0]; i<p->n; ++i)
		ft  +=  p->rp[i]*cos(i*2.0*M_PI*t/p->per) + p->rp[i+(p->n)-1]*sin(i*2.0*M_PI*t/p->per);
	return(ft);
}


