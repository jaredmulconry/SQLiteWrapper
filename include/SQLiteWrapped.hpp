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

#if !defined(SQLITEWRAPPED_HPP)
#include "sqlite3.h"
#include <memory>
#include <string>
#include <tuple>

#if !defined(WRAP_TEMPLATE)
#define WRAP_TEMPLATE(...) __VA_ARGS__
#endif//!defined(WRAP_TEMPLATE)

#if !defined(ALIAS_TYPE)
#if defined(USE_ALIASES)
#define ALIAS_TYPE(Type, Alias) using Alias = (Type)
#else
#define ALIAS_TYPE(Type, Alias) typedef Type Alias
#endif// defined(USE_ALIASES)
#endif//!defined(ALIAS_TYPE)

#if !defined(NOEXCEPT_SPEC)
#if defined(USE_NOEXCEPT)
#define NOEXCEPT_SPEC noexcept
#define NOEXCEPT_COND_SPEC(cond) noexcept(cond)
#else
#define NOEXCEPT_SPEC
#define NOEXCEPT_COND_SPEC(cond)
#endif// defined(USE_NOEXCEPT)
#endif//!defined(NOEXCEPT_SPEC)

#if !defined(CONSTEXPR_SPEC)
#if defined(USE_CONSTEXPR)
#define CONSTEXPR_SPEC constexpr
#else
#define CONSTEXPR_SPEC
#endif// defined(USE_CONSTEXPR)
#endif//!defined(CONSTEXPR_SPEC)

namespace Sqlt3
{
	ALIAS_TYPE(::sqlite3*, sqlite3_t);
	ALIAS_TYPE(::sqlite3_stmt*, sqlite3_stmt_t);
	ALIAS_TYPE(::sqlite3_backup*, sqlite3_backup_t);
	ALIAS_TYPE(::sqlite3_value*, sqlite3_value_t);
	ALIAS_TYPE(::sqlite3_int64, sqlite3_int64_t);
	ALIAS_TYPE(::sqlite3_uint64, sqlite3_uint64_t);

	namespace detail
	{
		struct openflag_t
		{
			ALIAS_TYPE(int, core_type);
			ALIAS_TYPE(unsigned int, value_type);

			value_type flag;

			openflag_t() NOEXCEPT_SPEC = default;
			CONSTEXPR_SPEC openflag_t(core_type flag) NOEXCEPT_SPEC
				: flag(static_cast<value_type>(flag))
			{
			}
			openflag_t(value_type flag) NOEXCEPT_SPEC : flag(flag)
			{
			}
			openflag_t(const openflag_t&) NOEXCEPT_SPEC = default;
			openflag_t& operator=(const openflag_t&) NOEXCEPT_SPEC = default;

			core_type operator()() const NOEXCEPT_SPEC
			{
				return static_cast<core_type>(flag);
			}

			openflag_t& operator|=(const openflag_t& y) NOEXCEPT_SPEC
			{
				flag |= y.flag;
				return *this;
			}
			openflag_t& operator&=(const openflag_t& y) NOEXCEPT_SPEC
			{
				flag &= y.flag;
				return *this;
			}
			openflag_t& operator^=(const openflag_t& y) NOEXCEPT_SPEC
			{
				flag ^= y.flag;
				return *this;
			}

			inline friend bool operator==(const openflag_t& x,
										  const openflag_t& y) NOEXCEPT_SPEC
			{
				return x() == y();
			}
			inline friend bool operator!=(const openflag_t& x,
										  const openflag_t& y) NOEXCEPT_SPEC
			{
				return !(x == y);
			}
			inline friend bool operator<(const openflag_t& x,
										 const openflag_t& y) NOEXCEPT_SPEC
			{
				return x() < y();
			}
			inline friend bool operator<=(const openflag_t& x,
										  const openflag_t& y) NOEXCEPT_SPEC
			{
				return !(y < x);
			}
			inline friend bool operator>(const openflag_t& x,
										 const openflag_t& y) NOEXCEPT_SPEC
			{
				return y < x;
			}
			inline friend bool operator>=(const openflag_t& x,
										  const openflag_t& y) NOEXCEPT_SPEC
			{
				return !(x < y);
			}
			inline friend openflag_t
				operator|(const openflag_t& x,
						  const openflag_t& y) NOEXCEPT_SPEC
			{
				openflag_t res = x;
				res |= y;
				return res;
			}
			inline friend openflag_t operator&(const openflag_t& x,
											   const openflag_t& y)NOEXCEPT_SPEC
			{
				openflag_t res = x;
				res &= y;
				return res;
			}
			inline friend openflag_t
				operator^(const openflag_t& x,
						  const openflag_t& y) NOEXCEPT_SPEC
			{
				openflag_t res = x;
				res ^= y;
				return res;
			}
		};
		struct BackupDeleter
		{
			ALIAS_TYPE(sqlite3_backup_t, pointer);
			void operator()(pointer p) const NOEXCEPT_SPEC;
		};
		struct ConnectionDeleter
		{
			ALIAS_TYPE(sqlite3_t, pointer);
			void operator()(pointer p) const NOEXCEPT_SPEC;
		};
		struct StatementDeleter
		{
			ALIAS_TYPE(sqlite3_stmt_t, pointer);
			void operator()(pointer p) const NOEXCEPT_SPEC;
		};
		class initialize_t
		{
			bool moved = false;

		public:
			initialize_t() NOEXCEPT_SPEC = default;
			initialize_t(const initialize_t&) = delete;
			initialize_t(initialize_t&& x) NOEXCEPT_SPEC;
			initialize_t& operator=(initialize_t&& x) NOEXCEPT_SPEC;
			~initialize_t() NOEXCEPT_SPEC;
		};

		enum class db_status_t : int
		{
		};
		enum class limit_t : int
		{
		};
		enum class step_result_t : int
		{
		};
		enum class text_encoding_t : unsigned char
		{
		};
		enum class type_t : int
		{
		};
		enum class scan_status_t : int
		{
		};
		enum class status_t : int
		{
		};
		enum class status_counter_t : int
		{
		};
	}

