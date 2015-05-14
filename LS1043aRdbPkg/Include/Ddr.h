/** @Ddr.h
  Header defining the Ddr controller constants (Base addresses, sizes, flags),
  function prototype, structures etc

  Copyright (c) 2014, Freescale Ltd. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __DDR_H__
#define __DDR_H__

#include "LS1043aRdb.h"
#include "Common.h"
#include "Bitops.h"
#include <Library/BaseMemoryLib/MemLibInternals.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>

typedef UINT64 PhysSizeT;

#define SDRAM_TYPE_DDR3    7
#define CONFIG_FSL_SDRAM_TYPE      SDRAM_TYPE_DDR3

#define CONFIG_SYS_LS1_DDR_BLOCK1_SIZE  ((PhysSizeT)2 << 30)
#define CONFIG_MAX_MEM_MAPPED           CONFIG_SYS_LS1_DDR_BLOCK1_SIZE

/**
  DDR_SDRAM_CFG - DDR SDRAM Control Configuration
**/
#define SDRAM_CFG_DBW_MASK         0x00180000
#define SDRAM_CFG_DBW_SHIFT        19

#define SDRAM_CFG2_D_INIT          0x00000010
#define SDRAM_CFG2_ODT_ONLY_READ   2

/**
  DIMM params
**/
#define EDC_ECC             	2

#define HWCONFIG_BUFFER_SIZE	128

#define DDR3_RTT_OFF			0
#define DDR3_RTT_120_OHM		2 /** RTT_Nom = RZQ/2 */
#define DDR3_RTT_40_OHM		3 /** RTT_Nom = RZQ/6 */
#define DDR3_RTT_20_OHM		4 /** RTT_Nom = RZQ/12 */
#define DDR3_RTT_30_OHM            5 /** RTT_Nom = RZQ/8 */

#define FSL_DDR_ODT_NEVER			0x0
#define FSL_DDR_ODT_CS			0x1
#define FSL_DDR_ODT_OTHER_DIMM            0x3
#define FSL_DDR_ODT_ALL			0x4
#define FSL_DDR_ODT_SAME_DIMM             0x5
#define FSL_DDR_ODT_CS_AND_OTHER_DIMM     0x6

#define DDR_BL4		4	/** burst length 4 */
#define DDR_BC4		DDR_BL4	/* burst chop for ddr3 */
#define DDR_OTF             6      /** on-the-fly BC4 and BL8 */
#define DDR_BL8             8      /** burst length 8 */

#define FSL_DDR_MIN_TCKE_PULSE_WIDTH_DDR  (3)    /** FIXME */

/**
  define memory controller interleaving mode
**/
#define FSL_DDR_CACHE_LINE_INTERLEAVING	0x0
#define FSL_DDR_PAGE_INTERLEAVING  	0x1
#define FSL_DDR_BANK_INTERLEAVING		0x2
#define FSL_DDR_SUPERBANK_INTERLEAVING    0x3
#define FSL_DDR_256B_INTERLEAVING  	0x8
#define FSL_DDR_3WAY_1KB_INTERLEAVING     0xA
#define FSL_DDR_3WAY_4KB_INTERLEAVING     0xC
#define FSL_DDR_3WAY_8KB_INTERLEAVING     0xD
///
///placeholder for 4-way interleaving
///
#define FSL_DDR_4WAY_1KB_INTERLEAVING     0x1A
#define FSL_DDR_4WAY_4KB_INTERLEAVING     0x1C
#define FSL_DDR_4WAY_8KB_INTERLEAVING     0x1D

/**
  define bank(chip select) interleaving mode
**/
#define FSL_DDR_CS0_CS1			0x40
#define FSL_DDR_CS2_CS3			0x20
#define FSL_DDR_CS0_CS1_AND_CS2_CS3	(FSL_DDR_CS0_CS1 | FSL_DDR_CS2_CS3)
#define FSL_DDR_CS0_CS1_CS2_CS3		(FSL_DDR_CS0_CS1_AND_CS2_CS3 | 0x04)

#define CONFIG_FSL_DDR_INTERACTIVE /** Interactive Debugging */
#define CONFIG_SYS_DDR_RAW_TIMING
#define CONFIG_SYS_FSL_DDR3        /** Use DDR3 memory */

#define CONFIG_DIMM_SLOTS_PER_CTLR		1
#define CONFIG_CHIP_SELECTS_PER_CTRL	4

#define CONFIG_NUM_DDR_CONTROLLERS		1

