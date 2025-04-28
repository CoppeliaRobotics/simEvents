#include "probe.h"

using std::function;

using json = jsoncons::json;

Probe::Probe(const function<void(Probe *probe, const json&)> &callback_, const conditions::Condition *condition_)
    : callback(callback_), condition(condition_)
{
}

void Probe::onEvent(const sim::EventInfo &info, const json &data)
{
    if(condition && condition->matches(info, data))
    {
        json hdr = json::object();
        hdr["event"] = info.event;
        hdr["seq"] = info.seq;
        hdr["uid"] = info.uid;
        hdr["handle"] = info.handle;
        hdr["data"] = data;
        callback(this, hdr);
    }
}

const conditions::Condition * Probe::setCondition(const conditions::Condition *newCondition)
{
    auto oldCondition = condition;
    condition = newCondition;
    return oldCondition;
}
