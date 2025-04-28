#include "event.h"

using conditions::Event;

Event::Event(const string &eventType_)
    : eventType(eventType_)
{
}

bool Event::matches(const sim::EventInfo &info, const json &data) const
{
    return info.event == eventType;
}
