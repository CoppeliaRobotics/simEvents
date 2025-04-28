#pragma once

#include <string>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::string;

using json = jsoncons::json;

namespace conditions { struct Event; }

struct conditions::Event : public conditions::Condition
{
    Event(const string &eventType_);
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const string eventType;
};
