/*
Licence:
	The MIT License (MIT)

	Copyright (c) 2015 Jared Mulconry

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to
	deal in the Software without restriction, including without limitation the
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
	sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
	IN THE SOFTWARE.

Purpose:
	A very thin wrapper around SQLite3, written in C++.
	The C interface of SQLite3 is a good interface for the most part.
	The goal of this wrapper is to add type safety where it is useful,
	apply RAII to outputs and generally make the library cleaner to call
	from C++.
*/

#include "SQLiteWrapped.hpp"
#include <stdexcept>
#include <type_traits>

namespace Sqlt3
{
	ALIAS_TYPE(WRAP_TEMPLATE(std::char_traits<char>), utf8_traits);
	ALIAS_TYPE(WRAP_TEMPLATE(std::char_traits<char16_t>), utf16_traits);

	std::string error_string_without_details(int code)
	{
		std::string err{"SQLite error("};
		err += std::to_string(code) + ")";
		return err;
	}
	inline bool result_is_error(int code)
	{
		return !(code == SQLITE_OK || code == SQLITE_ROW ||
				 code == SQLITE_DONE);
	}

	template <typename... Args>
	inline void throw_error(int code, const Args&...)
	{
		throw std::runtime_error(error_string_without_details(code) + ": " +
								 ::sqlite3_errstr(code));
	}
	template <typename... Args>
	inline void throw_error(int code, sqlite3_t db, const Args&...)
	{
		throw std::runtime_error(error_string_without_details(code) + ": " +
								 ::sqlite3_errmsg(db));
	}

	template <typename F, typename... Args>
	inline auto invoke_with_result_error(F&& f, Args&&... args)
		-> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
	{
		auto result_code = std::forward<F>(f)(std::forward<Args>(args)...);
		if(result_is_error(result_code)) throw_error(result_code, args...);

		return result_code;
	}
	template <typename F, typename... Args>
	inline auto invoke_with_result(F&& f, Args&&... args) NOEXCEPT_SPEC
		-> decltype(std::forward<F>(f)(std::forward<Args>(args)...))
	{
		return std::forward<F>(f)(std::forward<Args>(args)...);
	}

	template <typename F, typename S, typename... Args>
	unique_connection open_connection(F&& openOp, S&& file, Args&&... args)
	{
		auto connect = sqlite3_t(nullptr);
		auto code =
			invoke_with_result(std::forward<F>(openOp), std::forward<S>(file),
							   &connect, std::forward<Args>(args)...);
		auto connection = unique_connection(connect);
		if(result_is_error(code)) throw_error(code, connect);
		return connection;
	}

