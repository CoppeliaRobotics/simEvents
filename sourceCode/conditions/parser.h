#pragma once

#include <vector>

#include <jsoncons/json.hpp>

#include "condition.h"

using std::vector;

using json = jsoncons::json;

namespace conditions { struct Parser; }

struct conditions::Parser
{
    static vector<Condition*> parse(const json &expr, int start, int end = -1);
    static Condition * parse(const json &expr);
};
