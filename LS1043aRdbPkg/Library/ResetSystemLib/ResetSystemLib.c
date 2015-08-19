/** ResetSystemLib.c
  Do a generic Cold Reset for LS1043a
  
  Copyright (c) 2013, Freescale Ltd. All rights reserved.
  Author: Bhupesh Sharma <bhupesh.sharma@freescale.com>

  
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>

#include <Library/ArmLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <LS1043aRdb.h>

VOID
ShutdownEfi (
  VOID
  )
{
  EFI_STATUS              Status;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   MapKey;
  UINTN                   DescriptorSize;
  UINT32                   DescriptorVersion;
  UINTN                   Pages;

  MemoryMap = NULL;
  MemoryMapSize = 0;
  do {
    Status = gBS->GetMemoryMap (
                    &MemoryMapSize,
                    MemoryMap,
                    &MapKey,
                    &DescriptorSize,
                    &DescriptorVersion
                    );
    if (Status == EFI_BUFFER_TOO_SMALL) {

      Pages = EFI_SIZE_TO_PAGES (MemoryMapSize) + 1;
      MemoryMap = AllocatePages (Pages);
    
      //
      // Get System MemoryMap
      //
      Status = gBS->GetMemoryMap (
                      &MemoryMapSize,
                      MemoryMap,
                      &MapKey,
                      &DescriptorSize,
                      &DescriptorVersion
                      );
      // Don't do anything between the GetMemoryMap() and ExitBootServices()
      if (!EFI_ERROR (Status)) {
        Status = gBS->ExitBootServices (gImageHandle, MapKey);
        if (EFI_ERROR (Status)) {
          FreePages (MemoryMap, Pages);
          MemoryMap = NULL;
          MemoryMapSize = 0;
        }
      }
    }
  } while (EFI_ERROR (Status));

  //Clean and invalidate caches.
  WriteBackInvalidateDataCache();
  InvalidateInstructionCache();

  //Turning off Caches and MMU
  ArmDisableDataCache ();
  ArmDisableInstructionCache ();
  ArmDisableMmu ();
}

typedef
VOID
(EFIAPI *CALL_STUB)(
  VOID
);


/**
  Resets the entire platform.

  @param  ResetType             The type of reset to perform.
  @param  ResetStatus           The status code for the reset.
  @param  DataSize              The size, in bytes, of WatchdogData.
  @param  ResetData             For a ResetType of EfiResetCold, EfiResetWarm, or
                                EfiResetShutdown the data buffer starts with a Null-terminated
                                Unicode string, optionally followed by additional binary data.

**/
EFI_STATUS
EFIAPI
LibResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN CHAR16           *ResetData OPTIONAL
  )
{
  CALL_STUB   StartOfFv;
	UINT16 Val;

  if (ResetData != NULL) {
    DEBUG((EFI_D_ERROR, "%s", ResetData));
  }

  ShutdownEfi ();

  switch (ResetType) {
  case EfiResetWarm:
    //Perform warm reset of the system by jumping to the begining of the FV
    StartOfFv = (CALL_STUB)(UINTN)PcdGet32(PcdFvBaseAddress);
    StartOfFv ();
    break;
  case EfiResetCold:
  case EfiResetShutdown:
  default:
    //Perform cold reset of the system.
		Val = MmioReadBe16 (WDOG1_BASE_ADDR + WDOG_WCR_OFFSET);
		Val &= 0xff;
		Val |= WDOG_WCR_WDE;
    MmioWriteBe16 (WDOG1_BASE_ADDR + WDOG_WCR_OFFSET, Val);
		MmioWriteBe16(WDOG1_BASE_ADDR + WDOG_WSR_OFFSET, WDOG_SERVICE_SEQ1);
		MmioWriteBe16(WDOG1_BASE_ADDR + WDOG_WSR_OFFSET, WDOG_SERVICE_SEQ2);
		/* FIXME: Need to put a delay > 0.5 secs here, or else ASSERT fails */
    break;
  }

  // If the reset didn't work, return an error.
  ASSERT (FALSE);
  return EFI_DEVICE_ERROR;
}
  


/**
  Initialize any infrastructure required for LibResetSystem () to function.

  @param  ImageHandle   The firmware allocated handle for the EFI image.
  @param  SystemTable   A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
LibInitializeResetSystem (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EFI_SUCCESS;
}
