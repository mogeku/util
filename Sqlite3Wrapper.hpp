#ifndef SQLITE3WRAPPER_HPP
#define SQLITE3WRAPPER_HPP

#include <string>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <map>

#include "sqlite3.h"

#include "LogUtil.hpp"
#include "util/FileUtil.hpp"

struct SQLiteMapHash
{
	std::size_t operator()(const std::pair<sqlite3*, std::chrono::system_clock::time_point> &k) const
	{
		size_t h1 = std::hash<sqlite3*>()(k.first);
		size_t h2 = std::hash<uint64_t>()(std::chrono::duration_cast<std::chrono::milliseconds>(k.second.time_since_epoch()).count());
		h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);

		return h1;
	}
};

struct SQLiteMapEqual
{
	bool operator()(const std::pair<sqlite3*, std::chrono::system_clock::time_point> &lhs, const std::pair<sqlite3*, std::chrono::system_clock::time_point> &rhs) const
	{
		return lhs.first == rhs.first && lhs.second == rhs.second;
	}
};

class SQLite3Stmt
{
	friend class SQLite3Wrapper;
public:
	SQLite3Stmt(const std::string &sql = "") : stmt_(nullptr), db_(nullptr), sql_(sql)
	{

	}

	SQLite3Stmt(SQLite3Stmt &&rhs) : stmt_(rhs.stmt_), db_(rhs.db_), sql_(std::move(rhs.sql_)), status_(rhs.status_), open_time_(rhs.open_time_)
	{
		rhs.SetNull();
	}

	SQLite3Stmt &operator=(SQLite3Stmt &&rhs)
	{
		if (&rhs != this)
		{
			Finalize();
			sql_ = rhs.sql_;
			stmt_ = rhs.stmt_;
			db_ = rhs.db_;
			open_time_ = rhs.open_time_;
			status_ = rhs.status_;
			rhs.SetNull();
		}

		return *this;
	}

	~SQLite3Stmt()
	{
		if (stmt_)
			Finalize();
	}

	int Status() const
	{
		return status_;
	}

	int Prepare(sqlite3 *db, std::chrono::system_clock::time_point open_time, const char **pzTail)
	{
		db_ = db;
		open_time_ = open_time;
		auto sqlite_close_flag_it = sqlite_db_close_map.find({ db_, open_time_ });
		if (sqlite_close_flag_it == sqlite_db_close_map.end())
			sqlite_db_close_map.insert({ { db_, open_time_ }, false });


		return status_ = sqlite3_prepare_v2(db, sql_.data(), (int)sql_.size(), &stmt_, pzTail);
	}

	int Finalize()
	{
		int ret = 0;
		if (stmt_)
		{
			auto close_flag_it = sqlite_db_close_map.find({ db_ ,open_time_ });
			if (close_flag_it != sqlite_db_close_map.end())
			{
				if (!close_flag_it->second)
				{
					ret = sqlite3_finalize(stmt_);
				}
				else
				{
					sqlite_db_close_map.erase(close_flag_it);
				}
			}
			stmt_ = nullptr;
		}

		status_ = ret;
		return ret;
	}

	int BindInt(int pos, int val)
	{
		int ret = 0;
		if (stmt_)
			ret = sqlite3_bind_int(stmt_, pos, val);

		status_ = ret;
		return ret;
	}

	int BindInt64(int pos, int64_t val)
	{
		int ret = 0;
		if (stmt_)
			ret = sqlite3_bind_int64(stmt_, pos, val);

		status_ = ret;
		return ret;
	}

	int BindDouble(int pos, double val)
	{
		int ret = 0;
		if (stmt_)
			ret = sqlite3_bind_double(stmt_, pos, val);

		status_ = ret;
		return ret;
	}

	int BindText(int pos, const std::string &str, void(*destructor_type)(void*))
	{
		int ret = 0;
		if (stmt_)
			ret = sqlite3_bind_text(stmt_, pos, str.c_str(), str.size(), destructor_type);

		status_ = ret;
		return ret;
	}

	int BindBlob(int pos, const std::vector<unsigned char> blob_data, void(*destructor_type)(void*))
	{
		int ret = 0;
		if (stmt_)
			ret = sqlite3_bind_blob(stmt_, pos, blob_data.data(), blob_data.size(), destructor_type);

		status_ = ret;
		return ret;
	}

    int BindNull(int pos)
    {
        int ret = 0;
        if (stmt_)
            ret = sqlite3_bind_null(stmt_, pos);
        status_ = ret;
        return ret;
    }

	int GetColumnInt(int pos)
	{
		int ret = 0;
		if (stmt_)
			ret = sqlite3_column_int(stmt_, pos);

		status_ = ret;
		return ret;
	}

	int64_t GetColumnInt64(int pos)
	{
        int64_t ret = 0;
		if (stmt_)
			ret = sqlite3_column_int64(stmt_, pos);

		status_ = ret;
		return ret;
	}

