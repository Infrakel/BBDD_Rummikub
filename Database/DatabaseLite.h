/**
 * @file connection.cc
 * @author Guillermo Martorell Hurtado (martorellhu@esat-alumni.com)
 * @brief Implementation of Database interface for SQLite database
 * (declarations)
 */

#pragma once

#include <sqlite3.h>

#include <string>

#include "Database.h"
class SQLiteDatabase : public Database {
   public:
    SQLiteDatabase(std::string filePath);
    ~SQLiteDatabase();
    bool Connect() override;
    void Disconnect() override;
    bool IsConnected() const override;
    int GetTables(char** tables, int max_tables) override;
    int GetTableData(const char* table_name, char** column_names, char*** data,
                     int max_rows, int max_columns, int offset,
                     int& column_count) override;
    int GetTableRowCount(const char* table_name) override;
    int GetTableColumns(char* table_name, ColumnInfo columns[],
                        int max_columns) override;
    bool InsertRow(char* table_name, ColumnInfo columns[], char values[][256],
                   int column_count) override;
    bool UpdateRow(char* table_name, ColumnInfo columns[], char values[][256],
                   int column_count) override;
    bool DeleteRow(char* table_name, char* primary_key,
                   char* primary_key_value) override;
    int GetFreestyleData(const char* query, char** column_names, char*** data,
                         int max_rows, int max_columns,
                         int& column_count) override;

   private:
    sqlite3* connection;
    std::string filePath;
};