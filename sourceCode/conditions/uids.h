#pragma once

#include <vector>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::vector;

using json = jsoncons::json;

namespace conditions { struct UIDs; }

struct conditions::UIDs : public conditions::Condition
{
    UIDs(const vector<long long> &uids_);
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const vector<long long> uids;
};
