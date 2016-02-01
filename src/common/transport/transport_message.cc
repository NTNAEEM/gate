/******************************************************************
 * Generated by Hottentot CC Generator
 * Date: 30-01-2016 04:23:06
 * Name: transport_message.cc
 * Description:
 *   This file contains implementation of TransportMessage class.
 ******************************************************************/

#include <iostream>
#include <string.h>

#include "transport_message.h"


namespace ir {
namespace ntnaeem {
namespace gate {
namespace transport {
  unsigned char *
  TransportMessage::Serialize(uint32_t *length_ptr) {
    uint32_t totalLength = 0;
    uint32_t length0 = 0;
    unsigned char *data0 = masterMId_.Serialize(&length0);
    totalLength += length0;
    uint32_t length1 = 0;
    unsigned char *data1 = slaveId_.Serialize(&length1);
    totalLength += length1;
    uint32_t length2 = 0;
    unsigned char *data2 = slaveMId_.Serialize(&length2);
    totalLength += length2;
    uint32_t length3 = 0;
    unsigned char *data3 = relMId_.Serialize(&length3);
    totalLength += length3;
    uint32_t length4 = 0;
    unsigned char *data4 = relLabel_.Serialize(&length4);
    if (length4 <= 127) {
      totalLength += 1 + length4;
    } else if (length4 <= (256 * 256 - 1)) {
      totalLength += 3 + length4;
    } else if (length4 <= (256 * 256 * 256 - 1)) {
      totalLength += 3 + length4;
    }
    uint32_t length5 = 0;
    unsigned char *data5 = label_.Serialize(&length5);
    if (length5 <= 127) {
      totalLength += 1 + length5;
    } else if (length5 <= (256 * 256 - 1)) {
      totalLength += 3 + length5;
    } else if (length5 <= (256 * 256 * 256 - 1)) {
      totalLength += 3 + length5;
    }
    uint32_t length6 = 0;
    unsigned char *data6 = content_.Serialize(&length6);
    if (length6 <= 127) {
      totalLength += 1 + length6;
    } else if (length6 <= (256 * 256 - 1)) {
      totalLength += 3 + length6;
    } else if (length6 <= (256 * 256 * 256 - 1)) {
      totalLength += 3 + length6;
    }
    unsigned char *data = new unsigned char[totalLength];
    uint32_t c = 0;
    for (uint32_t i = 0; i < length0; i++) {
      data[c++] = data0[i];
    }
    for (uint32_t i = 0; i < length1; i++) {
      data[c++] = data1[i];
    }
    for (uint32_t i = 0; i < length2; i++) {
      data[c++] = data2[i];
    }
    for (uint32_t i = 0; i < length3; i++) {
      data[c++] = data3[i];
    }
    if (length4 <= 127) {
      data[c] = length4;
      c += 1;
    } else if (length4 <= (256 * 256 - 1)) {
      data[c] = 0x82;
      data[c + 1] = length4 / 256;
      data[c + 2] = length4 % 256;
      c += 3;
    } else if (length4 <= (256 * 256 * 256 - 1)) {
      data[c] = 0x83;
      data[c + 1] = length4 / (256 * 256);
      data[c + 2] = (length4 - data[c + 1] * (256 * 256)) / 256;
      data[c + 3] = length4 % (256 * 256);
      c += 4;
    }
    for (uint32_t i = 0; i < length4; i++) {
      data[c++] = data4[i];
    }
    if (length5 <= 127) {
      data[c] = length5;
      c += 1;
    } else if (length5 <= (256 * 256 - 1)) {
      data[c] = 0x82;
      data[c + 1] = length5 / 256;
      data[c + 2] = length5 % 256;
      c += 3;
    } else if (length5 <= (256 * 256 * 256 - 1)) {
      data[c] = 0x83;
      data[c + 1] = length5 / (256 * 256);
      data[c + 2] = (length5 - data[c + 1] * (256 * 256)) / 256;
      data[c + 3] = length5 % (256 * 256);
      c += 4;
    }
    for (uint32_t i = 0; i < length5; i++) {
      data[c++] = data5[i];
    }
    if (length6 <= 127) {
      data[c] = length6;
      c += 1;
    } else if (length6 <= (256 * 256 - 1)) {
      data[c] = 0x82;
      data[c + 1] = length6 / 256;
      data[c + 2] = length6 % 256;
      c += 3;
    } else if (length6 <= (256 * 256 * 256 - 1)) {
      data[c] = 0x83;
      data[c + 1] = length6 / (256 * 256);
      data[c + 2] = (length6 - data[c + 1] * (256 * 256)) / 256;
      data[c + 3] = length6 % (256 * 256);
      c += 4;
    }
    for (uint32_t i = 0; i < length6; i++) {
      data[c++] = data6[i];
    }
    if (c != totalLength) {
      std::cout << "Struct Serialization: Inconsistency in length of serialized byte array." << std::endl;
      exit(1);
    };
    if (length_ptr) {
      *length_ptr = totalLength;
    }
    return data;
  }
  void
  TransportMessage::Deserialize(unsigned char *data, uint32_t length) {
    uint32_t c = 0, elength = 0;
    masterMId_.Deserialize(data + c, 8);
    c += 8;
    slaveId_.Deserialize(data + c, 4);
    c += 4;
    slaveMId_.Deserialize(data + c, 8);
    c += 8;
    relMId_.Deserialize(data + c, 8);
    c += 8;
    if ((data[c] & 0x80) == 0) {
      elength = data[c];
      c++;
    } else {
      uint8_t ll = data[c] & 0x0f;
      if (ll == 2) {
        elength == data[c] * 256 + data[c + 1];
        c += 2;
      } else if (ll == 3) {
        elength == data[c] * 256 * 256 + data[c + 1] * 256 + data[c + 2];
        c += 3;
      }
    }
    relLabel_.Deserialize(data + c, elength);
    c += elength;
    if ((data[c] & 0x80) == 0) {
      elength = data[c];
      c++;
    } else {
      uint8_t ll = data[c] & 0x0f;
      if (ll == 2) {
        elength == data[c] * 256 + data[c + 1];
        c += 2;
      } else if (ll == 3) {
        elength == data[c] * 256 * 256 + data[c + 1] * 256 + data[c + 2];
        c += 3;
      }
    }
    label_.Deserialize(data + c, elength);
    c += elength;
    if ((data[c] & 0x80) == 0) {
      elength = data[c];
      c++;
    } else {
      uint8_t ll = data[c] & 0x0f;
      if (ll == 2) {
        elength == data[c] * 256 + data[c + 1];
        c += 2;
      } else if (ll == 3) {
        elength == data[c] * 256 * 256 + data[c + 1] * 256 + data[c + 2];
        c += 3;
      }
    }
    content_.Deserialize(data + c, elength);
    c += elength;
    if (c != length) {
      std::cout << "Struct Deserialization: Inconsistency in length of deserialized byte array." << std::endl;
      exit(1);
    };
  }
} // END OF NAMESPACE transport
} // END OF NAMESPACE gate
} // END OF NAMESPACE ntnaeem
} // END OF NAMESPACE ir