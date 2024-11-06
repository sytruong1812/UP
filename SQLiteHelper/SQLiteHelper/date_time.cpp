#include <iomanip>
#include "date_time.h"

namespace SQLiteHelper 
{
    //Singleton 
    DateTime* DateTime::s_instance = 0;

    DateTime::DateTime()
    {
        to_ = 0LL;
        from_ = 0LL;
        wildcard = std::string();
        column_utc_ = std::string();
        //TODO: Edit current date + year here!
        expires_utc_ = Utils::getTimeFromString(Utils::getCurrentDateTime(1));
    }

    DateTime::~DateTime()
    {
        column_utc_.clear();
    }

    void DateTime::setColumn_utc(const std::string& column_utc)
    {
        column_utc_ = column_utc;
    }

    std::string DateTime::getColumn_utc()
    {
        return column_utc_;
    }

    void DateTime::setWildCard(const std::string& character)
    {
        wildcard = character;
    }

    std::string DateTime::getWildCard()
    {
        return wildcard;
    }

    void DateTime::setDateTimeTo(const std::string& to)
    {
        to_ = Utils::getTimeFromString(to);
    }

    std::time_t DateTime::getDateTimeTo()
    {
        return to_;
    }

    void DateTime::setDateTimeFrom(const std::string& from)
    {
        from_ = Utils::getTimeFromString(from);
    }

    std::time_t DateTime::getDateTimeFrom()
    {
        return from_;
    }
    void DateTime::setExpiresUtc(const std::string& time)
    {
        expires_utc_ = Utils::getTimeFromString(time);
    }
    std::time_t DateTime::getExpiresUtc()
    {
        return expires_utc_;
    }
}