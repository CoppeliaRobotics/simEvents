#pragma once

#include <string>

#include <simPlusPlus/Plugin.h>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::string;

using json = jsoncons::json;

namespace conditions { struct Has; }

struct conditions::Has : public conditions::Condition
{
    Has(const string &fieldName_);
    bool matches(const sim::EventInfo &info, const json &data) const override;

private:
    const string fieldName;
};