	void sqlite3_backup_finish(unique_backup&& b)
	{
		invoke_with_result_error(::sqlite3_backup_finish, b.get());
		b.release();
	}
	unique_backup sqlite3_backup_init(sqlite3_t dest_con,
									  utf8_string_in_t dest_db,
									  sqlite3_t src_con,
									  utf8_string_in_t src_db)
	{
		auto backup_ptr = invoke_with_result(::sqlite3_backup_init, dest_con,
											 dest_db, src_con, src_db);
		if(backup_ptr == nullptr) {
			throw_error(::sqlite3_extended_errcode(dest_con), dest_con);
		}

		return unique_backup{backup_ptr};
	}
	int sqlite3_backup_pagecount(sqlite3_backup_t b) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_backup_pagecount, b);
	}
	int sqlite3_backup_remaining(sqlite3_backup_t b) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_backup_remaining, b);
	}
	step_result_t sqlite3_backup_step(sqlite3_backup_t b, int n)
	{
		return static_cast<step_result_t>(
			invoke_with_result_error(::sqlite3_backup_step, b, n));
	}

	void sqlite3_bind(sqlite3_stmt_t s, int i, const void* blob, int bytes,
					  sqlite3_destructor_type_t destructor)
	{
		invoke_with_result_error(::sqlite3_bind_blob, s, i, blob, bytes,
								 destructor);
	}
	void sqlite3_bind(sqlite3_stmt_t s, int i, const void* blob,
					  sqlite3_uint64_t bytes,
					  sqlite3_destructor_type_t destructor)
	{
		invoke_with_result_error(::sqlite3_bind_blob64, s, i, blob, bytes,
								 destructor);
	}
	void sqlite3_bind(sqlite3_stmt_t s, int i, double v)
	{
		invoke_with_result_error(::sqlite3_bind_double, s, i, v);
	}
	void sqlite3_bind(sqlite3_stmt_t s, int i, int v)
	{
		invoke_with_result_error(::sqlite3_bind_int, s, i, v);
	}
	void sqlite3_bind(sqlite3_stmt_t s, int i, sqlite3_int64_t v)
	{
		invoke_with_result_error(::sqlite3_bind_int64, s, i, v);
	}
	void sqlite3_bind(sqlite3_stmt_t s, int i, const sqlite3_value_t v)
	{
		invoke_with_result_error(::sqlite3_bind_value, s, i, v);
	}
	void sqlite3_bind(sqlite3_stmt_t s, int i)
	{
		invoke_with_result_error(::sqlite3_bind_null, s, i);
	}
	int sqlite3_bind_parameter_count(sqlite3_stmt_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_bind_parameter_count, s);
	}
	int sqlite3_bind_parameter_index(sqlite3_stmt_t s,
									 utf8_string_in_t name) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_bind_parameter_index, s, name);
	}
	utf8_string_out_t sqlite3_bind_parameter_name(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_bind_parameter_name, s, i);
		if(str == nullptr) return utf8_string_out_t();
		return str;
	}
	void sqlite3_bind_text(sqlite3_stmt_t s, int i, utf8_string_in_t str)
	{
		invoke_with_result_error(::sqlite3_bind_text, s, i, str,
								 utf8_traits::length(str) *
									 sizeof(utf8_traits::char_type),
								 sqlite_transient);
	}
	void sqlite3_bind_text(sqlite3_stmt_t s, int i, utf8_string_in_t str,
						   text_encoding_t encoding)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<text_encoding_t>::type),
				   encode_t);
		invoke_with_result_error(::sqlite3_bind_text64, s, i, str,
								 utf8_traits::length(str) *
									 sizeof(utf8_traits::char_type),
								 sqlite_transient,
								 static_cast<encode_t>(encoding));
	}
	void sqlite3_bind_text(sqlite3_stmt_t s, int i, utf16_string_in_t str)
	{
		invoke_with_result_error(::sqlite3_bind_text16, s, i,
								 static_cast<const void*>(str),
								 utf16_traits::length(str) *
									 sizeof(utf16_traits::char_type),
								 sqlite_transient);
	}
	void sqlite3_bind_zeroblob(sqlite3_stmt_t s, int i, int n)
	{
		invoke_with_result_error(::sqlite3_bind_zeroblob, s, i, n);
	}

	void sqlite3_busy_handler(sqlite3_t c, int (*callback)(void*, int), void* d)
	{
		invoke_with_result_error(::sqlite3_busy_handler, c, callback, d);
	}
	void sqlite3_busy_timeout(sqlite3_t c, int ms)
	{
		invoke_with_result_error(::sqlite3_busy_timeout, c, ms);
	}

	int sqlite3_changes(sqlite3_t c) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_changes, c);
	}

	void sqlite3_clear_bindings(sqlite3_stmt_t s)
	{
		invoke_with_result_error(::sqlite3_clear_bindings, s);
	}

	void sqlite3_close(unique_connection&& c)
	{
		invoke_with_result_error(::sqlite3_close, c.get());
		c.release();
	}
	void sqlite3_close_v2(unique_connection c)
	{
		invoke_with_result_error(::sqlite3_close_v2, c.get());
	}

	const void* sqlite3_column_blob(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_blob, s, i);
	}
	int sqlite3_column_bytes(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_bytes, s, i);
	}
	int sqlite3_column_bytes16(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_bytes16, s, i);
	}
	int sqlite3_column_count(sqlite3_stmt_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_count, s);
	}

	double sqlite3_column_double(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_double, s, i);
	}
	int sqlite3_column_int(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_int, s, i);
	}
	sqlite3_int64_t sqlite3_column_int64(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_int, s, i);
	}
	utf8_string_out_t sqlite3_column_name(sqlite3_stmt_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_name, s, i);
		if(result == nullptr) return utf8_string_out_t();

		return result;
	}
	utf16_string_out_t sqlite3_column_name16(sqlite3_stmt_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_name16, s, i);
		if(result == nullptr) return utf16_string_out_t();

		return static_cast<utf16_string_out_t::const_pointer>(result);
	}
