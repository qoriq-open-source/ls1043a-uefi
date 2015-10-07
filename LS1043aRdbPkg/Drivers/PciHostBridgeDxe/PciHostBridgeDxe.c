/** PciHostBridgeDxe.c
 Provides all functions of PCI Host Bridge Resource Allocation Protocol

 Based on PCI HosttBridge implementation in
 ArmPlatformPkg/ArmJunoPkg/Drivers/PciHostBridgeDxe/PciHostBridgeResourceAllocation.c

 Copyright (c) 2011-2015, ARM Ltd. All rights reserved.
 Copyright (c) 2015, Freescale Semiconductor, Inc. All rights reserved.

 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution. The full text of the license may be found 
 http://opensource.org/licenses/bsd-license.php

 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 

#include <PciHostBridge.h>
#define PCI_MEM32_BASE      FixedPcdGet64 (PcdPciMmio32Base)
#define PCI_MEM32_SIZE      FixedPcdGet64 (PcdPciMmio32Size)
#define PCI_MEM64_BASE      FixedPcdGet64 (PcdPciMmio64Base)
#define PCI_MEM64_SIZE      FixedPcdGet64 (PcdPciMmio64Size)

//
// Hard code: Root Bridge Number within the host bridge
//            Root Bridge's attribute
//            Root Bridge's device path
//            Root Bridge's resource aperture
//
UINTN RootBridgeNumber[3] = { 1, 1, 1 };

UINT64 RootBridgeAttribute[1][1] = { {EFI_PCI_HOST_BRIDGE_MEM64_DECODE | EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM} };

EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath1 = {
  {
        { ACPI_DEVICE_PATH,
          ACPI_DP,
          {
            (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
            (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
          }
        },
        EISA_PNP_ID(0x0A03),
        0
      },

      {
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        {
          END_DEVICE_PATH_LENGTH,
          0
        }
      }
};

EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath3 = {
  {
        { ACPI_DEVICE_PATH,
          ACPI_DP,
          {
            (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
            (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
          }
        },
        EISA_PNP_ID(0x0A03),
        0
      },

      {
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        {
          END_DEVICE_PATH_LENGTH,
          0
        }
      }
};

EFI_PCI_ROOT_BRIDGE_DEVICE_PATH mEfiPciRootBridgeDevicePath2 = {
  {
    {     ACPI_DEVICE_PATH,
          ACPI_DP,
          {
            (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
            (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8)
          }
        },
        EISA_PNP_ID(0x0A03),
        0
      },

      {
        END_DEVICE_PATH_TYPE,
        END_ENTIRE_DEVICE_PATH_SUBTYPE,
        {
          END_DEVICE_PATH_LENGTH,
          0
        }
      }
};
  
EFI_HANDLE mPciDriverImageHandle;

PCI_HOST_BRIDGE_INSTANCE gPciHostBridgeInstanceTemplate = {
  PCI_HOST_BRIDGE_SIGNATURE,	// Signature
  NULL,				// HostBridgeHandle
  NULL,				// ImageHandle
  {0, 0, 0},			// RootBridgeNumber
  NULL,				// RootBridgeInstance
  {NULL, NULL},			// Head
  FALSE,			// ResourceSubiteed
  TRUE,				// CanRestarted
  {				// ResAlloc
    PciNotifyPhase,
    PciGetNextRootBridge,
    PciGetAllocAttributes,
    PciStartBusEnumeration,
    PciSetBusNumbers,
    PciSubmitResources,
    PciGetProposedResources,
    PciPreprocessController
  }
};
  
PCI_HOST_BRIDGE_INSTANCE    *HostBridge[3];
PCI_ROOT_BRIDGE_INSTANCE    *PrivateData[3];

//
// Implementation
//

/**
  Entry point of this driver

  @param ImageHandle     Handle of driver image
  @param SystemTable     Point to EFI_SYSTEM_TABLE

  @retval EFI_ABORTED           PCI host bridge not present
  @retval EFI_OUT_OF_RESOURCES  Can not allocate memory resource
  @retval EFI_DEVICE_ERROR      Can not install the protocol instance
  @retval EFI_SUCCESS           Success to initialize the Pci host bridge.
**/
EFI_STATUS
EFIAPI
PciHostBridgeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                  Status;
  UINTN                       Loop1;
  UINTN                       Loop2;

  DEBUG ((EFI_D_INFO, "PCI HostBridgeInit Entry\n"));

  //
  // Create Host Bridge Device Handle
  //
  DEBUG ((EFI_D_INFO, "PCI : Going to Install Host Bridges\n"));
  for (Loop1 = 0; Loop1 < HOST_BRIDGE_NUMBER; Loop1++) {
    DEBUG ((EFI_D_INFO, "PCI : Allocate Host Bridge Num: %d\n", (Loop1 + 1)));
    HostBridge[Loop1] = AllocateCopyPool (sizeof(PCI_HOST_BRIDGE_INSTANCE), &gPciHostBridgeInstanceTemplate);
    if (HostBridge[Loop1] == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  
    HostBridge[Loop1]->RootBridgeNumber[Loop1] = RootBridgeNumber[Loop1];
    DEBUG ((EFI_D_INFO, "PCI : Allocated Pool for HostBridge: %d,RootBridge:%d\n", Loop1, RootBridgeNumber[Loop1]));
    InitializeListHead (&HostBridge[Loop1]->Head);
    DEBUG ((EFI_D_INFO, "PCI : Install Protocol for HostBridge: %d\n", Loop1));

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &HostBridge[Loop1]->HostBridgeHandle,              
                    &gEfiPciHostBridgeResourceAllocationProtocolGuid, &HostBridge[Loop1]->ResAlloc,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "%a: Fail to install resource alloc\n", __FUNCTION__));
      FreePool (HostBridge[Loop1]);
      return EFI_DEVICE_ERROR;
    } else {
      DEBUG ((EFI_D_INFO, "%a: Succeed to install resource alloc\n", __FUNCTION__));
    }   
    
    HostBridge[Loop1]->ImageHandle = ImageHandle; 
    //
    // Create Root Bridge Device Handle in this Host Bridge
    //
    for (Loop2 = 0; Loop2 < HostBridge[Loop1]->RootBridgeNumber[Loop1]; Loop2++) {
      DEBUG ((EFI_D_INFO, "PCI : Going to Allocate Root Bridge No: %d\n", RootBridgeNumber[Loop1]));
      PrivateData[Loop1] = AllocateZeroPool (sizeof(PCI_ROOT_BRIDGE_INSTANCE));
      if (PrivateData[Loop1] == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      PrivateData[Loop1]->Signature = PCI_ROOT_BRIDGE_SIGNATURE;
      if (Loop1 == 0) {
        CopyMem (&(PrivateData[Loop1]->DevicePath), &mEfiPciRootBridgeDevicePath1, sizeof (EFI_PCI_ROOT_BRIDGE_DEVICE_PATH));
        // Set Device Path for this Root Bridge
        PrivateData[Loop1]->DevicePath.AcpiDevicePath.UID = 0;
      }  
 
      PrivateData[Loop1]->Info = AllocateZeroPool (sizeof(struct LsPcieInfo));
      PrivateData[Loop1]->Pcie = AllocateZeroPool (sizeof(struct LsPcie));
      
      SetLSPcieInfo(PrivateData[Loop1]->Info, (Loop1+1));

      DEBUG ((EFI_D_INFO, "PCI : LsPcieInfo\n"));
      DEBUG ((EFI_D_INFO, "PCI : LsPcieInfo:PCIE1: %016llx, 2: %016llx, 3: %016llx\n",
	      CONFIG_SYS_PCIE1_PHYS_ADDR,
	      CONFIG_SYS_PCIE2_PHYS_ADDR,
	      CONFIG_SYS_PCIE3_PHYS_ADDR));
      DEBUG((EFI_D_INFO, "Loop1:%d PciNum:%d Info CFG Values: %016llx %016llx:%016llx %016llx %016llx\n",
             Loop1,			     
	     (UINT64)PrivateData[Loop1]->Info->PciNum,
	     (UINT64)PrivateData[Loop1]->Info->Regs,
	     (UINT64)PrivateData[Loop1]->Info->Cfg0Phys,
	     (UINT64)PrivateData[Loop1]->Info->Cfg0Size,
	     (UINT64)PrivateData[Loop1]->Info->Cfg1Phys,
	     (UINT64)PrivateData[Loop1]->Info->Cfg1Size));
      DEBUG((EFI_D_INFO, "Loop1:%d Info Mem Values: %016llx:%016llx %016llx\n",
	     Loop1,
	     (UINT64)PrivateData[Loop1]->Info->MemBus,
	     (UINT64)PrivateData[Loop1]->Info->MemPhys,
	     (UINT64)PrivateData[Loop1]->Info->MemSize));
      DEBUG((EFI_D_INFO, "Loop1:%d Info IO Values: %016llx:%016llx %016llx\n",
	     Loop1,
	     (UINT64)PrivateData[Loop1]->Info->IoBus,
	     (UINT64)PrivateData[Loop1]->Info->IoPhys,
	     (UINT64)PrivateData[Loop1]->Info->IoSize));

      DEBUG ((EFI_D_INFO, "Sending PciNum: %d\n", PrivateData[Loop1]->Info->PciNum));
      DEBUG ((EFI_D_INFO, "Sending Regs: %016lx\n", PrivateData[Loop1]->Info->Regs));
      HostBridge[Loop1]->RootBridge = PrivateData[Loop1];

      DEBUG ((EFI_D_INFO, "Installed Host Bridges successfully\n"));

      PciRbInitialize(
        PrivateData[Loop1], 
        (EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *)&PrivateData[Loop1]->Io, 
        PrivateData[Loop1]->Info, 
        HostBridge[Loop1]->HostBridgeHandle, 
        RootBridgeAttribute[1][1],
        (Loop1+1),	
	0
        );
   
      DEBUG ((EFI_D_INFO, "Going to Install MultipleProtocolInterfaces\n")); 
      Status = gBS->InstallMultipleProtocolInterfaces(
                      &PrivateData[Loop1]->Handle,              
                      &gEfiDevicePathProtocolGuid,      &PrivateData[Loop1]->DevicePath,
                      &gEfiPciRootBridgeIoProtocolGuid, &PrivateData[Loop1]->Io,
                      NULL
                      );
      if (EFI_ERROR (Status)) {
	DEBUG ((EFI_D_ERROR, "Failed to install protocol\n"));
        FreePool(PrivateData[Loop1]);
        return EFI_DEVICE_ERROR;
      }
      
      DEBUG ((EFI_D_INFO, "Successfully Installed protocol\n"));
      InsertTailList (&HostBridge[Loop1]->Head, &PrivateData[Loop1]->Link);
    }
  }

 // PCI 32bit Memory Space
  Status = gDS->AddMemorySpace (
          EfiGcdMemoryTypeMemoryMappedIo,
          PCI_MEM32_BASE,
          PCI_MEM32_SIZE,
          0
          );

  Status = gDS->AddMemorySpace (
          EfiGcdMemoryTypeMemoryMappedIo,
          PCI_MEM64_BASE,
          PCI_MEM64_SIZE,
          0
          );

  return EFI_SUCCESS;
}


