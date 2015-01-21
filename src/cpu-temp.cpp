/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

#include "cpu-temp.hpp"


// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
  CPUTemp::CPUTemp()
  {
  }

  static io_connect_t conn;

  UInt32
  CPUTemp::_strtoul(char *str, int size, int base)
  {
      UInt32 total = 0;
      int i;

      for (i = 0; i < size; i++)
      {
          if (base == 16)
              total += str[i] << (size - 1 - i) * 8;
          else
             total += (unsigned char) (str[i] << (size - 1 - i) * 8);
      }
      return total;
  }

  void
  CPUTemp::_ultostr(char *str, UInt32 val)
  {
      str[0] = '\0';
      sprintf(str, "%c%c%c%c",
              (unsigned int) val >> 24,
              (unsigned int) val >> 16,
              (unsigned int) val >> 8,
              (unsigned int) val);
  }

  kern_return_t
  CPUTemp::SMCOpen(void)
  {
      kern_return_t result;
      io_iterator_t iterator;
      io_object_t   device;

      CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
      result = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictionary, &iterator);
      if (result != kIOReturnSuccess)
      {
          printf("Error: IOServiceGetMatchingServices() = %08x\n", result);
          return 1;
      }

      device = IOIteratorNext(iterator);
      IOObjectRelease(iterator);
      if (device == 0)
      {
          printf("Error: no SMC found\n");
          return 1;
      }

      result = IOServiceOpen(device, mach_task_self(), 0, &conn);
      IOObjectRelease(device);
      if (result != kIOReturnSuccess)
      {
          printf("Error: IOServiceOpen() = %08x\n", result);
          return 1;
      }

      return kIOReturnSuccess;
  }

  kern_return_t
  CPUTemp::SMCClose()
  {
      return IOServiceClose(conn);
  }


  kern_return_t
  CPUTemp::SMCCall(int index, SMCKeyData_t *inputStructure, SMCKeyData_t *outputStructure)
  {
      size_t   structureInputSize;
      size_t   structureOutputSize;

      structureInputSize = sizeof(SMCKeyData_t);
      structureOutputSize = sizeof(SMCKeyData_t);

      #if MAC_OS_X_VERSION_10_5
      return IOConnectCallStructMethod( conn, index,
                              // inputStructure
                              inputStructure, structureInputSize,
                              // ouputStructure
                              outputStructure, &structureOutputSize );
      #else
      return IOConnectMethodStructureIStructure0( conn, index,
                                                  structureInputSize, /* structureInputSize */
                                                  &structureOutputSize,   /* structureOutputSize */
                                                  inputStructure,        /* inputStructure */
                                                  outputStructure);       /* ouputStructure */
      #endif

  }

  kern_return_t
  CPUTemp::SMCReadKey(UInt32Char_t key, SMCVal_t *val)
  {
      kern_return_t result;
      SMCKeyData_t  inputStructure;
      SMCKeyData_t  outputStructure;

      memset(&inputStructure, 0, sizeof(SMCKeyData_t));
      memset(&outputStructure, 0, sizeof(SMCKeyData_t));
      memset(val, 0, sizeof(SMCVal_t));

      inputStructure.key = _strtoul(key, 4, 16);
      inputStructure.data8 = SMC_CMD_READ_KEYINFO;

      result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
      if (result != kIOReturnSuccess)
          return result;

      val->dataSize = outputStructure.keyInfo.dataSize;
      _ultostr(val->dataType, outputStructure.keyInfo.dataType);
      inputStructure.keyInfo.dataSize = val->dataSize;
      inputStructure.data8 = SMC_CMD_READ_BYTES;

      result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
      if (result != kIOReturnSuccess)
          return result;

      memcpy(val->bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

      return kIOReturnSuccess;
  }

  double
  CPUTemp::SMCGetTemperature(char *key)
  {
      SMCVal_t val;
      kern_return_t result;

      result = SMCReadKey(key, &val);
      if (result == kIOReturnSuccess) {
          // read succeeded - check returned value
          if (val.dataSize > 0) {
              if (strcmp(val.dataType, DATATYPE_SP78) == 0) {
                  // convert fp78 value to temperature
                  int intValue = (val.bytes[0] * 256 + val.bytes[1]) >> 2;
                  return intValue / 64.0;
              }
          }
      }
      // read failed
      return 0.0;
  }

} // namespace ndn
