/******************************************************************
 * Generated by Hottentot CC Generator
 * Date: 30-01-2016 04:23:06
 * Name: gate_service_proxy_builder.h
 * Description:
 *   This file contains definition of the proxy builder class.
 ******************************************************************/

#ifndef _IR_NTNAEEM_GATE__PROXY__GATE_SERVICE_PROXY_BUILDER_H_
#define _IR_NTNAEEM_GATE__PROXY__GATE_SERVICE_PROXY_BUILDER_H_

#include <stdint.h>
#include <string>


namespace ir {
namespace ntnaeem {
namespace gate {
class GateService;
namespace proxy {
  class GateServiceProxyBuilder {
  public:
    static GateService* Create(std::string /* host */, uint32_t /* port */);
    static void Destroy(GateService *);
  };
} // END OF NAMESPACE proxy
} // END OF NAMESPACE gate
} // END OF NAMESPACE ntnaeem
} // END OF NAMESPACE ir

#endif