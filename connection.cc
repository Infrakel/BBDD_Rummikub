/**
 * @file connection.cc
 * @author Andreu Sánchez Castelló (sanchezcas@esat-alumni.com)
 * @brief Tests connection to MariaDB database
 */

#include <mysql.h>

#include <iostream>

int main() {
    std::cout << "[1] Initializing MariaDB..." << std::endl;

    MYSQL* connection = mysql_init(nullptr);

    if (connection == nullptr) {
        std::cerr << "[ERROR] mysql_init() fails." << std::endl;
        return 1;
    }

    std::cout << "[2] mysql_init() OK" << std::endl;

    // Issue with the TLS connection, need to desactivate
    my_bool verify = 0;

    if (mysql_optionsv(connection, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify) !=
        0) {
        std::cerr << "[ERROR] Cant download server verification to SSL."
                  << std::endl;

        mysql_close(connection);
        return 1;
    }

    std::cout << "[3] Verification SSL desactivate." << std::endl;

    if (mysql_real_connect(connection, "194.164.171.36", "andreu",
                           "~v6ZRF13vpqlsx@l", "practica-rummi", 3306, nullptr,
                           0) == nullptr) {
        std::cerr << "[ERROR] Connection fails:" << std::endl;

        std::cerr << "        " << mysql_error(connection) << std::endl;

        mysql_close(connection);
        return 1;
    }

    std::cout << "[4] THIS IS WORKING!" << std::endl;

    std::cout << "[5] MariaDB: " << mysql_get_server_info(connection)
              << std::endl;

    std::cout << "\n[6] Tables from practica-rummi database:" << std::endl;

    if (mysql_query(connection, "SHOW TABLES") != 0) {
        std::cerr << "[ERROR] SHOW TABLES fails:" << std::endl;

        std::cerr << "        " << mysql_error(connection) << std::endl;

        mysql_close(connection);
        return 1;
    }

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr) {
        std::cerr << "[ERROR] Damn it, some issue is here:" << std::endl;

        std::cerr << "        " << mysql_error(connection) << std::endl;

        mysql_close(connection);
        return 1;
    }

    MYSQL_ROW row;

    bool found = false;

    while ((row = mysql_fetch_row(result)) != nullptr) {
        found = true;

        std::cout << "  - " << row[0] << std::endl;
    }

    if (!found) {
        std::cout << "  (Doesnt have tables)" << std::endl;
    }

    mysql_free_result(result);

    std::cout << "\n[7] Close in it." << std::endl;

    mysql_close(connection);

    std::cout << "[8] Mamba out" << std::endl;

    return 0;
}