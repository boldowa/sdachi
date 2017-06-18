#include "sdachiBuildTime.h"

sdachiBuildTime::sdachiBuildTime()
: dateTime(__DATE__ " " __TIME__)
{
}

sdachiBuildTime::~sdachiBuildTime()
{
}

const char* sdachiBuildTime::GetDateTime()
{
    return dateTime;
}

