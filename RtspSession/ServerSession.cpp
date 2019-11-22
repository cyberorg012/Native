#include "ServerSession.h"


namespace rtsp {

ServerSession::ServerSession(
    const std::function<void (const rtsp::Response*)>& cb) noexcept :
    _responseCallback(cb)
{
}

void ServerSession::sendResponse(const rtsp::Response* response) noexcept
{
    _responseCallback(response);
}

bool ServerSession::handleOptionsRequest(
    std::unique_ptr<rtsp::Request>&) noexcept
{
    return false;
}

bool ServerSession::handleDescribeRequest(
    std::unique_ptr<rtsp::Request>&) noexcept
{
    return false;
}

bool ServerSession::handleSetupRequest(
    std::unique_ptr<rtsp::Request>&) noexcept
{
    return false;
}

bool ServerSession::handlePlayRequest(
    std::unique_ptr<rtsp::Request>&) noexcept
{
    return false;
}

bool ServerSession::handleTeardownRequest(
    std::unique_ptr<rtsp::Request>&) noexcept
{
    return false;
}

bool ServerSession::handleRequest(
    std::unique_ptr<rtsp::Request>& requestPtr) noexcept
{
    switch(requestPtr->method) {
    case rtsp::Method::OPTIONS:
        return handleOptionsRequest(requestPtr);
    case rtsp::Method::DESCRIBE:
        return handleDescribeRequest(requestPtr);
    case rtsp::Method::SETUP:
        return handleRequest(requestPtr);
    case rtsp::Method::PLAY:
        return handlePlayRequest(requestPtr);
    case rtsp::Method::TEARDOWN:
        return handleTeardownRequest(requestPtr);
    default:
        return false;
    }
}

}