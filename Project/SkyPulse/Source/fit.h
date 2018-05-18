#ifndef					FIT_H
#define					FIT_H

enum _fittype		{FIT_POW, FIT_TRIG, FIT_EXP, FIT_NEXP};

class _FIT {
	private:
		float	*tp,*fp,per;
		float	det(float *,int,int,int);
	public:
		int			n,typ;
		float	*rp;			
		_FIT(int = 3, _fittype=FIT_POW);
		_FIT(const _FIT &);
		~_FIT();
		
		int			Sample(float, float);
		int			Sample(float, float, float);
		float	*Compute(void);
		float	Eval(float);
	
	};
#endif
