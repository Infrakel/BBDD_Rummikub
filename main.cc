/**
 * @file connection.cc
 * @author Andreu Sánchez Castelló (sanchezcas@esat-alumni.com)
 * @brief Program entry point
 */

#include <esat/draw.h>
#include <esat/input.h>
#include <esat/sprite.h>
#include <esat/time.h>
#include <esat/window.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <memory>

// Data Access layer
#include "Database/Database.h"
#include "Database/DatabaseLite.h"
#include "Database/DatabaseMaria.h"

// Native file dialog
#include <nfd.h>

// Our interface, we are gonna use ImGui
#include <esat_extra/imgui.h>

double current_time, last_time;
int fps = 60;

float windowX = 800, windowY = 600;

// names of columns to insert and values to insert too, modify is included
ColumnInfo columns[10];
int column_count = 0;
char insert_values[10][256] = {};
bool show_insert = false;
bool show_update = false;
bool show_delete = false;

// freestyle query
bool show_freestyle = false;
bool show_result = false;
char freestyle_query[2048] = {};

// structure button
bool show_structure = false;

// id field name used for the delete operation
char delete_id[20] = {};

// boolean guards to avoid multiple sql executions per frame
bool table_data_loaded = false;
bool columns_loaded = false;

bool InitializeDB(Database* db) {
    if (!db->Connect()) {
        std::cerr << "[ERROR] Could not connect to our database!" << std::endl;
        return false;
    }

    std::cout << "[GOOD] Connected!" << std::endl;

    return true;
}

void InitializeImgui() {
    ImGui::CreateContext();

    ImGui::SetNextWindowSize(ImVec2(windowX, windowY));

    // position on top left
    ImGui::SetNextWindowPos(ImVec2(0, 0));
}

void DrawTables(int table_count, char* tables[], char** selected_table,
                int& current_page) {
    ImGui::Begin("Tables");

    for (int i = 0; i < table_count; i++) {
        // will store table names, in uppercase
        char button_name[256];

        strcpy_s(button_name, "[ ");
        strcat_s(button_name, tables[i]);
        strcat_s(button_name, " ]");

        // minus to mayus using ASCII code
        for (int j = 0; button_name[j] != '\0'; j++) {
            if (button_name[j] >= 'a' && button_name[j] <= 'z') {
                // difference in all letters between their minus to mayus
                button_name[j] -= 'a' - 'A';
            }
        }

        if (ImGui::Button(button_name)) {
            // reset page when we change table view
            current_page = 0;
            *selected_table = tables[i];

            // selecting a table hides the structure view
            show_structure = false;

            // debug
            std::cout << "Selected table: " << tables[i] << std::endl;

            // cleans the inerted values in the form when the table changes
            memset(insert_values, 0, sizeof(insert_values));
            show_insert = false;
            table_data_loaded = false;
        }
    }

    // Structure button
    if (ImGui::Button("[ STRUCTURE ]")) {
        show_structure = !show_structure;
    }

    ImGui::End();
}

void DrawInsertForm(Database& db, char* selected_table) {
    if (!columns_loaded) {
        column_count = db.GetTableColumns(selected_table, columns, 10);
        columns_loaded = true;
    }

    // Same position as list of registries window
    ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);

    // this have the focus now
    ImGui::SetNextWindowFocus();

    // 0 opacity, not registry fields located
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::Begin("Insert", &show_insert, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("INSERT INTO %s", selected_table);

    ImGui::Separator();

    for (int i = 0; i < column_count; i++) {
        ImGui::Text("%s (%s)", columns[i].name, columns[i].type);

        ImGui::InputText(columns[i].name, insert_values[i], 256);
    }

    ImGui::Separator();

    if (ImGui::Button("INSERT ROW")) {
        if (db.InsertRow(selected_table, columns, insert_values,
                         column_count)) {
            std::cout << "[GOOD] Row inserted!" << std::endl;

            // all users add, deleted it
            memset(insert_values, 0, sizeof(insert_values));

            // close window when we end
            show_insert = false;

            // force table data to reload when we finish all
            table_data_loaded = false;
        }
    }

    ImGui::End();
}

