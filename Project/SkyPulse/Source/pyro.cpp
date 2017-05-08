/**
	******************************************************************************
	* @file		pyro.cpp
	* @author	Fotona d.d.
	* @version
	* @date
	* @brief	thermopile sensor class
	*
	*/
	
/** @addtogroup
* @{
*/
#include	"pyro.h"
#include	<math.h>
#include	<vector>

static		_PYRO*	me;
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
#define 	_Ts				10																	// transmission bit time. us
#define 	_To				10																	// base sampling time, ms
#define 	_MAXBITS	32																	// number of bits

void	_PYRO::ISR(_PYRO *p) {
			if(p) {																						// klic za prijavo instance
				me=p;
				me->buffer=_buffer_init(3*100*sizeof(short));
			} else																						// klic iz ISR, instanca in buffer morata bit ze formirana 																							
				if(nbits++) {																		// if not first bit ...
					temp = temp<<1;																// shift >> in	
					if(GPIO_ReadInputDataBit(PYRO_PORT,PYRO_BIT)==SET)
						temp |= 1;
						TIM7->ARR=_Ts-1;														// set bit timeout 
				} else
					TIM7->ARR=1000-(_MAXBITS-1)*_Ts-1;						// set 1'st bit (sample) timeout (1ms minus transfer time)

				PYRO_PORT->BSRRH   =  PYRO_BIT;									// pull next bit, set output low
				PYRO_PORT->OTYPER	&= ~PYRO_BIT;									// set pin to pushpull output 
				PYRO_PORT->BSRRL   =  PYRO_BIT;									// set output high
				PYRO_PORT->OTYPER |=  PYRO_BIT;									// set pin to opendrain  output

				if(nbits > _MAXBITS) {													// data finished...?
//short			i=__time__ - sync,
short			i=__time__,
					j=temp &  0x3fff,
					k=(short)((temp >> 15) - (0x10000 - 0x2000));	// 17 bit = 1, offset v senzorju, 0x1000 offset da se izognes negativnim vredostim
					count += _To;																	// increment data counter
					temp=nbits=0;
					if(j && abs(j-amb0) < 2000) {									// j=0 pri izpuljenem, 2000 je 10 stopinj odsopanja of starega !!!
						if(error_count)
							--error_count;
						TIM7->ARR=1000*(_To-1)-1;										//
						if(Enabled && count >= Period) {						// if output enabled && output period reached ...							
							count=0;																	// reset data counter
							amb0=j;
							_buffer_push(buffer,&i,sizeof(short));
							_buffer_push(buffer,&j,sizeof(short));
							_buffer_push(buffer,&k,sizeof(short));
						}
					}
					else {
						if(error_count < 10)
							++error_count;
						else {
							amb0=j;
						}
						TIM7->ARR=1000*(_To/2-1)-1;											//
					}
				}
		}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_PYRO::_PYRO() {	
			nbits=temp=count=error_count=nsamples=0;
			Period=10;
			sync=0;
			Enabled=false;
			S1.numStages=0;
			ISR(this);
	
GPIO_InitTypeDef					
			GPIO_InitStructure;
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
			GPIO_InitStructure.GPIO_Pin = PYRO_BIT;
			GPIO_Init(PYRO_PORT, &GPIO_InitStructure);
			GPIO_SetBits(PYRO_PORT,PYRO_BIT);

			TIM_TimeBaseInitTypeDef TIM;
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
			TIM_TimeBaseStructInit(&TIM);
			TIM.TIM_Prescaler = (SystemCoreClock/2000000)-1;
			TIM.TIM_Period = 1000;
			TIM.TIM_ClockDivision = 0;
			TIM.TIM_CounterMode = TIM_CounterMode_Up;
			TIM_TimeBaseInit(TIM7,&TIM);
			TIM_ARRPreloadConfig(TIM7,DISABLE);
			TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
			NVIC_EnableIRQ(TIM7_IRQn);
			TIM_Cmd(TIM7,ENABLE);

			memset (&S2,0, sizeof(arm_fir_instance_f32));
			arm_fir_init_f32(&S2, _MAX_TAPS, firCoeffs32, firStateF32, _BLOCKSIZE);
			memset(firStateF32,0,sizeof(firStateF32));
			memset(in,0,sizeof(in));
			memset(out,0,sizeof(out));
			
			for(int i=0; i< _MAX_TAPS; ++i)
				firCoeffs32[i]=1.0/_MAX_TAPS;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_PYRO::~_PYRO() {	
			TIM_ITConfig(TIM7,TIM_IT_Update,DISABLE);
			NVIC_DisableIRQ(TIM7_IRQn);
			_buffer_close(buffer);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_PYRO::LoadSettings(FILE *f) {
char	c[128];
			fgets(c,sizeof(c),f);
			sscanf(c,"%d",&Period);
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/

void	_PYRO::LoadFit(FILE *f) {
double tp[4][4] = { 
				{1460.15,	19725.31, 42432.27, 63938.46},
				{13020.96,46874.81, 77632.92, 104778.85},
				{10672.19,42838.35, 75621.04, 104585.23},
				{8652.35,	34044.73, 58773.31, 75964.92}
};

double oph[4][4] = { 
				{0.7,6.5,16,26},  
				{14,44,72,100},  
				{19,61,110,160},  
				{23,70,120,195}
};

//double tp[4][4] = { 
//				{4,5,6,7},
//				{14,15,16,17},
//				{24,25,26,27},
//				{34,35,36,37},
//};

//double oph[4][4] = { 
//				{4,5,6,7},
//				{14,15,16,17},
//				{24,25,26,27},
//				{34,35,36,37},
//};

int			pw[]={30,40,50,60};
int			hz[]={2,5,10,20};

std::vector<_FIT> _tp(3, _FIT(3,FIT_POW)),_oph(3, _FIT(3,FIT_POW));

				for(int i=0; i<sizeof(hz)/sizeof(int); ++i) {
_FIT			__tp(3,FIT_POW),__oph(3,FIT_POW);
					
					for(int j=0; j<sizeof(pw)/sizeof(int); ++j) {
						__tp.Sample(pw[j],tp[i][j]);
						__oph.Sample(pw[j],oph[i][j]);
					}
					
					if(!__tp.Compute())
						break;
					if(!__oph.Compute())
						break;

					for(int j=0; j<_tp.size(); ++j) {
						_tp[j].Sample(hz[i],__tp.rp[j]);
						_oph[j].Sample(hz[i],__oph.rp[j]);
					}
			}
			
			for(int i=0; i<_tp.size(); ++i) {
				if(_tp[i].Compute()) {
					for(int j=0; j<_tp[i].n; ++j)
						printf("%lf ",_tp[i].rp[j]);
					printf("...TP\r\n ");
				}
				if(_oph[i].Compute()) {
					for(int j=0; j<_oph[i].n; ++j)
						printf("%lf ",_oph[i].rp[j]);
					printf(" ...OPH\r\n");
				}	
			}
}
/*******************************************************************************/
/**
	* @brief	TIM3 IC2 ISR
	* @param	: None
	* @retval : None
	*/
void	_PYRO::SaveSettings(FILE *f) {
			fprintf(f,"%5d                                   /.. pyro\r\n",Period);
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_PYRO::Increment(int a, int b) {	
			Period 		= __min(__max(10,Period+10*a),200);	
			printf("\r:thermopile  %3d",Period);		
			return Period;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_PYRO::Error() {	
			if(error_count==10)
				return 1<< pyroNoresp;
			else
				return 0;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_PYRO::initFilter() {
			memset (&S1,0, sizeof(arm_biquad_casd_df1_inst_f32));
			nsamples=0;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void	_PYRO::printFilter() {
			for(int i=0; i<S1.numStages; ++i)
				printf("\r\n+f %f,%f,%f,%f,%f",iirCoeffs32[5*i],iirCoeffs32[5*i+1],iirCoeffs32[5*i+2],iirCoeffs32[5*i+3],iirCoeffs32[5*i+4]);
}
/*******************************************************************************
* Function Name	:
* Description		:
* Output				:
* Return				:
*******************************************************************************/
void	_PYRO::addFilter(char *c) {
			int	i=S1.numStages;
			arm_biquad_cascade_df1_init_f32(&S1,i+1,iirCoeffs32,iirStateF32);
			sscanf(c,"%f,%f,%f,%f,%f",&iirCoeffs32[5*i],&iirCoeffs32[5*i+1],&iirCoeffs32[5*i+2],&iirCoeffs32[5*i+3],&iirCoeffs32[5*i+4]);
			memset(iirStateF32,0,++i*sizeof(float32_t));
			memset(in,0,_BLOCKSIZE*sizeof(float32_t));
			memset(out,0,_BLOCKSIZE*sizeof(float32_t));
			nsamples=0;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int		_PYRO::addSample(int i) {
			in[nsamples]=i;			
			i=out[nsamples++];					
			if(nsamples==_BLOCKSIZE) {
				nsamples=0;
				if(S1.numStages > 0)
					arm_biquad_cascade_df1_f32(&S1, in, out, _BLOCKSIZE);
				else
					memcpy(out,in,sizeof(out));
//				int m,n;
//				for(m=n=0; n<_BLOCKSIZE; ++n)
//					m += out[n];
//				m /=_BLOCKSIZE;
//				for(int n=0; n<_BLOCKSIZE; ++n)
//					out[n]=m;
			}
			return i;
}

extern "C" {
/*******************************************************************************/
/**
	* @brief	TIM7_IRQHandler, klice staticni ISR handler, indikacija je NULL pointer,
	*					sicer pointer vsebuje parent class !!! Mora bit extern C zaradi overridanja 
						vektorjev v startupu
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void	TIM7_IRQHandler(void) {
			if (TIM_GetITStatus(TIM7,TIM_IT_Update) != RESET) {
				TIM_ClearITPendingBit(TIM7,TIM_IT_Update);
				me->ISR(NULL);
				}
			}
}
/**
* @}
*/

