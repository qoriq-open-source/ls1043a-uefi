/** SdxcLib.C
  Sdxc Library Containing Functions for Reset Read, Write, Initialize Etc

  Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

  This Program And The Accompanying Materials
  Are Licensed And Made Available Under The Terms And Conditions Of The BSD License
  Which Accompanies This Distribution.  The Full Text Of The License May Be Found At
  Http://Opensource.Org/Licenses/Bsd-License.Php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/Sdxc.h>

extern struct Mmc *gMmc;

struct SdxcOperations SdxcOps = {
       .SdxcSendCmd     = SdxcSendCmd,
       .SdxcSetIos      = SdxcSetIos,
       .SdxcInit        = SdxcInit,
       .SdxcGetcd       = SdxcGetcd,
};

VOID *
GetDmaBuffer (
  IN struct DmaData *DmaData
  )
{
  EFI_STATUS Status;
  EFI_PHYSICAL_ADDRESS PhyAddr;

  Status = DmaAllocateBuffer(EfiBootServicesData,
		EFI_SIZE_TO_PAGES(DmaData->Bytes),
		&(DmaData->DmaAddr));
  if (Status) {
    DEBUG((EFI_D_ERROR,"DmaAllocateBuffer failed\n"));
    return NULL;
  }

  Status = DmaMap (DmaData->MapOperation, DmaData->DmaAddr,
		&DmaData->Bytes, &PhyAddr, &DmaData->Mapping); 
  if (Status) {
    DEBUG((EFI_D_ERROR,"DmaMap failed %d \n", Status));
    return NULL;
  }
  return (VOID *)PhyAddr;
}

EFI_STATUS
FreeDmaBuffer (
  IN struct DmaData *DmaData
  )
{
  EFI_STATUS Status;

  Status = DmaUnmap(DmaData->Mapping);
  if (Status) {
    DEBUG((EFI_D_ERROR,"DmaUnmap failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  Status = DmaFreeBuffer(EFI_SIZE_TO_PAGES (DmaData->Bytes), DmaData->DmaAddr);
  if (Status) {
    DEBUG((EFI_D_ERROR,"DmaFreeBuffer failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/* return The XFERTYPE Flags for A Given Command And Data Packet */
UINT32
SdxcXfertype (
  IN  struct SdCmd *Cmd,
  IN  struct SdData *Data
  )
{
  UINT32 Xfertype = 0;

  if (Data) {
    Xfertype |= XFERTYPE_DPSEL;

    if (PcdGetBool(PcdSdxcDmaSupported))
      Xfertype |= XFERTYPE_DMAEN;

    if (Data->Blocks > 1) {
      Xfertype |= XFERTYPE_MSBSEL;
      Xfertype |= XFERTYPE_BCEN;
    }

    if (Data->Flags & MMC_DATA_READ)
      Xfertype |= XFERTYPE_DTDSEL;
  }

  if (Cmd->RespType & MMC_RSP_CRC)
    Xfertype |= XFERTYPE_CCCEN;
  if (Cmd->RespType & MMC_RSP_OPCODE)
    Xfertype |= XFERTYPE_CICEN;
  if (Cmd->RespType & MMC_RSP_136)
    Xfertype |= XFERTYPE_RSPTYP_136;
  else if (Cmd->RespType & MMC_RSP_BUSY)
    Xfertype |= XFERTYPE_RSPTYP_48_BUSY;
  else if (Cmd->RespType & MMC_RSP_PRESENT)
    Xfertype |= XFERTYPE_RSPTYP_48;

  if (Cmd->CmdIdx == EMMC_CMD_STOP_TRANSMISSION)
    Xfertype |= XFERTYPE_CMDTYP_ABORT;

  return XFERTYPE_CMD(Cmd->CmdIdx) | Xfertype;
}

