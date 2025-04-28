#include "eq.h"

using conditions::Eq;

Eq::Eq(const string &fieldName_, const json &fieldValue_)
    : fieldName(fieldName_), fieldValue(fieldValue_)
{
}

bool Eq::matches(const sim::EventInfo &info, const json &data) const
{
    if(!data.contains(fieldName))
        return false;

    return data[fieldName] == fieldValue;
}
