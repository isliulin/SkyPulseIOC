/**
	******************************************************************************
	* @file		lm.cpp
	* @author	Fotona d.d.
	* @version
	* @date		
	* @brief	
	*
	*/

/** @addtogroup
* @{
*/

#include "ff.h"
#include "mp3.h"
#include "tim.h"
#include "isr.h"
extern "C" {
#include "stm32f4_discovery_audio_codec.h"
}
/*******************************************************************************
* Function Name	:
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_MP3::_MP3() {
				file = (FIL *)malloc(sizeof(FIL));
				mp3 = MP3InitDecoder();
				readBuf= (unsigned char *)malloc(0);	
				readBuf= (unsigned char *)malloc(READBUF_SIZE);	
				outPtr=_TIM::Instance()->speaker;
				bytesLeft=0;
				readPtr=(unsigned char *)readBuf;
				if(!file || !readBuf)
					printf("memory error...\r\n");		
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
_MP3::~_MP3() {
				free(file);
				free(readBuf);
				readBuf=NULL;
				file=NULL;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
FIL			*_MP3::Decode(short *out) {
				unsigned int	n;
				if(file != NULL) {
					Watchdog();
					f_read(file, readBuf + bytesLeft, READBUF_SIZE - bytesLeft, &n);
					bytesLeft += n;
					int offset = MP3FindSyncWord(readPtr, bytesLeft);
					if(offset >= 0) {
						readPtr += offset;
						bytesLeft -= offset;
						MP3Decode(mp3, &readPtr, (int *)&bytesLeft,out,0);
						MP3GetLastFrameInfo(mp3, &mp3inf);
					} else {
						f_close(file);
						file=NULL;
					}
				}
				return file;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int			_MP3::Open(char *filename) {
				Watchdog();
				if(f_open(file,filename,FA_READ)==FR_OK) {
					bytesLeft=0;
					readPtr=(unsigned char *)readBuf;
					Decode(outPtr);
					
					printf("\r\nBitrate     :%d",mp3inf.bitrate);
					printf("\r\nbpSample    :%d",mp3inf.bitsPerSample);
					printf("\r\nChannels    :%d",mp3inf.nChans);
					printf("\r\nSample Rate :%d",mp3inf.samprate);
					printf("\r\nLayer       :%d",mp3inf.layer);
					printf("\r\nVersion     :%d",mp3inf.version);
					printf("\r\nSamples     :%d",mp3inf.outputSamps);
					
	//				Decode(outPtr + mp3inf.nChans*mp3inf.outputSamps*sizeof(short));
					Decode(&outPtr[mp3inf.nChans*mp3inf.outputSamps]);

#ifdef __PFM6__
					for(int i=0; i < 2*mp3inf.nChans*mp3inf.outputSamps; ++i)
								outPtr[i]=300+outPtr[i]/100;					if(!_thread_active((void *)Play,(void *)this))
						_thread_add((void *)Play,(void *)this,(char *)"play",0);

					TIM_SetAutoreload(TIM4,120000000/mp3inf.samprate);
					DMA1_Stream6->NDTR= 2*mp3inf.nChans*mp3inf.outputSamps;
					DMA_ClearFlag(DMA1_Stream6,DMA_FLAG_TCIF6|DMA_FLAG_HTIF6|DMA_FLAG_TEIF6|DMA_FLAG_DMEIF6|DMA_FLAG_FEIF6);
					DMA_Cmd(DMA1_Stream6, ENABLE);
					TIM_Cmd(TIM4,ENABLE);
#endif
#ifdef __DISCO__
					EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);
					EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, 75, 44100 );  
					EVAL_AUDIO_Play((uint16_t *)outPtr, 2*mp3inf.nChans*mp3inf.outputSamps*sizeof(uint16_t));
#endif
				}
				return 0;
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
void		_MP3::Play(_MP3 *v) { 

				if(DMA_GetFlagStatus(DMA1_Stream6, DMA_FLAG_HTIF6) != RESET) {
					DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_HTIF6);
					if(v->Decode(v->outPtr))
						for(int i=0; i < v->mp3inf.nChans*v->mp3inf.outputSamps; ++i)
								v->outPtr[i]=300+v->outPtr[i]/100;
					else
							DMA_Cmd(DMA1_Stream6, DISABLE);
				}
				if(DMA_GetFlagStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET) {
					DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6);
					if(v->Decode(&v->outPtr[v->mp3inf.nChans*v->mp3inf.outputSamps]))
						for(int i=v->mp3inf.nChans*v->mp3inf.outputSamps; i<2*v->mp3inf.nChans*v->mp3inf.outputSamps; ++i)
								v->outPtr[i]=300+v->outPtr[i]/100;
					else
							DMA_Cmd(DMA1_Stream6, DISABLE);

				}
}

/*--------------------------------
Callbacks implementation:
the callbacks prototypes are defined in the stm324xg_eval_audio_codec.h file
and their implementation should be done in the user code if they are needed.
Below some examples of callback implementations.
--------------------------------------------------------*/
/**
* @brief  Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
  /* Calculate the remaining audio data in the file and the new size 
  for the DMA transfer. If the Audio files size is less than the DMA max 
  data transfer size, so there is no calculation to be done, just restart 
  from the beginning of the file ... */
  /* Check if the end of file has been reached */
  
#ifdef AUDIO_MAL_MODE_NORMAL  
  
#if defined MEDIA_IntFLASH

#if defined PLAY_REPEAT_OFF
  LED_Toggle = 4;
  RepeatState = 1;
  EVAL_AUDIO_Stop(CODEC_PDWN_HW);
#else
  /* Replay from the beginning */
  AudioFlashPlay((uint16_t*)(AUDIO_SAMPLE + AUIDO_START_ADDRESS),AUDIO_FILE_SZE,AUIDO_START_ADDRESS);
#endif  
  
#elif defined MEDIA_USB_KEY  
  XferCplt = 1;
  if (WaveDataLength) WaveDataLength -= _MAX_SS;
  if (WaveDataLength < _MAX_SS) WaveDataLength = 0;
    
#endif 
    
#else /* #ifdef AUDIO_MAL_MODE_CIRCULAR */
  
  
#endif /* AUDIO_MAL_MODE_CIRCULAR */
}

/**
* @brief  Manages the DMA Half Transfer complete interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{  
#ifdef AUDIO_MAL_MODE_CIRCULAR
    
#endif /* AUDIO_MAL_MODE_CIRCULAR */
  
  /* Generally this interrupt routine is used to load the buffer when 
  a streaming scheme is used: When first Half buffer is already transferred load 
  the new data to the first half of buffer while DMA is transferring data from 
  the second half. And when Transfer complete occurs, load the second half of 
  the buffer while the DMA is transferring from the first half ... */
  /* 
  ...........
  */
}

/**
* @brief  Manages the DMA FIFO error interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_Error_CallBack(void* pData)
{
  /* Stop the program with an infinite loop */
  while (1)
  {}
  
  /* could also generate a system reset to recover from the error */
  /* .... */
}

/**
* @brief  Get next data sample callback
* @param  None
* @retval Next data sample to be sent
*/
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
  return 0;
}


#ifndef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{   
  return (0);
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */
/*----------------------------------------------------------------------------*/
