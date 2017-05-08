typedef struct 	samples	
{
		double	*tp,*fp,*rp,per;
		int		n;
} samples;
				
int		n_samples(samples *);
samples *freesamples(samples *),
		*init_samples(samples *,int);
void	add_sample(samples *,double,double),
		add_sample_t(samples *,double,double,double),
		add_sample_e(samples *,double,double);

double	*solve(samples *);
double	polyp(samples *,double),
		polye(samples *,double),
		polyt(samples *,double);
