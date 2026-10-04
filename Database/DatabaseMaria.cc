#include "DatabaseMaria.h"
#include <iostream>
#include <mysql.h>

MariaDBDatabase::MariaDBDatabase() : connection_mysql(nullptr)
{
}

MariaDBDatabase::~MariaDBDatabase() {
    Disconnect();
}

bool MariaDBDatabase::Connect()
{
    connection_mysql = mysql_init(nullptr);

    if (connection_mysql == nullptr) {
        std::cerr << "[ERROR] mysql_init() failed." << std::endl;
        return false;
    }

    my_bool verify = 0;

    if (mysql_optionsv(
        connection_mysql,
        MYSQL_OPT_SSL_VERIFY_SERVER_CERT,
        &verify) != 0) {
        std::cerr << "[ERROR] Could not disable SSL verification." << std::endl;

        Disconnect();
        return false;
    }

    if (mysql_real_connect(
        connection_mysql,
        "194.164.171.36",
        "andreu",
        "~v6ZRF13vpqlsx@l",
        "practica-rummi",
        3306,
        nullptr,
        0) == nullptr) {
        std::cerr << "[ERROR] Connection failed: " << mysql_error(connection_mysql) << std::endl;

        Disconnect();
        return false;
    }

    return true;
}

void MariaDBDatabase::Disconnect()
{
    // Check if connection is already available
    if (connection_mysql != nullptr) {
        mysql_close(connection_mysql);
        connection_mysql = nullptr;
    }
}

bool MariaDBDatabase::IsConnected() const
{
    return connection_mysql != nullptr;
}

int MariaDBDatabase::GetTables(char** tables, int max_tables)
{
    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    // statement to take all tables
    if (mysql_query(connection_mysql, "SHOW TABLES") != 0) {
        std::cerr << "[ERROR] Could not get tables: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_RES* result = mysql_store_result(connection_mysql);

    if (result == nullptr) {
        std::cerr << "[ERROR] Could not retrieve tables: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_ROW row;

    int table_count = 0;

    // we need need to know how much tables we have in our db, fetch is to take the result as a rows
    while ((row = mysql_fetch_row(result)) != nullptr) {
        if (table_count >= max_tables) {
            break;
        }

        int length = strlen(row[0]);

        // we use to decalre then in this position we will include and array of char's
        tables[table_count] = new char[length + 1];

        // copy strings into a char** (which literally is a string)
        strcpy_s(tables[table_count], length + 1, row[0]);

        table_count++;
    }

    // free memory on sql, same as free casual for dynamic memory
    mysql_free_result(result);

    return table_count;
}

int MariaDBDatabase::GetTableData(const char* table_name, char** column_names, char*** data, int max_rows, int max_columns, int offset, int& column_count) {
    
    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;

        return 0;
    }

    char query[512];

    sprintf_s(
        query,
        "SELECT * FROM %s LIMIT %d OFFSET %d",
        table_name,
        max_rows,
        offset
    );

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not execute SELECT: "
                  << mysql_error(connection_mysql)
                  << std::endl;

        return 0;
    }

    MYSQL_RES* result = mysql_store_result(connection_mysql);

    if (result == nullptr) {
        std::cerr << "[ERROR] Could not retrieve result: "
                  << mysql_error(connection_mysql)
                  << std::endl;

        return 0;
    }

    // Name of columns
    int real_column_count = mysql_num_fields(result);
    column_count = real_column_count;

    if (column_count > max_columns) {
        column_count = max_columns;
    }

    // Columns names
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    for (int i = 0; i < column_count; i++) {
        int length = strlen(fields[i].name);

        column_names[i] = (char*)malloc(length + 1);

        strcpy_s(
            column_names[i],
            length + 1,
            fields[i].name
        );
    }

    // same rows to the x table
    MYSQL_ROW row;

    int row_count = 0;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        if (row_count >= max_rows) {
            break;
        }

        data[row_count] = (char**)malloc(
            sizeof(char*) * column_count
        );

        for (int i = 0; i < column_count; i++) {
            if (row[i] == nullptr) {
                // NULL value to show on the ImGui interface
                data[row_count][i] = (char*)malloc(5);

                strcpy_s(
                    data[row_count][i],
                    5,
                    "NULL"
                );
            } else {
                int length = strlen(row[i]);

                data[row_count][i] = (char*)malloc(
                    length + 1
                );

                strcpy_s(
                    data[row_count][i],
                    length + 1,
                    row[i]
                );
            }
        }

        row_count++;
    }

    mysql_free_result(result);

    return row_count;
}

