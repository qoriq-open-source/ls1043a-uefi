/** @file
  Declaration of internal functions for Base Memory Library.

  The following BaseMemoryLib instances contain the same copy of this file:
    BaseMemoryLib
    BaseMemoryLibMmx
    BaseMemoryLibSse2
    BaseMemoryLibRepStr
    BaseMemoryLibOptDxe
    BaseMemoryLibOptPei

  Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __MEM_LIB_INTERNALS__
#define __MEM_LIB_INTERNALS__

#include <Base.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  Copy Length bytes from Source to Destination.

  @param  DestinationBuffer Target of copy
  @param  SourceBuffer      Place to copy from
  @param  Length            Number of bytes to copy

  @return Destination

**/
VOID *
EFIAPI
InternalMemCopyMem (
  OUT     VOID                      *DestinationBuffer,
  IN      CONST VOID                *SourceBuffer,
  IN      UINTN                     Length
  );

/**
  Set Buffer to Value for Size bytes.

  @param  Buffer   Memory to set.
  @param  Length   Number of bytes to set
  @param  Value    Value of the set operation.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT8                     Value
  );

/**
  Fills a target buffer with a 16-bit value, and returns the target buffer.

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Count of 16-bit value to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem16 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  );

/**
  Fills a target buffer with a 32-bit value, and returns the target buffer.

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Count of 32-bit value to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem32 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT32                    Value
  );

/**
  Fills a target buffer with a 64-bit value, and returns the target buffer.

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Count of 64-bit value to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem64 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT64                    Value
  );

/**
  Set Buffer to 0 for Size bytes.

  @param  Buffer Memory to set.
  @param  Length Number of bytes to set

  @return Buffer

**/
VOID *
EFIAPI
InternalMemZeroMem (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length
  );

/**
  Compares two memory buffers of a given length.

  @param  DestinationBuffer First memory buffer
  @param  SourceBuffer      Second memory buffer
  @param  Length            Length of DestinationBuffer and SourceBuffer memory
                            regions to compare. Must be non-zero.

  @return 0                 All Length bytes of the two buffers are identical.
  @retval Non-zero          The first mismatched byte in SourceBuffer subtracted from the first
                            mismatched byte in DestinationBuffer.

**/
INTN
EFIAPI
InternalMemCompareMem (
  IN      CONST VOID                *DestinationBuffer,
  IN      CONST VOID                *SourceBuffer,
  IN      UINTN                     Length
  );

/**
  Scans a target buffer for an 8-bit value, and returns a pointer to the
  matching 8-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Count of 8-bit value to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem8 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT8                     Value
  );

/**
  Scans a target buffer for a 16-bit value, and returns a pointer to the
  matching 16-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Count of 16-bit value to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem16 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  );

/**
  Scans a target buffer for a 32-bit value, and returns a pointer to the
  matching 32-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Count of 32-bit value to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem32 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT32                    Value
  );

/**
  Scans a target buffer for a 64-bit value, and returns a pointer to the
  matching 64-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Count of 64-bit value to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem64 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT64                    Value
  );

/**
  Checks whether the contents of a buffer are all zeros.

  @param  Buffer  The pointer to the buffer to be checked.
  @param  Length  The size of the buffer (in bytes) to be checked.

  @retval TRUE    Contents of the buffer are all zeros.
  @retval FALSE   Contents of the buffer are not all zeros.

**/
BOOLEAN
EFIAPI
InternalMemIsZeroBuffer (
  IN CONST VOID  *Buffer,
  IN UINTN       Length
  );

#endif
