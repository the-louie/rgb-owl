#pragma once
// HTTP server on port 80 (only while on the home network; the setup portal
// uses port 80 itself). JSON API under /api.

#include <functional>

class WebServer;

namespace owl::http {

void loop();
WebServer& server();  // for other modules to register routes before the first loop()

// Wraps a handler so it only runs in debug mode (403 otherwise). In the boot window only
// /api/debug and /api/devmode are served.
std::function<void()> devOnly(std::function<void()> handler);

}  // namespace owl::http
