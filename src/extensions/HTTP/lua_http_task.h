#pragma once
#include "network/async_cgi_task_dispatcher.h"
namespace network {
    class LuaHttpTask : public HttpTask{
        public :
        BusinessThreadID threadId;
        explicit LuaHttpTask(const std::string& name);
        virtual ~LuaHttpTask();
        virtual void ProgressFeedBack(network::BaseConnection* connection, int64 dltotal, int64 dlnow, int64 ultotal, int64 ulnow);
        virtual int OnResponse(network::ProtocolErrorCode error_code,
                               const HTTP_HEADERS& headers,
                               const std::string& resp, long http_code);
    };
}
