#include "and.h"

using conditions::And;

And::And(const vector<Condition*> &conditions_)
    : conditions(conditions_)
{
}

And::~And()
{
    for(const auto condition : conditions)
        delete condition;
}

bool And::matches(const sim::EventInfo &info, const json &data) const
{
    for(const auto condition : conditions)
        if(!condition->matches(info, data))
            return false;
    return true;
}
