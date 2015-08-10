#include <Library/Sdxc.h>

extern struct Mmc *gMmc;

EFI_STATUS
DetectMmcPresence (
  OUT UINT8*  Data
  )
{
  if (MmcGetcd() == 0) {
    DEBUG((EFI_D_ERROR, "MMC, No Card Present\n"));
    *Data = FALSE;
    return EFI_NO_MEDIA;
  } else {
    *Data = TRUE;
    return EFI_SUCCESS;
  }
}

EFI_STATUS
MmcRcvResp (
  IN   UINT32   RespType,
  OUT  UINT32*  Buffer
  )
{
  EFI_STATUS Err;

  Err = ReceiveResponse(NULL, RespType, Buffer);

  return Err;
}

EFI_STATUS
MmcSendCommand (
  IN  struct MmcCmd *Cmd
  )
{
  return SdxcSendCmd(gMmc, Cmd, NULL);
}

EFI_STATUS
MmcSendReset (
  VOID
  )
{
  return MmcGoIdle(gMmc);
}

EFI_STATUS
InitMmc (
  IN EFI_BLOCK_IO_MEDIA *Media
  )
{
  EFI_STATUS Status;

  Status = DoMmcInfo();

  if (Status == EFI_SUCCESS) {
    Media->BlockSize = gMmc->BlockDev.Blksz;
    Media->LastBlock = gMmc->BlockDev.Lba;
  }

  return Status;
}

VOID
DestroyMmc (
  IN VOID
  )
{
  if (gMmc) {
    FreePool(gMmc->Cfg);
    FreePool(gMmc->Priv);
    FreePool(gMmc);
  }
}

VOID
SelectSdxc (
  IN VOID
  )
{
  UINT8 Data = 0;

  /* Enable soft mux */
  Data = CPLD_READ(soft_mux_on);
  if ((Data & (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX)) 
	!= (ENABLE_SDXC_SOFT_MUX | ENABLE_RCW_SOFT_MUX))
    CPLD_WRITE(soft_mux_on, (Data | (ENABLE_SDXC_SOFT_MUX |
					ENABLE_RCW_SOFT_MUX)));

  /* Enable sdhc */
  Data = CPLD_READ(sdhc_spics_sel);
  if ((Data & 0x01) != 0x00)
    CPLD_WRITE(sdhc_spics_sel, (Data & 0xFE));

  /* Enable sdhc clock */
  Data = CPLD_READ(tdmclk_mux_sel);
  if ((Data & 0x01) != 0x01)
    CPLD_WRITE(tdmclk_mux_sel, (Data | 0x01));

  /* configure SW4 and SW5 for SDXC/eMMC */
  Data = CPLD_READ(cfg_rcw_src1);
  if ((Data & SELECT_SW4_SDXC) != SELECT_SW4_SDXC)
    CPLD_WRITE(cfg_rcw_src1, SELECT_SW4_SDXC);

  Data = CPLD_READ(cfg_rcw_src2);
  if ((Data & SELECT_SW5_SDXC) != SELECT_SW5_SDXC)
    CPLD_WRITE(cfg_rcw_src2, SELECT_SW5_SDXC);
}

EFI_STATUS
MmcInitialize (
  VOID
  )
{
  SelectSdxc();

  if (BoardMmcInit() < 0)
    if (SdxcMmcInit() < 0)
      return EFI_DEVICE_ERROR;

  return EFI_SUCCESS;
}