	double GetColumnDouble(int pos)
	{
		double ret = 0;
		if (stmt_)
			ret = sqlite3_column_double(stmt_, pos);

		status_ = ret;
		return ret;
	}

	std::string GetColumnText(int pos)
	{
		if (stmt_)
		{
			const char *str = (const char *)sqlite3_column_text(stmt_, pos);
			if (str)
			{
				return str;
			}
		}

		return "";
	}

	std::vector<unsigned char> GetColumnBlob(int pos)
	{
		if (stmt_)
		{
			const unsigned char *byte_ptr = (const unsigned char *)sqlite3_column_blob(stmt_, pos);
			int byte_len = sqlite3_column_bytes(stmt_, pos);

			return std::vector<unsigned char>(byte_ptr, byte_ptr + byte_len);
		}

		return {};
	}

	int GetColumnBytes(int pos)
	{
		int ret = 0;
		if (stmt_)
		{
			ret = sqlite3_column_bytes(stmt_, pos);
		}

		return ret;
	}

	int GetColumnCount()
	{
		int ret = 0;
		if (stmt_)
		{
			ret = sqlite3_column_count(stmt_);
		}

		return ret;
	}

	int GetColumnType(int pos)
	{
		int ret = 0;
		if (stmt_)
		{
			ret = sqlite3_column_type(stmt_, pos);
		}

		return ret;
	}

	std::string GetColumnName(int pos)
	{
		if (stmt_)
		{
			const char *str = (const char *)sqlite3_column_name(stmt_, pos);
			if (str)
			{
				return str;
			}
		}

		return "";
	}

	int Step()
	{
		return status_ = sqlite3_step(stmt_);
	}

	int ResetStmt()
	{
		return status_ = sqlite3_reset(stmt_);
	}

    int GetErrCode() const
    {
        return sqlite3_errcode(db_);
    }

    std::string GetErrMsg() const
    {
        return sqlite3_errmsg(db_);
    }

private:
	void SetNull()
	{
		stmt_ = nullptr;
	}

	int status_;
	sqlite3_stmt *stmt_;
	sqlite3 *db_;
	std::chrono::system_clock::time_point open_time_;
	std::string sql_;

	//static std::unordered_map<std::pair<sqlite3*, std::chrono::system_clock::time_point>, bool, SQLiteMapHash, SQLiteMapEqual> sqlite_db_close_map;
	static std::map<std::pair<sqlite3*, std::chrono::system_clock::time_point>, bool> sqlite_db_close_map;
};

class SQLite3Wrapper
{
public:
	typedef int(*SQLiteCallback)(void*, int, char**, char**);

	static void SQLiteErrorLog(const SQLite3Wrapper &db_manager)
	{
		LOGE("SQLite Error : %s\n", sqlite3_errmsg(db_manager.db_));
	}

	static std::string SQLiteErrorMsg(const SQLite3Wrapper& db_manager)
	{
		const char* msg_ptr = sqlite3_errmsg(db_manager.db_);
		std::string ret_msg;
		if (msg_ptr)
		{
			ret_msg = std::move(std::string(msg_ptr));
		}

		return ret_msg;
	}

	static int SQLiteErrorCode(const SQLite3Wrapper& db_manager)
	{
		return sqlite3_errcode(db_manager.db_);
	}

public:
    SQLite3Wrapper() : db_(nullptr) {}
	SQLite3Wrapper(const std::string &db_file_path) : db_(nullptr), db_file_path_(db_file_path) {}
	SQLite3Wrapper(SQLite3Wrapper &&rhs)
	{
		db_ = rhs.db_;
		db_file_path_ = rhs.db_file_path_;
		open_time_ = rhs.open_time_;
		rhs.SetNull();
	}

	~SQLite3Wrapper()
	{
		if (db_)
		{
			CloseDB();
		}
	}

	SQLite3Wrapper &operator=(SQLite3Wrapper &&rhs)
	{
		if (&rhs != this)
		{
			CloseDB();
			db_ = rhs.db_;
			db_file_path_ = rhs.db_file_path_;
			open_time_ = rhs.open_time_;

			rhs.SetNull();
		}

		return *this;
	}

	int OpenDB()
	{
		int ret = 0;
#ifdef WIN32
		std::wstring db_fullpathw = Utf8ToUtf16(db_file_path_);
		ret = sqlite3_open16(db_fullpathw.c_str(), &db_);
#else
		ret = sqlite3_open(db_file_path_.c_str(), &db_);
#endif // WIN32
		open_time_ = std::chrono::system_clock::now();
		return ret;
	}