#define CONFIG_SYS_NUM_DDR_CTLRS CONFIG_NUM_DDR_CONTROLLERS
#define CONFIG_SYS_DIMM_SLOTS_PER_CTLR CONFIG_DIMM_SLOTS_PER_CTLR

#define CONFIG_SYS_DDR_SDRAM_BASE  0x80000000UL
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY CONFIG_SYS_DDR_SDRAM_BASE

#define RD_TO_PRE_MASK		0x7
#define RD_TO_PRE_SHIFT		13
#define WR_DATA_DELAY_MASK		0x7
#define WR_DATA_DELAY_SHIFT		10

/**
  DDR_SDRAM_CFG - DDR SDRAM Control Configuration
**/
#define SDRAM_CFG_ECC_EN           0x20000000
#define SDRAM_CFG_MEM_EN		0x80000000
#define SDRAM_CFG_BI               0x00000001
#define SDRAM_CFG_SDRAM_TYPE_MASK  0x07000000
#define SDRAM_CFG_SDRAM_TYPE_SHIFT 24
#define SDRAM_CFG_32_BE            0x00080000
#define SDRAM_CFG_16_BE            0x00100000

#define SDRAM_TYPE_DDR1    2
#define SDRAM_TYPE_DDR2    3
#define SDRAM_TYPE_DDR3    7

/**
  Compute steps
**/
#define STEP_GET_SPD                 (1 << 0)
#define STEP_COMPUTE_DIMM_PARMS      (1 << 1)
#define STEP_COMPUTE_COMMON_PARMS    (1 << 2)
#define STEP_GATHER_OPTS             (1 << 3)
#define STEP_ASSIGN_ADDRESSES        (1 << 4)
#define STEP_COMPUTE_REGS            (1 << 5)
#define STEP_PROGRAM_REGS            (1 << 6)
#define STEP_ALL                     0xFFF

/**
  Byte 3 Key Byte / Module Type for DDR3 SPD
**/
#define DDR3_SPD_MODULETYPE_MASK		(0x0f)
#define DDR3_SPD_MODULETYPE_RDIMM		(0x01)
#define DDR3_SPD_MODULETYPE_UDIMM		(0x02)
#define DDR3_SPD_MODULETYPE_SO_DIMM	(0x03)
#define DDR3_SPD_MODULETYPE_MICRO_DIMM	(0x04)
#define DDR3_SPD_MODULETYPE_MINI_RDIMM	(0x05)
#define DDR3_SPD_MODULETYPE_MINI_UDIMM	(0x06)
#define DDR3_SPD_MODULETYPE_MINI_CDIMM	(0x07)
#define DDR3_SPD_MODULETYPE_72B_SO_UDIMM	(0x08)
#define DDR3_SPD_MODULETYPE_72B_SO_RDIMM	(0x09)
#define DDR3_SPD_MODULETYPE_72B_SO_CDIMM	(0x0A)
#define DDR3_SPD_MODULETYPE_LRDIMM		(0x0B)
#define DDR3_SPD_MODULETYPE_16B_SO_DIMM	(0x0C)
#define DDR3_SPD_MODULETYPE_32B_SO_DIMM	(0x0D)

/**
  Byte 2 Fundamental Memory Types.
**/
#define SPD_MEMTYPE_DDR3	(0x0B)

/**
  DDR_CDR1
**/
#define DDR_CDR1_DHC_EN	0x80000000
#define DDR_CDR1_ODT_SHIFT	17
#define DDR_CDR1_ODT_MASK	0x6
#define DDR_CDR2_ODT_MASK	0x1
#define DDR_CDR1_ODT(x) 	((x & DDR_CDR1_ODT_MASK) << DDR_CDR1_ODT_SHIFT)
#define DDR_CDR2_ODT(x) 	(x & DDR_CDR2_ODT_MASK)
#define DDR_CDR_ODT_75ohm	0x0	/** TODO */

/**
  To avoid 64-bit full-divides, we factor this here
**/
#define ULL_2E12	2000000000000ULL
#define UL_5POW12	244140625UL
#define UL_2POW13	(1UL << 13)
#define ULL_8FS	0xFFFFFFFFULL

#if defined(CONFIG_DDR_SPD) || defined(CONFIG_SPD_EEPROM)
/**
  Bind the main DDR setup driver's generic data
  to this specific DDR technology.

  @param  Spd			structure containing SPD settings.
  @param  Pdimm		structure containing dimm parameters
  @param  DimmNumber		dimm number

**/
STATIC
__inline__
INT32
ComputeDimmParameters(
  IN	CONST GenericSpdEepromT 	*Spd,
  OUT	DimmParamsT 			*Pdimm,
  IN	UINT32 			DimmNumber
  )
{
  return DdrComputeDimmParameters(Spd, Pdimm, DimmNumber);
};
#endif

