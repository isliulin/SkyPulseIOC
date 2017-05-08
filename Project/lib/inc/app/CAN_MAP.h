#define CANRXPFM 0x40
#define CANRXHC  0x41
#define CANRXPVC 0x42
#define CANRXEC  0x43
#define CANRXEC2  0x44

#define CANTXPFM 0x20
#define CANTXHC  0x21
#define CANTXPVC 0x22
#define CANTXEC  0x23

//---------------------------------------//
#define COM_ALL_16		0xffff

#define PFM_COM_SIMM1	0x0001
#define PFM_COM_SIMM2	0x0002
#define PFM_COM_DISCHG	0x0004
#define PFM_COM_SCRTEST	0x0008
#define PFM_COM_CBDSBL	0x0010
#define PFM_COM_CBRST	0x0020
#define PFM_COM_NREADYRST	0x0040

#define PFM_STAT_SIMM1	0x0001
#define PFM_STAT_SIMM2	0x0002
#define PFM_STAT_DSCHG	0x0004
#define PFM_STAT_UBHIGH	0x0008
#define PFM_STAT_PSRDY	0x0100

#define PFM_ERR_SIMM1	0x0001
#define PFM_ERR_SIMM2	0x0002
#define PFM_ERR_UB  	0x0004
#define PFM_ERR_LNG 	0x0008
#define PFM_ERR_TEMP	0x0010
#define PFM_ERR_DRVERR	0x0020
#define PFM_SCRFIRED  	0x0040
#define PFM_ERR_PULSEENABLE	0x0080
#define PFM_ERR_PSRDYN	0x0100
#define PFM_ERR_48V  	0x0200
#define PFM_ERR_15V 	0x0400
//---------------------------------------//

#define EC_COM_RST		0x0001

#define EC_ERR_MV270	0x00000001
#define EC_ERR_MV100	0x00000002
#define EC_ERR_MV085  	0x00000004
#define EC_ERR_PFC430 	0x00000008
#define EC_ERR_PFC450	0x00000010
#define EC_ERR_PFC375	0x00000020
#define EC_ERR_PFC350  	0x00000040
#define EC_ERR_U400 	0x00000080
#define EC_ERR_U800 	0x00000100
#define EC_ERR_U48  	0x00000200
#define EC_ERR_POWER  	0x00000400
#define EC_ERR_FREQ 	0x00000800
#define EC_ERR_PFCTW	0x00001000			   // Temperature Warning
#define EC_ERR_RES1TW	0x00002000			   // Temperature Warning
#define EC_ERR_RES2TW	0x00004000			   // Temperature Warning
#define EC_ERR_PFCTE	0x00008000			   // Temperature Error
#define EC_ERR_RES1TE	0x00010000			   // Temperature Error
#define EC_ERR_RES2TE	0x00020000			   // Temperature Error
#define EC_ERR_PFCR  	0x00040000
#define EC_ERR_PFCS 	0x00080000
#define EC_ERR_RESR 	0x00100000
#define EC_ERR_RESS 	0x00200000
#define EC_ERR_PFCU  	0x00400000
#define EC_ERR_RESU 	0x00800000

#define EC_ERR_ALL 	(EC_ERR_RESS | EC_ERR_PFCS | EC_ERR_RES2TE | EC_ERR_RES1TE | EC_ERR_PFCTE | EC_ERR_FREQ | EC_ERR_POWER | EC_ERR_PFC350 | EC_ERR_PFC450 | EC_ERR_MV085 | EC_ERR_MV270)
#define EC_ERR_HIGH		0x00FF0000
#define EC_ERR_Low		0x0000FFFF
#define EC_ERR_PFC_MASK	0x00000078
#define EC_ERR_TE_MASK	0x00038000
#define EC_ERR_STOP_MASK (EC_ERR_PFCS | EC_ERR_RESS)
//---------------------------------------//
