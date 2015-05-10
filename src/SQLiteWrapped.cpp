#include "SQLiteWrapped.hpp"
#include <stdexcept>
#include <type_traits>

namespace Sqlt3
{
	ALIAS_TYPE(WRAP_TEMPLATE(std::char_traits<char>), utf8_traits);
	ALIAS_TYPE(WRAP_TEMPLATE(std::char_traits<char16_t>), utf16_traits);

	std::string error_string_without_details(int code)
	{
		std::string err{ "SQLite error(" };
		err += std::to_string(code) + ")";
		return err;
	}
	inline bool result_is_error(int code)
	{
		return !(code == SQLITE_OK || code == SQLITE_ROW ||
				 code == SQLITE_DONE);
	}

	template <typename... Args>
	void throw_error(int code, const Args&...)
	{
		throw std::runtime_error(error_string_without_details(code) + ": " +
								 ::sqlite3_errstr(code));
	}
	template <typename... Args>
	void throw_error(int code, connection_t db, const Args&...)
	{
		throw std::runtime_error(error_string_without_details(code) + ": " +
								 ::sqlite3_errmsg(db));
	}

	template <typename F, typename... Args>
	inline void invoke_with_result_error(F&& f, Args&&... args)
	{
		auto result_code = std::forward<F>(f)(std::forward<Args>(args)...);
		if(result_is_error(result_code)) throw_error(result_code, args...);
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
		auto connect = connection_t(nullptr);
		auto code =
			invoke_with_result(std::forward<F>(openOp), std::forward<S>(file),
							   &connect, std::forward<Args>(args)...);
		auto connection = unique_connection(connect);
		if(result_is_error(code)) throw_error(code, connect);
		return connection;
	}