#if defined(SQLITE_ENABLE_COLUMN_METADATA)
	utf8_string_out_t sqlite3_column_database_name(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_column_database_name, s, i);
		if(str == nullptr) {
			return utf8_string_out_t{};
		}

		return str;
	}
	utf16_string_out_t sqlite3_column_database_name16(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_column_database_name16, s, i);
		if(str == nullptr) {
			return utf16_string_out_t{};
		}

		return static_cast<utf16_string_out_t::const_pointer>(str);
	}
	utf8_string_out_t sqlite3_column_origin_name(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_column_origin_name, s, i);
		if(str == nullptr) {
			return utf8_string_out_t{};
		}

		return str;
	}
	utf16_string_out_t sqlite3_column_origin_name16(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_column_origin_name16, s, i);
		if(str == nullptr) {
			return utf16_string_out_t{};
		}

		return static_cast<utf16_string_out_t::const_pointer>(str);
	}
	utf8_string_out_t sqlite3_column_table_name(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_column_table_name, s, i);
		if(str == nullptr) {
			return utf8_string_out_t{};
		}

		return str;
	}
	utf16_string_out_t sqlite3_column_table_name16(sqlite3_stmt_t s, int i)
	{
		auto str = invoke_with_result(::sqlite3_column_table_name16, s, i);
		if(str == nullptr) {
			return utf16_string_out_t{};
		}

		return static_cast<utf16_string_out_t::const_pointer>(str);
	}
