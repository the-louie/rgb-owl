#pragma once
// HTTP server on port 80 (only while on the home network; the setup portal
// uses port 80 itself). JSON API under /api.

class WebServer;

namespace owl::http {

void loop();
WebServer& server();  // for other modules to register routes before the first loop()

}  // namespace owl::http
