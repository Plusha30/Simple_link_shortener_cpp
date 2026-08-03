#include "crow.h"
#include "database.h"
#include "cpr/cpr.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

signed main() {
    //Logging
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/logs.txt", 1024 * 1024 * 5, 3
    );
    std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("logger", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::info);
    spdlog::set_default_logger(logger);
    //Create database
    SQL_Module::SQL_Result res = SQL_Module::SQL_Interface().exec_stmt(
                                        std::string("CREATE TABLE IF NOT EXISTS urls ("
                                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                        "url TEXT);"), {});
    //App itself
    crow::SimpleApp app;
    CROW_ROUTE(app, "/ping")([](const crow::request& req, crow::response& res) {
        spdlog::info("GET /ping");
        res.code = 200;
        res.write("ping");
        res.end();
        return;
    });
    CROW_ROUTE(app, "/")([](const crow::request& req, crow::response& res) {
        spdlog::info("GET /");
        res.code = 200;
        res.write("This is my own link shortener API, written in C++ and Crow framework!");
        res.end();
    });
    CROW_ROUTE(app, "/add_url").methods(crow::HTTPMethod::POST)([](const crow::request& req, crow::response& res) {
        spdlog::info("POST /add_url, body length: {}", req.body.size());
        std::string url = req.body;
        if (!url.size()) {
            spdlog::info("Empty URL recieved");
            res.code = 400;
            res.write("URL_NOT_GIVEN");
            res.end();
            return;
        }
        cpr::Response r = cpr::Get(cpr::Url(url), cpr::ConnectTimeout{3000});
        if (r.error) {
            spdlog::info("Invalid URL: {}", url);
            res.code = 400;
            res.write("URL_INVALID");
            res.end();
            return;
        }
        SQL_Module::SQL_Result sql_res = SQL_Module::SQL_Interface().exec_stmt(std::string(
            "INSERT INTO urls (url) VALUES (?) RETURNING id;"
        ), {url});
        if (sql_res.success == false) {
            spdlog::error("DB insert failed with error {} for URL: {}", sql_res.err, url);
            res.code = 500;
            res.write("INTERNAL_ERROR");
            res.end();
            return;
        }
        res.code = 201;
        res.write(std::to_string(std::get<int64_t>(sql_res.rows[0][0])));
        spdlog::info("Success, inserted URL {} in id {}", url, std::get<int64_t>(sql_res.rows[0][0]));
        res.end();
        return;
    });
    CROW_ROUTE(app, "/<int>")([](const crow::request& req, crow::response& res, int64_t id){
        spdlog::info("GET /{}", id);
        SQL_Module::SQL_Result sql_res = SQL_Module::SQL_Interface().exec_stmt(std::string(
            "SELECT url FROM urls WHERE id = ?"), {id});
        if (sql_res.success == false) {
            spdlog::error("DB select failed with error {} for ID: {}", sql_res.err, id);
            res.code = 500;
            res.write("INTERNAL_ERROR");
            res.end();
            return;
        }
        if (sql_res.rows.size() == 0) {
            spdlog::info("Required ID {} was not found in DB", id);
            res.code = 404;
            res.write("URL_NOT_FOUND");
            res.end();
            return;
        }
        spdlog::info("Success, redirected user from ID {} onto URL {}", id, std::get<std::string>(sql_res.rows[0][0]));
        res.code = 301;
        res.redirect(std::get<std::string>(sql_res.rows[0][0]));
        res.end();
        return;
    });
    app.port(8000).multithreaded().run();
    return 0;
}