EFI_STATUS
SdxcSetupData (
  struct SdxcRegs *Regs,
  IN  UINT32 Clk,
  IN  struct SdData *Data
  )
{
  INT32 Timeout = 0;
  UINT32 WmlVal = 0;
  UINT32 Dsaddr = 0;;

  WmlVal = Data->Blocksize/4;

  if (Data->Flags & MMC_DATA_READ) {
    InternalMemCopyMem(&Dsaddr, Data->Dest, sizeof(UINT32));

    if (WmlVal > WML_RD_MAX)
      WmlVal = WML_RD_MAX_VAL;

    MmioClearSetBe32((UINTN)&Regs->Wml, WML_RD_MASK, WmlVal);

    if (PcdGetBool(PcdSdxcDmaSupported)) {
      EFI_PHYSICAL_ADDRESS Addr = (EFI_PHYSICAL_ADDRESS)Data->Dest;
      MmioWriteBe32((UINTN)&Regs->Dsaddr, Addr);
    }
  } else {
    InternalMemCopyMem(&Dsaddr, Data->Src, sizeof(UINT32));
    if (WmlVal > WML_WR_MAX)
      WmlVal = WML_WR_MAX_VAL;

    if ((MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_WPSPL) == 0) {
      DEBUG((EFI_D_ERROR, "The SD Card Is Locked. Can Not Write To A Locked Card.\n"));
      return EFI_TIMEOUT;
    }

    MmioClearSetBe32((UINTN)&Regs->Wml, WML_WR_MASK, WmlVal << 16);

    if (PcdGetBool(PcdSdxcDmaSupported)) {
      EFI_PHYSICAL_ADDRESS Addr = (EFI_PHYSICAL_ADDRESS)Data->Src;
      MmioWriteBe32((UINTN)&Regs->Dsaddr,Addr);
    }
  }

  MmioWriteBe32((UINTN)&Regs->Blkattr, Data->Blocks << 16 | Data->Blocksize);

  /* Calculate The Timeout Period for Data Transactions */
  Timeout = GenericFls(Clk/4);
  Timeout -= 13;

  if (Timeout > 14)
    Timeout = 14;

  if (Timeout < 0)
    Timeout = 0;

  MmioClearSetBe32((UINTN)&Regs->Sysctl, SYSCTL_TIMEOUT_MASK, Timeout << 16);

  return EFI_SUCCESS;
}

VOID
ResetCmdFailedData (
  IN  struct SdxcRegs *Regs,
  IN  struct SdData *Data
  )
{
  INT32 Timeout = 10000;

  /* Reset CMD And DATA Portions On Error */
  MmioWriteBe32((UINTN)&Regs->Sysctl, MmioReadBe32((UINTN)&Regs->Sysctl) |
           SYSCTL_RSTC);

  while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTC) && Timeout--);
  if (Timeout <= 0)
    DEBUG((EFI_D_ERROR, "Failed to reset CMD Portion On Error\n"));

  Timeout = 10000;
  if (Data) {
    MmioWriteBe32((UINTN)&Regs->Sysctl,
           MmioReadBe32((UINTN)&Regs->Sysctl) | SYSCTL_RSTD);
    while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTD) && Timeout--)
     		;
    if (Timeout <= 0)
      DEBUG((EFI_D_ERROR, "Failed to reset DATA Portion On Error\n"));
  }

  MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

}