/**
  Functions declarations for Host Bridge Resource allocation protocol

  @param ImageHandle     Handle of driver image
  @param SystemTable     Point to EFI_SYSTEM_TABLE

  @retval EFI_ABORTED           PCI host bridge not present
  @retval EFI_OUT_OF_RESOURCES  Can not allocate memory resource
  @retval EFI_DEVICE_ERROR      Can not install the protocol instance
  @retval EFI_SUCCESS           Success to initialize the Pci host bridge.
**/
EFI_STATUS
PciNotifyPhase (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE     Phase
  )
{
  PCI_HOST_BRIDGE_INSTANCE    *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE    *RootBridgeInstance;
  EFI_STATUS                  Status;
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  UINT64                      AddrLen;
  UINTN                       BitsOfAlignment;

  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);

  DEBUG((EFI_D_INFO, "Entered HB PCI NotifyPhase()\n"));

  // Check RootBridge Signature
  ASSERT (HostBridgeInstance->RootBridge->Signature == PCI_ROOT_BRIDGE_SIGNATURE);
  
  // The enumeration cannot be restarted after the process has been further than the first phase
  if (Phase == EfiPciHostBridgeBeginEnumeration) {
    if (!HostBridgeInstance->CanRestarted) {
      return EFI_NOT_READY;
    }
  } else {
    HostBridgeInstance->CanRestarted = FALSE;
  }

  switch (Phase) {
  case EfiPciHostBridgeBeginEnumeration:
    DEBUG((EFI_D_INFO, "PciNotifyPhase(EfiPciHostBridgeBeginEnumeration)\n"));
    RootBridgeInstance = HostBridgeInstance->RootBridge;
    break;

  case EfiPciHostBridgeBeginBusAllocation:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeBeginBusAllocation)\n"));
    // The bus allocation phase is about to begin
    break;

  case EfiPciHostBridgeEndBusAllocation:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeEndBusAllocation)\n"));
    // The bus allocation and bus programming phase is complete. All the PCI-to-PCI bridges have been given and written back
    // a bus number range into their configuration
    break;

  case EfiPciHostBridgeBeginResourceAllocation:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeBeginResourceAllocation)\n"));
    // The resource allocation phase is about to begin.
    break;

  case EfiPciHostBridgeAllocateResources:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeAllocateResources)\n"));
    // Allocates resources per previously submitted requests for all the PCI root bridges. The resources have been submitted to
    // PciHbRaSubmitResources() before.

    RootBridgeInstance = HostBridgeInstance->RootBridge;
    if (RootBridgeInstance->ResAlloc[ResTypeIo].Length != 0) {
      BitsOfAlignment = HighBitSet64 (RootBridgeInstance->ResAlloc[ResTypeIo].Alignment) + 1;     // Get the number of '1' in Alignment
      AddrLen = RootBridgeInstance->ResAlloc[ResTypeIo].Length;

      Status = gDS->AllocateIoSpace (
                  EfiGcdAllocateAnySearchBottomUp,
                  EfiGcdIoTypeIo,
                  BitsOfAlignment,
                  AddrLen,
                  &BaseAddress,
                  HostBridgeInstance->ImageHandle,
                  NULL
               );
      // If error then ResAlloc[n].Base ==0
      if (!EFI_ERROR (Status)) {
        RootBridgeInstance->ResAlloc[ResTypeIo].Base   = (UINTN)BaseAddress;
      }
    }

    if (RootBridgeInstance->ResAlloc[ResTypeMem32].Length != 0) {
      BitsOfAlignment = HighBitSet64 (RootBridgeInstance->ResAlloc[ResTypeMem32].Alignment) + 1;  // Get the number of '1' in Alignment
      AddrLen = RootBridgeInstance->ResAlloc[ResTypeMem32].Length;

      // Top of the 32bit PCI Memory space
      BaseAddress = FixedPcdGet32 (PcdPciMmio32Base) + FixedPcdGet32 (PcdPciMmio32Size);

      Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchTopDown,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  BitsOfAlignment,
                  AddrLen,
                  &BaseAddress,
                  HostBridgeInstance->ImageHandle,
                  NULL
               );

      // Ensure the allocation is in the 32bit PCI memory space
      if (!EFI_ERROR (Status) && (BaseAddress >= FixedPcdGet64 (PcdPciMmio32Base))) {
        RootBridgeInstance->ResAlloc[ResTypeMem32].Base   = (UINTN)BaseAddress;
      }
    }

    if (RootBridgeInstance->ResAlloc[ResTypePMem32].Length != 0) {
      BitsOfAlignment = HighBitSet64 (RootBridgeInstance->ResAlloc[ResTypePMem32].Alignment) + 1;  // Get the number of '1' in Alignment
      AddrLen = RootBridgeInstance->ResAlloc[ResTypePMem32].Length;

      // Top of the 32bit PCI Memory space
      BaseAddress = FixedPcdGet32 (PcdPciMmio32Base) + FixedPcdGet64 (PcdPciMmio32Size);

      Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchTopDown,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  BitsOfAlignment,
                  AddrLen,
                  &BaseAddress,
                  HostBridgeInstance->ImageHandle,
                  NULL
               );

      // Ensure the allocation is in the 32bit PCI memory space
      if (!EFI_ERROR (Status) && (BaseAddress >= FixedPcdGet64 (PcdPciMmio32Base))) {
        RootBridgeInstance->ResAlloc[ResTypePMem32].Base = (UINTN)BaseAddress;
      }
    }

    if (RootBridgeInstance->ResAlloc[ResTypeMem64].Length != 0) {
      BitsOfAlignment = HighBitSet64 (RootBridgeInstance->ResAlloc[ResTypeMem64].Alignment) + 1;  // Get the number of '1' in Alignment
      AddrLen = RootBridgeInstance->ResAlloc[ResTypeMem64].Length;

      // Top of the 64bit PCI Memory space
      BaseAddress = FixedPcdGet64 (PcdPciMmio64Base) + FixedPcdGet64 (PcdPciMmio64Size);
      Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchTopDown,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  BitsOfAlignment,
                  AddrLen,
                  &BaseAddress,
                  HostBridgeInstance->ImageHandle,
                  NULL
               );

      // Ensure the allocation is in the 64bit PCI memory space
      if (!EFI_ERROR (Status) && (BaseAddress >= FixedPcdGet64 (PcdPciMmio64Base))) {
        RootBridgeInstance->ResAlloc[ResTypeMem64].Base   = (UINTN)BaseAddress;
      }
    }
    if (RootBridgeInstance->ResAlloc[ResTypePMem64].Length != 0) {
      BitsOfAlignment = HighBitSet64 (RootBridgeInstance->ResAlloc[ResTypePMem64].Alignment) + 1;  //Get the number of '1' in Alignment
      AddrLen = RootBridgeInstance->ResAlloc[ResTypePMem64].Length;

      // Top of the 64bit PCI Memory space
      BaseAddress = FixedPcdGet64 (PcdPciMmio64Base) + FixedPcdGet64 (PcdPciMmio64Size);
      Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchTopDown,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  BitsOfAlignment,
                  AddrLen,
                  &BaseAddress,
                  HostBridgeInstance->ImageHandle,
                  NULL
               );

      // Ensure the allocation is in the 64bit PCI memory space
      if (!EFI_ERROR (Status) && (BaseAddress >= FixedPcdGet64 (PcdPciMmio64Base))) {
        RootBridgeInstance->ResAlloc[ResTypePMem64].Base   = (UINTN)BaseAddress;
      }
    }

    break;

  case EfiPciHostBridgeSetResources:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeSetResources)\n"));
    // Programs the host bridge hardware to decode previously allocated resources (proposed resources)
    // for all the PCI root bridges. The PCI bus driver will now program the resources
    break;

  case EfiPciHostBridgeFreeResources:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeFreeResources)\n"));
    // Deallocates resources that were previously allocated for all the PCI root bridges and resets the
    // I/O and memory apertures to their initial state.*/
    break;

  case EfiPciHostBridgeEndResourceAllocation:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeEndResourceAllocation)\n"));
    break;

  case EfiPciHostBridgeEndEnumeration:
    DEBUG((EFI_D_INFO, "PciHbRaNotifyPhase(EfiPciHostBridgeEndEnumeration)\n"));
    break;

  default:
    DEBUG ((EFI_D_INFO, "PciHbRaNotifyPhase(Phase:%d)\n", Phase));
    ASSERT (0);
  }

  return EFI_SUCCESS;

}

