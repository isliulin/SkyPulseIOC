#ifndef					PYRO_H
#define					PYRO_H

#include				"stm32f2xx.h"
#include				<stdio.h>
#include 				<stdlib.h>
#include				"isr.h"
#include				"fit.h"

#include 				"arm_math.h"
#if defined   (__IOC_V1__)
#define					PYRO_PORT	GPIOA
#define					PYRO_BIT	GPIO_Pin_0
#endif

#if defined   (__IOC_V2__)
#define					PYRO_PORT	GPIOB
#define					PYRO_BIT	GPIO_Pin_14
#endif

#if defined   (__DISCO__)
#define					PYRO_PORT	GPIOA
#define					PYRO_BIT	GPIO_Pin_15
#endif

#define					_MAX_STATE	10
#define					_BLOCKSIZE	8
#define					_MAX_TAPS		29

class	_PYRO {
	private:
		_io		*io;
		int		nbits, temp, count, nsamples;
		short	amb0;
		_FIT	*hz,*pw;


		float32_t	iirCoeffs32[_MAX_STATE*5];
		float32_t firCoeffs32[_MAX_TAPS];
		float32_t	iirStateF32[_MAX_STATE*5];
		float32_t	firStateF32[_BLOCKSIZE + _MAX_TAPS - 1];

		float32_t	in[_BLOCKSIZE],out[_BLOCKSIZE];

		arm_biquad_casd_df1_inst_f32	S1;
		arm_fir_instance_f32					S2;

	public:
		_PYRO();
		~_PYRO();

		_buffer	*buffer;
		unsigned int sync,error_count;	

		void		ISR(_PYRO *);
		void		ISRold(_PYRO *);
		int			Increment(int, int);

		bool		Enabled;
		int			Period;

		int			Error();
		void		LoadSettings(FILE *);
		void		LoadFit(FILE *);
		void		SaveSettings(FILE *);
		void		initFilter();
		void		addFilter(char *);
		int			addSample(int);
		void		printFilter(void);
	};

#endif

//	class energometer {

//    struct pyro {
//        pyro() { /* do some constructing here … */ }
//    };

//    static pyro cons;
//};

//// C++ needs to define static members externally.
//energometer::pyro energometer::cons;
