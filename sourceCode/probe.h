#pragma once

#include <functional>

#include <jsoncons/json.hpp>

#include <simPlusPlus/Plugin.h>

#include "conditions/condition.h"

using std::function;

using json = jsoncons::json;

struct Probe
{
    Probe(const function<void(Probe *probe, const json&)> &callback_, const conditions::Condition *condition_);
    void onEvent(const sim::EventInfo &info, const json &data);
    const conditions::Condition * setCondition(const conditions::Condition *newCondition);

private:
    function<void(Probe *probe, const json&)> callback;
    const conditions::Condition * condition;
};
