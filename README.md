# SQLiteWrapper
A very thin wrapper around SQLite3, written in C++.
The C interface of SQLite3 is a good interface for the most part. 
The goal of this wrapper is to add type safety where it is useful,
apply RAII to outputs and generally make the library cleaner to call
from C++.

The rules used for wrapping the SQLite3 library are as follows:
1. Match the library; type for type, function for function.
2. Matching type names will be suffixed with _t.
3. Throw exception on error.
4. Return output parameters.
5. Non-error flags each have a unique type. eg. SQLITE_OPEN_* and 
SQLITE_STATUS_*.
6. Constant values become constant variables of the same name (in lower case).
7. utf-16 characters will be of type char16_t.
8. Output strings are returned as std::basic_string.
9. Functions that create objects that must be cleaned up shall be returned as
std::unique_ptr<T> with a custom deleter for cleanup and will have the type 
name unique_*.
10. Multiple results are returned in a std::tuple, in the order they appear in
the original function declaration from left to right.
