#pragma once

#include <vector>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::vector;

using json = jsoncons::json;

namespace conditions { struct Handles; }

struct conditions::Handles : public conditions::Condition
{
    Handles(const vector<int> &handles_);
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const vector<int> handles;
};