static EFI_STATUS
SdxcReadWrite (
  IN  struct SdxcRegs *Regs,
  OUT struct SdData *Data
  )
{
  UINT32 Blocks;
  CHAR8 *Buffer;
  UINT32 DataBuf;
  UINT32 Size;
  UINT32 Irqstat;
  UINT32 TimeOut;

  if (Data->Flags & MMC_DATA_READ) {
    Blocks = Data->Blocks;
    Buffer = Data->Dest;
    while (Blocks) {
      TimeOut = CPU_POLL_TIMEOUT;
      Size = Data->Blocksize;
      Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);

      while (!(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_BREN)
             && --TimeOut);
      if (TimeOut <= 0) {
        DEBUG((EFI_D_ERROR, "Data Read Failed in PIO Mode 0x%x\n",
		MmioReadBe32((UINTN)&Regs->Prsstat)));
        return EFI_DEVICE_ERROR;
      }
      while (Size && (!(Irqstat & IRQSTATE_TC))) {
	 NanoSecondDelay(4000);  /* Wait before last byte transfer complete */
        Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
        DataBuf = MmioRead32((UINTN)&Regs->Datport);
        *((UINT32 *)Buffer) = DataBuf;
        Buffer += 4;
        Size -= 4;
      }
      Blocks--;
    }
  } else {
    Blocks = Data->Blocks;
    Buffer = (CHAR8 *)Data->Src;
    while (Blocks) {
      TimeOut = CPU_POLL_TIMEOUT;
      Size = Data->Blocksize;
      Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
      while (!(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_BWEN)
             && --TimeOut);
      if (TimeOut <= 0) {
        DEBUG((EFI_D_ERROR, "Data Write Failed in PIO Mode 0x%x\n",
		MmioReadBe32((UINTN)&Regs->Prsstat)));
        return EFI_DEVICE_ERROR;
      }
      while (Size && (!(Irqstat & IRQSTATE_TC))) {
	 NanoSecondDelay(4000);  /* Wait before last byte transfer complete */
        DataBuf = *((UINT32 *)Buffer);
        Buffer += 4;
        Size -= 4;
        Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);
        MmioWrite32((UINTN)&Regs->Datport, DataBuf);
      }
      Blocks--;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
RecvResp(
  IN  struct SdxcRegs *Regs,
  IN  struct SdData *Data,
  IN  UINT32 RespType,
  OUT UINT32 *Response
  )
{
  INT32 Timeout = 25000;
  EFI_STATUS Status = 0;

   /* Workaround for SDXC Errata ENGcm03648 */
  if (!Data && (RespType & MMC_RSP_BUSY)) {
    Timeout = 25000;

    /* Poll On DATA0 Line for Cmd With Busy Signal for 250 Ms */
    while (Timeout > 0 && !(MmioReadBe32((UINTN)&Regs->Prsstat) &
				PRSSTATE_DAT0)) {
      MicroSecondDelay(100);
      Timeout--;
    }

    if (Timeout <= 0) {
      DEBUG((EFI_D_ERROR, "Timeout Waiting for DAT0 To Go High!\n"));
      ResetCmdFailedData(Regs, Data);
      return EFI_TIMEOUT;
    }
  }

  /* Copy The Response To The Response Buffer */
  if (RespType & MMC_RSP_136) {
    UINT32 Rspns3, Rspns2, Rspns1, Rspns0;

    Rspns3 = MmioReadBe32((UINTN)&Regs->Rspns3);
    Rspns2 = MmioReadBe32((UINTN)&Regs->Rspns2);
    Rspns1 = MmioReadBe32((UINTN)&Regs->Rspns1);
    Rspns0 = MmioReadBe32((UINTN)&Regs->Rspns0);
    Response[0] = (Rspns3 << 8) | (Rspns2 >> 24);
    Response[1] = (Rspns2 << 8) | (Rspns1 >> 24);
    Response[2] = (Rspns1 << 8) | (Rspns0 >> 24);
    Response[3] = (Rspns0 << 8);
  } else
    Response[0] = MmioReadBe32((UINTN)&Regs->Rspns0);

  /* Wait Until All Of The Blocks Are Transferred */
  if (Data) {
    if (!PcdGetBool(PcdSdxcDmaSupported)) {
      Status = SdxcReadWrite(Regs, Data);
    } else {
      UINT32	Irqstat;
      Timeout = 10000000;
      do {

        Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);

        if (Irqstat & IRQSTATE_DTOE) {
          ResetCmdFailedData(Regs, Data);
          return EFI_TIMEOUT;
        }

        if (Irqstat & DATA_ERR) {
          ResetCmdFailedData(Regs, Data);
          return EFI_DEVICE_ERROR;
        }

	MicroSecondDelay(10);
	
      } while ((!(Irqstat & DATA_COMPLETE)) && Timeout--);

      if (Timeout <= 0) {
        DEBUG((EFI_D_ERROR, "Timeout Waiting for DATA_COMPLETE to set\n"));
        ResetCmdFailedData(Regs, Data);
        return EFI_TIMEOUT;
      }
    } /* end of DMA R/W */
  }

  MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

  return Status;
}


