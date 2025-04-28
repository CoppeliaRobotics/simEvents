#include "uids.h"

using conditions::UIDs;

UIDs::UIDs(const vector<long long> &uids_)
    : uids(uids_)
{
}

bool UIDs::matches(const sim::EventInfo &info, const json &data) const
{
    for(long long uid : uids)
        if(info.uid == uid)
            return true;
    return false;
}
