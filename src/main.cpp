#include "crow.h"

signed main() {
    crow::SimpleApp app;
    CROW_ROUTE(app, "/ping")([](const crow::request& req, crow::response& res) {
        res.code = 200;
        res.write("ping");
        res.end();
    });
    app.port(8000).multithreaded().run();
    return 0;
}