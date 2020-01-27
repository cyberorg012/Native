#include <thread>
#include <memory>

#include <CxxPtr/GlibPtr.h>

#include "Common/LwsLog.h"
#include "Signalling/Log.h"
#include "Signalling/WsServer.h"
#include "Signalling/ServerSession.h"
#include "Client/Log.h"
#include "Client/WsClient.h"
#include "Client/ClientSession.h"

#include "GstStreaming/LibGst.h"
#include "GstStreaming/GstRtspReStreamer.h"
#include "GstStreaming/GstClient.h"

#include "../InverseProxyServer/Log.h"
#include "../InverseProxyServer/InverseProxyServer.h"
#include "../InverseProxyClient/Log.h"
#include "../InverseProxyClient/InverseProxyClient.h"

#define ENABLE_STREAMER 1
#define ENABLE_SERVER 1
#define ENABLE_VIEWER 1

enum {
    RECONNECT_TIMEOUT = 5,
};


static std::unique_ptr<WebRTCPeer> CreateClientPeer()
{
    return std::make_unique<GstClient>();
}

static std::unique_ptr<rtsp::ClientSession> CreateClientSession (
    const std::string& sourceUri,
    const std::function<void (const rtsp::Request*) noexcept>& sendRequest,
    const std::function<void (const rtsp::Response*) noexcept>& sendResponse) noexcept
{
    return
        std::make_unique<ClientSession>(
            sourceUri,
            CreateClientPeer,
            sendRequest,
            sendResponse);
}

static void ClientDisconnected(client::WsClient* client) noexcept
{
    GSourcePtr timeoutSourcePtr(g_timeout_source_new_seconds(RECONNECT_TIMEOUT));
    GSource* timeoutSource = timeoutSourcePtr.get();
    g_source_set_callback(timeoutSource,
        [] (gpointer userData) -> gboolean {
            static_cast<client::WsClient*>(userData)->connect();
            return false;
        }, client, nullptr);
    g_source_attach(timeoutSource, g_main_context_get_thread_default());
}

int main(int argc, char *argv[])
{
    enum {
        FRONT_SERVER_PORT = 4001,
        BACK_SERVER_PORT = 4002,
    };

    LibGst libGst;

    const spdlog::level::level_enum logLevel = spdlog::level::trace;
    InitLwsLogger(logLevel);
    InitWsClientLogger(logLevel);
    InitWsServerLogger(logLevel);
    InitInverseProxyClientLogger(logLevel);
    InitInverseProxyServerLogger(logLevel);

    const std::string server = "localhost";
    const std::string sourceName = "source1";
    const std::string streamerName = "bars";
    const std::string streamer2Name = "green";
    const std::string sourceAuthToken = "dummyToken";

#if ENABLE_SERVER
    std::thread serverThread(
        [&sourceName, &sourceAuthToken] () {
            InverseProxyServerConfig config {
                .frontPort = FRONT_SERVER_PORT,
                .backPort = BACK_SERVER_PORT,
                .stunServer = "localhost:3478",
                .turnServer = "localhost:3478",
                .turnUsername = "anonymous",
                .turnCredential = "guest",
                .turnStaticAuthSecret = "dummySecret",
                .backAuthTokens = { {sourceName, sourceAuthToken} }
            };
            InverseProxyServerMain(config);
        });
#endif

#if ENABLE_STREAMER
    std::thread streamSourceClientThread(
        [&sourceName, &streamerName, &streamer2Name, &sourceAuthToken] () {
            InverseProxyClientConfig config {};
            config.clientConfig.server = "localhost";
            config.clientConfig.serverPort = BACK_SERVER_PORT;
            config.name = sourceName;
            config.authToken = sourceAuthToken;
            config.streamers.emplace(
                streamerName,
                StreamerConfig { StreamerConfig::Type::Test, streamerName });
            config.streamers.emplace(
                streamer2Name,
                StreamerConfig { StreamerConfig::Type::Test, streamer2Name });

            InverseProxyClientMain(config);
        });
#endif

#if ENABLE_VIEWER
    g_usleep(2 * G_USEC_PER_SEC);

    std::thread clientThread(
        [&server, &sourceName, &streamerName] () {
            client::Config config {};
            config.server = server;
            config.serverPort = FRONT_SERVER_PORT;

            GMainContextPtr clientContextPtr(g_main_context_new());
            GMainContext* clientContext = clientContextPtr.get();
            g_main_context_push_thread_default(clientContext);
            GMainLoopPtr loopPtr(g_main_loop_new(clientContext, FALSE));
            GMainLoop* loop = loopPtr.get();

            client::WsClient client(
                config,
                loop,
                std::bind(
                    CreateClientSession,
                    sourceName + '/' + streamerName,
                    std::placeholders::_1,
                    std::placeholders::_2),
                std::bind(ClientDisconnected, &client));

            if(client.init()) {
                client.connect();
                g_main_loop_run(loop);
            }
        });
#endif

#if ENABLE_SERVER
    if(serverThread.joinable())
        serverThread.join();
#endif

#if ENABLE_STREAMER
    if(streamSourceClientThread.joinable())
        streamSourceClientThread.join();
#endif

#if ENABLE_VIEWER
    if(clientThread.joinable())
        clientThread.join();
#endif

    return 0;
}
