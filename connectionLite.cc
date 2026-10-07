/**
 * @file connectionLite.cc
 * @author Guillermo Martorell (martorellhu@esat-alumni.com)
 * @brief Tests connection to SQLite database
 */

#include <iostream>

#include "sqlite3.h"

int main() {
    sqlite3* db = nullptr;

    // Open the file (relative or absolute path)
    if (sqlite3_open("SQLite/rummi.db", &db) != SQLITE_OK) {
        std::cerr << "Can't open DB: " << sqlite3_errmsg(db) << "\n";
        return 1;
    } else {
        std::cout << "Opened!" << std::endl;
    }

    const char* sql =
        "SELECT COUNT(name) FROM sqlite_master "
        "WHERE type = 'table' AND name NOT LIKE 'sqlite_%'";
    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(db, sql, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return 1;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        int count = sqlite3_column_int(statement, 0);
        // const unsigned char* name = sqlite3_column_text(statement, 1);
        std::cout << "Database has " << count << " tables\n";
    }

    sqlite3_finalize(statement);
    sqlite3_close(db);

    return 0;
}