/*
 * Sends A Command Out On The Bus.  Takes The Mmc Pointer,
 * A Command Pointer, And An Optional Data Pointer.
 */
EFI_STATUS
SdxcSendCmd (
  IN  struct SdxcRegs *Regs,
  IN  UINT32 Clk,
  IN  struct SdCmd *Cmd,
  IN  struct SdData *Data
  )
{
  EFI_STATUS  Status = 0;
  UINT32	Xfertype;
  UINT32	Irqstat;
  INT32	Timeout = 100000;

  MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

  asm("Dmb :");

  /* Wait for The Bus To Be Idle */
  while ((MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_CICHB) ||
		(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_CIDHB))
    NanoSecondDelay(10);

  /* Wait for data line to be active */
  while (MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_DLA)
    NanoSecondDelay(10);

  /* Wait Before The Next Command */
  MicroSecondDelay(1000);

  /* Set Up for A Data Transfer if We Have One */
  if (Data) {
    Status = SdxcSetupData(Regs, Clk, Data);
    if (Status)
      return Status;
  }

  /* Figure Out The Transfer Arguments */
  Xfertype = SdxcXfertype(Cmd, Data);

  /* Mask All Irqs */
  MmioWriteBe32((UINTN)&Regs->Irqsigen, 0);

  /* Send The Command */
  MmioWriteBe32((UINTN)&Regs->CmdArg, Cmd->CmdArg);
  MmioWriteBe32((UINTN)&Regs->Xfertype, Xfertype);

  /* Wait for The Command To Complete */
  Timeout = 100000;
  while ((!(MmioReadBe32((UINTN)&Regs->Irqstat) & (IRQSTATE_CC | IRQSTATE_CTOE)))
		&& Timeout--)
  	;

  if (Timeout <= 0) {
    DEBUG((EFI_D_ERROR, "Command not completed %d\n", Cmd->CmdIdx));
    return EFI_TIMEOUT;
  }

  Irqstat = MmioReadBe32((UINTN)&Regs->Irqstat);

  if (Irqstat & CMD_ERR) {
    Status = EFI_DEVICE_ERROR;
    DEBUG((EFI_D_ERROR, "SdxcSendCmd: Device Error for Cmd(%d)\n",Cmd->CmdIdx));
    goto Out;
  }

  if (Irqstat & IRQSTATE_CTOE) {
    Status = EFI_TIMEOUT;
    DEBUG((EFI_D_ERROR, "SdxcSendCmd: Timeout for Cmd(%d)\n",Cmd->CmdIdx));
    goto Out;
  }

  if (Cmd->RespType != 0xFF) {
    Status = RecvResp(Regs, Data, Cmd->RespType, Cmd->Response);
    if (Status == EFI_SUCCESS)
      return Status;
  }

Out:
  if (Status)
    ResetCmdFailedData(Regs, Data);
  else
    MmioWriteBe32((UINTN)&Regs->Irqstat, -1);

  return Status;
}