/**
  ODT (On die termination) parameters
**/
struct DynamicOdt {
  UINT32 OdtRdCfg;
  UINT32 OdtWrCfg;
  UINT32 OdtRttNorm;
  UINT32 OdtRttWr;
};

/**
  Structure containing board specific parameters
**/
struct BoardSpecificParameters {
  UINT32 NRanks;
  UINT32 DatarateMhzHigh;
  UINT32 RankGb;
  UINT32 ClkAdjust;
  UINT32 WrlvlStart;
  UINT32 WrlvlCtl2;
  UINT32 WrlvlCtl3;
};

/**
  Record of register values computed
**/
typedef struct FslDdrCfgRegsS {
  struct {
    UINT32 Bnds;
    UINT32 Config;
    UINT32 Config2;
  } Cs[CONFIG_CHIP_SELECTS_PER_CTRL];
  UINT32 TimingCfg3;
  UINT32 TimingCfg0;
  UINT32 TimingCfg1;
  UINT32 TimingCfg2;
  UINT32 DdrSdramCfg;
  UINT32 DdrSdramCfg2;
  UINT32 DdrSdramMode;
  UINT32 DdrSdramMode2;
  UINT32 DdrSdramMode3;
  UINT32 DdrSdramMode4;
  UINT32 DdrSdramMode5;
  UINT32 DdrSdramMode6;
  UINT32 DdrSdramMode7;
  UINT32 DdrSdramMode8;
  UINT32 DdrSdramMdCntl;
  UINT32 DdrSdramInterval;
  UINT32 DdrDataInit;
  UINT32 DdrSdramClkCntl;
  UINT32 DdrInitAddr;
  UINT32 DdrInitExtAddr;
  UINT32 TimingCfg4;
  UINT32 TimingCfg5;
  UINT32 DdrZqCntl;
  UINT32 DdrWrlvlCntl;
  UINT32 DdrWrlvlCntl2;
  UINT32 DdrWrlvlCntl3;
  UINT32 DdrSrCntr;
  UINT32 DdrSdramRcw1;
  UINT32 DdrSdramRcw2;
  UINT32 DdrEor;
  UINT32 DdrCdr1;
  UINT32 DdrCdr2;
  UINT32 ErrDisable;
  UINT32 ErrIntEn;
  UINT32 Debug[32];
} FslDdrCfgRegsT;

/**
  Record of Timing parameters
**/
typedef struct {
  UINT32 TckminXPs;
  UINT32 TckmaxPs;
  UINT32 TckmaxMaxPs;
  UINT32 TrcdPs;
  UINT32 TrpPs;
  UINT32 TrasPs;
  UINT32 TwrPs;	/** maximum = 63750 ps */
  UINT32 TwtrPs;	/** maximum = 63750 ps */
  UINT32 TrfcPs;	/** maximum = 255 ns + 256 ns + .75 ns
					= 511750 ps */
  UINT32 TrrdPs;	/** maximum = 63750 ps */
  UINT32 TrcPs;	/** maximum = 254 ns + .75 ns = 254750 ps */
  UINT32 RefreshRatePs;
  UINT32 ExtendedOpSrt;
  UINT32 TisPs;	/** byte 32, spd->ca_setup */
  UINT32 TihPs;	/** byte 33, spd->ca_hold */
  UINT32 TdsPs;	/** byte 34, spd->data_setup */
  UINT32 TdhPs;	/** byte 35, spd->data_hold */
  UINT32 TrtpPs;	/** byte 38, spd->trtp */
  UINT32 TdqsqMaxPs;	/** byte 44, spd->tdqsq */
  UINT32 TqhsPs;	/** byte 45, spd->tqhs */
  UINT32 NdimmsPresent;
  UINT32 LowestCommonSPDCaslat;
  UINT32 HighestCommonDeratedCaslat;
  UINT32 AdditiveLatency;
  UINT32 AllDimmsBurstLengthsBitmask;
  UINT32 AllDimmsRegistered;
  UINT32 AllDimmsUnbuffered;
  UINT32 AllDimmsEccCapable;
  UINTN TotalMem;
  UINTN BaseAddress;
  /** DDR3 RDIMM */
  UINT8 Rcw[16];	/** Register Control Word 0-15 */
} CommonTimingParamsT;

