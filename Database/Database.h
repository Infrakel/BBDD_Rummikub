#pragma once

struct ColumnInfo {
    char* name;
    char* type;
};

// I know we doesn't make classes yet, but i think i need to have this to make sure the
// structure we want to have on the database layer
class Database
{
    public:
        virtual ~Database() = default;
        virtual bool Connect() = 0;
        virtual void Disconnect() = 0;
        virtual bool IsConnected() const = 0;
        virtual int GetTables(char** tables, int max_tables) = 0;
        virtual int GetTableData(const char* table_name, char** column_names, char*** data, int max_rows, int max_columns, int offset, int& column_count) = 0;
        virtual int GetTableRowCount(const char* table_name) = 0;
        virtual int GetTableColumns(char* table_name, ColumnInfo columns[], int max_columns) = 0;
        virtual bool InsertRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) = 0;
        virtual bool UpdateRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) = 0;
        virtual bool DeleteRow(char* table_name, char* primary_key, char* primary_key_value) = 0;
        virtual int GetFreestyleData(const char* query, char** column_names, char*** data, int max_rows, int max_columns, int& column_count) = 0;
};