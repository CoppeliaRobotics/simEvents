#pragma once

#include <vector>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::vector;

using json = jsoncons::json;

namespace conditions { struct ChildrenMonitor; }

struct conditions::ChildrenMonitor : public conditions::Condition
{
    ChildrenMonitor(int parentHandle_, const vector<int> &children_);
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    int parentHandle;
    vector<int> children;
};
