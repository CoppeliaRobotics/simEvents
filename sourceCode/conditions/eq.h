#pragma once

#include <string>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::string;

using json = jsoncons::json;

namespace conditions { struct Eq; }

struct conditions::Eq : public conditions::Condition
{
    Eq(const string &fieldName_, const json &fieldValue_);
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const string fieldName;
    const json fieldValue;
};