/**
  Generalized parameters for memory controller Configuration,
  might be a little specific to the FSL memory controller
**/
typedef struct MemctlOptionsS {
  /**
    Memory Organization parameters

    if DIMM is present in the system
    where DIMMs are with respect to chip select
    where chip selects are with respect to memory boundaries
  **/
  UINT32 RegisteredDimmEn;    /** use Registered DIMM support */

  /** Options local to a Chip Select */
  struct CsLocalOptsS {
    UINT32 AutoPrecharge;
    UINT32 OdtRdCfg;
    UINT32 OdtWrCfg;
    UINT32 OdtRttNorm;
    UINT32 OdtRttWr;
  } CsLocalOpts[CONFIG_CHIP_SELECTS_PER_CTRL];
  /** Special Configurations for chip select */
  UINT32 MemctlInterleaving;
  UINT32 MemctlInterleavingMode;
  UINT32 BaIntlvCtl;
  UINT32 AddrHash;
  /** Operational mode parameters */
  UINT32 EccMode;	 /** Use ECC? */
  /** Initialize ECC using memory controller? */
  UINT32 EccInitUsingMemctl;
  UINT32 DqsConfig;	/** Use DQS? maybe only with DDR2? */
  /** SREN - self-refresh during sleep */
  UINT32 SelfRefreshInSleep;
  UINT32 DynamicPower;	/** DYN_PWR */
  /** memory data width to use (16-bit, 32-bit, 64-bit) */
  UINT32 DataBusWidth;
  UINT32 BurstLength;	/** BL4, OTF and BL8 */
  /** On-The-Fly Burst Chop enable */
  UINT32 OtfBurstChopEn;
  /** mirrior DIMMs for DDR3 */
  UINT32 MirroredDimm;
  UINT32 QuadRankPresent;
  UINT32 ApEn;	/** address parity enable for RDIMM */
  UINT32 X4En;	/** enable x4 devices */
  /** Global Timing Parameters */
  UINT32 CasLatencyOverride;
  UINT32 CasLatencyOverrideValue;
  UINT32 UseDeratedCaslat;
  UINT32 AdditiveLatencyOverride;
  UINT32 AdditiveLatencyOverrideValue;
  UINT32 ClkAdjust;
  UINT32 CpoOverride;
  UINT32 WriteDataDelay;		/** DQS adjust */
  UINT32 WrlvlOverride;
  UINT32 WrlvlSample;		/** Write leveling */
  UINT32 WrlvlStart;
  UINT32 WrlvlCtl2;
  UINT32 WrlvlCtl3;
  UINT32 HalfStrengthDriverEnable;
  UINT32 TwotEn;
  UINT32 ThreetEn;
  UINT32 Bstopre;
  UINT32 TckeClockPulseWidthPs;	/** tCKE */
  UINT32 TfawWindowFourActivatesPs;	/** tFAW --  FOUR_ACT */
  /** Rtt impedance */
  UINT32 RttOverride;		/** RttOverride enable */
  UINT32 RttOverrideValue;		/** that is Rtt_Nom for DDR3 */
  UINT32 RttWrOverrideValue;	/** this is Rtt_WR for DDR3 */
	/** Automatic self refresh */
  UINT32 AutoSelfRefreshEn;
  UINT32 SrIt;
  /** ZQ calibration */
  UINT32 ZqEn;
  /** Write leveling */
  UINT32 WrlvlEn;
  /** RCW override for RDIMM */
  UINT32 RcwOverride;
  UINT32 Rcw1;
  UINT32 Rcw2;
  /** control register 1 */
  UINT32 DdrCdr1;
  UINT32 DdrCdr2;
  UINT32 TrwtOverride;
  UINT32 Trwt;			/** read-to-write turnaround */
} MemctlOptionsT;

