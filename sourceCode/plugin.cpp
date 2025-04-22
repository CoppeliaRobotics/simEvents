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


struct Probe
{
    Probe(int scriptID, const probe_opts &opts)
    {
        this->scriptID = scriptID;
        this->callback = opts.callback.value();
        if(opts.eventTypes)
        {
            this->eventTypes.emplace();
            for(const string &eventType : opts.eventTypes.value())
                this->eventTypes.value()[eventType] = true;
        }
        if(opts.handles)
        {
            this->handles.emplace();
            for(const int &handle : opts.handles.value())
                this->handles.value()[handle] = true;
        }
        if(opts.properties)
        {
            this->properties.emplace();
            for(const string &property : opts.properties.value())
                this->properties.value()[property] = true;
        }
    }

    bool matches(const sim::EventInfo &info, const json &data)
    {
        if(eventTypes.has_value())
        {
            try
            {
                if(eventTypes.value().at(info.event) == false)
                    return false;
            }
            catch(const std::out_of_range& ex)
            {
                return false;
            }
        }

        if(handles.has_value())
        {
            try
            {
                if(handles.value().at(info.handle) == false)
                    return false;
            }
            catch(const std::out_of_range& ex)
            {
                return false;
            }
        }

        if(properties.has_value())
        {
            for(const auto &p : properties.value())
            {
                if(data.contains(p.first))
                    return true;
            }
            return false;
        }

        return true;
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
    int scriptID;
    optional<map<string, bool>> eventTypes;
    optional<map<int, bool>> handles;
    optional<map<string, bool>> properties;
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
        for(auto c : handles.find(scriptHandle))
            delete handles.remove(c);
    }

    void onEvent(const sim::EventInfo &info, const json &data) override
    {
        int sceneID = sim::getInt32Param(sim_intparam_scene_unique_id);;
        for(const auto &probe : handles.findByScene(sceneID))
            probe->onEvent(info, data);
    }

    void addProbe(addProbe_in *in, addProbe_out *out)
    {
        auto probe = new Probe(in->_.scriptID, in->opts);
        out->probeHandle = handles.add(probe, in->_.scriptID);
    }

private:
    sim::Handles<Probe*> handles{"simEvents.Probe"};
};

SIM_PLUGIN(Plugin)
#include "stubsPlusPlus.cpp"
