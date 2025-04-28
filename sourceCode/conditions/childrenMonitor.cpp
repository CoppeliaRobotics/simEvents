#include "childrenMonitor.h"

using conditions::ChildrenMonitor;

ChildrenMonitor::ChildrenMonitor(int parentHandle_, const vector<int> &children_)
    : parentHandle(parentHandle_), children(children_)
{
}

bool ChildrenMonitor::matches(const sim::EventInfo &info, const json &data) const
{
    bool matchHandle = std::find(children.begin(), children.end(), info.handle) != children.end();
    bool hasParentHandle = data.contains("parentHandle");
    bool matchParentHandle = hasParentHandle ? (data["parentHandle"].as<int>() == parentHandle) : false;
    return ((info.event == "objectAdded" || info.event == "objectChanged") && matchParentHandle)
        || (info.event == "objectRemoved" && matchHandle)
        || (info.event == "objectChanged" && matchHandle && hasParentHandle);
}