// Know how many "filas" we have on each table
int MariaDBDatabase::GetTableRowCount(const char* table_name)
{
    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    char query[512];

    // query to take count of registries to tables
    sprintf_s(
        query,
        "SELECT COUNT(*) FROM %s",
        table_name
    );

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not count rows: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_RES* result = mysql_store_result(connection_mysql);

    if (result == nullptr) {
        std::cerr << "[ERROR] Could not retrieve count: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    int row_count = 0;

    if (row != nullptr && row[0] != nullptr) {
        // atoi to text it all
        row_count = atoi(row[0]);
    }

    mysql_free_result(result);

    return row_count;
}

int MariaDBDatabase::GetTableColumns(
    char* table_name,
    ColumnInfo columns[],
    int max_columns)
{
    char query[256];

    sprintf_s(
        query,
        "DESCRIBE `%s`",
        table_name
    );

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not get table columns: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_RES* result = mysql_store_result(connection_mysql);

    if (result == nullptr) {
        std::cerr << "[ERROR] Could not store columns result: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_ROW row;

    int column_count = 0;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        if (column_count >= max_columns) {
            break;
        }

        columns[column_count].name = _strdup(row[0]);
        columns[column_count].type = _strdup(row[1]);

        column_count++;
    }

    mysql_free_result(result);

    return column_count;
}

bool MariaDBDatabase::InsertRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) {
    char query[2048];

    strcpy_s(
        query,
        "INSERT INTO `"
    );

    strcat_s(
        query,
        table_name
    );

    strcat_s(
        query,
        "` ("
    );

    // column names
    for (int i = 0; i < column_count; i++)
    {
        strcat_s(
            query,
            "`"
        );

        strcat_s(
            query,
            columns[i].name
        );

        strcat_s(
            query,
            "`"
        );

        if (i < column_count - 1)
        {
            strcat_s(
                query,
                ", "
            );
        }
    }

    strcat_s(
        query,
        ") VALUES ("
    );

    // values to insert, same as columns
    for (int i = 0; i < column_count; i++) {
        // parseo SQL statement
        strcat_s(
            query,
            "'"
        );

        strcat_s(
            query,
            values[i]
        );

        strcat_s(
            query,
            "'"
        );

        if (i < column_count - 1)
        {
            strcat_s(
                query,
                ", "
            );
        }
    }

    strcat_s(
        query,
        ")"
    );

    // Debug
    std::cout << "[SQL] " << query << std::endl;

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not insert row: " << mysql_error(connection_mysql) << std::endl;

        return false;
    }

    std::cout << "[GOOD] Row inserted!" << std::endl;

    return true;
}

bool MariaDBDatabase::UpdateRow(char* table_name, ColumnInfo columns[], char values[][256], int column_count) {
    char query[2048];

    strcpy_s(
        query,
        "UPDATE `"
    );

    strcat_s(
        query,
        table_name
    );

    strcat_s(
        query,
        "` SET "
    );

    // logic column = value
    for (int i = 0; i < column_count; i++) {
        strcat_s(
            query,
            "`"
        );

        strcat_s(
            query,
            columns[i].name
        );

        strcat_s(
            query,
            "` = '"
        );

        strcat_s(
            query,
            values[i]
        );

        strcat_s(
            query,
            "'"
        );

        if (i < column_count - 1) {
            strcat_s(
                query,
                ", "
            );
        }
    }

    // start the filter to indicate which registry want to update
    strcat_s(
        query,
        " WHERE `"
    );

    // use the name of the first column to identify as primary key
    strcat_s(
        query,
        columns[0].name
    );

    strcat_s(
        query,
        "` = '"
    );

    strcat_s(
        query,
        values[0]
    );

    strcat_s(
        query,
        "'"
    );

    // same debug as insert
    std::cout << "[SQL] " << query << std::endl;

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not update row: " << mysql_error(connection_mysql) << std::endl;

        return false;
    }

    std::cout << "[GOOD] Row updated!" << std::endl;

    return true;
}

bool MariaDBDatabase::DeleteRow(char* table_name, char* primary_key, char* primary_key_value) {

    char query[1024] = {};

    // put like this because is just a delete
    strcpy_s(query, "DELETE FROM `");
    strcat_s(query, table_name);
    strcat_s(query, "` WHERE `");
    strcat_s(query, primary_key);
    strcat_s(query, "` = '");
    strcat_s(query, primary_key_value);
    strcat_s(query, "'");

    // debug
    std::cout << "[SQL] " << query << std::endl;

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not delete row: " << mysql_error(connection_mysql) << std::endl;

        return false;
    }

    return true;
}

int MariaDBDatabase::GetFreestyleData(const char* query, char** column_names, char*** data, int max_rows, int max_columns, int& column_count) {

    // i use return 0 because is a simple way to focus the problem on the main file, if i have 0 count, means it doesnt have tables, so gg
    if (!IsConnected()) {
        std::cerr << "[ERROR] Database is not connected!" << std::endl;
        return 0;
    }

    if (mysql_query(connection_mysql, query) != 0) {
        std::cerr << "[ERROR] Could not execute query: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    MYSQL_RES* result = mysql_store_result(connection_mysql);

    if (result == nullptr) {
        std::cerr << "[ERROR] Could not retrieve result: " << mysql_error(connection_mysql) << std::endl;

        return 0;
    }

    int real_column_count = mysql_num_fields(result);

    column_count = real_column_count;

    if (column_count > max_columns) {
        column_count = max_columns;
    }

    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    for (int i = 0; i < column_count; i++) {

        int length = strlen(fields[i].name);

        column_names[i] = (char*)malloc(length + 1);

        strcpy_s(
            column_names[i],
            length + 1,
            fields[i].name
        );
    }

    MYSQL_ROW row;

    int row_count = 0;

    while ((row = mysql_fetch_row(result)) != nullptr) {

        if (row_count >= max_rows) {
            break;
        }

        data[row_count] = (char**)malloc(sizeof(char*) * column_count);

        for (int i = 0; i < column_count; i++) {

            if (row[i] == nullptr) {

                data[row_count][i] = (char*)malloc(5);

                strcpy_s(
                    data[row_count][i],
                    5,
                    "NULL"
                );

            } else {

                int length = strlen(row[i]);

                data[row_count][i] = (char*)malloc(length + 1);

                strcpy_s(
                    data[row_count][i],
                    length + 1,
                    row[i]
                );
            }
        }

        row_count++;
    }

    mysql_free_result(result);

    return row_count;
}
