#include "crow.h"
#include "database.h"
#include "cpr/cpr.h"

signed main() {
    SQL_Module::SQL_Result res = SQL_Module::SQL_Interface().exec_stmt(
                                        std::string("CREATE TABLE IF NOT EXISTS urls ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                        "url TEXT);"), {});
    crow::SimpleApp app;
    CROW_ROUTE(app, "/ping")([](const crow::request& req, crow::response& res) {
        res.code = 200;
        res.write("ping");
        res.end();
    });
    CROW_ROUTE(app, "/")([](const crow::request& req, crow::response& res) {
        res.code = 200;
        res.write("This is my own link shortener API, written in C++ and Crow framework!");
        res.end();
    });
    CROW_ROUTE(app, "/add_url").methods(crow::HTTPMethod::POST)([](const crow::request& req, crow::response& res) {
        std::string url = req.body;
        if (!url.size()) {
            res.code = 400;
            res.write("URL_NOT_GIVEN");
            res.end();
            return;
        }
        cpr::Response r = cpr::Get(cpr::Url(url));
        if (r.error) {
            res.code = 400;
            res.write("URL_INVALID");
            res.end();
            return;
        }
        SQL_Module::SQL_Result sql_res = SQL_Module::SQL_Interface().exec_stmt(std::string(
            "INSERT INTO urls (url) VALUES ? RETURNING id;"
        ), {url});
        if (sql_res.success == false) {
            res.code = 500;
            res.write("INTERNAL_ERROR");
            res.end();
            return;
        }
        res.code = 201;
        res.write(std::to_string(std::get<int64_t>(sql_res.rows[0][0])));
        res.end();
        return;
    });
    app.port(8000).multithreaded().run();
    return 0;
}