/**
 * PciHbRaGetNextRootBridge() returns the next root bridge attached to the 'This' PCI Host Bridge.
 * As we have only got one PCI Root Bridge in this PCI interface, we return either this root bridge
 * if it the first time we call this function (*RootBridgeHandle == NULL) or we return EFI_NOT_FOUND
 **/
EFI_STATUS
PciGetNextRootBridge (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN OUT EFI_HANDLE                                    *RootBridgeHandle
  )
{
  PCI_HOST_BRIDGE_INSTANCE              *HostBridgeInstance;

  DEBUG((EFI_D_INFO, "PciHbRaGetNextRootBridge()\n"));

  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  ASSERT (HostBridgeInstance->RootBridge != NULL);

  //Check RootBridge Signature
  ASSERT (HostBridgeInstance->RootBridge->Signature == PCI_ROOT_BRIDGE_SIGNATURE);

  if (*RootBridgeHandle == NULL) {
    *RootBridgeHandle = HostBridgeInstance->RootBridge->Handle;
    return EFI_SUCCESS;
  } else if (*RootBridgeHandle == HostBridgeInstance->RootBridge->Handle) {
    return EFI_NOT_FOUND;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

/** PciHbRaGetAllocAttributes() returns the resource allocation attributes supported by this PCI Root Bridge.
 *  A PCI Root bridge could support these types :
 *      - EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM : does not support separate windows for nonprefetchable and prefetchable memory.
 *      - EFI_PCI_HOST_BRIDGE_MEM64_DECODE : supports 64-bit memory windows
 **/
EFI_STATUS
PciGetAllocAttributes (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN  EFI_HANDLE                                        RootBridgeHandle,
  OUT UINT64                                           *Attributes
  )
{
  PCI_HOST_BRIDGE_INSTANCE    *HostBridgeInstance;

  DEBUG((EFI_D_INFO, "PciHbRaGetAllocAttributes()\n"));

  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);

  // Check if the RootBridgeHandle is the one managed by this PCI Host Bridge
  ASSERT (HostBridgeInstance->RootBridge != NULL);
  if (HostBridgeInstance->RootBridge->Handle != RootBridgeHandle) {
    return EFI_INVALID_PARAMETER;
  }

  *Attributes = HostBridgeInstance->RootBridge->RootBridgeAttrib;
  return EFI_SUCCESS;
}





EFI_STATUS
PciStartBusEnumeration (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN  EFI_HANDLE                                        RootBridgeHandle,
  OUT VOID                                            **Configuration
  )
{
  VOID    *Buffer;
  UINT8   *Ptr;
  PCI_HOST_BRIDGE_INSTANCE    *HostBridgeInstance;

  // Fill an ACPI descriptor table with the Bus Number Range. This information will be used by the PCI Bus driver
  // to set bus numbers to PCI-to-PCI bridge.
  DEBUG((EFI_D_INFO, "Pci Host Bridge StartBusEnumeration()\n"));

  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);

  Buffer = AllocateZeroPool (sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) + sizeof (EFI_ACPI_END_TAG_DESCRIPTOR));
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Ptr = (UINT8 *)Buffer;

  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->Desc = ACPI_ADDRESS_SPACE_DESCRIPTOR;    // QWORD Address space Descriptor
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->Len  = 0x2B;    // Length of this descriptor in bytes not including the first two fields
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->ResType = ACPI_ADDRESS_SPACE_TYPE_BUS;  // Resource Type Bus Number Range
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->GenFlag = 0;
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->SpecificFlag = 0;
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrSpaceGranularity = 0;
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrRangeMin = HostBridgeInstance->RootBridge->BusStart;   // Bus Start
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrRangeMax = 0;    // Bus Max
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrTranslationOffset = 0;
  ((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrLen = FixedPcdGet32 (PcdPciBusMax) - FixedPcdGet32 (PcdPciBusMin) + 1;

  Ptr = Ptr + sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
  ((EFI_ACPI_END_TAG_DESCRIPTOR *)Ptr)->Desc = ACPI_END_TAG_DESCRIPTOR;
  ((EFI_ACPI_END_TAG_DESCRIPTOR *)Ptr)->Checksum = 0x0;

  *Configuration = Buffer;
  return EFI_SUCCESS;
}


EFI_STATUS
PciSetBusNumbers (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN EFI_HANDLE                                         RootBridgeHandle,
  IN VOID                                              *Configuration
  )
{
  PCI_HOST_BRIDGE_INSTANCE    *HostBridgeInstance;
  UINT8                       *Ptr;
  UINTN                       BusStart;
  UINTN                       BusEnd;
  UINTN                       BusLen;

  DEBUG((EFI_D_INFO,"PciHbRaSetBusNumbers()\n"));

  Ptr = Configuration;
  if (*Ptr != ACPI_ADDRESS_SPACE_DESCRIPTOR) {
    return EFI_INVALID_PARAMETER;
  }

  // Check if the passed ACPI descriptor table define a Bus Number Range
  if (((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->ResType != ACPI_ADDRESS_SPACE_TYPE_BUS) {
    return EFI_INVALID_PARAMETER;
  }

  // Check if the Configuration only passed one ACPI Descriptor (+ End Descriptor)
  if (*((UINT8*)(Ptr + sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR))) != ACPI_END_TAG_DESCRIPTOR) {
    return EFI_INVALID_PARAMETER;
  }

  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
  ASSERT (HostBridgeInstance->RootBridge != NULL);
  if (HostBridgeInstance->RootBridge->Handle != RootBridgeHandle) {
    return EFI_INVALID_PARAMETER;
  }

  BusStart = (UINTN)((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrRangeMin;
  BusLen = (UINTN)((EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr)->AddrLen;
  BusEnd = BusStart + BusLen - 1;

  ASSERT (BusStart <= BusEnd); // We should at least have PCI_BUS_ROOT and PCI_SWITCH_BUS
  ASSERT ((BusStart >= HostBridgeInstance->RootBridge->BusStart) && (BusLen <= HostBridgeInstance->RootBridge->BusLength));

  HostBridgeInstance->RootBridge->BusStart  = BusStart;
  HostBridgeInstance->RootBridge->BusLength = BusLen;

  return EFI_SUCCESS;
}


/**
 * This function is used to submit all the I/O and memory resources that are required by the specified
 * PCI root bridge.
 **/
EFI_STATUS
PciSubmitResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN EFI_HANDLE                                         RootBridgeHandle,
  IN VOID                                              *Configuration
  )
{
  PCI_HOST_BRIDGE_INSTANCE            *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE            *RootBridgeInstance;
  UINT8                               *Ptr;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR   *Desc;
  PCI_RESOURCE_TYPE                   ResType;
 
  DEBUG((EFI_D_INFO, "PciHbRaSubmitResources()\n"));
  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
 
  if (Configuration == NULL) {
    DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): NULL COnf\n"));
    return EFI_INVALID_PARAMETER;
  }

  // Check if the ACPI Descriptor tables is conformed
  Ptr = (UINT8 *)Configuration;
  while (*Ptr == ACPI_ADDRESS_SPACE_DESCRIPTOR) { // QWORD Address Space descriptor
    Ptr += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) ;
  }
  if (*Ptr != ACPI_END_TAG_DESCRIPTOR) { // End tag
    DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): END tag issue\n"));
    return EFI_INVALID_PARAMETER;
  }

  // Check the RootBridgeHandle
  RootBridgeInstance = HostBridgeInstance->RootBridge;
  ASSERT (RootBridgeInstance != NULL);
  if (RootBridgeHandle != HostBridgeInstance->RootBridge->Handle) {
    DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): HB/RB dont match\n"));
    return EFI_INVALID_PARAMETER;
  }

  Ptr = (UINT8 *)Configuration;
  while ( *Ptr == ACPI_ADDRESS_SPACE_DESCRIPTOR) { // While the entry is an ACPI Descriptor Table
    Desc = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr;

    // Check if the description is valid
    if (Desc->AddrLen > 0xffffffff) {
      DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): Invalid addr length\n"));
      return EFI_INVALID_PARAMETER;
    }

    if ((Desc->AddrRangeMax >= 0xffffffff) || (Desc->AddrRangeMax != (GetPowerOfTwo64 (Desc->AddrRangeMax + 1) - 1))) {
      DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): Invalid addr range\n"));
      return EFI_INVALID_PARAMETER;
    }

    switch (Desc->ResType) {
    case ACPI_ADDRESS_SPACE_TYPE_MEM:
      // Check invalid Address Space Granularity
      if ((Desc->AddrSpaceGranularity != 32) && (Desc->AddrSpaceGranularity != 64)) {
        DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): Invalid addr space granul\n"));
        return EFI_INVALID_PARAMETER;
      }

      // check the memory resource request is supported by PCI root bridge
      if (RootBridgeInstance->MemAllocAttributes == EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM && Desc->SpecificFlag == 0x06) {
        DEBUG((EFI_D_INFO, "PciHbRaSubmitResources(): Invalid memalloc attri \n"));
        return EFI_INVALID_PARAMETER;
      }

      if (Desc->AddrSpaceGranularity == 32) {
        if (Desc->SpecificFlag == ACPI_SPECFLAG_PREFETCHABLE) {
          ResType = ResTypePMem32;
        } else {
          ResType = ResTypeMem32;
        }
      } else {
        if (Desc->SpecificFlag == ACPI_SPECFLAG_PREFETCHABLE) {
          ResType = ResTypePMem64;
        } else {
          ResType = ResTypeMem64;
        }
      }
      RootBridgeInstance->ResAlloc[ResType].Length = Desc->AddrLen;
      RootBridgeInstance->ResAlloc[ResType].Alignment = Desc->AddrRangeMax;
      RootBridgeInstance->ResAlloc[ResType].Base = Desc->AddrRangeMin;
      break;
    case ACPI_ADDRESS_SPACE_TYPE_IO:
      RootBridgeInstance->ResAlloc[ResTypeIo].Length = Desc->AddrLen;
      RootBridgeInstance->ResAlloc[ResTypeIo].Alignment = Desc->AddrRangeMax;
      RootBridgeInstance->ResAlloc[ResTypeIo].Base = 0;
      break;
    default:
      ASSERT (0); // Could be the case Desc->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS
      break;
    }
    Ptr += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
  }


  return EFI_SUCCESS;
}



