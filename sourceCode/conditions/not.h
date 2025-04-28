#pragma once

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using json = jsoncons::json;

namespace conditions { struct Not; }

struct conditions::Not : public conditions::Condition
{
    Not(const Condition *condition_);
    virtual ~Not();
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const Condition *condition;
};