/**
  Parameters for a DDR3 dimm computed from the SPD
**/
typedef struct DimmParamsS {
  /** DIMM Organization parameters */
  INT8 Mpart[19];		/** gauranteed null terminated */
  UINT32 NRanks;
  UINTN RankDensity;
  UINTN Capacity;
  UINT32 DataWidth;
  UINT32 PrimarySdramWidth;
  UINT32 EcSdramWidth;
  UINT32 RegisteredDimm;
  UINT32 DeviceWidth;	/** x4, x8, x16 components */
  /** SDRAM device parameters */
  UINT32 NRowAddr;
  UINT32 NColAddr;
  UINT32 EdcConfig;	/** 0 = none, 1 = parity, 2 = ECC */
  UINT32 NBanksPerSdramDevice;
  UINT32 BurstLengthsBitmask;	/** BL=4 bit 2, BL=8 = bit 3 */
  UINT32 RowDensity;
  /** used in computing base address of DIMMs */
  UINTN BaseAddress;
  /** mirrored DIMMs */
  UINT32 MirroredDimm;	/** only for ddr3 */
  /** DIMM timing parameters */
  UINT32 MtbPs;	/** medium timebase ps, only for ddr3 */
  UINT32 Ftb10thPs;  /** fine timebase, in 1/10 ps, only for ddr3 */
  UINT32 TaaPs;	/** minimum CAS latency time, only for ddr3 */
  UINT32 TfawPs;	/** four active window delay, only for ddr3 */
  /**
    SDRAM clock periods
    The range for these are 1000-10000 so a short should be sufficient
  **/
  UINT32 TckminXPs;
  UINT32 TckminXMinus1Ps;
  UINT32 TckminXMinus2Ps;
  UINT32 TckmaxPs;
  /** SPD-defined CAS latencies */
  UINT32 CaslatX;
  UINT32 CaslatXMinus1;
  UINT32 CaslatXMinus2;
  UINT32 CaslatLowestDerated;	/** Derated CAS latency */
  /** basic timing parameters */
  UINT32 TrcdPs;
  UINT32 TrpPs;
  UINT32 TrasPs;
  UINT32 TwrPs;	/** maximum = 63750 ps */
  UINT32 TwtrPs;	/** maximum = 63750 ps */
  UINT32 TrfcPs; /** max = 255 ns + 256 ns + .75 ns
  			 = 511750 ps */
  UINT32 TrrdPs;	/** maximum = 63750 ps */
  UINT32 TrcPs;	/** maximum = 254 ns + .75 ns = 254750 ps */
  UINT32 RefreshRatePs;
  UINT32 ExtendedOpSrt;
  /** DDR3 doesn't need these as below */
  UINT32 TisPs;	/** byte 32, spd->ca_setup */
  UINT32 TihPs;	/** byte 33, spd->ca_hold */
  UINT32 TdsPs;	/** byte 34, spd->data_setup */
  UINT32 TdhPs;	/** byte 35, spd->data_hold */
  UINT32 TrtpPs;	/** byte 38, spd->trtp */
  UINT32 TdqsqMaxPs;	/** byte 44, spd->tdqsq */
  UINT32 TqhsPs;	/** byte 45, spd->tqhs */
  /** DDR3 RDIMM */
  UINT8 Rcw[16];	/** Register Control Word 0-15 */
} DimmParamsT;