/** Returns the proposed resource settings for the specified PCI root bridge. The resources have been submitted by
  * PciHbRaSubmitResources() before
  **/
EFI_STATUS
PciGetProposedResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN  EFI_HANDLE                                        RootBridgeHandle,
  OUT VOID                                            **Configuration
  )
{
  PCI_HOST_BRIDGE_INSTANCE            *HostBridgeInstance;
  PCI_ROOT_BRIDGE_INSTANCE            *RootBridgeInstance;
  UINT32                              i;
  UINT32                              ResAllocCount;
  VOID                                *Buffer;
  UINT8                               *Ptr;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR   *Desc;

  DEBUG((EFI_D_INFO, "PciHbRaGetProposedResources()\n"));
  HostBridgeInstance = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);

  // Check the RootBridgeHandle
  RootBridgeInstance = HostBridgeInstance->RootBridge;
  ASSERT (RootBridgeInstance != NULL);
  if (RootBridgeHandle != HostBridgeInstance->RootBridge->Handle) {
    return EFI_INVALID_PARAMETER;
  }

  // Count the number of Resource Allocated for this Root Bridge
  ResAllocCount = 0;
  for (i = 0; i < ResTypeMax; i++) {
    if (RootBridgeInstance->ResAlloc[i].Length != 0)  ResAllocCount++;
  }

  if (ResAllocCount ==  0) {
    return EFI_INVALID_PARAMETER;
  }

  Buffer = AllocateZeroPool (ResAllocCount * sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR) + sizeof (EFI_ACPI_END_TAG_DESCRIPTOR));
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Ptr = Buffer;
  for (i = 0; i < ResTypeMax; i++) {
    DEBUG((EFI_D_INFO, "GetProposedResources:%d\n", ResTypeMax));
    if (RootBridgeInstance->ResAlloc[i].Length != 0) {    // Base != 0 if the resource has been allocated
      Desc = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Ptr;

      Desc->Desc = ACPI_ADDRESS_SPACE_DESCRIPTOR;
      Desc->Len  = 0x2B;
      Desc->GenFlag = 0;
      Desc->AddrRangeMax = 0;

      switch (i) {
      case ResTypeIo:
        Desc->Desc = 0x8A;
        Desc->Len  = 0x2B;
        Desc->ResType = 1;
        Desc->GenFlag = 0;
        Desc->SpecificFlag = 0;
        Desc->AddrSpaceGranularity = 0;
        Desc->AddrRangeMin = RootBridgeInstance->ResAlloc[i].Base;
        Desc->AddrLen = RootBridgeInstance->ResAlloc[i].Length;
        break;
      case ResTypeMem32:
        Desc->ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
        Desc->Desc = 0x8A;
        Desc->Len  = 0x2B;
        Desc->ResType = 0;
        Desc->GenFlag = 0;
        Desc->SpecificFlag = 0;
        Desc->AddrSpaceGranularity = 32;
        Desc->AddrRangeMin = RootBridgeInstance->ResAlloc[i].Base;
        Desc->AddrLen = RootBridgeInstance->ResAlloc[i].Length;
        break;
      case ResTypePMem32:
        Desc->Desc = 0x8A;
        Desc->Len  = 0x2B;
        Desc->ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
        Desc->GenFlag = 0;
        Desc->SpecificFlag = 6;
	Desc->AddrSpaceGranularity = 32;
        Desc->AddrRangeMin = 0;
        Desc->AddrTranslationOffset = EFI_RESOURCE_NONEXISTENT;
        Desc->AddrLen = 0;
        break;
      case ResTypeMem64:
        Desc->Desc = 0x8A;
        Desc->Len  = 0x2B;
        Desc->ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
        Desc->GenFlag = 0;
        Desc->SpecificFlag = 0;
        Desc->AddrSpaceGranularity = 64;
        Desc->AddrRangeMin = RootBridgeInstance->ResAlloc[i].Base;
        Desc->AddrLen = RootBridgeInstance->ResAlloc[i].Length;
        break;
      case ResTypePMem64:
        Desc->Desc = 0x8A;
        Desc->Len  = 0x2B;
        Desc->ResType = ACPI_ADDRESS_SPACE_TYPE_MEM;
        Desc->GenFlag = 0;
        Desc->SpecificFlag = 6;
	Desc->AddrSpaceGranularity = 64;
        Desc->AddrRangeMin = 0;
        Desc->AddrTranslationOffset = EFI_RESOURCE_NONEXISTENT;
        Desc->AddrLen = 0;
        break;
      }
      Desc->AddrRangeMin = RootBridgeInstance->ResAlloc[i].Base;
      Desc->AddrTranslationOffset = EFI_RESOURCE_SATISFIED;
      Desc->AddrLen = RootBridgeInstance->ResAlloc[i].Length;
      Ptr += sizeof (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR);
    }
  }

  ((EFI_ACPI_END_TAG_DESCRIPTOR *)Ptr)->Desc = ACPI_END_TAG_DESCRIPTOR;
  ((EFI_ACPI_END_TAG_DESCRIPTOR *)Ptr)->Checksum = 0x0;

  *Configuration = Buffer;
  return EFI_SUCCESS;
}


