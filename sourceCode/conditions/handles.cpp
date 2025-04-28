#include "handles.h"

using conditions::Handles;

Handles::Handles(const vector<int> &handles_)
    : handles(handles_)
{
}

bool Handles::matches(const sim::EventInfo &info, const json &data) const
{
    for(int handle : handles)
        if(info.handle == handle)
            return true;
    return false;
}
