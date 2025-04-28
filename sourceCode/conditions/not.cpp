#include "not.h"

using conditions::Not;

Not::Not(const Condition *condition_)
    : condition(condition_)
{
}

Not::~Not()
{
    delete condition;
}

bool Not::matches(const sim::EventInfo &info, const json &data) const
{
    return !condition->matches(info, data);
}
