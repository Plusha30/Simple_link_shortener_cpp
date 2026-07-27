#include "sqlite3.h"
#include "database.h"
#include <string>
#include <variant>
#include <vector>

namespace SQL_Module {

    SQL_Result::SQL_Result() : success(false), rows({}) {}
    SQL_Result::SQL_Result(bool _success) : success(_success), rows({}) {}
    SQL_Result::SQL_Result(bool _success, std::vector<std::vector<SQL_Value>> _rows)
        : success(_success), rows(std::move(_rows)) {}

    SQL_Result SQL_Interface::exec_stmt(const std::string& query, const std::vector<SQL_Value>& params) {
        sqlite3* db;
        sqlite3_stmt* stmt = nullptr;
        int resultcode = sqlite3_open("../database.db", &db);
        if (resultcode != SQLITE_OK)
            return SQL_Result();
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return SQL_Result();
        }
        for (u_int32_t i = 0; i < params.size(); i++) {
            int idx = static_cast<int>(i) + 1;
            const SQL_Value& value = params[i];
            std::visit(overloaded {
                [&](std::nullptr_t) {
                    sqlite3_bind_null(stmt, idx);
                },
                [&](int64_t v) {
                    sqlite3_bind_int64(stmt, idx, v);
                },
                [&](double v) {
                    sqlite3_bind_double(stmt, idx, v);
                },
                [&](const std::string v) {
                    sqlite3_bind_text(stmt, idx, v.c_str(), -1, SQLITE_TRANSIENT);
                }
            }, params[i]);
        }
        SQL_Result res;
        while ((resultcode = sqlite3_step(stmt)) == SQLITE_ROW) {
            int columnCount = sqlite3_column_count(stmt);
            std::vector<SQL_Value> row;
            row.reserve(columnCount);
            for (int col = 0; col < columnCount; col++) {
                int type = sqlite3_column_type(stmt, col);
                switch (type) {
                    case SQLITE_INTEGER:
                        row.push_back(static_cast<int64_t>(sqlite3_column_int64(stmt, col)));
                        break;
                    case SQLITE_FLOAT:
                        row.push_back(static_cast<double>(sqlite3_column_double(stmt, col)));
                        break;
                    case SQLITE_TEXT: {
                        const std::string text = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, col)));
                        row.push_back(text);
                        break;
                    }
                    case SQLITE_NULL:
                    default:
                        row.push_back(nullptr);
                        break;
                }
            }
            res.rows.push_back(row);
        }
        if (resultcode != SQLITE_DONE) {
            return SQL_Result();
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return res;
    }
}