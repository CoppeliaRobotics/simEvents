#include <string>
#include <vector>
#include <map>
#include <optional>

#include <simPlusPlus/Plugin.h>
#include <simPlusPlus/Handles.h>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

#include "config.h"
#include "plugin.h"
#include "stubs.h"

using std::string;
using std::vector;
using std::map;
using std::optional;
using std::runtime_error;

using json = jsoncons::json;


struct Condition
{
    virtual ~Condition() {}
    virtual bool matches(const sim::EventInfo &info, const json &data) const = 0;
};

struct AndCondition : public Condition
{
    AndCondition(const vector<Condition*> &conditions_)
        : conditions(conditions_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        for(const auto condition : conditions)
            if(!condition->matches(info, data))
                return false;
        return true;
    }

private:
    const vector<Condition*> conditions;
};

struct OrCondition : public Condition
{
    OrCondition(const vector<Condition*> &conditions_)
        : conditions(conditions_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        for(const auto condition : conditions)
            if(condition->matches(info, data))
                return true;
        return false;
    }

private:
    const vector<Condition*> conditions;
};

struct NotCondition : public Condition
{
    NotCondition(const Condition *c_)
        : c(c_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        return !c->matches(info, data);
    }

private:
    const Condition *c;
};

struct EventTypeCondition : public Condition
{
    EventTypeCondition(const string &eventType_)
        : eventType(eventType_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        return info.event == eventType;
    }

private:
    const string eventType;
};

struct HandlesCondition : public Condition
{
    HandlesCondition(const vector<int> &handles_)
        : handles(handles_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        for(int handle : handles)
            if(info.handle == handle)
                return true;
        return false;
    }

private:
    const vector<int> handles;
};

struct UIDsCondition : public Condition
{
    UIDsCondition(const vector<long long> &uids_)
        : uids(uids_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        for(long long uid : uids)
            if(info.uid == uid)
                return true;
        return false;
    }

private:
    const vector<long long> uids;
};

struct EventDataCondition : public Condition
{
    EventDataCondition(const string &fieldName_)
        : fieldName(fieldName_)
    {
    }

    EventDataCondition(const string &fieldName_, const json &fieldValue_)
        : fieldName(fieldName_), fieldValue(fieldValue_)
    {
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        if(!data.contains(fieldName))
            return false;

        if(!fieldValue.has_value())
            return true;

        return data[fieldName] == fieldValue.value();
    }

private:
    const string fieldName;
    const optional<json> fieldValue;
};

struct Probe
{
    Probe(int scriptID, const string &callback)
    {
        this->scriptID = scriptID;
        this->callback = callback;
    }

    void setCondition(Condition *c)
    {
        this->condition = c;
    }

    bool matches(const sim::EventInfo &info, const json &data)
    {
        if(!condition) return false;
        return condition->matches(info, data);
    }

    void dispatch(const sim::EventInfo &info, const json &data)
    {
        int stack = sim::createStack();
        sim::pushValueOntoStack(stack, data);
        sim::callScriptFunctionEx(scriptID, callback.c_str(), stack);
        sim::releaseStack(stack);
    }

    void onEvent(const sim::EventInfo &info, const json &data)
    {
        if(matches(info, data))
            dispatch(info, data);
    }

private:
    string callback;
    int scriptID {-1};
    Condition *condition {nullptr};
};

class Plugin : public sim::Plugin
{
public:
    void onInit() override
    {
        if(!registerScriptStuff())
            throw runtime_error("failed to register script stuff");

        setExtVersion("Events Plugin");
        setBuildDate(BUILD_DATE);
    }

    void onScriptStateAboutToBeDestroyed(int scriptHandle, long long scriptUid) override
    {
        for(auto probe : probeHandles.find(scriptHandle))
            delete probeHandles.remove(probe);
        for(auto condition : conditionHandles.find(scriptHandle))
            delete conditionHandles.remove(condition);
    }

    void onEvent(const sim::EventInfo &info, const json &data) override
    {
        int sceneID = sim::getInt32Param(sim_intparam_scene_unique_id);;
        for(const auto &probe : probeHandles.findByScene(sceneID))
            probe->onEvent(info, data);
    }

    void addProbe(addProbe_in *in, addProbe_out *out)
    {
        auto probe = new Probe(in->_.scriptID, in->callback);
        if(in->conditionHandle.has_value())
            probe->setCondition(conditionHandles.get(in->conditionHandle.value()));
        out->probeHandle = probeHandles.add(probe, in->_.scriptID);
    }

    void removeProbe(removeProbe_in *in, removeProbe_out *out)
    {
        auto probe = probeHandles.get(in->probeHandle);
        delete probeHandles.remove(probe);
    }

    void setProbeCondition(setProbeCondition_in *in, setProbeCondition_out *out)
    {
        auto probe = probeHandles.get(in->probeHandle);
        auto condition = conditionHandles.get(in->conditionHandle);
        probe->setCondition(condition);
    }

    void addAndCondition(addAndCondition_in *in, addAndCondition_out *out)
    {
        vector<Condition*> conditions;
        for(const auto &conditionHandle : in->conditionHandles)
            conditions.push_back(conditionHandles.get(conditionHandle));
        auto condition = new AndCondition(conditions);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void addOrCondition(addOrCondition_in *in, addOrCondition_out *out)
    {
        vector<Condition*> conditions;
        for(const auto &conditionHandle : in->conditionHandles)
            conditions.push_back(conditionHandles.get(conditionHandle));
        auto condition = new OrCondition(conditions);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void addNotCondition(addNotCondition_in *in, addNotCondition_out *out)
    {
        auto condition1 = conditionHandles.get(in->conditionHandle);
        auto condition = new NotCondition(condition1);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void addEventTypeCondition(addEventTypeCondition_in *in, addEventTypeCondition_out *out)
    {
        auto condition = new EventTypeCondition(in->eventType);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void addHandlesCondition(addHandlesCondition_in *in, addHandlesCondition_out *out)
    {
        auto condition = new HandlesCondition(in->handles);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void addUIDsCondition(addUIDsCondition_in *in, addUIDsCondition_out *out)
    {
        auto condition = new UIDsCondition(in->uids);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void addEventDataCondition(addEventDataCondition_in *in, addEventDataCondition_out *out)
    {
        auto condition = in->fieldValue.has_value()
            ? new EventDataCondition(in->fieldName, in->fieldValue)
            : new EventDataCondition(in->fieldName);
        out->conditionHandle = conditionHandles.add(condition, in->_.scriptID);
    }

    void removeCondition(removeCondition_in *in, removeCondition_out *out)
    {
        auto condition = conditionHandles.get(in->conditionHandle);
        delete conditionHandles.remove(condition);
    }

private:
    sim::Handles<Probe*> probeHandles{"simEvents.Probe"};
    sim::Handles<Condition*> conditionHandles{"simEvents.Condition"};
};

SIM_PLUGIN(Plugin)
#include "stubsPlusPlus.cpp"
