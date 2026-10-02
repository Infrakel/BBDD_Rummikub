#pragma once

#include <mysql.h>
#include <vector>
#include <string>

struct ColumnInfo {
    char* name;
    char* type;
};

// I know we doesn't make classes yet, but i think i need to have this to make sure the
// structure we want to have on the database layer
class Database
{
public:
    Database();
    
    // ~ means the destruction of this class, disconnect called 
    ~Database();

    bool Connect();
    void Disconnect();

    // I make it this const because i doesnt want that function modify the object Dtabase
    bool IsConnected() const;

    int GetTables(char** tables, int max_tables);

    // the *** is because we need an array to arrays of texts, thats why i use this
    int GetTableData(const char* table_name, char** column_names, char*** data, int max_rows, int max_columns, int offset, int& column_count);

    // we need this for the paginate
    int GetTableRowCount(const char* table_name);

    int GetTableColumns(char* table_name, ColumnInfo columns[], int max_columns);

    // way to insert
    bool InsertRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count);

    // way to update
    bool UpdateRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count);

    // delete registries with the primary key
    bool DeleteRow(char* table_name, char* primary_key, char* primary_key_value);

    // query to do what we want and save the result
    int GetFreestyleData(const char* query, char** column_names, char*** data, int max_rows, int max_columns, int& column_count);

private:
    MYSQL* connection_mysql;
};