void DrawUpdateForm(Database& db, char* selected_table) {
    // same logic as insert
    if (!columns_loaded) {
        column_count = db.GetTableColumns(selected_table, columns, 10);

        columns_loaded = true;
    }

    // Same position and size as the table window
    ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);

    // Put UPDATE window above the table
    ImGui::SetNextWindowFocus();

    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::Begin("Update", &show_update, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("UPDATE %s", selected_table);

    ImGui::Separator();

    // Values to update
    for (int i = 0; i < column_count; i++) {
        ImGui::Text("%s (%s)", columns[i].name, columns[i].type);

        ImGui::InputText(columns[i].name, insert_values[i], 256);
    }

    ImGui::Separator();

    if (ImGui::Button("UPDATE ROW")) {
        if (db.UpdateRow(selected_table, columns, insert_values,
                         column_count)) {
            std::cout << "[GOOD] Row updated!" << std::endl;

            memset(insert_values, 0, sizeof(insert_values));

            // ¡close window
            show_update = false;

            // force again
            table_data_loaded = false;
        }
    }

    ImGui::End();
}

void DrawDeleteForm(Database& db, char* selected_table) {
    if (!columns_loaded) {
        column_count = db.GetTableColumns(selected_table, columns, 10);

        columns_loaded = true;
    }

    ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);

    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::Begin("Delete", &show_delete, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("DELETE FROM %s", selected_table);

    ImGui::Separator();

    // first column acts as primary_key for the delete operation
    ImGui::Text("%s (%s)", columns[0].name, columns[0].type);

    ImGui::InputText("##DeleteID", delete_id, 256);

    ImGui::Separator();

    if (ImGui::Button("DELETE ROW")) {
        if (db.DeleteRow(selected_table, columns[0].name, delete_id)) {
            std::cout << "[GOOD] Row deleted!" << std::endl;
            // clears id value
            memset(delete_id, 0, sizeof(delete_id));
            show_delete = false;
            table_data_loaded = false;
        }
    }

    ImGui::End();
}

void DrawFreestyleInput(Database& db) {
    ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);

    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowBgAlpha(1.0f);

    ImGui::Begin("Query", &show_freestyle, ImGuiWindowFlags_NoCollapse);

    ImGui::Separator();

    ImGui::Text("Query to execute:");

    // multiline takes a bigger area in screen
    ImGui::InputTextMultiline("##Query", freestyle_query, 2048,
                              ImVec2(450, 200));

    // logic to execute the query
    if (ImGui::Button("Execute!")) {
        std::cout << "[GOOD] Query saved!" << std::endl;

        show_freestyle = false;
        show_result = true;

        show_insert = false;
        show_update = false;
        show_delete = false;
    }

    ImGui::End();
}

