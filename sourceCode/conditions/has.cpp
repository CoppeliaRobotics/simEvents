#include "has.h"

using conditions::Has;

Has::Has(const string &fieldName_)
    : fieldName(fieldName_)
{
}

bool Has::matches(const sim::EventInfo &info, const json &data) const
{
    return data.contains(fieldName);
}
