#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>

#include <naeem/os.h>

#include <naeem++/date/helper.h>

#include <naeem/hottentot/runtime/configuration.h>
#include <naeem/hottentot/runtime/logger.h>
#include <naeem/hottentot/runtime/proxy/proxy_runtime.h>

#include <gate/message.h>
#include <gate/proxy/gate_service_proxy.h>
#include <gate/proxy/gate_service_proxy_builder.h>


#include <naeem/gate/client/receiver_thread.h>
#include <naeem/gate/client/runtime.h>


namespace naeem {
namespace gate {
namespace client {
  void
  ReceiverThread::Start() { 
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, ReceiverThread::ThreadBody, this);
  }
  void
  ReceiverThread::Shutdown() {
    {
      std::lock_guard<std::mutex> guard(terminationLock_);
      terminated_ = true;
    }
    ::naeem::hottentot::runtime::Logger::GetOut() << 
      "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " <<
        "Waiting for submitter thread to exit ..." << std::endl;
    while (true) {
      std::lock_guard<std::mutex> guard(terminationLock_);
      if (threadTerminated_) {
        ::naeem::hottentot::runtime::Logger::GetOut() << 
          "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " <<
            "Receiver thread exited." << std::endl;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }
  void*
  ReceiverThread::ThreadBody(void *thisObject) {
    ::naeem::gate::client::ReceiverThread *me =
      (::naeem::gate::client::ReceiverThread*)(thisObject);
    bool cont = true;
    time_t lastTime = time(NULL);
    while (cont) {
      try {
        if (cont) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        {
          std::lock_guard<std::mutex> guard(me->terminationLock_);
          if (me->terminated_) {
            if (::naeem::hottentot::runtime::Configuration::Verbose()) {
              ::naeem::hottentot::runtime::Logger::GetOut() << 
                "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " <<
                  "Receiver Thread: Received TERM SIGNAL ..." << std::endl;
            }
            cont = false;
            break;
          }
        }
        if (cont) {
          bool proceed = false;
          if ((time(NULL) - lastTime) > 5) {
            lastTime = time(NULL);
            proceed = true;
          }
          if (proceed) {
            /*
             * Aquiring main lock by creating guard object
             */
            std::lock_guard<std::mutex> guard(me->runtime_->mainLock_);
            /* ----------------------------------------------------
             * Reading messages
             * ----------------------------------------------------
             */
            ::ir::ntnaeem::gate::proxy::GateService *proxy = 
              ::ir::ntnaeem::gate::proxy::GateServiceProxyBuilder::Create(me->gateHost_, me->gatePort_);
            if (::naeem::hottentot::runtime::Configuration::Verbose()) {
              ::naeem::hottentot::runtime::Logger::GetOut() << 
                "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                  "Proxy object is created." << std::endl;
            }
            uint64_t readMessages = 0;
            try {
              if (dynamic_cast< ::naeem::hottentot::runtime::proxy::Proxy*>(proxy)->IsServerAlive()) {
                ::naeem::hottentot::runtime::types::Boolean hasMore;
                ::naeem::hottentot::runtime::types::Utf8String labelString(me->popLabel_);
                proxy->HasMore(labelString, hasMore);
                while (hasMore.GetValue()) {
                  ::ir::ntnaeem::gate::Message message;
                  uint64_t messageId;
                  proxy->PopNext(labelString, message);
                  {
                    std::lock_guard<std::mutex> guard(me->runtime_->messageIdCounterLock_);
                    messageId = me->runtime_->messageIdCounter_;
                    me->runtime_->messageIdCounter_++;
                    NAEEM_os__write_to_file (
                      (NAEEM_path)me->workDirPath_.c_str(), 
                      (NAEEM_string)"mco", 
                      (NAEEM_data)&(me->runtime_->messageIdCounter_), 
                      (NAEEM_length)sizeof(me->runtime_->messageIdCounter_)
                    );
                  }
                  std::stringstream ss;
                  ss << messageId;
                  NAEEM_data data;
                  NAEEM_length dataLength;
                  data = message.Serialize(&dataLength);
                  NAEEM_os__write_to_file (
                    (NAEEM_path)(me->workDirPath_ + "/r").c_str(),
                    (NAEEM_string)ss.str().c_str(),
                    data,
                    dataLength
                  );
                  NAEEM_os__write_to_file (
                    (NAEEM_path)(me->workDirPath_ + "/ra").c_str(),
                    (NAEEM_string)ss.str().c_str(),
                    data,
                    dataLength
                  );
                  delete [] data;
                  uint64_t gateId = message.GetId().GetValue();
                  NAEEM_os__write_to_file (
                    (NAEEM_path)(me->workDirPath_ + "/s").c_str(),
                    (NAEEM_string)(ss.str() + ".gid").c_str(),
                    (NAEEM_data)(&gateId),
                    sizeof(gateId)
                  );
                  if (me->runtime_->received_.find(me->popLabel_) == me->runtime_->received_.end()) {
                    me->runtime_->received_.insert(
                      std::pair<std::string, std::deque<uint64_t>*>(
                        me->popLabel_, new std::deque<uint64_t>));
                  }
                  me->runtime_->received_[me->popLabel_]->push_back(messageId);
                  ::naeem::hottentot::runtime::types::UInt64 messageIdVar(message.GetId().GetValue());
                  proxy->Ack(messageIdVar);
                  readMessages++;
                  proxy->HasMore(labelString, hasMore);
                }
              } else {
                if (::naeem::hottentot::runtime::Configuration::Verbose()) {
                  std::cout << 
                    "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                      "[Gate-Client] Slave gate is not available. Send failed." << std::endl;
                }
              }
              if (::naeem::hottentot::runtime::Configuration::Verbose() || 
                    readMessages > 0) {
                std::cout << 
                  "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                    "[Gate-Client] Number of read messages: " << readMessages << std::endl;
              }
              ::ir::ntnaeem::gate::proxy::GateServiceProxyBuilder::Destroy(proxy);
              if (::naeem::hottentot::runtime::Configuration::Verbose()) {
                ::naeem::hottentot::runtime::Logger::GetOut() << 
                  "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                    "Proxy object is destroyed." << std::endl;
              }
            } catch (std::exception &e) {
              ::naeem::hottentot::runtime::Logger::GetError() << 
                "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                  "ERROR: " << e.what() << std::endl;
              ::ir::ntnaeem::gate::proxy::GateServiceProxyBuilder::Destroy(proxy);
              if (::naeem::hottentot::runtime::Configuration::Verbose()) {
                ::naeem::hottentot::runtime::Logger::GetOut() << 
                  "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                    "Proxy object is destroyed." << std::endl;
              }
            } catch (...) {
              ::naeem::hottentot::runtime::Logger::GetError() << 
                "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                  "[Gate-Client] Unknown error." << std::endl;
              ::ir::ntnaeem::gate::proxy::GateServiceProxyBuilder::Destroy(proxy);
              if (::naeem::hottentot::runtime::Configuration::Verbose()) {
                ::naeem::hottentot::runtime::Logger::GetOut() << 
                  "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
                    "Proxy object is destroyed." << std::endl;
              }
            }
          }
        }
      } catch(std::exception &e) {
        ::naeem::hottentot::runtime::Logger::GetError() << 
          "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
            "[Gate-Client] ERROR: " << e.what() << std::endl;
      } catch(...) {
        ::naeem::hottentot::runtime::Logger::GetError() << 
          "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
            "[Gate-Client] Unknown error." << std::endl;
      }
    }
    if (::naeem::hottentot::runtime::Configuration::Verbose()) {
      ::naeem::hottentot::runtime::Logger::GetOut() << 
        "[" << ::naeem::date::helper::GetCurrentUTCTimeString() << "]: " << 
          "[Gate-Client] Receiver thread is exiting ..." << std::endl;
    }
    std::lock_guard<std::mutex> guard(me->terminationLock_);
    me->threadTerminated_ = true;
    pthread_exit(NULL);
  }
}
}
}