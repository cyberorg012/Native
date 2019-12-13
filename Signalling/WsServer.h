#pragma once

#include <string>
#include <memory>
#include <functional>

#include <glib.h>

#include "RtspSession/ServerSession.h"

#include "Config.h"


namespace signalling {

class WsServer
{
public:
    typedef std::function<
        std::unique_ptr<rtsp::ServerSession> (
            const std::function<void (const rtsp::Request*)>& sendRequest,
            const std::function<void (const rtsp::Response*)>& sendResponse) noexcept> CreateSession;

    WsServer(const Config&, GMainLoop*, const CreateSession&);
    bool init();
    ~WsServer();

private:
    struct Private;
    std::unique_ptr<Private> _p;
};

}