void DrawResult(Database& db) {
    char* column_names[10];

    char** data[50];

    int column_count = 0;

    int rows = db.GetFreestyleData(freestyle_query, column_names, data, 50, 10,
                                   column_count);

    ImGui::SetNextWindowPos(ImVec2(300, 300), ImGuiCond_Always);

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Always);

    ImGui::Begin("Query Result", &show_result, ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Query result:");

    ImGui::Separator();

    if (rows > 0 && column_count > 0) {
        if (ImGui::BeginTable(
                "Result", column_count,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            // Declares columns on ImGuitable
            for (int i = 0; i < column_count; i++) {
                ImGui::TableSetupColumn(column_names[i]);
            }

            // Draws header row
            ImGui::TableHeadersRow();

            // Iteratively draws row data
            for (int row = 0; row < rows; row++) {
                ImGui::TableNextRow();
                for (int column = 0; column < column_count; column++) {
                    ImGui::TableSetColumnIndex(column);
                    ImGui::Text("%s", data[row][column]);
                }
            }
            ImGui::EndTable();
        }

    } else {
        ImGui::Text("No data found.");
    }

    ImGui::End();

    // free all memory, good practice
    for (int i = 0; i < column_count; i++) {
        free(column_names[i]);
    }

    for (int row = 0; row < rows; row++) {
        for (int column = 0; column < column_count; column++) {
            free(data[row][column]);
        }

        free(data[row]);
    }
}

void DrawSelectedTableData(Database& db, char* selected_table,
                           int& current_page, int& rows_per_page) {
    // prevents multiple sql statements in same framw
    if (!table_data_loaded) {
        char* column_names[10];

        // set this as much as we want to have
        char** data[50];

        // we set this because have logic on we count how many columns we have
        int column_count = 0;

        // check this to know which columns and type for the insert and update
        ColumnInfo columns[10];
        int info_column_count = db.GetTableColumns(selected_table, columns, 10);

        // How many rows exist in the table
        int total_rows = db.GetTableRowCount(selected_table);

        // Calculate how many pages we have
        int total_pages = 0;

        if (total_rows > 0) {
            total_pages = (total_rows + rows_per_page - 1) / rows_per_page;
        }

        // Calculate the offset depending on the current page
        int offset = current_page * rows_per_page;

        int rows = db.GetTableData(selected_table, column_names, data,
                                   rows_per_page, 10, offset, column_count);

        // Settings to take at right place all the window created right now
        ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);

        ImGui::Begin(selected_table);

        ImGui::Text("Table: %s", selected_table);

        ImGui::Separator();

        if (rows > 0 && column_count > 0) {
            if (ImGui::BeginTable(
                    "TableData", column_count,
                    ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                // Declares column of ImGui table
                for (int i = 0; i < column_count; i++) {
                    ImGui::TableSetupColumn(column_names[i]);
                }

                // Draws header row
                ImGui::TableHeadersRow();

                // Iteratively draws row data
                for (int row = 0; row < rows; row++) {
                    ImGui::TableNextRow();

                    for (int column = 0; column < column_count; column++) {
                        ImGui::TableSetColumnIndex(column);

                        ImGui::Text("%s", data[row][column]);
                    }
                }
                ImGui::EndTable();
            }
        } else {
            ImGui::Text("No data found.");
        }

        ImGui::Separator();

        // Pages

        if (current_page > 0) {
            if (ImGui::Button("< Previous")) {
                current_page--;
            }
        } else {
            ImGui::Button("< Previous");
        }

        ImGui::SameLine();

        ImGui::Text("Page %d / %d", current_page + 1, total_pages);

        ImGui::SameLine();

        if (current_page < total_pages - 1) {
            if (ImGui::Button("Next >")) {
                current_page++;
            }
        } else {
            ImGui::Button("Next >");
        }

        ImGui::Separator();

        // Toolbar

        if (ImGui::Button("INSERT")) {
            show_insert = !show_insert;

            if (show_insert) {
                columns_loaded = false;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("UPDATE")) {
            show_update = !show_update;

            if (show_update) {
                // clean previous values to insert or modify
                memset(insert_values, 0, sizeof(insert_values));

                // we can reuse the same columns
                columns_loaded = false;

                // or update or insert, not both at the same time
                show_insert = false;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("DELETE")) {
            show_delete = !show_delete;

            if (show_delete) {
                memset(delete_id, 0, sizeof(delete_id));

                // just delete, not showing anything else
                show_insert = false;
                show_update = false;

                columns_loaded = false;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("QUERY")) {
            std::cout << "QUERY BUTTON PRESSED!" << std::endl;

            show_freestyle = true;
            show_result = false;

            show_insert = false;
            show_update = false;
            show_delete = false;

            memset(freestyle_query, 0, sizeof(freestyle_query));

            // set default query using the selected_table
            sprintf_s(freestyle_query, "SELECT * FROM `%s`", selected_table);
        }

        ImGui::End();

        // Free memory
        for (int i = 0; i < column_count; i++) {
            free(column_names[i]);
        }

        for (int row = 0; row < rows; row++) {
            for (int column = 0; column < column_count; column++) {
                free(data[row][column]);
            }

            free(data[row]);
        }
    }
}

void DrawStructure(Database& db, int table_count, char* tables[]) {
    // no entry on .ini file of IMGUI, thats why we use ImGuiCond_FirstUseEver
    ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_FirstUseEver);

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);

    ImGui::Begin("Structure");

    ImGui::Text("DATABASE STRUCTURE");

    ImGui::Separator();

    // scroll, needed because we have a lot of tables
    ImGui::BeginChild("StructureScroll", ImVec2(0, 0), true);

    // all tables
    for (int i = 0; i < table_count; i++) {
        ColumnInfo columns[10];

        int column_count = db.GetTableColumns(tables[i], columns, 10);

        // table names
        ImGui::Text("%s", tables[i]);

        ImGui::Separator();

        // column's name
        for (int j = 0; j < column_count; j++) {
            ImGui::Text("    %s (%s)", columns[j].name, columns[j].type);
        }

        // space between table, more design
        ImGui::Spacing();
        ImGui::Spacing();
    }

    ImGui::EndChild();

    ImGui::End();
}

/**
 * @brief Opens a dialog to select a datbase file from the file system
 *
 * @return nfdu8char_t* Full path to the selected file
 */
nfdu8char_t* PickDatabaseFile() {
    if (NFD_Init() != NFD_OKAY) {
        return nullptr;
    }
    char* defaultPath = _fullpath(nullptr, ".", _MAX_PATH);
    nfdu8char_t* outPath = nullptr;
    nfdu8filteritem_t filters[1] = {{"Database file", "db"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;
    args.title = "Select Database File";
    args.defaultPath = defaultPath;
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY) {
        std::cout << "Database file selected: " << outPath << std::endl;
    } else if (result == NFD_CANCEL) {
        outPath = nullptr;
    } else {
        std::cout << "[ERROR] Could not pick file: " << NFD_GetError()
                  << std::endl;
        outPath = nullptr;
    }
    NFD_Quit();
    return outPath;
}

/**
 * @brief Database provider selector
 *
 * @return `int` menu choice, ensures valid option is chosen
 */
int SelectDatabase() {
    std::cout << "Select a database provider:\n"
              << "  [1] MariaDB\n"
              << "  [2] SQLite\n"
              << "  [0] Quit\n"
              << "> ";

    int choice;
    bool correctInput;
    bool validOption;
    do {
        validOption = true;
        correctInput = (bool)(std::cin >> choice);
        if (!correctInput) {
            std::cin.clear();  // clears cin bit flags
            std::cout << "Please enter a number.\n> ";
        } else if (choice < 0 || choice > 2) {
            std::cout << "Invalid option.\n> ";
            validOption = false;
        }
        // clear the input buffer (ignores rest of tokens)
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } while (!correctInput || !validOption);

    return choice;
}

int esat::main(int argc, char** argv) {
    // Database selection
    Database* dbptr;
    nfdu8char_t* sqlitePath;
    int menuSelection;
    bool valid = true;
    do {
        valid = true;
        menuSelection = SelectDatabase();
        switch (menuSelection) {
            // sets up MariaDB connection
            case 1:
                dbptr = new MariaDBDatabase();
                break;
            // sets up sqlitePath, with a file picker
            case 2:
                sqlitePath = PickDatabaseFile();
                if (sqlitePath) {
                    dbptr = new SQLiteDatabase(strdup(sqlitePath));
                    NFD_FreePathU8(sqlitePath);
                } else {
                    std::cerr
                        << "[ERROR] Must pick a valid SQLite database file"
                        << std::endl;
                    valid = false;
                }
                break;
            // exits program
            case 0:
                return 0;
        }
    } while (menuSelection < 0 || menuSelection > 2 || !valid);
    Database& db = *dbptr;
    bool isConnected = InitializeDB(&db);

    // return error, close all
    if (!isConnected) {
        return 1;
    }

    // at the beginning this table is not selected
    char* selected_table = nullptr;
    const int MAX_TABLES = 20;
    int current_page = 0;
    int rows_per_page = 10;
    char* tables[MAX_TABLES];

    // call one time for each program to take the table names
    int table_count = db.GetTables(tables, MAX_TABLES);

    esat::WindowInit(windowX, windowY);
    esat::WindowSetMouseVisibility(true);

    InitializeImgui();

    while (esat::WindowIsOpened() &&
           !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time();

        esat::DrawBegin();
        esat::DrawClear(0, 0, 0);

        DrawTables(table_count, tables, &selected_table, current_page);

        if (selected_table != nullptr) {
            DrawSelectedTableData(db, selected_table, current_page,
                                  rows_per_page);
        }

        if (show_insert && selected_table != nullptr) {
            DrawInsertForm(db, selected_table);
        }

        if (show_update && selected_table != nullptr) {
            DrawUpdateForm(db, selected_table);
        }

        if (show_delete && selected_table != nullptr) {
            DrawDeleteForm(db, selected_table);
        }

        // structure button
        if (show_structure) {
            DrawStructure(db, table_count, tables);
        }

        // this both below is for show the input of a query and then their
        // result
        if (show_freestyle && selected_table != nullptr) {
            DrawFreestyleInput(db);
        }

        if (show_result) {
            DrawResult(db);
        }

        esat::DrawEnd();

        do {
            current_time = esat::Time();
        } while ((current_time - last_time) <= 1000.0 / fps);

        esat::WindowFrame();
    }

    esat::WindowDestroy();
    delete dbptr;
    return 0;
}