VOID
SetSysctl (
  IN  struct Mmc *Mmc,
  UINT32 Clock
  )
{
  INT32 Div, PreDiv;
  struct SdxcCfg *Cfg = Mmc->Private;
  volatile struct SdxcRegs *Regs = (struct SdxcRegs *)Cfg->SdxcBase;
  INT32 SdhcClk = Cfg->SdhcClk;
  UINT32 Clk;

  if (Clock < Mmc->Cfg->FMin)
    Clock = Mmc->Cfg->FMin;

  if (SdhcClk / 16 > Clock) {
    for (PreDiv = 2; PreDiv < 256; PreDiv *= 2)
      if ((SdhcClk / PreDiv) <= (Clock * 16))
        break;
  } else
    PreDiv = 2;

  for (Div = 1; Div <= 16; Div++)
    if ((SdhcClk / (Div * PreDiv)) <= Clock)
      break;

  PreDiv >>= Mmc->DdrMode ? DIV_2 : DIV_1;
  Div -= 1;

  Clk = (PreDiv << 8) | (Div << 4);

  MmioClearBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_CKEN);

  MmioClearSetBe32((UINTN)&Regs->Sysctl, SYSCTL_CLOCK_MASK, Clk);

  MicroSecondDelay(100);

  Clk = SYSCTL_PEREN | SYSCTL_CKEN;

  MmioSetBitsBe32((UINTN)&Regs->Sysctl, Clk);
}

VOID
SdxcSetIos (
  IN  struct Mmc *Mmc
  )
{
  struct SdxcCfg *Cfg = Mmc->Private;
  struct SdxcRegs *Regs = (struct SdxcRegs *)Cfg->SdxcBase;

  /* Set The Clock Speed */
  SetSysctl(Mmc, Mmc->Clock);

  /* Set The Bus Width */
  MmioClearBitsBe32((UINTN)&Regs->Proctl, PRCTL_DTW_4 | PRCTL_DTW_8);

  if (Mmc->BusWidth == 4)
    MmioSetBitsBe32((UINTN)&Regs->Proctl, PRCTL_DTW_4);
  else if (Mmc->BusWidth == 8)
    MmioSetBitsBe32((UINTN)&Regs->Proctl, PRCTL_DTW_8);
}

INT32
SdxcInit (
  IN  struct Mmc *Mmc
  )
{
  struct SdxcCfg *Cfg = Mmc->Private;
  struct SdxcRegs *Regs = (struct SdxcRegs *)Cfg->SdxcBase;
  INT32 Timeout = 1000;

  /* Reset The Entire Host Controller */
  MmioSetBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_RSTA);

  /* Wait Until The Controller Is Available */
  while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTA) && --Timeout)
    MicroSecondDelay(1000);

  /* Enable Cache Snooping */
  MmioWriteBe32((UINTN)&Regs->Scr, 0x00000040);

  MmioSetBitsBe32((UINTN)&Regs->Sysctl, SYSCTL_HCKEN | SYSCTL_IPGEN);

  /* Set The Initial Clock Speed */
  SdxcSetClock(Mmc, 400000);

  /* Disable The BRR And BWR Bits In IRQSTAT */
  MmioClearBitsBe32((UINTN)&Regs->Irqstaten, IRQSTATE_EN_BRR | IRQSTATE_EN_BWR);

  /* Set Little Endian mode for data Buffer */
  MmioWriteBe32((UINTN)&Regs->Proctl, PRCTL_INIT);

  /* Set Timout To The Maximum Value */
  MmioClearSetBe32((UINTN)&Regs->Sysctl, SYSCTL_TIMEOUT_MASK, 14 << 16);

  return EFI_SUCCESS;
}

INT32
SdxcGetcd (
  IN  struct Mmc *Mmc
  )
{
  struct SdxcCfg *Cfg = Mmc->Private;
  struct SdxcRegs *Regs = (struct SdxcRegs *)Cfg->SdxcBase;
  INT32 Timeout = 1000;

  while (!(MmioReadBe32((UINTN)&Regs->Prsstat) & PRSSTATE_CINS) && --Timeout)
    MicroSecondDelay(10);

  return Timeout > 0;
}

