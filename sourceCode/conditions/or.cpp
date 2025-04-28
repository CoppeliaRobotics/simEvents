#include "or.h"

using conditions::Or;

Or::Or(const vector<Condition*> &conditions_)
    : conditions(conditions_)
{
}

Or::~Or()
{
    for(const auto condition : conditions)
        delete condition;
}

bool Or::matches(const sim::EventInfo &info, const json &data) const
{
    for(const auto condition : conditions)
        if(condition->matches(info, data))
            return true;
    return false;
}
