#include "Sqlite3Wrapper.hpp"

//std::unordered_map<std::pair<sqlite3*, std::chrono::system_clock::time_point>, bool, SQLiteMapHash, SQLiteMapEqual> SQLite3Stmt::sqlite_db_close_map = std::unordered_map<std::pair<sqlite3*, std::chrono::system_clock::time_point>, bool, SQLiteMapHash, SQLiteMapEqual>();

std::map<std::pair<sqlite3*, std::chrono::system_clock::time_point>, bool> SQLite3Stmt::sqlite_db_close_map = std::map<std::pair<sqlite3*, std::chrono::system_clock::time_point>, bool>();