	void sqlite3_bind(statement_t s, int i, const void* blob, int bytes,
					  void (*destructor)(void*))
	{
		invoke_with_result_error(::sqlite3_bind_blob, s, i, blob, bytes,
								 destructor);
	}
	void sqlite3_bind(statement_t s, int i, const void* blob,
					  sqlite3_uint64 bytes, void (*destructor)(void*))
	{
		invoke_with_result_error(::sqlite3_bind_blob64, s, i, blob, bytes,
								 destructor);
	}
	void sqlite3_bind(statement_t s, int i, double v)
	{
		invoke_with_result_error(::sqlite3_bind_double, s, i, v);
	}
	void sqlite3_bind(statement_t s, int i, int v)
	{
		invoke_with_result_error(::sqlite3_bind_int, s, i, v);
	}
	void sqlite3_bind(statement_t s, int i, sqlite3_int64 v)
	{
		invoke_with_result_error(::sqlite3_bind_int64, s, i, v);
	}
	void sqlite3_bind(statement_t s, int i, const value_t v)
	{
		invoke_with_result_error(::sqlite3_bind_value, s, i, v);
	}
	void sqlite3_bind(statement_t s, int i)
	{
		invoke_with_result_error(::sqlite3_bind_null, s, i);
	}
	int sqlite3_bind_parameter_count(statement_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_bind_parameter_count, s);
	}
	int sqlite3_bind_parameter_index(statement_t s,
									 utf8_string_in_t name) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_bind_parameter_index, s, name);
	}
	utf8_string_out_t sqlite3_bind_parameter_name(statement_t s, int index)
	{
		auto str = invoke_with_result(::sqlite3_bind_parameter_name, s, index);
		if(str == nullptr) return utf8_string_out_t();
		return str;
	}
	void sqlite3_bind_text(statement_t s, int i, utf8_string_in_t str)
	{
		invoke_with_result_error(::sqlite3_bind_text, s, i, str,
								 utf8_traits::length(str) *
									 sizeof(utf8_traits::char_type),
								 SQLITE_TRANSIENT);
	}
	void sqlite3_bind_text(statement_t s, int i, utf8_string_in_t str,
						   text_encoding_t encoding)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<text_encoding_t>::type),
				   encode_t);
		invoke_with_result_error(::sqlite3_bind_text64, s, i, str,
								 utf8_traits::length(str) *
									 sizeof(utf8_traits::char_type),
								 SQLITE_TRANSIENT,
								 static_cast<encode_t>(encoding));
	}
	void sqlite3_bind_text(statement_t s, int i, utf16_string_in_t str)
	{
		invoke_with_result_error(::sqlite3_bind_text16, s, i,
								 static_cast<const void*>(str),
								 utf16_traits::length(str) *
									 sizeof(utf16_traits::char_type),
								 SQLITE_TRANSIENT);
	}
	void sqlite3_bind_zeroblob(statement_t s, int i, int n)
	{
		invoke_with_result_error(::sqlite3_bind_zeroblob, s, i, n);
	}

	int sqlite3_changes(connection_t c) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_changes, c);
	}

	void sqlite3_clear_bindings(statement_t s)
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

	std::tuple<const void*, int> sqlite3_column_blob(statement_t s,
													 int i) NOEXCEPT_SPEC
	{
		return std::make_tuple(invoke_with_result(::sqlite3_column_blob, s, i),
							   invoke_with_result(::sqlite3_column_bytes, s,
												  i));
	}
	int sqlite3_column_count(statement_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_count, s);
	}
	double sqlite3_column_double(statement_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_double, s, i);
	}
	int sqlite3_column_int(statement_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_int, s, i);
	}
	sqlite3_int64 sqlite3_column_int64(statement_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_int, s, i);
	}
	utf8_string_out_t sqlite3_column_name(statement_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_name, s, i);
		if(result == nullptr) return utf8_string_out_t();

		return result;
	}
	utf16_string_out_t sqlite3_column_name16(statement_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_name16, s, i);
		if(result == nullptr) return utf16_string_out_t();

		return static_cast<utf16_string_out_t::const_pointer>(result);
	}
	utf8_string_out_t sqlite3_column_text(statement_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_text, s, i);
		if(result == nullptr) return utf8_string_out_t();

		return reinterpret_cast<utf8_string_out_t::const_pointer>(result);
	}
	utf16_string_out_t sqlite3_column_text16(statement_t s, int i)
	{
		auto result = invoke_with_result(::sqlite3_column_text16, s, i);
		if(result == nullptr) return utf16_string_out_t();

		return static_cast<utf16_string_out_t::const_pointer>(result);
	}
	type_t sqlite3_column_type(statement_t s, int i) NOEXCEPT_SPEC
	{
		return static_cast<type_t>(
			invoke_with_result(::sqlite3_column_type, s, i));
	}
	value_t sqlite3_column_value(statement_t s, int i) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_column_value, s, i);
	}

	void sqlite3_exec(connection_t c, utf8_string_in_t sql,
					  int (*callback)(void*, int, char**, char**), void* arg1)
	{
		char* errorOut = nullptr;
		auto code = invoke_with_result(::sqlite3_exec, c, sql, callback, arg1,
									   &errorOut);
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

	statement_t sqlite3_next_stmt(connection_t c, statement_t s) NOEXCEPT_SPEC
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
		sqlite3_prepare(connection_t c, utf8_string_in_t sql)
	{
		auto stmt = statement_t(nullptr);
		decltype(sql) pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare, c, sql,
								 utf8_traits::length(sql) *
									 sizeof(utf8_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt}, sql + (pos - sql));
	}
	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare_v2(connection_t c, utf8_string_in_t sql)
	{
		auto stmt = statement_t(nullptr);
		decltype(sql) pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare_v2, c, sql,
								 utf8_traits::length(sql) *
									 sizeof(utf8_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt}, sql + (pos - sql));
	}
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare(connection_t c, utf16_string_in_t sql)
	{
		auto stmt = statement_t(nullptr);
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
		sqlite3_prepare_v2(connection_t c, utf16_string_in_t sql)
	{
		auto stmt = statement_t(nullptr);
		const void* pos = nullptr;
		invoke_with_result_error(::sqlite3_prepare16_v2, c, sql,
								 utf16_traits::length(sql) *
									 sizeof(utf16_traits::char_type),
								 &stmt, &pos);

		return std::make_tuple(unique_statement{stmt},
							   sql + (reinterpret_cast<decltype(sql)>(pos) -
									  sql));
	}

	void sqlite3_reset(statement_t s)
	{
		invoke_with_result_error(::sqlite3_reset, s);
	}

	void* sqlite3_profile(connection_t c,
						  void (*callback)(void*, const char*, sqlite3_uint64),
						  void* d) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_profile, c, callback, d);
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
	std::tuple<sqlite3_int64, sqlite3_int64> sqlite3_status64(status_t s,
															  bool r)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<status_t>::type),
				   inner_t);

		sqlite3_int64 current = 0;
		sqlite3_int64 highwater = 0;
		invoke_with_result_error(::sqlite3_status64, static_cast<inner_t>(s),
								 &current, &highwater, r);
		return std::make_tuple(current, highwater);
	}

	step_result_t sqlite3_step(statement_t s)
	{
		auto code = invoke_with_result(::sqlite3_step, s);
		if(result_is_error(code)) {
			throw_error(code);
		}
		return static_cast<step_result_t>(code);
	}

	bool sqlite3_stmt_busy(statement_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_stmt_busy, s) != 0;
	}
	bool sqlite3_stmt_readonly(statement_t s) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_stmt_readonly, s) != 0;
	}

	int sqlite3_stmt_status(statement_t s, status_counter_t c,
							bool r) NOEXCEPT_SPEC
	{
		ALIAS_TYPE(WRAP_TEMPLATE(std::underlying_type<status_counter_t>::type),
				   counter_t);
		return invoke_with_result(::sqlite3_stmt_status, s,
								  static_cast<counter_t>(c),
								  static_cast<int>(r));
	}

	std::tuple<utf8_string_out_t, utf8_string_out_t, int, int, int>
		sqlite3_table_column_metadata(connection_t c, utf8_string_in_t db,
									  utf8_string_in_t t, utf8_string_in_t col)
	{
		const char* type = nullptr, *colSeq = nullptr;
		int notNull = 0, primaryKey = 0, autoInc = 0;

		invoke_with_result_error(::sqlite3_table_column_metadata, c, db, t, col,
								 &type, &colSeq, &notNull, &primaryKey,
								 &autoInc);

		return std::make_tuple(utf8_string_out_t{type},
							   utf8_string_out_t{colSeq}, notNull, primaryKey,
							   autoInc);
	}

	int sqlite3_total_changes(connection_t c) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_total_changes, c);
	}

	void* sqlite3_trace(connection_t c, void (*callback)(void*, const char*),
						void* d) NOEXCEPT_SPEC
	{
		return invoke_with_result(::sqlite3_trace, c, callback, d);
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
