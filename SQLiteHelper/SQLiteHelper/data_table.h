#pragma once
#include <map>
#include <list>
#include <vector>
#include <string>
#pragma warning( push )
#pragma warning( disable : 4005 )

namespace SQLiteHelper 
{
    #define VALUE       std::tuple<int /*type*/, __int64 /*INT*/, double /*REAL*/, std::wstring /*TEXT*/, std::string /*BLOB*/>
    #define ROW_VALUE   std::map<std::wstring, VALUE>
    #define TABLE       std::list<ROW_VALUE>
    #define TABLES      std::map<std::wstring,TABLE>

    typedef enum
    {
        SQLITE_TYPE_INT64 = 1,
        SQLITE_TYPE_DOUBLE = 2,
        SQLITE_TYPE_TEXT = 3,
        SQLITE_TYPE_BLOB = 4,
        SQLITE_TYPE_NULL = 5,
    } sqlite_type_t;

    typedef std::list<std::string> TABLE_NAMES;
    typedef std::map<int, int> COLUMN_TYPE;
}
#pragma warning( pop )