	///<summary>
	/// A flag type that controls what data is reported from
	///<see cref="sqlite3_db_status"/>.
	///</summary>
	ALIAS_TYPE(detail::db_status_t, db_status_t);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/c_limit_attached.html"/>.
	/// A flag type that controls which runtime limits to set in a call to
	///<see cref="sqlite3_limit"/>.
	///</summary>
	ALIAS_TYPE(detail::limit_t, limit_t);
	///<summary>
	/// Alias of a function pointer type that represents a generic destructor
	/// for data passed to SQLite3.
	/// Constants <see cref="sqlite_static"/> and <see cref="sqlite_transient"/>
	/// are also valid.
	///</summary>
	ALIAS_TYPE(::sqlite3_destructor_type, sqlite3_destructor_type_t);
	///<summary>
	/// A flag type to specify how a database connection should be established.
	///</summary>
	ALIAS_TYPE(detail::openflag_t, openflag_t);
	///<summary>
	/// The result of stepping a prepared statement or backup process.
	///</summary>
	ALIAS_TYPE(detail::step_result_t, step_result_t);
	///<summary>
	/// A flag type to control what information is reported from
	///<see cref="sqlite3_stmt_scanstatus"/>.
	///</summary>
	ALIAS_TYPE(detail::scan_status_t, scan_status_t);
	///<summary>
	/// A flag type that controls what data is reported from
	///<see cref="sqlite3_status"/> and <see cref="sqlite3_status64"/>.
	///</summary>
	ALIAS_TYPE(detail::status_t, status_t);
	///<summary>
	/// A flag type that controls what counter information is reported from
	///<see cref="sqlite3_stmt_status"/>.
	///</summary>
	ALIAS_TYPE(detail::status_counter_t, status_counter_t);
	///<summary>
	/// A flag type that specifies text encoding.
	///</summary>
	ALIAS_TYPE(detail::text_encoding_t, text_encoding_t);
	///<summary>
	/// Represents the possible types of a table column.
	///</summary>
	ALIAS_TYPE(detail::type_t, type_t);
	///<summary>
	/// RAII wrapper of a backup process. Upon destruction, automatically
	/// releases all resources allocated by the backup process. Errors on
	/// closure are not thrown.
	///</summary>
	ALIAS_TYPE(
		WRAP_TEMPLATE(std::unique_ptr<sqlite3_backup_t, detail::BackupDeleter>),
		unique_backup);
	///<summary>
	/// RAII wrapper of a database connection. Upon destruction, automatically
	/// closes the connection. Errors on closure are not thrown.
	///</summary>
	ALIAS_TYPE(
		WRAP_TEMPLATE(std::unique_ptr<sqlite3, detail::ConnectionDeleter>),
		unique_connection);
	///<summary>
	/// RAII wrapper of a prepared statement. Upon destruction, automatically
	/// closes the connection. Errors on closure are not thrown.
	///</summary>
	ALIAS_TYPE(
		WRAP_TEMPLATE(std::unique_ptr<sqlite3_stmt, detail::StatementDeleter>),
		unique_statement);
	///<summary>
	/// An alias of UTF-8 input strings.
	///</summary>
	ALIAS_TYPE(const char*, utf8_string_in_t);
	///<summary>
	/// An alias of UTF-8 output strings.
	///</summary>
	ALIAS_TYPE(std::string, utf8_string_out_t);
	///<summary>
	/// An alias of UTF-16 input strings.
	///</summary>
	ALIAS_TYPE(const char16_t*, utf16_string_in_t);
	///<summary>
	/// An alias of UTF-16 output strings.
	///</summary>
	ALIAS_TYPE(std::u16string, utf16_string_out_t);