EFI_STATUS
PciPreprocessController (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN  EFI_HANDLE                                        RootBridgeHandle,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS       PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE      Phase
  )
{
  PCI_HOST_BRIDGE_INSTANCE*   HostBridge;
  PCI_ROOT_BRIDGE_INSTANCE*   RootBridge;
  UINT32                      CapabilityPtr;
  UINT32                      CapabilityEntry;
  UINT16                      CapabilityID;
  UINT32                      DeviceCapability;

  DEBUG((EFI_D_INFO, "PciHbRaPreprocessController()\n"));

  if (FeaturePcdGet (PcdPciMaxPayloadFixup)) {
    // Do Max payload fixup for every devices
    if (Phase == EfiPciBeforeResourceCollection) {
      // Get RootBridge Instance from Host Bridge Instance
      HostBridge = INSTANCE_FROM_RESOURCE_ALLOCATION_THIS (This);
      RootBridge = HostBridge->RootBridge;

      // Get the first PCI Capability
      CapabilityPtr = PCI_CAPBILITY_POINTER_OFFSET;
      RootBridge->Io.Pci.Read (
          &RootBridge->Io,
          EfiPciWidthUint8,
          EFI_PCI_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, CapabilityPtr),
          1,
          &CapabilityPtr
      );
      CapabilityPtr &= 0x1FF;

      // Get Pci Express Capability
      while (CapabilityPtr != 0) {
        RootBridge->Io.Pci.Read (
            &RootBridge->Io,
            EfiPciWidthUint16,
            EFI_PCI_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, CapabilityPtr),
            1,
            &CapabilityEntry
            );

        CapabilityID = (UINT8)CapabilityEntry;

        // Is PCIe capability ?
        if (CapabilityID == EFI_PCI_CAPABILITY_ID_PCIEXP) {
          // Get PCIe Device Capabilities
          RootBridge->Io.Pci.Read (
              &RootBridge->Io,
              EfiPciWidthUint32,
              EFI_PCI_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, CapabilityPtr + 0x8),
              1,
              &DeviceCapability
              );

          // Force the Max Payload to 128 Bytes (128 Bytes Max Payload Size = 0)
          DeviceCapability &= ~ ((UINT32)(0x7 << 5 ));
          // Max Read Request Size to 128 Bytes (128 Bytes Max Read Request Size = 0)
          DeviceCapability &= ~ ((UINT32)(0x7 << 12));
          // Enable all error reporting
          DeviceCapability |= 0xF;

          RootBridge->Io.Pci.Write (
              &RootBridge->Io,
              EfiPciWidthUint32,
              EFI_PCI_ADDRESS (PciAddress.Bus, PciAddress.Device, PciAddress.Function, CapabilityPtr + 0x8),
              1,
              &DeviceCapability
              );

          return EFI_SUCCESS;
        }
        CapabilityPtr = (CapabilityEntry >> 8) & 0xFF;
      }
    }
  }

  return EFI_SUCCESS;
}