	int CloseDB()
	{
		int ret_code = 0;
		if (db_)
		{
#ifdef WIN32
			int rc = sqlite3_close(db_);
#else
			int rc = sqlite3_close_v2(db_);
#endif // WIN32
			if (rc == SQLITE_BUSY)
			{
				sqlite3_stmt * stmt;
				while ((stmt = sqlite3_next_stmt(db_, NULL)) != NULL)
				{
					sqlite3_finalize(stmt);
				}
#ifdef WIN32
				rc = sqlite3_close(db_);
#else
				rc = sqlite3_close_v2(db_);
#endif // WIN32
				if (rc != SQLITE_OK)
				{
					ret_code = rc;
				}
			}

			auto close_flag_it = SQLite3Stmt::sqlite_db_close_map.find({ db_, open_time_ });
			if (close_flag_it != SQLite3Stmt::sqlite_db_close_map.end())
			{
				close_flag_it->second = true;
			}
		}

		if (!ret_code)
			SetNull();

		return ret_code;
	}

	int Begin()
	{
		return sqlite3_exec(db_, "BEGIN", NULL, NULL, NULL);
	}

	int Commit()
	{
		return sqlite3_exec(db_, "COMMIT", NULL, NULL, NULL);
	}

	int Execute(const std::string &sql, SQLiteCallback callback_func, void *arg, char **err_msg)
	{
		return sqlite3_exec(db_, sql.c_str(), callback_func, arg, err_msg);
	}

	SQLite3Stmt GetDBStmt(const std::string &sql)
	{
		SQLite3Stmt stmt(sql);
		stmt.Prepare(db_, open_time_, nullptr);

		return stmt;
	}

	bool IsOpen() const
	{
		return db_ != nullptr;
	}

    int GetErrCode() const
    {
        return sqlite3_errcode(db_);
    }

    std::string GetErrMsg() const
    {
        return sqlite3_errmsg(db_);
    }

	//int Finalize()
	//{
	//	int ret = 0;
	//	if (stmt_)
	//	{
	//		ret = sqlite3_finalize(stmt_);
	//		stmt_ = nullptr;
	//	}

	//	return ret;
	//}

	//int Prepare(const std::string &sql, const char **pzTail)
	//{
	//	return sqlite3_prepare_v2(db_, sql.data(), (int)sql.size(), &stmt_, pzTail);
	//}

	//int BindInt(int pos, int val)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_bind_int(stmt_, pos, val);

	//	return ret;
	//}

	//int BindInt64(int pos, int64_t val)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_bind_int64(stmt_, pos, val);

	//	return ret;
	//}

	//int BindDouble(int pos, double val)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_bind_double(stmt_, pos, val);

	//	return ret;
	//}

	//int BindText(int pos, const std::string &str, void(*destructor_type)(void*))
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_bind_text(stmt_, pos, str.c_str(), str.size(), destructor_type);

	//	return ret;
	//}

	//int BindBlob(int pos, const std::vector<unsigned char> blob_data, void(*destructor_type)(void*))
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_bind_blob(stmt_, pos, blob_data.data(), blob_data.size(), destructor_type);

	//	return ret;
	//}

	//int GetColumnInt(int pos)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_column_int(stmt_, pos);

	//	return ret;
	//}

	//int64_t GetColumnInt64(int pos)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_column_int64(stmt_, pos);

	//	return ret;
	//}

	//double GetColumnDouble(int pos)
	//{
	//	double ret = 0;
	//	if (stmt_)
	//		ret = sqlite3_column_double(stmt_, pos);

	//	return ret;
	//}

	//std::string GetColumnText(int pos)
	//{
	//	if (stmt_)
	//	{
	//		const char *str = (const char *)sqlite3_column_text(stmt_, pos);
	//		if (str)
	//		{
	//			return str;
	//		}
	//	}

	//	return "";
	//}

	//std::vector<unsigned char> GetColumnBlob(int pos)
	//{
	//	if (stmt_)
	//	{
	//		const unsigned char *byte_ptr = (const unsigned char *)sqlite3_column_blob(stmt_, pos);
	//		int byte_len = sqlite3_column_bytes(stmt_, pos);

	//		return std::vector<unsigned char>(byte_ptr, byte_ptr + byte_len);
	//	}

	//	return {};
	//}

	//int GetColumnBytes(int pos)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//	{
	//		ret = sqlite3_column_bytes(stmt_, pos);
	//	}

	//	return ret;
	//}

	//int GetColumnCount()
	//{
	//	int ret = 0;
	//	if (stmt_)
	//	{
	//		ret = sqlite3_column_count(stmt_);
	//	}

	//	return ret;
	//}

	//int GetColumnType(int pos)
	//{
	//	int ret = 0;
	//	if (stmt_)
	//	{
	//		ret = sqlite3_column_type(stmt_, pos);
	//	}

	//	return ret;
	//}

	//int Step()
	//{
	//	return sqlite3_step(stmt_);
	//}

	//int ResetStmt()
	//{
	//	return sqlite3_reset(stmt_);
	//}

private:
    void SetNull()
    {
        db_ = nullptr;
    }

    sqlite3 *db_;
    std::chrono::system_clock::time_point open_time_;
    std::string db_file_path_;
};

#endif // !SQLITE3WRAPPER_HPP
