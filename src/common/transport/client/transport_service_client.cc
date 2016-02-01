/******************************************************************
 * Generated by Hottentot CC Generator
 * Date: 30-01-2016 04:23:06
 * Name: transport_service_client.h
 * Description:
 *   This file contains a sample client.
 ******************************************************************/

#include <iostream>

#include <naeem/hottentot/runtime/configuration.h>
#include <naeem/hottentot/runtime/logger.h>
#include <naeem/hottentot/runtime/proxy/proxy_runtime.h>

#include "../accept_report.h"
#include "../transport_message_status.h"
#include "../transport_message.h"
#include "../transport_service.h"
#include "../proxy/transport_service_proxy_builder.h"


int 
main(int argc, char **argv) {
  try {
    ::naeem::hottentot::runtime::Logger::Init();  
    ::naeem::hottentot::runtime::proxy::ProxyRuntime::Init(argc, argv);
    if (::naeem::hottentot::runtime::Configuration::Verbose()) {
      ::naeem::hottentot::runtime::Logger::GetOut() << "Proxy runtime is initialized." << std::endl;
    }
    ::ir::ntnaeem::gate::transport::TransportService *proxy = 
        ::ir::ntnaeem::gate::transport::proxy::TransportServiceProxyBuilder::Create("127.0.0.1", 2000);
    if (::naeem::hottentot::runtime::Configuration::Verbose()) {
      ::naeem::hottentot::runtime::Logger::GetOut() << "Proxy object is created." << std::endl;
    }
    // TODO Do whatever you want with proxy objects.
    ::ir::ntnaeem::gate::transport::proxy::TransportServiceProxyBuilder::Destroy(proxy);
    if (::naeem::hottentot::runtime::Configuration::Verbose()) {
      ::naeem::hottentot::runtime::Logger::GetOut() << "Proxy object is destroyed." << std::endl;
    }
    // Delete allocated objects
  } catch (...) {
    ::naeem::hottentot::runtime::Logger::GetOut() << "Error." << std::endl;
    return 1;
  }
  return 0;
}
