/*
    Made it by Guillermo Martorell Hurtado
*/

#include "DatabaseLite.h"
#include <iostream>
#include <sqlite3.h>

namespace {
    char* stringToValue(char* value) {
        return 0 == strlen(value) ? nullptr : value;
    }

    char* valueToString(const char* value) {
        return value ? value : "NULL";
    }
}

Database::Database() : connection(nullptr)
{
}

Database::~Database()
{
    Disconnect();
}

bool Database::Connect()
{
    // Open the file (relative or absolute path)
    if (sqlite3_open("SQLite/rummi.db", &connection) != SQLITE_OK) {
        std::cerr << "[ERROR] Can't open DB: " << sqlite3_errmsg(connection) << "\n";
        return 1;
    }
    else{
        std::cout << "Opened!" << std::endl;
    }

    return true;
}

void Database::Disconnect()
{
    // Check if connection is already available
    if (connection != nullptr) {
        sqlite3_close(connection);
        connection = nullptr;
    }
}

bool Database::IsConnected() const
{
    return connection != nullptr;
}

int Database::GetTables(char** tables, int max_tables)
{
    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    // SQLite has no SHOW TABLES; the list lives in sqlite_master
    const char* query =
        "SELECT name FROM sqlite_master "
        "WHERE type = 'table' AND name NOT LIKE 'sqlite_%'";

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not get tables: " << sqlite3_errmsg(connection) << std::endl;

        return 0;
    }

    //get table data
    int table_count = 0;
    while (sqlite3_step(statement) == SQLITE_ROW && table_count < max_tables) {
        const char* name = (const char*)sqlite3_column_text(statement, 0);
        tables[table_count] = strdup(name);
        table_count++;
    }

    sqlite3_finalize(statement);

    return table_count;
}

int Database::GetTableData(const char* table_name, char** column_names, char*** data, int max_rows, int max_columns, int offset, int& column_count) {

    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    char query[512];

    sprintf_s(query, "SELECT * FROM `%s` LIMIT ?1 OFFSET ?2", table_name);

    //Pepare query
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not execute SELECT: "
                  << sqlite3_errmsg(connection)
                  << std::endl;

        return 0;
    }

    //bind parameters in the query to their values
    //table names cannot be bound to parameters
    sqlite3_bind_int(statement, 1, max_rows);
    sqlite3_bind_int(statement, 2, offset);

    // Number of columns, with column_count as an upper bound
    int real_column_count = sqlite3_column_count(statement);
    column_count = real_column_count;
    if (column_count > max_columns) {
        column_count = max_columns;
    }

    // Column names
    for (int i = 0; i < column_count; i++) {
        const char* name = sqlite3_column_name(statement, i);
        column_names[i] = strdup(name);
    }

    //get data row by row
    int row_count = 0;
    while (sqlite3_step(statement) == SQLITE_ROW && row_count < max_rows) {
        data[row_count] = (char**)malloc(sizeof(char*) * column_count);
        for (int i = 0; i < column_count; i++) {
            // returns nullptr for NULL values
            const char* value = (const char*)sqlite3_column_text(statement, i);
            data[row_count][i] = strdup(valueToString(value));
        }
        row_count++;
    }

    sqlite3_finalize(statement);

    return row_count;
}

// Know how many "filas" we have on each table
int Database::GetTableRowCount(const char* table_name) {
    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    //prepare statement
    char query[512];
    sprintf_s(query, "SELECT COUNT(*) FROM `%s`", table_name);
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) !=
        SQLITE_OK) {
        std::cerr << "[ERROR] Could not count rows: "
                  << sqlite3_errmsg(connection) << std::endl;
        return 0;
    }

    //execute statement
    int row_count = 0;
    if (sqlite3_step(statement) == SQLITE_ROW) {
        row_count = sqlite3_column_int(statement, 0);
    }

    sqlite3_finalize(statement);
    return row_count;
}

int Database::GetTableColumns(
    char* table_name,
    ColumnInfo columns[],
    int max_columns)
{
    char query[256];

    // DESCRIBE doesn't exist in SQLite; PRAGMA table_info is the equivalent.
    // It returns: cid | name | type | notnull | dflt_value | pk
    sprintf_s(
        query,
        "PRAGMA table_info(`%s`)",
        table_name
    );

    sqlite3_stmt* statement = nullptr;

    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not get table columns: " << sqlite3_errmsg(connection) << std::endl;
        return 0;
    }

    int column_count = 0;

    while (sqlite3_step(statement) == SQLITE_ROW) {
        if (column_count >= max_columns) {
            break;
        }

        // name is column 1, type is column 2 (column 0 is just the index)
        columns[column_count].name = _strdup((const char*)sqlite3_column_text(statement, 1));
        columns[column_count].type = _strdup((const char*)sqlite3_column_text(statement, 2));

        column_count++;
    }

    sqlite3_finalize(statement);

    return column_count;
}