#endif// defined(SQLITE_ENABLE_COLUMN_METADATA)
	utf8_string_out_t sqlite3_column_text(sqlite3_stmt_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_text, s, i);
		if(result == nullptr) return utf8_string_out_t();

		return reinterpret_cast<utf8_string_out_t::const_pointer>(result);
	}
	utf16_string_out_t sqlite3_column_text16(sqlite3_stmt_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_text16, s, i);
		if(result == nullptr) return utf16_string_out_t();

		return static_cast<utf16_string_out_t::const_pointer>(result);
	}
	type_t sqlite3_column_type(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return static_cast<type_t>(
			invoke_with_result(::sqlite3_column_type, s, i));
	}
	sqlite3_value_t sqlite3_column_value(sqlite3_stmt_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_value, s, i);
	}

	void* sqlite3_commit_hook(sqlite3_t c, int (*callback)(void*),
							  void* d) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_commit_hook, c, callback, d);
	}

	bool sqlite3_complete(utf8_string_in_t sql)
	{
		auto code = invoke_with_result(::sqlite3_complete, sql);

		if(code != 0 && code != 1) {
			throw_error(code);
		}
		return code != 0;
	}
	bool sqlite3_complete16(utf16_string_in_t sql)
	{
		auto code = invoke_with_result(::sqlite3_complete16, sql);

		if(code != 0 && code != 1) {
			throw_error(code);
		}
		return code != 0;
	}

	std::tuple<int, int> sqlite3_db_status(sqlite3_t c, db_status_t o, bool r)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<db_status_t>::type),
				   inner_t);
		int current = 0, highwater = 0;
		invoke_with_result_error(::sqlite3_db_status, c,
								 static_cast<inner_t>(o), &current, &highwater,
								 static_cast<int>(r));

		return std::make_tuple(current, highwater);
	}

	void sqlite3_exec(sqlite3_t c, utf8_string_in_t sql,
					  int (*callback)(void*, int, char**, char**), void* d)
	{
		char* errorOut = nullptr;
		auto code =
			invoke_with_result(::sqlite3_exec, c, sql, callback, d, &errorOut);
		if(result_is_error(code)) {
			auto err = error_string_without_details(code) + ": " + errorOut;
			sqlite3_free(errorOut);
			throw std::runtime_error(err);
		}
	}

	void sqlite3_finalize(unique_statement&& s)
	{
		invoke_with_result_error(::sqlite3_finalize, s.release());
	}

	detail::initialize_t sqlite3_initialize()
	{
		invoke_with_result_error(::sqlite3_initialize);
		return {};
	}

	void sqlite3_interrupt(sqlite3_t c) NOEXCEPT_SPEC
	{
		invoke_with_result(::sqlite3_interrupt, c);
	}

	int sqlite3_limit(sqlite3_t c, limit_t l, int v) NOEXCEPT_SPEC
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<limit_t>::type), inner_t);
		return invoke_with_result(::sqlite3_limit, c, static_cast<inner_t>(l),
								  v);
	}

	sqlite3_stmt_t sqlite3_next_stmt(sqlite3_t c,
									 sqlite3_stmt_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_next_stmt, c, s);
	}

	unique_connection sqlite3_open(utf8_string_in_t file)
	{
		return open_connection(::sqlite3_open, file);
	}
	unique_connection sqlite3_open(utf16_string_in_t file)
	{
		return open_connection(::sqlite3_open16, file);
	}
	unique_connection sqlite3_open_v2(utf8_string_in_t file, openflag_t flags,
									  utf8_string_in_t vfs)
	{
		return open_connection(::sqlite3_open_v2, file, flags(), vfs);
	}

	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare(sqlite3_t c, utf8_string_in_t sql)
	{
		auto stmt = sqlite3_stmt_t(nullptr);
		decltype(sql) pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare, c, sql,
								 utf8_traits::length(sql) *
									 sizeof(utf8_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt}, sql + (pos - sql));
	}
	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare_v2(sqlite3_t c, utf8_string_in_t sql)
	{
		auto stmt = sqlite3_stmt_t(nullptr);
		decltype(sql) pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare_v2, c, sql,
								 utf8_traits::length(sql) *
									 sizeof(utf8_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt}, sql + (pos - sql));
	}
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare(sqlite3_t c, utf16_string_in_t sql)
	{
		auto stmt = sqlite3_stmt_t(nullptr);
		const void* pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare16, c, sql,
								 utf16_traits::length(sql) *
									 sizeof(utf16_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt},
							   sql + (reinterpret_cast<decltype(sql)>(pos) -
									  sql));
	}
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare_v2(sqlite3_t c, utf16_string_in_t sql)
	{
		auto stmt = sqlite3_stmt_t(nullptr);
		const void* pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare16_v2, c, sql,
								 utf16_traits::length(sql) *
									 sizeof(utf16_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt},
							   sql + (reinterpret_cast<decltype(sql)>(pos) -
									  sql));
	}

	void* sqlite3_profile(sqlite3_t c, void (*callback)(void*, const char*,
														sqlite3_uint64_t),
						  void* d) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_profile, c, callback, d);
	}

	void sqlite3_progress_handler(sqlite3_t c, int inst, int (*callback)(void*),
								  void* d) NOEXCEPT_SPEC
	{
		invoke_with_result(::sqlite3_progress_handler, c, inst, callback, d);
	}

	void sqlite3_reset(sqlite3_stmt_t s)
	{
		invoke_with_result_error(::sqlite3_reset, s);
	}

	void* sqlite3_rollback_hook(sqlite3_t c, void (*callback)(void*),
								void* d) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_rollback_hook, c, callback, d);
	}

	void sqlite3_shutdown(detail::initialize_t init)
	{
	}

	std::tuple<int, int> sqlite3_status(status_t s, bool r)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<status_t>::type),
				   inner_t);

		int current = 0;
		int highwater = 0;
		invoke_with_result_error(::sqlite3_status, static_cast<inner_t>(s),
								 &current, &highwater, r);
		return std::make_tuple(current, highwater);
	}
	std::tuple<sqlite3_int64_t, sqlite3_int64_t> sqlite3_status64(status_t s,
																  bool r)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<status_t>::type),
				   inner_t);

		sqlite3_int64_t current = 0;
		sqlite3_int64_t highwater = 0;
		invoke_with_result_error(::sqlite3_status64, static_cast<inner_t>(s),
								 &current, &highwater, r);
		return std::make_tuple(current, highwater);
	}

	step_result_t sqlite3_step(sqlite3_stmt_t s)
	{
		return static_cast<step_result_t>(
			invoke_with_result_error(::sqlite3_step, s));
	}

	bool sqlite3_stmt_busy(sqlite3_stmt_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_stmt_busy, s) != 0;
	}
	bool sqlite3_stmt_readonly(sqlite3_stmt_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_stmt_readonly, s) != 0;
	}

	int sqlite3_stmt_status(sqlite3_stmt_t s, status_counter_t c,
							bool r) NOEXCEPT_SPEC
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<status_counter_t>::type),
				   counter_t);
		return invoke_with_result(::sqlite3_stmt_status, s,
								  static_cast<counter_t>(c),
								  static_cast<int>(r));
	}