VOID
SdxcReset (
  IN struct SdxcRegs *Regs
  )
{
  UINT64 Timeout = 100;

  /* Reset The Controller */
  MmioWriteBe32((UINTN)&Regs->Sysctl, SYSCTL_RSTA);

  /* Hardware Clears The Bit When It Is Done */
  while ((MmioReadBe32((UINTN)&Regs->Sysctl) & SYSCTL_RSTA) && --Timeout)
    MicroSecondDelay(10);

  if (!Timeout)
    DEBUG((EFI_D_ERROR, "MMC/SD: Reset Never Completed.\n"));
}


EFI_STATUS
SdxcInitialize (
  IN struct SdxcCfg *Cfg
  )
{
  struct SdxcRegs *Regs;
  struct SysInfo SocSysInfo;

  UINT32 Caps, VoltageCaps;

  if (!Cfg)
    return EFI_INVALID_PARAMETER;

  Regs = (struct SdxcRegs *)Cfg->SdxcBase;

  /* First Reset The SDXC Controller */
  SdxcReset(Regs);

  InternalMemZeroMem(&Cfg->Cfg, sizeof(Cfg->Cfg));

  VoltageCaps = 0;
  Caps = MmioReadBe32((UINTN)&Regs->Hostcapblt);

  Caps = Caps | SDXC_HOSTCAPBLT_VS33;

  if (Caps & SDXC_HOSTCAPBLT_VS30)
    VoltageCaps |= MMC_VDD_29_30 | MMC_VDD_30_31;
  if (Caps & SDXC_HOSTCAPBLT_VS33)
    VoltageCaps |= MMC_VDD_32_33 | MMC_VDD_33_34;
  if (Caps & SDXC_HOSTCAPBLT_VS18)
    VoltageCaps |= MMC_VDD_165_195;

  Cfg->Cfg.Name = "FSL_SDXC";
  Cfg->Cfg.Ops = &SdxcOps;
  Cfg->Cfg.Voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
  if ((Cfg->Cfg.Voltages & VoltageCaps) == 0) {
    DEBUG((EFI_D_ERROR, "Voltage Not Supported By Controller\n"));
    return EFI_DEVICE_ERROR;
  }

  Cfg->Cfg.HostCaps = MMC_MODE_4_BIT | MMC_MODE_8_BIT | MMC_MODE_HC;

  if (Cfg->MaxBusWidth > 0) {
    if (Cfg->MaxBusWidth < 8)
      Cfg->Cfg.HostCaps &= ~MMC_MODE_8_BIT;
    if (Cfg->MaxBusWidth < 4)
      Cfg->Cfg.HostCaps &= ~MMC_MODE_4_BIT;
  }

  if (Caps & SDXC_HOSTCAPBLT_HSS)
    Cfg->Cfg.HostCaps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

  GetSysInfo(&SocSysInfo);
  Cfg->Cfg.FMin = 400000;
  Cfg->Cfg.FMax = Min((UINT32)SocSysInfo.FreqSdhc, 52000000);
  Cfg->Cfg.BMax = CONFIG_SYS_MMC_MAX_BLK_COUNT;

  if (gMmc) {
    FreePool(gMmc->Cfg);
    FreePool(gMmc->Private);
    FreePool(gMmc);
  }

  gMmc = (struct Mmc *)CreateMmcNode(&Cfg->Cfg, Cfg);

  if (gMmc == NULL) {
    FreePool(Cfg);
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SdxcMmcInit (
  IN VOID
  )
{
  struct SdxcCfg *Cfg;
  struct SysInfo SocSysInfo;

  Cfg = (struct SdxcCfg *)AllocatePool(sizeof(struct SdxcCfg));
  InternalMemZeroMem(Cfg, sizeof(struct SdxcCfg));
  Cfg->SdxcBase = (VOID *)SDXC_BASE_ADDR;
 
  GetSysInfo(&SocSysInfo);
 
  Cfg->SdhcClk = (UINT32)SocSysInfo.FreqSdhc;
  return SdxcInitialize(Cfg);
}
