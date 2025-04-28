#include <string>
#include <vector>
#include <map>
#include <optional>
#include <functional>

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
using std::function;
using std::runtime_error;

using json = jsoncons::json;


struct Condition
{
    virtual ~Condition() {}
    virtual bool matches(const sim::EventInfo &info, const json &data) const = 0;

    virtual void dumpLua(std::ostream &os) const = 0;

    static Condition * parse(const json &expr);

private:
    static vector<Condition*> parse(const json &expr, int start, int end = -1)
    {
        if(end == -1) end = expr.size();
        vector<Condition*> result;
        for(int i = start; i < end; i++) result.push_back(parse(expr[i]));
        return result;
    }
};

struct AndCondition : public Condition
{
    AndCondition(const vector<Condition*> &conditions_)
        : conditions(conditions_)
    {
    }

    virtual ~AndCondition()
    {
        for(const auto condition : conditions)
            delete condition;
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        for(const auto condition : conditions)
            if(!condition->matches(info, data))
                return false;
        return true;
    }

    void dumpLua(std::ostream &os) const override
    {
        os << "{'and'";
        for(const auto condition : conditions)
        {
            os << ", ";
            condition->dumpLua(os);
        }
        os << "}";
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

    virtual ~OrCondition()
    {
        for(const auto condition : conditions)
            delete condition;
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        for(const auto condition : conditions)
            if(condition->matches(info, data))
                return true;
        return false;
    }

    void dumpLua(std::ostream &os) const override
    {
        os << "{'or'";
        for(const auto condition : conditions)
        {
            os << ", ";
            condition->dumpLua(os);
        }
        os << "}";
    }

private:
    const vector<Condition*> conditions;
};

struct NotCondition : public Condition
{
    NotCondition(const Condition *condition_)
        : condition(condition_)
    {
    }

    virtual ~NotCondition()
    {
        delete condition;
    }

    bool matches(const sim::EventInfo &info, const json &data) const override
    {
        return !condition->matches(info, data);
    }

    void dumpLua(std::ostream &os) const override
    {
        os << "{'not', ";
        condition->dumpLua(os);
        os << "}";
    }

private:
    const Condition *condition;
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

