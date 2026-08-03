#pragma once
#include "sqlite3.h"
#include <string>
#include <variant>
#include <vector>
#include <mutex>

struct sqlite3;
struct sqlite3_stmt;

namespace SQL_Module {
    using SQL_Value = std::variant<std::nullptr_t, int64_t, double, std::string>;
    
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    class SQL_Result {
    public:
        bool success = true;
        std::string err;
        std::vector<std::vector<SQL_Value>> rows = {};

        SQL_Result(bool _success, std::vector<std::vector<SQL_Value>> _rows);
        SQL_Result(bool _success, std::string err);
        SQL_Result(bool _success);
    };

    class SQL_Interface {
    public:
        SQL_Result exec_stmt(const std::string& query, const std::vector<SQL_Value>& params);
    private:
        std::mutex db_mutex;
    };
}