/**
  Record for SPD settings
**/
typedef struct Ddr3SpdEepromS {
  /** General Section: Bytes 0-59 */
  UINT8 InfoSizeCrc;    /**  0 # bytes written into serial memory,
    				                     CRC coverage */
  UINT8 SpdRev;         /**  1 Total # bytes of SPD mem device */
  UINT8 MemType;        /**  2 Key Byte / Fundamental mem type */
  UINT8 ModuleType;     /**  3 Key Byte / Module Type */
  UINT8 DensityBanks;   /**  4 SDRAM Density and Banks */
  UINT8 Addressing;     /**  5 SDRAM Addressing */
  UINT8 ModuleVdd;      /**  6 Module nominal voltage, VDD */
  UINT8 Organization;   /**  7 Module Organization */
  UINT8 BusWidth;       /**  8 Module Memory Bus Width */
  UINT8 FtbDiv;         /**  9 Fine Timebase (FTB)
  			Dividend / Divisor */
  UINT8 MtbDividend;    /** 10 Medium Timebase (MTB) Dividend */
  UINT8 MtbDivisor;     /** 11 Medium Timebase (MTB) Divisor */
  UINT8 TckMin;         /** 12 SDRAM Minimum Cycle Time */
  UINT8 Res13;          /** 13 Reserved */
  UINT8 CaslatLsb;      /** 14 CAS Latencies Supported,
  			     Least Significant Byte */
  UINT8 CaslatMsb;      /** 15 CAS Latencies Supported,
			     Most Significant Byte */
  UINT8 TaaMin;         /** 16 Min CAS Latency Time */
  UINT8 TwrMin;         /** 17 Min Write REcovery Time */
  UINT8 TrcdMin;        /** 18 Min RAS# to CAS# Delay Time */
  UINT8 TrrdMin;        /** 19 Min Row Active to
			     Row Active Delay Time */
  UINT8 TrpMin;         /** 20 Min Row Precharge Delay Time */
  UINT8 TrasTrcExt;     /** 21 Upper Nibbles for tRAS and tRC */
  UINT8 TrasMinLsb;     /** 22 Min Active to Precharge
	  				     Delay Time */
  UINT8 TrcMinLsb;      /** 23 Min Active to Active/Refresh
	  				     Delay Time, LSB */
  UINT8 TrfcMinLsb;     /** 24 Min Refresh Recovery Delay Time */
  UINT8 TrfcMinMsb;     /** 25 Min Refresh Recovery Delay Time */
  UINT8 TwtrMin;        /** 26 Min Internal Write to
			     Read Command Delay Time */
  UINT8 TrtpMin;        /** 27 Min Internal Read to Precharge
	  				     Command Delay Time */
  UINT8 TfawMsb;        /** 28 Upper Nibble for tFAW */
  UINT8 TfawMin;        /** 29 Min Four Activate Window
  				     Delay Time*/
  UINT8 OptFeatures;    /** 30 SDRAM Optional Features */
  UINT8 ThermRefOpt;    /** 31 SDRAM Thermal and Refresh Opts */
  UINT8 ThermSensor;    /** 32 Module Thermal Sensor */
  UINT8 DeviceType;     /** 33 SDRAM device type */
  INT8 FineTckMin;	   /** 34 Fine offset for tCKmin */
  INT8 FineTaaMin;	   /** 35 Fine offset for tAAmin */
  INT8 FineTrcdMin;	   /** 36 Fine offset for tRCDmin */
  INT8 FineTrpMin;	   /** 37 Fine offset for tRPmin */
  INT8 FinetrcMin;	   /** 38 Fine offset for tRCmin */
  UINT8 Res3959[21];    /** 39-59 Reserved, General Section */
  /** Module-Specific Section: Bytes 60-116 */
  union {
  	struct {
	  /** 60 (Unbuffered) Module Nominal Height */
	  UINT8 ModHeight;
	  /** 61 (Unbuffered) Module Maximum Thickness */
	  UINT8 ModThickness;
	  /** 62 (Unbuffered) Reference Raw Card Used */
	  UINT8 RefRawCard;
	  /** 63 (Unbuffered) Address Mapping from
		        Edge Connector to DRAM */
	  UINT8 AddrMapping;
	  /** 64-116 (Unbuffered) Reserved */
	  UINT8 Res64116[53];
	} Unbuffered;
	struct {
	  /** 60 (Registered) Module Nominal Height */
	  UINT8 ModHeight;
	  /** 61 (Registered) Module Maximum Thickness */
	  UINT8 ModThickness;
	  /** 62 (Registered) Reference Raw Card Used */
	  UINT8 RefRawCard;
	  /** 63 DIMM Module Attributes */
	  UINT8 ModuAttr;
	  /** 64 RDIMM Thermal Heat Spreader Solution */
	  UINT8 Thermal;
	  /** 65 Register Manufacturer ID Code, Least Significant Byte */
	  UINT8 RegIdLo;
	  /** 66 Register Manufacturer ID Code, Most Significant Byte */
	  UINT8 RegIdHi;
	  /** 67 Register Revision Number */
	  UINT8 RegRev;
	  /** 68 Register Type */
	  UINT8 RegType;
	  /** 69-76 RC1,3,5...15 (MS Nibble) / RC0,2,4...14 (LS Nibble) */
	  UINT8 Rcw[8];
	} Registered;
	UINT8 Uc[57]; /** 60-116 Module-Specific Section */
  } ModSection;
  /** Unique Module ID: Bytes 117-125 */
  UINT8 MmidLsb;        /** 117 Module MfgID Code LSB - JEP-106 */
  UINT8 MmidMsb;        /** 118 Module MfgID Code MSB - JEP-106 */
  UINT8 Mloc;           /** 119 Mfg Location */
  UINT8 Mdate[2];       /** 120-121 Mfg Date */
  UINT8 Sernum[4];      /** 122-125 Module Serial Number */
  /** CRC: Bytes 126-127 */
  UINT8 Crc[2];         /** 126-127 SPD CRC */
  /** Other Manufacturer Fields and User Space: Bytes 128-255 */
  UINT8 Mpart[18];      /** 128-145 Mfg's Module Part Number */
  UINT8 Mrev[2];        /** 146-147 Module Revision Code */
  UINT8 DmidLsb;        /** 148 DRAM MfgID Code LSB - JEP-106 */
  UINT8 DmidMsb;        /** 149 DRAM MfgID Code MSB - JEP-106 */
  UINT8 Msd[26];        /** 150-175 Mfg's Specific Data */
  UINT8 Cust[80];       /** 176-255 Open for Customer Use */
}Ddr3SpdEepromT;