#if defined(SQLITE_ENABLE_STMT_SCANSTATUS)
	namespace detail
	{
		void get_scanstatus(sqlite3_stmt_t s, int i, scan_status_t stat,
							void* d)
		{
			ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<scan_status_t>::type),
					   inner_t);
			invoke_with_result_error(::sqlite3_stmt_scanstatus, s, i,
									 static_cast<inner_t>(stat), d);
		}
	}

	void sqlite3_stmt_scanstatus_reset(sqlite3_stmt_t s) NOEXCEPT_SPEC
	{
		invoke_with_result(::sqlite3_stmt_scanstatus_reset, s);
	}
#endif// defined(SQLITE_ENABLE_STMT_SCANSTATUS)

	std::tuple<utf8_string_out_t, utf8_string_out_t, bool, bool, bool>
		sqlite3_table_column_metadata(sqlite3_t c, utf8_string_in_t db,
									  utf8_string_in_t t, utf8_string_in_t col)
	{
		const char* type = nullptr, * colSeq = nullptr;
		int notNull = 0, primaryKey = 0, autoInc = 0;

		invoke_with_result_error(::sqlite3_table_column_metadata, c, db, t, col,
								 &type, &colSeq, &notNull, &primaryKey,
								 &autoInc);

		return std::make_tuple(utf8_string_out_t{type},
							   utf8_string_out_t{colSeq}, notNull != 0,
							   primaryKey != 0, autoInc != 0);
	}

	int sqlite3_total_changes(sqlite3_t c) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_total_changes, c);
	}

	void* sqlite3_trace(sqlite3_t c, void (*callback)(void*, const char*),
						void* d) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_trace, c, callback, d);
	}

	bool sqlite3_threadsafe() NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_threadsafe) != 0;
	}

	namespace detail
	{
		initialize_t::initialize_t(initialize_t&& x) NOEXCEPT_SPEC
		{
			x.moved = true;
		}
		initialize_t& initialize_t::operator=(initialize_t&& x) NOEXCEPT_SPEC
		{
			if(this != &x) {
				x.moved = true;
			}
			return *this;
		}
		initialize_t::~initialize_t() NOEXCEPT_SPEC
		{
			if(!moved) {
				::sqlite3_shutdown();
			}
		}
		void BackupDeleter::operator()(pointer p) const
		{
			::sqlite3_backup_finish(p);
		}
		void ConnectionDeleter::operator()(pointer p) const NOEXCEPT_SPEC
		{
			::sqlite3_close(p);
		}
		void StatementDeleter::operator()(pointer p) const NOEXCEPT_SPEC
		{
			::sqlite3_finalize(p);
		}
	}
}
