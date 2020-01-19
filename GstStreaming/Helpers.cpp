#include "Helpers.h"

#include <gst/gst.h>


namespace GstStreaming
{

IceServerType ParseIceServerType(const std::string& iceServer)
{
    if(0 == iceServer.compare(0, 5, "turn:"))
        return IceServerType::Turn;
    else if(0 == iceServer.compare(0, 5, "stun:"))
        return IceServerType::Stun;
    else
        return IceServerType::Unknown;
}

void SetIceServers(
    GstElement* rtcbin,
    const std::deque<std::string>& iceServers)
{
    for(const std::string& iceServer: iceServers) {
        using namespace GstStreaming;
        switch(ParseIceServerType(iceServer)) {
            case IceServerType::Stun:
                g_object_set(
                    rtcbin,
                    "stun-server", iceServer.c_str(),
                    nullptr);
                break;
            case IceServerType::Turn: {
                gboolean ret;
                g_signal_emit_by_name(
                    rtcbin,
                    "add-turn-server", iceServer.c_str(),
                    &ret);
                break;
            }
            default:
                break;
        }
    }
}

}