    void dumpLua(std::ostream &os) const override
    {
        os << "{'event', '" << eventType << "'}";
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

    void dumpLua(std::ostream &os) const override
    {
        os << "{'handles', {";
        string sep = "";
        for(int handle : handles)
        {
            os << sep << handle;
            sep = ", ";
        }
        os << "}}";
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

    void dumpLua(std::ostream &os) const override
    {
        os << "{'uids', {";
        string sep = "";
        for(int uid : uids)
        {
            os << sep << uid;
            sep = ", ";
        }
        os << "}}";

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

    void dumpLua(std::ostream &os) const override
    {
        if(fieldValue.has_value())
            os << "{'eq', '" << fieldName << "', " << fieldValue << "}";
        else
            os << "{'has', '" << fieldName << "'}";
    }

private:
    const string fieldName;
    const optional<json> fieldValue;
};

Condition * Condition::parse(const json &expr)
{
    if(!expr.is_array() || expr.size() < 1 || !expr[0].is_string())
        throw std::runtime_error("invalid condition");

    string type = expr[0].as<string>();

    if(type == "and")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"and\" requires one or more arguments");
        return new AndCondition(parse(expr, 1));
    }
    else if(type == "or")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"or\" requires one or more arguments");
        return new OrCondition(parse(expr, 1));
    }
    else if(type == "not")
    {
        if(expr.size() != 2)
            throw std::runtime_error("\"not\" requires exactly one argument");
        return new NotCondition(parse(expr[1]));
    }
    else if(type == "event")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"event\" requires exactly one argument");
        if(!expr[1].is_string())
            throw std::runtime_error("\"event\" argument must be a string");
        string eventType = expr[1].as<string>();
        return new EventTypeCondition(eventType);
    }
    else if(type == "handles")
    {
        if(expr.size() != 2)
            throw std::runtime_error("\"handles\" requires exactly one argument");
        const auto &arg = expr[1];
        if(!arg.is_array())
            throw std::runtime_error("\"handles\" argument must be an array");
        vector<int> handles;
        for(int i = 0; i < arg.size(); i++)
        {
            if(!arg[i].is_int64())
                throw std::runtime_error("\"handles\" argument items must be int");
            int handle = arg[i].as<int64_t>();
            handles.push_back(handle);
        }
        return new HandlesCondition(handles);
    }
    else if(type == "uids")
    {
        if(expr.size() != 2)
            throw std::runtime_error("\"uids\" requires exactly one argument");
        const auto &arg = expr[1];
        if(!arg.is_array())
            throw std::runtime_error("\"uids\" argument must be an array");
        vector<long long> uids;
        for(int i = 0; i < arg.size(); i++)
        {
            if(!arg[i].is_int64())
                throw std::runtime_error("\"uids\" argument items must be int");
            long long uid = arg[i].as<int64_t>();
            uids.push_back(uid);
        }
        return new UIDsCondition(uids);
    }
    else if(type == "has")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"has\" requires exactly one argument");
        if(!expr[1].is_string())
            throw std::runtime_error("\"has\" argument must be a string");
        string fieldName = expr[1].as<string>();
        return new EventDataCondition(fieldName);
    }
    else if(type == "eq")
    {
        if(expr.size() < 2)
            throw std::runtime_error("\"eq\" requires exactly two arguments");
        if(!expr[1].is_string())
            throw std::runtime_error("\"eq\" argument 1 must be a string");
        string fieldName = expr[1].as<string>();
        json fieldValue = expr[2];
        return new EventDataCondition(fieldName, fieldValue);
    }
    else
        throw std::runtime_error("invalid condition type: \"" + type + "\"");
}

struct Probe
{
    Probe(const function<void(const json&)> &callback_, const Condition *condition_)
        : callback(callback_), condition(condition_)
    {
    }

    void onEvent(const sim::EventInfo &info, const json &data)
    {
        if(condition && condition->matches(info, data))
        {
            json hdr = json::object();
            hdr["event"] = info.event;
            hdr["seq"] = info.seq;
            hdr["uid"] = info.uid;
            hdr["handle"] = info.handle;
            hdr["data"] = data;
            callback(hdr);
        }
    }

private:
    const function<void(const json&)> callback;
    const Condition * const condition;
    friend class Plugin;
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
    }

    void onEvent(const sim::EventInfo &info, const json &data) override
    {
        int sceneID = sim::getInt32Param(sim_intparam_scene_unique_id);;
        for(const auto &probe : probeHandles.findByScene(sceneID))
            probe->onEvent(info, data);
    }

    void addProbe(addProbe_in *in, addProbe_out *out)
    {
        json condition_json;
        sim::getStackValue(in->_.stackID, &condition_json);
        auto condition = Condition::parse(condition_json);

        int scriptID = in->_.scriptID;
        string callback = in->callback;

        auto probe = new Probe(
            [=] (const json &eventData)
            {
                int stack = sim::createStack();
                sim::pushValueOntoStack(stack, eventData);
                sim::callScriptFunctionEx(scriptID, callback.c_str(), stack);
                sim::releaseStack(stack);
            },
            condition
        );

        out->probeHandle = probeHandles.add(probe, in->_.scriptID);
    }

    void removeProbe(removeProbe_in *in, removeProbe_out *out)
    {
        auto probe = probeHandles.get(in->probeHandle);
        delete probe->condition;
        delete probeHandles.remove(probe);
    }

private:
    sim::Handles<Probe*> probeHandles{"simEvents.Probe"};
};

SIM_PLUGIN(Plugin)
#include "stubsPlusPlus.cpp"
