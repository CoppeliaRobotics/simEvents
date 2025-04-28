#pragma once

#include <vector>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::vector;

using json = jsoncons::json;

namespace conditions { struct And; }

struct conditions::And : public conditions::Condition
{
    And(const vector<Condition*> &conditions_);
    virtual ~And();
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const vector<Condition*> conditions;
};