typedef Ddr3SpdEepromT GenericSpdEepromT;

/**
  Record of all information about DDR
**/
typedef struct {
  GenericSpdEepromT
    SpdInstalledDimms[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_SYS_DIMM_SLOTS_PER_CTLR];
  struct DimmParamsS
    DimmParams[CONFIG_SYS_NUM_DDR_CTLRS][CONFIG_SYS_DIMM_SLOTS_PER_CTLR];
  MemctlOptionsT MemctlOpts[CONFIG_SYS_NUM_DDR_CTLRS];
  CommonTimingParamsT CommonTimingParams[CONFIG_SYS_NUM_DDR_CTLRS];
  FslDdrCfgRegsT FslDdrConfigReg[CONFIG_SYS_NUM_DDR_CTLRS];
} FslDdrInfoT;

/**
  DDR memory controller registers
**/
struct CcsrDdr {
  UINT32	Cs0Bnds;		/** Chip Select 0 Memory Bounds */
  CHAR8	Res04[4];
  UINT32	Cs1Bnds;		/** Chip Select 1 Memory Bounds */
  CHAR8	Res0c[4];
  UINT32	Cs2Bnds;		/** Chip Select 2 Memory Bounds */
  CHAR8	Res14[4];
  UINT32	Cs3Bnds;		/** Chip Select 3 Memory Bounds */
  CHAR8	Res1c[100];
  UINT32	Cs0Config;		/** Chip Select Configuration */
  UINT32	Cs1Config;		/** Chip Select Configuration */
  UINT32	Cs2Config;		/** Chip Select Configuration */
  UINT32	Cs3Config;		/** Chip Select Configuration */
  CHAR8	Res90[48];
  UINT32	Cs0Config2;		/** Chip Select Configuration 2 */
  UINT32	Cs1Config2;		/** Chip Select Configuration 2 */
  UINT32	Cs2Config2;		/** Chip Select Configuration 2 */
  UINT32	Cs3Config2;		/** Chip Select Configuration 2 */
  CHAR8	Resd0[48];
  UINT32	TimingCfg3;		/** SDRAM Timing Configuration 3 */
  UINT32	TimingCfg0;		/** SDRAM Timing Configuration 0 */
  UINT32	TimingCfg1;		/** SDRAM Timing Configuration 1 */
  UINT32	TimingCfg2;		/** SDRAM Timing Configuration 2 */
  UINT32	SdramCfg;		/** SDRAM Control Configuration */
  UINT32	SdramCfg2;		/** SDRAM Control Configuration 2 */
  UINT32	SdramMode;		/** SDRAM Mode Configuration */
  UINT32	SdramMode2;		/** SDRAM Mode Configuration 2 */
  UINT32	SdramMdCntl;		/** SDRAM Mode Control */
  UINT32	SdramInterval;	/** SDRAM Interval Configuration */
  UINT32	SdramDataInit;	/** SDRAM Data initialization */
  CHAR8	Res12c[4];
  UINT32	SdramClkCntl;		/** SDRAM Clock Control */
  CHAR8	Res134[20];
  UINT32	InitAddr;		/** training init addr */
  UINT32	InitExtAddr;		/** training init extended addr */
  CHAR8	Res150[16];
  UINT32	TimingCfg4;		/** SDRAM Timing Configuration 4 */
  UINT32	TimingCfg5;		/** SDRAM Timing Configuration 5 */
  CHAR8	Reg168[8];
  UINT32	DdrZqCntl;		/** ZQ calibration control*/
  UINT32	DdrWrlvlCntl;		/** write leveling control*/
  CHAR8	Reg178[4];
  UINT32	DdrSrCntr;		/** self refresh counter */
  UINT32	DdrSdramRcw1;		/** Control Words 1 */
  UINT32	DdrSdramRcw2;		/** Control Words 2 */
  CHAR8	Reg188[8];
  UINT32	DdrWrlvlCntl2;	/** write leveling control 2 */
  UINT32	DdrWrlvlCntl3;	/** write leveling control 3 */
  CHAR8	Res198[104];
  UINT32	SdramMode3;		/** SDRAM Mode Configuration 3 */
  UINT32	SdramMode4;		/** SDRAM Mode Configuration 4 */
  UINT32	SdramMode5;		/** SDRAM Mode Configuration 5 */
  UINT32	SdramMode6;		/** SDRAM Mode Configuration 6 */
  UINT32	SdramMode7;		/** SDRAM Mode Configuration 7 */
  UINT32	SdramMode8;		/** SDRAM Mode Configuration 8 */
  CHAR8	Res218[0x908];
  UINT32	DdrDsr1;		/** Debug Status 1 */
  UINT32	DdrDsr2;		/** Debug Status 2 */
  UINT32	DdrCdr1;		/** Control Driver 1 */
  UINT32	DdrCdr2;		/** Control Driver 2 */
  CHAR8	ResB30[200];
  UINT32	IpRev1;		/** IP Block Revision 1 */
  UINT32	IpRev2;		/** IP Block Revision 2 */
  UINT32	Eor;			/** Enhanced Optimization Register */
  CHAR8	ResC04[252];
  UINT32	Mtcr;			/** Memory Test Control Register */
  CHAR8	ResD04[28];
  UINT32	Mtp1;			/** Memory Test Pattern 1 */
  UINT32	Mtp2;			/** Memory Test Pattern 2 */
  UINT32	Mtp3;			/** Memory Test Pattern 3 */
  UINT32	Mtp4;			/** Memory Test Pattern 4 */
  UINT32	Mtp5;			/** Memory Test Pattern 5 */
  UINT32	Mtp6;			/** Memory Test Pattern 6 */
  UINT32	Mtp7;			/** Memory Test Pattern 7 */
  UINT32	Mtp8;			/** Memory Test Pattern 8 */
  UINT32	Mtp9;			/** Memory Test Pattern 9 */
  UINT32	Mtp10;			/** Memory Test Pattern 10 */
  CHAR8	ResD48[184];
  UINT32	DataErrInjectHi;	/** Data Path Err Injection Mask High */
  UINT32	DataErrInjectLo;	/** Data Path Err Injection Mask Low */
  UINT32	EccErrInject;		/** Data Path Err Injection Mask ECC */
  CHAR8	ResE0c[20];
  UINT32	CaptureDataHi;	/** Data Path Read Capture High */
  UINT32	CaptureDataLo;	/** Data Path Read Capture Low */
  UINT32	CaptureEcc;		/** Data Path Read Capture ECC */
  CHAR8	ResE2c[20];
  UINT32	ErrDetect;		/** Error Detect */
  UINT32	ErrDisable;		/** Error Disable */
  UINT32	ErrIntEn;
  UINT32	CaptureAttributes;	/** Error Attrs Capture */
  UINT32	CaptureAddress;	/** Error Addr Capture */
  UINT32	CaptureExtAddress;	/** Error Extended Addr Capture */
  UINT32	ErrSbe;		/** Single-Bit ECC Error Management */
  CHAR8	ResE5c[164];
  UINT32	Debug[32];		/** Debug_1 to Debug_32 */
  CHAR8	ResF80[128];
};

