#pragma once
#include <string>
#include "utils.h"

namespace SQLiteHelper 
{
    class DateTime
    {
    private:
        DateTime();
        ~DateTime();
        static DateTime* s_instance;
    private: 
        std::time_t to_;
        std::time_t from_;
        std::string wildcard;
        std::string column_utc_;
        std::time_t expires_utc_;
    public:
        static DateTime* instance()
        {
            if (!s_instance)
            {
                s_instance = new DateTime;
            }
            return s_instance;
        }
    public:
        void setColumn_utc(const std::string& column_utc);
        std::string getColumn_utc();
        void setWildCard(const std::string& character);
        std::string getWildCard();
        void setDateTimeTo(const std::string& to);
        std::time_t getDateTimeTo();
        void setDateTimeFrom(const std::string& from);
        std::time_t getDateTimeFrom();
        void setExpiresUtc(const std::string& time);
        std::time_t getExpiresUtc();
    };
}