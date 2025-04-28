#pragma once

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

using json = jsoncons::json;

namespace conditions { struct Condition; }

struct conditions::Condition
{
    virtual ~Condition() {}
    virtual bool matches(const sim::EventInfo &info, const json &data) const = 0;
};
