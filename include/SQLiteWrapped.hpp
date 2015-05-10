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
		struct initialize_t
		{
			bool moved = false;

			initialize_t() NOEXCEPT_SPEC = default;
			initialize_t(const initialize_t&) = delete;
			initialize_t(initialize_t&& x) NOEXCEPT_SPEC;
			initialize_t& operator=(initialize_t&& x) NOEXCEPT_SPEC;
			~initialize_t() NOEXCEPT_SPEC;
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
		enum class status_t : int
		{
		};
		enum class status_counter_t : int
		{
		};
	}

	ALIAS_TYPE(detail::openflag_t, openflag_t);
	ALIAS_TYPE(detail::step_result_t, step_result_t);
	ALIAS_TYPE(detail::status_t, status_t);
	ALIAS_TYPE(detail::status_counter_t, status_counter_t);
	ALIAS_TYPE(detail::text_encoding_t, text_encoding_t);
	ALIAS_TYPE(detail::type_t, type_t);
	ALIAS_TYPE(
		WRAP_TEMPLATE(std::unique_ptr<sqlite3, detail::ConnectionDeleter>),
		unique_connection);
	ALIAS_TYPE(
		WRAP_TEMPLATE(std::unique_ptr<sqlite3_stmt, detail::StatementDeleter>),
		unique_statement);
	ALIAS_TYPE(const char*, utf8_string_in_t);
	ALIAS_TYPE(std::string, utf8_string_out_t);
	ALIAS_TYPE(const char16_t*, utf16_string_in_t);
	ALIAS_TYPE(std::u16string, utf16_string_out_t);

	const CONSTEXPR_SPEC auto sqlite_open_readonly =
		openflag_t(SQLITE_OPEN_READONLY);
	const CONSTEXPR_SPEC auto sqlite_open_readwrite =
		openflag_t(SQLITE_OPEN_READWRITE);
	const CONSTEXPR_SPEC auto sqlite_open_create =
		openflag_t(SQLITE_OPEN_CREATE);
	const CONSTEXPR_SPEC auto sqlite_open_deleteonclose =
		openflag_t(SQLITE_OPEN_DELETEONCLOSE);
	const CONSTEXPR_SPEC auto sqlite_open_exclusive =
		openflag_t(SQLITE_OPEN_EXCLUSIVE);
	const CONSTEXPR_SPEC auto sqlite_open_autoproxy =
		openflag_t(SQLITE_OPEN_AUTOPROXY);
	const CONSTEXPR_SPEC auto sqlite_open_uri = openflag_t(SQLITE_OPEN_URI);
	const CONSTEXPR_SPEC auto sqlite_open_memory =
		openflag_t(SQLITE_OPEN_MEMORY);
	const CONSTEXPR_SPEC auto sqlite_open_main_db =
		openflag_t(SQLITE_OPEN_MAIN_DB);
	const CONSTEXPR_SPEC auto sqlite_open_temp_db =
		openflag_t(SQLITE_OPEN_TEMP_DB);
	const CONSTEXPR_SPEC auto sqlite_open_transient_db =
		openflag_t(SQLITE_OPEN_TRANSIENT_DB);
	const CONSTEXPR_SPEC auto sqlite_open_main_journal =
		openflag_t(SQLITE_OPEN_MAIN_JOURNAL);
	const CONSTEXPR_SPEC auto sqlite_open_temp_journal =
		openflag_t(SQLITE_OPEN_TEMP_JOURNAL);
	const CONSTEXPR_SPEC auto sqlite_open_subjournal =
		openflag_t(SQLITE_OPEN_SUBJOURNAL);
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

	void sqlite3_bind(sqlite3_stmt_t, int, const void*, int, void (*)(void*));
	void sqlite3_bind(sqlite3_stmt_t, int, const void*, sqlite3_uint64_t,
					  void (*)(void*));
	void sqlite3_bind(sqlite3_stmt_t, int, double);
	void sqlite3_bind(sqlite3_stmt_t, int, int);
	void sqlite3_bind(sqlite3_stmt_t, int, sqlite3_int64_t);
	void sqlite3_bind(sqlite3_stmt_t, int, const sqlite3_value_t);
	void sqlite3_bind(sqlite3_stmt_t, int);
	int sqlite3_bind_parameter_count(sqlite3_stmt_t) NOEXCEPT_SPEC;
	int sqlite3_bind_parameter_index(sqlite3_stmt_t,
									 utf8_string_in_t) NOEXCEPT_SPEC;
	utf8_string_out_t sqlite3_bind_parameter_name(sqlite3_stmt_t, int);
	void sqlite3_bind_text(sqlite3_stmt_t, int, utf8_string_in_t);
	void sqlite3_bind_text(sqlite3_stmt_t, int, utf8_string_in_t,
						   detail::text_encoding_t);
	void sqlite3_bind_text(sqlite3_stmt_t, int, utf16_string_in_t);
	void sqlite3_bind_zeroblob(sqlite3_stmt_t, int, int);

	int sqlite3_changes(sqlite3_t) NOEXCEPT_SPEC;

	void sqlite3_clear_bindings(sqlite3_stmt_t);

	void sqlite3_close(unique_connection&&);
	void sqlite3_close_v2(unique_connection);

	std::tuple<const void*, int> sqlite3_column_blob(sqlite3_stmt_t,
													 int) NOEXCEPT_SPEC;
	int sqlite3_column_count(sqlite3_stmt_t) NOEXCEPT_SPEC;
	double sqlite3_column_double(sqlite3_stmt_t, int) NOEXCEPT_SPEC;
	int sqlite3_column_int(sqlite3_stmt_t, int) NOEXCEPT_SPEC;
	sqlite3_int64_t sqlite3_column_int64(sqlite3_stmt_t, int) NOEXCEPT_SPEC;
	utf8_string_out_t sqlite3_column_name(sqlite3_stmt_t, int);
	utf16_string_out_t sqlite3_column_name16(sqlite3_stmt_t, int);
	utf8_string_out_t sqlite3_column_text(sqlite3_stmt_t, int);
	utf16_string_out_t sqlite3_column_text16(sqlite3_stmt_t, int);
	detail::type_t sqlite3_column_type(sqlite3_stmt_t, int) NOEXCEPT_SPEC;
	sqlite3_value_t sqlite3_column_value(sqlite3_stmt_t, int) NOEXCEPT_SPEC;

	void sqlite3_exec(sqlite3_t, utf8_string_in_t,
					  int (*)(void*, int, char**, char**), void*);

	void sqlite3_finalize(unique_statement&&);

	detail::initialize_t sqlite3_initialize();

	sqlite3_stmt_t sqlite3_next_stmt(sqlite3_t, sqlite3_stmt_t) NOEXCEPT_SPEC;

	unique_connection sqlite3_open(utf8_string_in_t);
	unique_connection sqlite3_open(utf16_string_in_t);
	unique_connection sqlite3_open_v2(utf8_string_in_t, detail::openflag_t,
									  utf8_string_in_t);

	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare(sqlite3_t, utf8_string_in_t);
	std::tuple<unique_statement, utf8_string_in_t>
		sqlite3_prepare_v2(sqlite3_t, utf8_string_in_t);
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare(sqlite3_t, utf16_string_in_t);
	std::tuple<unique_statement, utf16_string_in_t>
		sqlite3_prepare_v2(sqlite3_t, utf16_string_in_t);

	void* sqlite3_profile(sqlite3_t,
						  void (*)(void*, const char*, sqlite3_uint64_t),
						  void*) NOEXCEPT_SPEC;

	void sqlite3_reset(sqlite3_stmt_t);

	inline void sqlite3_shutdown(detail::initialize_t)
	{
	}

	std::tuple<int, int> sqlite3_status(status_t, bool);
	std::tuple<sqlite3_int64_t, sqlite3_int64_t> sqlite3_status64(status_t,
																  bool);

	detail::step_result_t sqlite3_step(sqlite3_stmt_t);

	bool sqlite3_stmt_busy(sqlite3_stmt_t) NOEXCEPT_SPEC;
	bool sqlite3_stmt_readonly(sqlite3_stmt_t) NOEXCEPT_SPEC;
	int sqlite3_stmt_status(sqlite3_stmt_t, status_counter_t,
							bool) NOEXCEPT_SPEC;

	std::tuple<utf8_string_out_t, utf8_string_out_t, int, int, int>
		sqlite3_table_column_metadata(sqlite3_t, utf8_string_in_t,
									  utf8_string_in_t, utf8_string_in_t);

	int sqlite3_total_changes(sqlite3_t) NOEXCEPT_SPEC;

	void* sqlite3_trace(sqlite3_t, void (*)(void*, const char*),
						void*) NOEXCEPT_SPEC;
}

#define SQLITEWRAPPED_HPP
#endif// SQLITEWRAPPED_HPP