	const CONSTEXPR_SPEC auto sqlite_dbstatus_lookaside_used =
		db_status_t(SQLITE_DBSTATUS_LOOKASIDE_USED);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_cache_used =
		db_status_t(SQLITE_DBSTATUS_CACHE_USED);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_schema_used =
		db_status_t(SQLITE_DBSTATUS_SCHEMA_USED);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_stmt_used =
		db_status_t(SQLITE_DBSTATUS_STMT_USED);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_lookaside_hit =
		db_status_t(SQLITE_DBSTATUS_LOOKASIDE_HIT);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_miss_size =
		db_status_t(SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_miss_full =
		db_status_t(SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_cache_hit =
		db_status_t(SQLITE_DBSTATUS_CACHE_HIT);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_cache_miss =
		db_status_t(SQLITE_DBSTATUS_CACHE_MISS);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_cache_write =
		db_status_t(SQLITE_DBSTATUS_CACHE_WRITE);
	const CONSTEXPR_SPEC auto sqlite_dbstatus_deferred_fks =
		db_status_t(SQLITE_DBSTATUS_DEFERRED_FKS);

	const CONSTEXPR_SPEC auto sqlite_limit_length =
		limit_t(SQLITE_LIMIT_LENGTH);
	const CONSTEXPR_SPEC auto sqlite_limit_sql_length =
		limit_t(SQLITE_LIMIT_SQL_LENGTH);
	const CONSTEXPR_SPEC auto sqlite_limit_column =
		limit_t(SQLITE_LIMIT_COLUMN);
	const CONSTEXPR_SPEC auto sqlite_limit_expr_depth =
		limit_t(SQLITE_LIMIT_EXPR_DEPTH);
	const CONSTEXPR_SPEC auto sqlite_limit_compound_select =
		limit_t(SQLITE_LIMIT_COMPOUND_SELECT);
	const CONSTEXPR_SPEC auto sqlite_limit_vdbe_op =
		limit_t(SQLITE_LIMIT_VDBE_OP);
	const CONSTEXPR_SPEC auto sqlite_limit_function_arg =
		limit_t(SQLITE_LIMIT_FUNCTION_ARG);
	const CONSTEXPR_SPEC auto sqlite_limit_attached =
		limit_t(SQLITE_LIMIT_ATTACHED);
	const CONSTEXPR_SPEC auto sqlite_limit_like_pattern_length =
		limit_t(SQLITE_LIMIT_LIKE_PATTERN_LENGTH);
	const CONSTEXPR_SPEC auto sqlite_limit_variable_number =
		limit_t(SQLITE_LIMIT_VARIABLE_NUMBER);
	const CONSTEXPR_SPEC auto sqlite_limit_trigger_depth =
		limit_t(SQLITE_LIMIT_TRIGGER_DEPTH);
	const CONSTEXPR_SPEC auto sqlite_limit_worker_threads =
		limit_t(SQLITE_LIMIT_WORKER_THREADS);

	///<summary>
	/// A constant that takes the place of a custom destructor for some
	/// functions. This is interpreted as the provided data is constant, will
	/// never change for the life of the program and do not need to be
	/// destroyed.
	///</summary>
	const CONSTEXPR_SPEC auto sqlite_static =
		sqlite3_destructor_type_t(SQLITE_STATIC);
	///<summary>
	/// A constant that takes the place of a custom destructor for some
	/// functions. This is interpreted as the provided data is likely to change
	/// after the call. SQLite will make a private copy of the data.
	///</summary>
	const CONSTEXPR_SPEC auto sqlite_transient =
		sqlite3_destructor_type_t(SQLITE_TRANSIENT);

	const CONSTEXPR_SPEC auto sqlite_open_readonly =
		openflag_t(SQLITE_OPEN_READONLY);
	const CONSTEXPR_SPEC auto sqlite_open_readwrite =
		openflag_t(SQLITE_OPEN_READWRITE);
	const CONSTEXPR_SPEC auto sqlite_open_create =
		openflag_t(SQLITE_OPEN_CREATE);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_deleteonclose =
		openflag_t(SQLITE_OPEN_DELETEONCLOSE);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_exclusive =
		openflag_t(SQLITE_OPEN_EXCLUSIVE);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_autoproxy =
		openflag_t(SQLITE_OPEN_AUTOPROXY);
	const CONSTEXPR_SPEC auto sqlite_open_uri = openflag_t(SQLITE_OPEN_URI);
	const CONSTEXPR_SPEC auto sqlite_open_memory =
		openflag_t(SQLITE_OPEN_MEMORY);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_main_db =
		openflag_t(SQLITE_OPEN_MAIN_DB);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_temp_db =
		openflag_t(SQLITE_OPEN_TEMP_DB);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_transient_db =
		openflag_t(SQLITE_OPEN_TRANSIENT_DB);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_main_journal =
		openflag_t(SQLITE_OPEN_MAIN_JOURNAL);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_temp_journal =
		openflag_t(SQLITE_OPEN_TEMP_JOURNAL);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_subjournal =
		openflag_t(SQLITE_OPEN_SUBJOURNAL);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_master_journal =
		openflag_t(SQLITE_OPEN_MASTER_JOURNAL);
	const CONSTEXPR_SPEC auto sqlite_open_nomutex =
		openflag_t(SQLITE_OPEN_NOMUTEX);
	const CONSTEXPR_SPEC auto sqlite_open_fullmutex =
		openflag_t(SQLITE_OPEN_FULLMUTEX);
	const CONSTEXPR_SPEC auto sqlite_open_sharedcache =
		openflag_t(SQLITE_OPEN_SHAREDCACHE);
	const CONSTEXPR_SPEC auto sqlite_open_privatecache =
		openflag_t(SQLITE_OPEN_PRIVATECACHE);
	///<summary>Not valid to pass to <see cref="sqlite3_open_v2"/></summary>
	const CONSTEXPR_SPEC auto sqlite_open_wal = openflag_t(SQLITE_OPEN_WAL);

	const CONSTEXPR_SPEC auto sqlite_ok = step_result_t(SQLITE_OK);
	const CONSTEXPR_SPEC auto sqlite_row = step_result_t(SQLITE_ROW);
	const CONSTEXPR_SPEC auto sqlite_done = step_result_t(SQLITE_DONE);

	const CONSTEXPR_SPEC auto sqlite_utf8 = text_encoding_t(SQLITE_UTF8);
	const CONSTEXPR_SPEC auto sqlite_utf16le = text_encoding_t(SQLITE_UTF16LE);
	const CONSTEXPR_SPEC auto sqlite_utf16be = text_encoding_t(SQLITE_UTF16BE);
	const CONSTEXPR_SPEC auto sqlite_utf16 = text_encoding_t(SQLITE_UTF16);
	const CONSTEXPR_SPEC auto sqlite_any = text_encoding_t(SQLITE_ANY);
	const CONSTEXPR_SPEC auto sqlite_utf16_aligned =
		text_encoding_t(SQLITE_UTF16_ALIGNED);

	const CONSTEXPR_SPEC auto sqlite_integer = type_t(SQLITE_INTEGER);
	const CONSTEXPR_SPEC auto sqlite_float = type_t(SQLITE_FLOAT);
	const CONSTEXPR_SPEC auto sqlite_text = type_t(SQLITE_TEXT);
	const CONSTEXPR_SPEC auto sqlite_blob = type_t(SQLITE_BLOB);
	const CONSTEXPR_SPEC auto sqlite_null = type_t(SQLITE_NULL);

	const CONSTEXPR_SPEC auto sqlite_scanstat_nloop =
		scan_status_t(SQLITE_SCANSTAT_NLOOP);
	const CONSTEXPR_SPEC auto sqlite_scanstat_nvisit =
		scan_status_t(SQLITE_SCANSTAT_NVISIT);
	const CONSTEXPR_SPEC auto sqlite_scanstat_est =
		scan_status_t(SQLITE_SCANSTAT_EST);
	const CONSTEXPR_SPEC auto sqlite_scanstat_name =
		scan_status_t(SQLITE_SCANSTAT_NAME);
	const CONSTEXPR_SPEC auto sqlite_scanstat_explain =
		scan_status_t(SQLITE_SCANSTAT_EXPLAIN);
	const CONSTEXPR_SPEC auto sqlite_scanstat_selectid =
		scan_status_t(SQLITE_SCANSTAT_SELECTID);

	const CONSTEXPR_SPEC auto sqlite_status_memory_used =
		status_t(SQLITE_STATUS_MEMORY_USED);
	const CONSTEXPR_SPEC auto sqlite_status_pagecache_used =
		status_t(SQLITE_STATUS_PAGECACHE_USED);
	const CONSTEXPR_SPEC auto sqlite_status_pagecache_overflow =
		status_t(SQLITE_STATUS_PAGECACHE_OVERFLOW);
	const CONSTEXPR_SPEC auto sqlite_status_scratch_used =
		status_t(SQLITE_STATUS_SCRATCH_USED);
	const CONSTEXPR_SPEC auto sqlite_status_scratch_overflow =
		status_t(SQLITE_STATUS_SCRATCH_OVERFLOW);
	const CONSTEXPR_SPEC auto sqlite_status_malloc_size =
		status_t(SQLITE_STATUS_MALLOC_SIZE);
	const CONSTEXPR_SPEC auto sqlite_status_parser_stack =
		status_t(SQLITE_STATUS_PARSER_STACK);
	const CONSTEXPR_SPEC auto sqlite_status_pagecache_size =
		status_t(SQLITE_STATUS_PAGECACHE_SIZE);
	const CONSTEXPR_SPEC auto sqlite_status_scratch_size =
		status_t(SQLITE_STATUS_SCRATCH_SIZE);
	const CONSTEXPR_SPEC auto sqlite_status_malloc_count =
		status_t(SQLITE_STATUS_MALLOC_COUNT);

	const CONSTEXPR_SPEC auto sqlite_stmtstatus_fullscan_step =
		status_counter_t(SQLITE_STMTSTATUS_FULLSCAN_STEP);
	const CONSTEXPR_SPEC auto sqlite_stmtstatus_sort =
		status_counter_t(SQLITE_STMTSTATUS_SORT);
	const CONSTEXPR_SPEC auto sqlite_stmtstatus_autoindex =
		status_counter_t(SQLITE_STMTSTATUS_AUTOINDEX);
	const CONSTEXPR_SPEC auto sqlite_stmtstatus_vm_step =
		status_counter_t(SQLITE_STMTSTATUS_VM_STEP);

	namespace detail
	{
		template <scan_status_t Status>
		struct scan_status_result_t
		{
		};
		template <>
		struct scan_status_result_t<sqlite_scanstat_nloop>
		{
			ALIAS_TYPE(sqlite3_int64_t, result_type);
			ALIAS_TYPE(result_type, param_type);
		};
		template <>
		struct scan_status_result_t<sqlite_scanstat_nvisit>
		{
			ALIAS_TYPE(sqlite3_int64_t, result_type);
			ALIAS_TYPE(result_type, param_type);
		};
		template <>
		struct scan_status_result_t<sqlite_scanstat_est>
		{
			ALIAS_TYPE(double, result_type);
			ALIAS_TYPE(result_type, param_type);
		};
		template <>
		struct scan_status_result_t<sqlite_scanstat_name>
		{
			ALIAS_TYPE(utf8_string_out_t, result_type);
			ALIAS_TYPE(utf8_string_in_t, param_type);
		};
		template <>
		struct scan_status_result_t<sqlite_scanstat_explain>
		{
			ALIAS_TYPE(utf8_string_out_t, result_type);
			ALIAS_TYPE(utf8_string_in_t, param_type);
		};
		template <>
		struct scan_status_result_t<sqlite_scanstat_selectid>
		{
			ALIAS_TYPE(int, result_type);
			ALIAS_TYPE(result_type, param_type);
		};
	}

	///<summary>
	///<see
	/// cref="https://www.sqlite.org/c3ref/backup_finish.html#sqlite3backupfinish"/>.
	/// Releases all resources acquired as part of the backup process.
	///</summary>
	///<param name="backup">RAII wrapper of a backup process.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_backup_finish(unique_backup&& backup);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/backup_finish.html"/>.
	/// Starts up a backup process.
	///</summary>
	///<param name="destinationConnection">The destination connection.</param>
	///<param name="destinationDbName">Name of the destination database.</param>
	///<param name="sourceConnection">The source connection.</param>
	///<param name="sourceDbName">Name of the source database.</param>
	///<returns>RAII wrapper around the new backup process.</returns>
	///<exception name="std::runtime_error"/>
	unique_backup sqlite3_backup_init(sqlite3_t destinationConnection,
									  utf8_string_in_t destinationDbName,
									  sqlite3_t sourceConnection,
									  utf8_string_in_t sourceDbName);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/backup_finish.html"/>.
	/// Retrieves the total number of pages remaining in the source database
	/// after the most recent call to <see cref="sqlite3_backup_step"/>.
	///</summary>
	///<param name="backup">Backup process.</param>
	///<returns>Total pages remaining in source.</returns>
	int sqlite3_backup_pagecount(sqlite3_backup_t backup) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/backup_finish.html"/>.
	/// Retrieves the number of pages still to be backed up after the most
	/// recent
	/// call to <see cref="sqlite3_backup_step"/>.
	///</summary>
	///<param name="backup">Backup process.</param>
	///<returns>Number of pages yet to be backed up.</returns>
	int sqlite3_backup_remaining(sqlite3_backup_t backup) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/backup_finish.html"/>.
	/// Copy up to <paramref name="numPages"/> pages from the source database
	/// to the destination.
	///</summary>
	///<param name="backup">Backup process.</param>
	///<param name="numPages">Number of pages to copy. If negative, all
	/// remaining pages are copied.</param>
	///<returns>
	///<see cref="sqlite_ok"/> if all pages were copied successfully, <see
	/// cref="sqlite_done"/> if all remaining pages were copied successfully.
	/// All other results are errors and are thrown.
	///</returns>
	///<exception name="std::runtime_error"/>
	step_result_t sqlite3_backup_step(sqlite3_backup_t backup, int numPages);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a blob of data to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind the data to.</param>
	///<param name="blob">Blob of arbitrary data to bind.</param>
	///<param name="bytes">Number of bytes in <paramref name="blob"/>.</param>
	///<param name="destruct">A destructor function for <paramref name="blob"/>.
	///</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index, const void* blob,
					  int bytes, sqlite3_destructor_type_t destruct);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a blob of data to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind the data to.</param>
	///<param name="blob">Blob of arbitrary data to bind.</param>
	///<param name="bytes">Number of bytes in <paramref name="blob"/>.</param>
	///<param name="destruct">A destructor function for <paramref name="blob"/>.
	///</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index, const void* blob,
					  sqlite3_uint64_t bytes,
					  sqlite3_destructor_type_t destruct);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a double to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind the data to.</param>
	///<param name="data">Data to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index, double data);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds an integer to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind the data to.</param>
	///<param name="data">Data to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index, int data);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a 64-bit integer to a specified bind point in a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind the data to.</param>
	///<param name="data">Data to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index, sqlite3_int64_t data);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a generic value to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind the data to.</param>
	///<param name="data">Data to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index,
					  const sqlite3_value_t data);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds null to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement with bind points.</param>
	///<param name="index">Index of a bind point to bind null to.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind(sqlite3_stmt_t stmt, int index);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_parameter_count.html"/>.
	/// Retrieve the number of bind points in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<returns>Number of bind points.</returns>
	///<remarks>This routine actually returns the index of the largest
	///(rightmost) parameter. For all forms except ?NNN, this will correspond to
	/// the number of unique parameters. If parameters of the ?NNN form are
	/// used,
	/// there may be gaps in the list.</remarks>
	int sqlite3_bind_parameter_count(sqlite3_stmt_t stmt) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_parameter_index.html"/>.
	/// Retrieve the index of a bind point in a prepared statement by its name.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="name">Bind point name.</param>
	///<returns>Bind point index or 0 if not found.</returns>
	int sqlite3_bind_parameter_index(sqlite3_stmt_t stmt,
									 utf8_string_in_t name) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_parameter_name.html"/>.
	/// Retrieve the name of a bind point of a prepared statement by its index.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="index">Index of a bind point.</param>
	///<returns>The name of the bind point.</returns>
	///<exception name="std::runtime_error"/>
	///<remarks>The first bind point has an index of 1, not 0.</remarks>
	utf8_string_out_t sqlite3_bind_parameter_name(sqlite3_stmt_t stmt,
												  int index);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a UTF-8 string to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="index">Index of a bind point.</param>
	///<param name="text">Text to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind_text(sqlite3_stmt_t stmt, int index,
						   utf8_string_in_t text);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a UTF-16 string to a specified bind point in a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="index">Index of a bind point.</param>
	///<param name="text">Text to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind_text(sqlite3_stmt_t stmt, int index,
						   utf16_string_in_t text);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a string with some encoding to a specified bind point in a
	/// prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="index">Index of a bind point.</param>
	///<param name="text">Text to bind.</param>
	///<param name="encode">Encoding of the text.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind_text(sqlite3_stmt_t stmt, int index,
						   utf8_string_in_t text,
						   detail::text_encoding_t encode);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/bind_blob.html"/>.
	/// Binds a zero-initialised blob to a specified bind point in a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="index">Index of a bind point.</param>
	///<param name="bytes">Number of zeroed bytes to bind.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_bind_zeroblob(sqlite3_stmt_t stmt, int index, int bytes);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/busy_handler.html"/>.
	/// Sets a callback to be invoked whenever an attempt is made to access a
	/// table associated with the specified database connection when another
	/// thread or process has the table locked.
	///</summary>
	///<param name="connection">Database connection</param>
	///<param name="callback">Callback function.
	/// Arg1: <paramref name="data"/>.
	/// Arg2: The number of times the callback has been invoked for the same
	/// locking event.
	/// Result: Non-zero to retry accessing the database, zero to give up and
	/// report an error.
	///</param>
	///<param name="data">Data to pass to the callback.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_busy_handler(sqlite3_t connection, int (*callback)(void*, int),
							  void* data);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/busy_timeout.html"/>.
	/// Sets a busy handler that sleeps for the specified number of milliseconds
	/// when a table associated with the provided database conenction is locked.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="milliseconds">Sleep time until error is reported.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_busy_timeout(sqlite3_t connection, int milliseconds);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/changes.html"/>.
	/// This function returns the number of rows modified, inserted or deleted
	/// by the most recently completed INSERT, UPDATE or DELETE statement on the
	/// database connection specified.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<returns>Number of row modifications.</returns>
	///
	int sqlite3_changes(sqlite3_t connection) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/clear_bindings.html"/>.
	/// Resets the value of all bind points to null.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_clear_bindings(sqlite3_stmt_t stmt);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/close.html"/>.
	/// Closes an existing database connection. This should be preferred over
	/// automatic closing, as this interface can report any errors that occur
	/// during closing.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_close(unique_connection&& connection);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/close.html"/>.
	/// Closes an existing database connection. This should be preferred over
	/// automatic closing, as this interface can report any errors that occur
	/// during closing.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<exception name="std::runtime_error"/>
	///<remarks>Does not fail to close the connection if prepared
	/// statements, blob handles or backup objects still exists.</remarks>
	void sqlite3_close_v2(unique_connection connection);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves a generic blob result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result
	/// from.</param>
	///<returns>The blob of data.</returns>
	///<remarks>The leftmost column of the result set has the index 0.
	/// May only be called if the most recent call to <see cref="sqlite3_step"/>
	/// returned <see cref="sqlite_row"/> and neither
	///<see cref="sqlite3_reset"/> nor <see cref="sqlite3_finalize"/> have been
	/// called subsequently.
	///</remarks>
	const void* sqlite3_column_blob(sqlite3_stmt_t stmt,
									int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves the number of bytes in a generic blob or UTF-8 string result
	/// from the provided column of a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>Numbers of bytes in the result.</returns>
	int sqlite3_column_bytes(sqlite3_stmt_t stmt, int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves the number of bytes in a generic blob or UTF-16 string result
	/// from the provided column of a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>Numbers of bytes in the result.</returns>
	int sqlite3_column_bytes16(sqlite3_stmt_t stmt, int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_count.html"/>.
	/// Retrieves the number of columns in the evaluated prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<returns>Number of columns in result.</returns>
	int sqlite3_column_count(sqlite3_stmt_t stmt) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves a double result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>Double from the provided column.</returns>
	double sqlite3_column_double(sqlite3_stmt_t stmt, int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves an int result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>Int from the provided column.</returns>
	int sqlite3_column_int(sqlite3_stmt_t stmt, int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves a 64-bit integer result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>64-bit integer from the provided column.</returns>
	sqlite3_int64_t sqlite3_column_int64(sqlite3_stmt_t stmt,
										 int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_name.html"/>.
	/// Retrieves the name of the indexed column in the result set of a select
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the name of.</param>
	///<returns>The column name.</returns>
	///<exception name="std::runtime_error"/>
	utf8_string_out_t sqlite3_column_name(sqlite3_stmt_t stmt, int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_name.html"/>.
	/// Retrieves the name of the indexed column in the result set of a select
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the name of.</param>
	///<returns>The column name.</returns>
	///<exception name="std::runtime_error"/>
	///<remarks>The name of a result column is the value of the "AS" clause for
	/// that column, if there is an AS clause. If there is no AS clause then the
	/// name of the column is unspecified and may change from one release of
	/// SQLite to the next. </remarks>
	utf16_string_out_t sqlite3_column_name16(sqlite3_stmt_t stmt, int column);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_database_name.html"/>.
	/// Retrieves the name of the database the specified column of the
	/// provided prepared statement belongs to. The result column in question
	/// must be the result of a SELECT statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Column index.</param>
	///<returns>The name of the database.</returns>
	///<exception name="std::runtime_error"/>
	utf8_string_out_t sqlite3_column_database_name(sqlite3_stmt_t stmt,
												   int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_database_name.html"/>.
	/// Retrieves the name of the database the specified column of the
	/// provided prepared statement belongs to. The result column in question
	/// must be the result of a SELECT statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Column index.</param>
	///<returns>The name of the database.</returns>
	///<exception name="std::runtime_error"/>
	utf16_string_out_t sqlite3_column_database_name16(sqlite3_stmt_t stmt,
													  int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_database_name.html"/>.
	/// Retrieves the name of the specified column of the provided prepared
	/// statement belongs to. The result column in question must be the result
	/// of a SELECT statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Column index.</param>
	///<returns>The name of the column.</returns>
	///<exception name="std::runtime_error"/>
	utf8_string_out_t sqlite3_column_origin_name(sqlite3_stmt_t stmt,
												 int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_database_name.html"/>.
	/// Retrieves the name of the specified column of the
	/// provided prepared statement belongs to. The result column in question
	/// must be the result of a SELECT statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Column index.</param>
	///<returns>The name of the column.</returns>
	///<exception name="std::runtime_error"/>
	utf16_string_out_t sqlite3_column_origin_name16(sqlite3_stmt_t stmt,
													int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_database_name.html"/>.
	/// Retrieves the name of the table the specified column of the
	/// provided prepared statement belongs to. The result column in question
	/// must be the result of a SELECT statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Column index.</param>
	///<returns>The name of the table.</returns>
	///<exception name="std::runtime_error"/>
	utf8_string_out_t sqlite3_column_table_name(sqlite3_stmt_t stmt,
												int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_database_name.html"/>.
	/// Retrieves the name of the table the specified column of the
	/// provided prepared statement belongs to. The result column in question
	/// must be the result of a SELECT statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Column index.</param>
	///<returns>The name of the table.</returns>
	///<exception name="std::runtime_error"/>
	utf16_string_out_t sqlite3_column_table_name16(sqlite3_stmt_t stmt,
												   int column);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves a UTF-8 string result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>UTF-8 string from the provided column.</returns>
	///<exception name="std::runtime_error"/>
	utf8_string_out_t sqlite3_column_text(sqlite3_stmt_t stmt, int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves a UTF-16 string result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>UTF-16 string from the provided column.</returns>
	///<exception name="std::runtime_error"/>
	utf16_string_out_t sqlite3_column_text16(sqlite3_stmt_t stmt, int column);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves the type of the indicated column of a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>Type of indicated column.</returns>
	type_t sqlite3_column_type(sqlite3_stmt_t stmt, int column) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/column_blob.html"/>.
	/// Retrieves a generic value result from the provided column of a prepared
	/// statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="column">Index of a column to retrieve the result from.
	///</param>
	///<returns>Generic value from the provided column.</returns>
	sqlite3_value_t sqlite3_column_value(sqlite3_stmt_t stmt,
										 int column) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/commit_hook.html"/>.
	/// Registers a callback to be invoked whenever a transaction is committed.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="callback">Callback for commits.
	/// Arg 1: <paramref name="data"/>.
	/// Result: Non-zero to turn the commit into a rollback.
	///</param>
	///<param name="data">Data to pass to the callback.</param>
	///<returns>The previous data passed in through <paramref name="data"/>.
	///</returns>
	void* sqlite3_commit_hook(sqlite3_t connection, int (*callback)(void*),
							  void* data) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/complete.html"/>.
	/// Determines whether the provided SQL text forms a complete SQL statement.
	///</summary>
	///<param name="sql">SQL text.</param>
	///<returns>Whether the text forms a complete statement.</returns>
	///<exception name="std::runtime_error"/>
	bool sqlite3_complete(utf8_string_in_t sql);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/complete.html"/>.
	/// Determines whether the provided SQL text forms a complete SQL statement.
	///</summary>
	///<param name="sql">SQL text.</param>
	///<returns>Whether the text forms a complete statement.</returns>
	///<exception name="std::runtime_error"/>
	bool sqlite3_complete16(utf16_string_in_t sql);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/db_status.html"/>.
	/// Retrieves runtime status information about the provided database
	/// connection.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="op">The database operation to retrieve status information
	/// about.</param>
	///<param name="reset">Whether to reset the 'highest instantaneous'
	/// value.</param>
	///<returns>The current value and the highest instantaneous value of the
	/// desired operation.</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<int, int> sqlite3_db_status(sqlite3_t connection, db_status_t op,
										   bool reset);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/exec.html"/>.
	/// Wraps calls to <see cref="sqlite3_prepare_v2"/>, <see
	/// cref="sqlite3_step"/> and <see cref="sqlite3_finalize"/> that allows an
	/// application to run multiple statements of SQL without having to write a
	/// lot of code.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="sql">SQL statement(s).</param>
	///<param name="callback">A callback function to be invoked for each result
	/// row.
	/// Arg1: <paramref name="data"/>.
	/// Arg2: The number of columns in the result.
	/// Arg3: Results for each column in the result, as if
	///<see cref="sqlite3_column_text"/> were called for each valid column
	/// index.
	/// Arg4: The names of each column in the result.
	///</param>
	///<param name="data">Data to pass to the callback.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_exec(sqlite3_t connection, utf8_string_in_t sql,
					  int (*callback)(void*, int, char**, char**), void* data);

	///<summary
	///<see cref="https://www.sqlite.org/c3ref/finalize.html"/>.
	/// Destroys a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_finalize(unique_statement&& stmt);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/initialize.html"/>.
	/// Initialises the SQLite3 library.
	///</summary>
	///<returns>RAII type that invokes <see cref="sqlite3_shutdown"/> upon
	/// destruction.
	///</returns>
	///<exception name="std::runtime_error"/>
	detail::initialize_t sqlite3_initialize();

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/interrupt.html"/>.
	/// Aborts any pending database operations.
	///</summary>
	///<param name="connection">Database connection.</param>
	void sqlite3_interrupt(sqlite3_t connection) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/limit.html"/>.
	/// Set a runtime limit on the specified constructs for the specified
	/// database connection.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="limit">The construct to limit.</param>
	///<param name="newValue">The new limit. A negative value will not change
	/// the current limit.</param>
	///<returns>The previous limit that was set for the specified construct.
	///</returns>
	int sqlite3_limit(sqlite3_t connection, limit_t limit,
					  int newValue) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/next_stmt.html"/>.
	/// Retrieves the next prepared statement associated with the provided
	/// database connection.
	///</summary>
	///<param cref="connection">Database connection.</param>
	///<param cref="stmt">Prepared statement.</param>
	///<returns>The next prepared statement.</returns>
	///<remarks>If <paramref name="stmt"/> is nullptr, the first prepared
	/// statement associated with the database connection.</remarks>
	sqlite3_stmt_t sqlite3_next_stmt(sqlite3_t connection,
									 sqlite3_stmt_t stmt) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/open.html"/>.
	/// Opens a database file.
	///</summary>
	///<param name="filename">Name of the database file.</param>
	///<returns>RAII wrapped database connection.</returns>
	///<exception name="std::runtime_error"/>
	unique_connection sqlite3_open(utf8_string_in_t filename);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/open.html"/>.
	/// Opens a database file.
	///</summary>
	///<param name="filename">Name of the database file.</param>
	///<returns>RAII wrapped database connection.</returns>
	///<exception name="std::runtime_error"/>
	unique_connection sqlite3_open(utf16_string_in_t filename);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/open.html"/>.
	/// Opens a database file.
	///</summary>
	///<param name="filename">Name of the database file.</param>
	///<param name="flags">Flags of type <see cref="openflag_t"/>. See the link
	/// above for more information.</param>
	///<param name="vfs">The name of a Virtual File System. Or nullptr for
	/// default.</param>
	///<returns>RAII wrapped database connection.</returns>
	///<exception name="std::runtime_error"/>
	unique_connection sqlite3_open_v2(utf8_string_in_t filename,
									  openflag_t flags, utf8_string_in_t vfs);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/prepare.html"/>.
	/// Generates a prepared statement from SQL text.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="sql">SQL text.</param>
	///<returns>The prepared statement and the position in the SQL text that has
	/// been parsed upto.</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare(sqlite3_t connection, utf8_string_in_t sql);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/prepare.html"/>.
	/// Generates a prepared statement from SQL text.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="sql">SQL text.</param>
	///<returns>The prepared statement and the position in the SQL text that has
	/// been parsed upto.</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare_v2(sqlite3_t connection, utf8_string_in_t sql);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/prepare.html"/>.
	/// Generates a prepared statement from SQL text.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="sql">SQL text.</param>
	///<returns>The prepared statement and the position in the SQL text that has
	/// been parsed upto.</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare(sqlite3_t connection, utf16_string_in_t sql);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/prepare.html"/>.
	/// Generates a prepared statement from SQL text.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="sql">SQL text.</param>
	///<returns>The prepared statement and the position in the SQL text that has
	/// been parsed upto.</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare_v2(sqlite3_t connection, utf16_string_in_t sql);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/profile.html"/>.
	/// Registers a callback for profiling SQL statement execution.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="profiler">Profiling callback.
	/// Arg1: <paramref name="data"/>.
	/// Arg2: The statement text that was executed.
	/// Arg3: Time taken by the statement. Measured in nanoseconds.
	///</param>
	///<param name="data">Data to pass to the callback.</param>
	///<returns>Unspecified.</returns>
	void* sqlite3_profile(sqlite3_t connection,
						  void (*profiler)(void*, const char*,
										   sqlite3_uint64_t),
						  void* data) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/progress_handler.html"/>.
	/// Register a callback that will be invoked during long running calls to
	///<see cref="sqlite3_exec"/> and <see cref="sqlite3_step"/>.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="instructions">The number of Virtual Machine Instructions
	/// that should be evaluated between successive invocations of the callback.
	/// If less than 1, the progress handler is disabled.</param>
	///<param name="callback">Progress callback. nullptr disables the progress
	/// handler.
	/// Arg 1: <paramref name="data"/>.
	/// Result: Non-zero interrupts the current operation that is currently
	/// active.
	///</param>
	///<param name="data">Data to pass to the callback.</param>
	void sqlite3_progress_handler(sqlite3_t connection, int instructions,
								  int (*callback)(void*),
								  void* data) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/reset.html"/>.
	/// Resets a prepared statement to its initial state. Does not affect values
	/// bound to bind points of the provided prepared statement, for that, you
	/// should call <see cref="sqlite3_clear_bindings"/>.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<exception name="std::runtime_error"/>
	void sqlite3_reset(sqlite3_stmt_t stmt);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/commit_hook.html"/>.
	/// Registers a callback to be invoked whenever a transaction is rolled
	/// back.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="callback">Callback for rollbacks.
	/// Arg 1: <paramref name="data"/>.
	///</param>
	///<param name="data">Data to pass to the callback.</param>
	///<returns>The previous data passed in through <paramref name="data"/>.
	///</returns>
	void* sqlite3_rollback_hook(sqlite3_t connection, void (*callback)(void*),
								void* data) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/initialize.html"/>.
	/// Deallocates any resources allocated by <see cref="sqlite3_initialize"/>.
	///</summary>
	///<param name="init">RAII wrapper produced by
	///<see cref="sqlite3_initialize"/>.
	///</param>
	void sqlite3_shutdown(detail::initialize_t init);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/status.html"/>.
	/// Retrieves SQLite runtime status counters.
	///</summary>
	///<param name="status">A flag indicating which counters to retrieve.
	///</param>
	///<param name="reset">Resets the counters for the specified status.</param>
	///<returns>The current counter value and the highest recorded value.
	///</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<int, int> sqlite3_status(status_t status, bool reset);
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/status.html"/>.
	/// Retrieves SQLite runtime status counters.
	///</summary>
	///<param name="status">A flag indicating which counters to retrieve.
	///</param>
	///<param name="reset">Resets the counters for the specified status.</param>
	///<returns>The current counter value and the highest recorded value.
	///</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<sqlite3_int64_t, sqlite3_int64_t>
		sqlite3_status64(status_t status, bool reset);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/step.html"/>.
	/// Evaluates a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<returns>The result of this evaluation.</returns>
	///<exception name="std::runtime_error"/>
	///<remarks>See the constants defined above for the possible values returned
	/// by this function.</remarks>
	step_result_t sqlite3_step(sqlite3_stmt_t stmt);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/stmt_busy.html"/>.
	/// Checks whether the provided prepared statement is busy. Where busy means
	/// it has been passed to <see cref="sqlite3_step"/>, but not run to
	/// completion, and has not been reset using <see cref="sqlite3_reset"/>.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<returns>Whether the prepared statement is busy.</returns>
	bool sqlite3_stmt_busy(sqlite3_stmt_t stmt) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/stmt_busy.html"/>.
	/// Checks whether the provided prepared statement is read-only.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<returns>Whether the prepared statement is read-only. Will be true
	/// only if the prepared statement makes no direct changes to the content of
	/// the database file.</returns>
	bool sqlite3_stmt_readonly(sqlite3_stmt_t stmt) NOEXCEPT_SPEC;
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/stmt_scanstatus.html"/>.
	/// Retrieves the value of various counters from a prepared statement.
	///</summary>
	///<param name="stmt">Prepared statement.</param>
	///<param name="status">The counter to query.</param>
	///<param name="reset">Whether the counter should be reset.</param>
	///<returns>The value of the desired counter.</returns>
	int sqlite3_stmt_status(sqlite3_stmt_t stmt, status_counter_t status,
							bool reset) NOEXCEPT_SPEC;

	namespace detail
	{
		///<summary>
		/// Helper function for retrieving data for the
		///<see cref="sqlite3_stmt_scanstatus"/> in an opaque way.
		///</summary>
		///<param name="statement">Prepared statement.</param>
		///<param name="index">Loop index.</param>
		///<param name="status">Status information to retrieve.</param>
		///<param name="data">Opaque data (output).</param>
		///<exception name="std::runtime_error"/>
		void get_scanstatus(sqlite3_stmt_t statement, int index,
							scan_status_t status, void* data);
	}

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/stmt_scanstatus_reset.html"/>.
	/// Retrieves information about the predicted and measured performance of
	/// the provided prepared statement.
	///</summary>
	///<param name="statement">Prepared statement.</param>
	///<param name="index">The 0-based index of a loop in the prepared
	/// statement.</param>
	///<returns>
	///<see cref="https://www.sqlite.org/c3ref/c_scanstat_est.html"/>.
	/// What this function returns depends on the provided template argument.
	///</returns>
	///<exception name="std::runtime_error"/>
	///<example><code>
	/// auto name =
	/// Sqlt3::sqlite3_stmt_scanstatus&lt;sqlite_scanstat_name&gt(stmt, index);
	///</code></example>
	template <scan_status_t Status>
	typename detail::scan_status_result_t<Status>::result_type
		sqlite3_stmt_scanstatus(sqlite3_stmt_t statement, int index)
	{
		ALIAS_TYPE(WRAP_TEMPLATE(typename detail::scan_status_result_t<
					   Status>::result_type),
				   result_t);
		ALIAS_TYPE(WRAP_TEMPLATE(typename detail::scan_status_result_t<
					   Status>::param_type),
				   param_t);

		param_t val;
		detail::get_scanstatus(statement, index, Status, &val);

		return result_t(val);
	}
	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/stmt_scanstatus_reset.html"/>.
	/// Zeros all <see ref=""/> related even counters.
	///</summary>
	///<param name="statement">Prepared statement to reset the counters of.
	///</param>
	void sqlite3_stmt_scanstatus_reset(sqlite3_stmt_t statement) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/table_column_metadata.html"/>.
	/// Retrieve the metadata of a column in table in a database.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="database">Name of a database.</param>
	///<param name="table">Name of a table in the database.</param>
	///<param name="column">Name of a column in the table.</param>
	///<returns>The type of data, name of default collation sequence, if it has
	/// the NOT NULL constraint, if it is part of the PRIMARY KEY and if it is
	/// AUTOINCREMENT.</returns>
	///<exception name="std::runtime_error"/>
	std::tuple<utf8_string_out_t, utf8_string_out_t, bool, bool, bool>
		sqlite3_table_column_metadata(sqlite3_t connection,
									  utf8_string_in_t database,
									  utf8_string_in_t table,
									  utf8_string_in_t column);

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/total_changes.html"/>
	/// Retrieves the total number of rows inserted, modified or deleted since
	/// the database connection was opened.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<returns>Total changes.</returns>
	int sqlite3_total_changes(sqlite3_t connection) NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/threadsafe.html"/>.
	/// Checks whether the underlying library was compiled in a thread-safe
	/// mode.
	///</summary>
	///<returns>Whether the library is thread-safe.</returns>
	bool sqlite3_threadsafe() NOEXCEPT_SPEC;

	///<summary>
	///<see cref="https://www.sqlite.org/c3ref/profile.html"/>.
	/// Register a callback for tracing the execution of SQL statements on the
	/// provided database connection.
	///</summary>
	///<param name="connection">Database connection.</param>
	///<param name="tracer">Callback for tracing.
	/// Arg1: <paramref name="data"/>.
	/// Arg2: UTF-8 rendering of the statement text as it first starts
	/// executing.</param>
	///<param name="data">Data to pass to the callback.</param>
	///<returns>Unspecified</returns>
	void* sqlite3_trace(sqlite3_t connection,
						void (*tracer)(void*, const char*),
						void* data) NOEXCEPT_SPEC;
}

#define SQLITEWRAPPED_HPP
#endif// SQLITEWRAPPED_HPP
