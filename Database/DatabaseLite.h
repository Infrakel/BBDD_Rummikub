/*
    Made it by Guillermo Martorell Hurtado
*/

#pragma once

#include "Database.h"
#include <sqlite3.h>

// I know we doesn't make classes yet, but i think i need to have this to make sure the
// structure we want to have on the database layer
class SQLiteDatabase : public Database 
{
    public:
        SQLiteDatabase();
        ~SQLiteDatabase();
        bool Connect() override;
        void Disconnect() override;
        bool IsConnected() const override;
        int GetTables(char** tables, int max_tables) override;
        int GetTableData(const char* table_name, char** column_names, char*** data, int max_rows, int max_columns, int offset, int& column_count) override;
        int GetTableRowCount(const char* table_name) override;
        int GetTableColumns(char* table_name, ColumnInfo columns[], int max_columns) override;
        bool InsertRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) override;
        bool UpdateRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) override;
        bool DeleteRow(char* table_name, char* primary_key, char* primary_key_value) override;
        int GetFreestyleData(const char* query, char** column_names, char*** data, int max_rows, int max_columns, int& column_count) override;
    private:
        sqlite3* connection;
};