/**
  Main function to initialize DDR
 **/
VOID
DramInit(
  VOID
  );

/**
  Function to compute data required to initialize DDR controller

  @param   Popts		ddr memory control parameter
  @param   Ddr		will contain data to initialize DDRC registers
  @param   CommonDimm	timing parameters
  @param   DimmParams	dimm parametrs
  @param   DbwCapAdj		rank density adjustment offset
  @param   SizeOnly		parameter to decide if need to calculate only
  sdram size

**/
UINT32
ComputeFslMemctlConfigRegs(
  IN	CONST MemctlOptionsT 	*Popts,
  OUT	FslDdrCfgRegsT 		*Ddr,
  IN   CONST CommonTimingParamsT 	*CommonDimm,
  IN   CONST DimmParamsT 		*DimmParams,
  IN   UINT32 			DbwCapAdj,
  IN   UINT32 			SizeOnly
  );

/**
  Round up MclkPs to nearest 1 ps in memory controller code
  if the error is 0.5ps or more.

  If an imprecise data rate is too high due to rounding error
  propagation, compute a suitably rounded MclkPs to compute
  a working memory controller Configuration.

  @retval  memory clock in picosecond

**/
UINT32
GetMemoryClkPeriodPs(
  VOID
  );

/**
  Function to return ddr frequency

  @retval   Ddr frequency

**/
UINTN
GetDdrFreq(
  VOID
  );

/**
  Funtion to divide N to base Base

  @param   N		Dividend
  @param   Base	Base

**/
UINT32
__Div64_32(
  IN   UINT64 	*N,
  IN   UINT32 	Base
  );

/**
  Function to dump DDRC registers

**/

VOID
DdrRegDump (
  VOID
  );

#endif
