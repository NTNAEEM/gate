#include <sstream>
#include <chrono>
#include <thread>

#include <naeem/os.h>

#include <naeem++/conf/config_manager.h>

#include <naeem/hottentot/runtime/configuration.h>
#include <naeem/hottentot/runtime/logger.h>
#include <naeem/hottentot/runtime/utils.h>
#include <naeem/hottentot/runtime/proxy/proxy.h>
#include <naeem/hottentot/runtime/proxy/proxy_runtime.h>

#include <gate/message.h>

#include <naeem/gate/client/default_message_receiver.h>
#include <naeem/gate/client/runtime.h>


namespace naeem {
namespace gate {
namespace client {
  void 
  DefaultMessageReceiver::Init (
    int argc, 
    char **argv
  ) {
    ::naeem::hottentot::runtime::Logger::Init();
    ::naeem::hottentot::runtime::proxy::ProxyRuntime::Init(argc, argv);
    if (::naeem::hottentot::runtime::Configuration::Verbose()) {
      ::naeem::hottentot::runtime::Logger::GetOut() << "Proxy runtime is initialized." << std::endl;
    }
    ::naeem::gate::client::Runtime::Init(workDirPath_, argc, argv);
    receiverThread_ = new ReceiverThread(gateHost_, gatePort_, popLabel_, workDirPath_);
    receiverThread_->Start();
  }
  void 
  DefaultMessageReceiver::Shutdown() {
    {
      std::lock_guard<std::mutex> guard(Runtime::termSignalLock_);
      Runtime::termSignal_ = true;
    }
    std::cout << "Waiting for receiver thread to exit ..." << std::endl;
    while (true) {
      std::lock_guard<std::mutex> guard(Runtime::termSignalLock_);
      if (Runtime::receiverThreadTerminated_) {
        std::cout << "Receiver thread exited." << std::endl;
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ::naeem::gate::client::Runtime::Shutdown();
    ::naeem::hottentot::runtime::proxy::ProxyRuntime::Shutdown();
    ::naeem::hottentot::runtime::Logger::Shutdown();
    delete receiverThread_;
  }
  std::vector<Message*>
  DefaultMessageReceiver::GetMessages () {
    std::vector<Message*> messages;
    {
      std::lock_guard<std::mutex> guard(Runtime::mainLock_);
      if (Runtime::poppedButNotAcked_.find(popLabel_) != Runtime::poppedButNotAcked_.end()) {
        if (Runtime::poppedButNotAcked_[popLabel_]->size() > 0) {
          for (std::map<uint64_t, uint64_t>::iterator it = Runtime::poppedButNotAcked_[popLabel_]->begin();
               it != Runtime::poppedButNotAcked_[popLabel_]->end();
               it++) {
            uint64_t currentTime = time(NULL);
            if ((currentTime - it->second) > ackTimeout_) {
              std::stringstream ss;
              ss << it->first;
              NAEEM_data data;
              NAEEM_length dataLength;
              NAEEM_os__read_file_with_path (
                (NAEEM_path)(workDirPath_ + "/pna").c_str(),
                (NAEEM_string)ss.str().c_str(),
                &data,
                &dataLength
              );
              ::ir::ntnaeem::gate::Message message;
              message.Deserialize(data, dataLength);
              free(data);
              /* ------------------------------------------ */
              ::naeem::gate::client::Message *clientMessage = 
                new ::naeem::gate::client::Message;
              clientMessage->SetId (it->first);
              if (message.GetRelId().GetValue() != 0) {
                std::stringstream crss;
                crss << message.GetRelId().GetValue();
                uint64_t clientRelId;
                if (NAEEM_os__file_exists (
                      (NAEEM_path)(workDirPath_ + "/s").c_str(),
                      (NAEEM_string)(crss.str() + ".cid").c_str()
                    )
                ) {
                  NAEEM_os__read_file3 (
                    (NAEEM_path)(workDirPath_ + "/s/" + crss.str() + ".cid").c_str(),
                    (NAEEM_data)&clientRelId,
                    0
                  );
                  clientMessage->SetRelId (clientRelId);
                } else {
                  clientMessage->SetRelId (0);
                }
              } else {
                clientMessage->SetRelId(0);
              }
              clientMessage->SetLabel(popLabel_);
              clientMessage->SetContent (
                ByteArray (
                  message.GetContent().GetValue(), 
                  message.GetContent().GetLength()
                )
              );
              messages.push_back(clientMessage);
              /* ------------------------------------------ */
              uint64_t currentTime = time(NULL);
              NAEEM_os__write_to_file (
                (NAEEM_path)(workDirPath_ + "/pnat").c_str(),
                (NAEEM_string)ss.str().c_str(),
                (NAEEM_data)&currentTime,
                sizeof(currentTime)
              );
              if (Runtime::poppedButNotAcked_.find(popLabel_) == Runtime::poppedButNotAcked_.end()) {
                Runtime::poppedButNotAcked_.insert(
                  std::pair<std::string, std::map<uint64_t, uint64_t>*>(
                    popLabel_, new std::map<uint64_t, uint64_t>()));
              }
              (*(Runtime::poppedButNotAcked_[popLabel_]))[it->first] = currentTime;
            }
          }
        }
      }
      if (Runtime::received_.find(popLabel_) == Runtime::received_.end()) {
        return messages;
      }
      if (Runtime::received_[popLabel_]->size() == 0) {
        return messages;
      }
      std::deque<uint64_t> receivedIds = std::move(*(Runtime::received_[popLabel_]));
      for (uint64_t i = 0; i < receivedIds.size(); i++) {
        std::stringstream ss;
        ss << receivedIds[i];
        NAEEM_data data;
        NAEEM_length dataLength;
        NAEEM_os__read_file_with_path (
          (NAEEM_path)(workDirPath_ + "/r").c_str(),
          (NAEEM_string)ss.str().c_str(),
          &data,
          &dataLength
        );
        ::ir::ntnaeem::gate::Message message;
        message.Deserialize(data, dataLength);
        free(data);
        /* ------------------------------------------ */
        ::naeem::gate::client::Message *clientMessage = 
          new ::naeem::gate::client::Message;
        clientMessage->SetId (receivedIds[i]);
        if (message.GetRelId().GetValue() != 0) {
          std::stringstream crss;
          crss << message.GetRelId().GetValue();
          uint64_t clientRelId;
          if (NAEEM_os__file_exists (
                (NAEEM_path)(workDirPath_ + "/s").c_str(),
                (NAEEM_string)(crss.str() + ".cid").c_str()
              )
          ) {
            NAEEM_os__read_file3 (
              (NAEEM_path)(workDirPath_ + "/s/" + crss.str() + ".cid").c_str(),
              (NAEEM_data)&clientRelId,
              0
            );
            clientMessage->SetRelId (clientRelId);
          } else {
            clientMessage->SetRelId (0);
          }
        } else {
          clientMessage->SetRelId (0);
        }
        clientMessage->SetLabel(popLabel_);
        clientMessage->SetContent (
          ByteArray (
            message.GetContent().GetValue(), 
            message.GetContent().GetLength()
          )
        );
        messages.push_back(clientMessage);
        /* ------------------------------------------ */
        NAEEM_os__copy_file (
          (NAEEM_path)(workDirPath_ + "/r").c_str(),
          (NAEEM_string)ss.str().c_str(),
          (NAEEM_path)(workDirPath_ + "/a").c_str(),
          (NAEEM_string)ss.str().c_str()
        );
        NAEEM_os__move_file (
          (NAEEM_path)(workDirPath_ + "/r").c_str(),
          (NAEEM_string)ss.str().c_str(),
          (NAEEM_path)(workDirPath_ + "/pna").c_str(),
          (NAEEM_string)ss.str().c_str()
        );
        uint64_t currentTime = time(NULL);
        NAEEM_os__write_to_file (
          (NAEEM_path)(workDirPath_ + "/pnat").c_str(),
          (NAEEM_string)ss.str().c_str(),
          (NAEEM_data)&currentTime,
          sizeof(currentTime)
        );
        if (Runtime::poppedButNotAcked_.find(popLabel_) == Runtime::poppedButNotAcked_.end()) {
          Runtime::poppedButNotAcked_.insert(
            std::pair<std::string, std::map<uint64_t, uint64_t>*>(
              popLabel_, new std::map<uint64_t, uint64_t>()));
        }
        (*(Runtime::poppedButNotAcked_[popLabel_]))[receivedIds[i]] = currentTime;
      }
    }
    return messages;
  }
  void
  DefaultMessageReceiver::Ack (
    std::vector<uint64_t> ids
  ) {
    {
      std::lock_guard<std::mutex> guard(Runtime::mainLock_);
      for (uint32_t i = 0; i < ids.size(); i++) {
        uint64_t messageId = ids[i];
        std::stringstream ss;
        ss << messageId;
        if (NAEEM_os__file_exists (
              (NAEEM_path)(workDirPath_ + "/pna").c_str(), 
              (NAEEM_string)ss.str().c_str()
            )
        ) {
          NAEEM_data data;
          NAEEM_length dataLength;
          NAEEM_os__read_file_with_path (
            (NAEEM_path)(workDirPath_ + "/pna").c_str(),
            (NAEEM_string)ss.str().c_str(),
            &data,
            &dataLength
          );
          ::ir::ntnaeem::gate::Message message;
          message.Deserialize(data, dataLength);
          free(data);
          if (Runtime::poppedButNotAcked_.find(popLabel_) 
                != Runtime::poppedButNotAcked_.end()) {
            Runtime::poppedButNotAcked_[popLabel_]->erase(messageId);
          }
          NAEEM_os__move_file (
            (NAEEM_path)(workDirPath_ + "/pna").c_str(),
            (NAEEM_string)ss.str().c_str(),
            (NAEEM_path)(workDirPath_ + "/pa").c_str(),
            (NAEEM_string)ss.str().c_str()
          );
          uint64_t currentTime = time(NULL);
          NAEEM_os__write_to_file (
            (NAEEM_path)(workDirPath_ + "/pnat").c_str(),
            (NAEEM_string)ss.str().c_str(),
            (NAEEM_data)&currentTime,
            sizeof(currentTime)
          );
        }
      }
    }
  }
}
}
}