bool Database::InsertRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) {
    // prepare query
    char query[2048];
    sprintf_s(query, "INSERT INTO `%s` ", table_name);

    // column names
    strcat_s(query, "(");
    for (int i = 0; i < column_count; i++)
    {
        strcat_s(query, "`");
        strcat_s(query, columns[i].name);
        strcat_s(query, "`");
        if (i < column_count - 1)
        {
            strcat_s(query, ", ");
        }
    }
    strcat_s(query, ") ");
    
    // values to insert, same as columns
    strcat_s(query, "VALUES (");
    for (int i = 0; i < column_count; i++) {
        //parameter placeholder
        strcat_s(query, "?");
        //parametor separator, if not last
        if (i < column_count - 1)
            strcat_s(query, ", ");
    }
    strcat_s(query, ")");

    //prepare request, check syntax
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not insert row: " << sqlite3_errmsg(connection) << std::endl;
        return 0;
    }

    //bind parameters
    for (int i = 0; i < column_count; i++) {
        //normalizes empty strings to NULL values
        char *value = stringToValue(values[i]);
        std::cout << columns[i].name << "(" << columns[i].type << "): " << valueToString(values[i]) << std::endl;
        sqlite3_bind_text(statement, i+1, value, -1, nullptr);
    }

    //execute query
    std::cout << "[SQL] " << query << std::endl;
    if (sqlite3_step(statement) != SQLITE_DONE) {
        std::cerr << "[ERROR] Could not insert row: " << sqlite3_errmsg(connection) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }

    std::cout << "[GOOD] Row inserted!" << std::endl;
    sqlite3_finalize(statement);

    return true;
}

bool Database::UpdateRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) {

    //prepare query
    char query[2048];
    sprintf_s(query, "UPDATE `%s` SET ", table_name);

    // logic column = value
    for (int i = 0; i < column_count; i++) {
        strcat_s(query, "`");
        strcat_s(query, columns[i].name);
        // param placeholder
        strcat_s(query, "` = ?");
        if (i < column_count - 1) {
            strcat_s(query, ", ");
        }
    }

    // use the first column as primary key
    strcat_s(query, " WHERE `");
    strcat_s(query, columns[0].name);
    strcat_s(query, "` = ?");

    //prepare statement
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not insert row: " << sqlite3_errmsg(connection) << std::endl;
        return 0;
    }

    //bind parameters
    int i;
    for (i = 0; i < column_count; i++) {
        //normalizes empty strings to NULL values
        char *value = stringToValue(values[i]);
        std::cout << columns[i].name << "(" << columns[i].type << "): " << valueToString(values[i]) << std::endl;
        sqlite3_bind_text(statement, i+1, value, -1, nullptr);
    }
    //bind id in WHERE clause
    sqlite3_bind_text(statement, i+1, stringToValue(values[0]), -1, nullptr);

    //execute the query
    std::cout << "[SQL] " << query << std::endl;
    if (sqlite3_step(statement) != SQLITE_DONE) {
        std::cerr << "[ERROR] Could not update row: " << sqlite3_errmsg(connection) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }
    std::cout << "[GOOD] Row updated!" << std::endl;
    sqlite3_finalize(statement);

    return true;
}

bool Database::DeleteRow(char* table_name, char* primary_key, char* primary_key_value) {

    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    //prepare query
    char query[1024] = {};
    sprintf_s(query, "DELETE FROM `%s` WHERE `%s` = ?", table_name, primary_key);
    sqlite3_stmt *statement;
    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not delete row: " << sqlite3_errmsg(connection) << std::endl;
        return 0;
    }

    //bind delete id
    sqlite3_bind_text(statement, 1, primary_key_value, -1, nullptr);

    //execute query
    std::cout << "[SQL] " << query << std::endl;
    if (sqlite3_step(statement) != SQLITE_DONE) {
        std::cerr << "[ERROR] Could not delete row: " << sqlite3_errmsg(connection) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }

    std::cout << "[GOOD] Row deleted!" << std::endl;
    sqlite3_finalize(statement);

    return true;
}

int Database::GetFreestyleData(const char* query, char** column_names, char*** data, int max_rows, int max_columns, int& column_count) {

    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    //prepare query
    sqlite3_stmt* statement = nullptr;
    if (sqlite3_prepare_v2(connection, query, -1, &statement, nullptr) != SQLITE_OK) {
        std::cerr << "[ERROR] Could not execute query: " << sqlite3_errmsg(connection) << std::endl;
        return 0;
    }

    
    //max_columns is an upper bound to the column count
    int real_column_count = sqlite3_column_count(statement);
    column_count = real_column_count;
    if (column_count > max_columns) {
        column_count = max_columns;
    }

    //columns
    for (int i = 0; i < column_count; i++) {
        const char* name = sqlite3_column_name(statement, i);
        column_names[i] = strdup(name);
    }

    //values
    int row_count = 0;
    while (sqlite3_step(statement) == SQLITE_ROW && row_count < max_rows) {
        data[row_count] = (char**)malloc(sizeof(char*) * column_count);
        for (int i = 0; i < column_count; i++) {
            const char* value = (const char*)sqlite3_column_text(statement, i);
            data[row_count][i] = strdup(valueToString(value));
        }
        row_count++;
    }

    sqlite3_finalize(statement);
    return row_count;
}