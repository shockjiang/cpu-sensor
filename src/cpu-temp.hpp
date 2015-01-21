/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014 Regents of the University of California.
 *
 * @author Lijing Wang <phdloli@ucla.edu>
 */

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
#ifndef CPU_TEMP_HPP
#define CPU_TEMP_HPP

#include <stdio.h>
#include <string.h>
#include <IOKit/IOKitLib.h>

#define VERSION               "0.01"

#define KERNEL_INDEX_SMC      2

#define SMC_CMD_READ_BYTES    5
#define SMC_CMD_WRITE_BYTES   6
#define SMC_CMD_READ_INDEX    8
#define SMC_CMD_READ_KEYINFO  9
#define SMC_CMD_READ_PLIMIT   11
#define SMC_CMD_READ_VERS     12

#define DATATYPE_FPE2         "fpe2"
#define DATATYPE_UINT8        "ui8 "
#define DATATYPE_UINT16       "ui16"
#define DATATYPE_UINT32       "ui32"
#define DATATYPE_SP78         "sp78"

// key values
#define SMC_KEY_CPU_TEMP      "TC0P"

namespace ndn {

  class CPUTemp
  {
    public:
      
      CPUTemp();
      // prototypes
      double SMCGetTemperature(char *key);
      kern_return_t SMCOpen(void);
      kern_return_t SMCClose();
//      kern_return_t SMCSetFanRpm(char *key, int rpm);
//      int SMCGetFanRpm(char *key);
      
    private:

      typedef struct {
          char                  major;
          char                  minor;
          char                  build;
          char                  reserved[1]; 
          UInt16                release;
      } SMCKeyData_vers_t;
      
      typedef struct {
          UInt16                version;
          UInt16                length;
          UInt32                cpuPLimit;
          UInt32                gpuPLimit;
          UInt32                memPLimit;
      } SMCKeyData_pLimitData_t;
      
      typedef struct {
          UInt32                dataSize;
          UInt32                dataType;
          char                  dataAttributes;
      } SMCKeyData_keyInfo_t;
      
      typedef char              SMCBytes_t[32]; 
      
      typedef struct {
        UInt32                  key; 
        SMCKeyData_vers_t       vers; 
        SMCKeyData_pLimitData_t pLimitData;
        SMCKeyData_keyInfo_t    keyInfo;
        char                    result;
        char                    status;
        char                    data8;
        UInt32                  data32;
        SMCBytes_t              bytes;
      } SMCKeyData_t;
      
      typedef char              UInt32Char_t[5];
      
      typedef struct {
        UInt32Char_t            key;
        UInt32                  dataSize;
        UInt32Char_t            dataType;
        SMCBytes_t              bytes;
      } SMCVal_t;

      UInt32 _strtoul(char *str, int size, int base);
      void _ultostr(char *str, UInt32 val);
      kern_return_t SMCReadKey(UInt32Char_t key, SMCVal_t *val);
      kern_return_t SMCCall(int index, SMCKeyData_t *inputStructure, SMCKeyData_t *outputStructure);

  }; 
} // namespace ndn
#endif
