#include <string>
#include <vector>

#include <simPlusPlus/Plugin.h>
#include <simPlusPlus/Handles.h>

#include <jsoncons/json.hpp>

#include "config.h"
#include "plugin.h"
#include "stubs.h"
#include "conditions/parser.h"
#include "conditions/childrenMonitor.h"
#include "probe.h"

using std::string;
using std::vector;
using std::runtime_error;

using json = jsoncons::json;

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
        auto condition = conditions::Parser::parse(condition_json);

        int scriptID = in->_.scriptID;
        string callback = in->callback;

        auto probe = new Probe(
            [=] (Probe *probe, const json &eventData)
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
        delete probe->setCondition(nullptr);
        delete probeHandles.remove(probe);
    }

    void addChildrenMonitor(addChildrenMonitor_in *in, addChildrenMonitor_out *out)
    {
        int parentHandle = in->parentHandle;
        int scriptID = in->_.scriptID;
        string callback = in->callback;

        auto notifyChildrenChanged = [parentHandle, scriptID, callback] ()
        {
            vector<int> children = sim::getObjectsInTree(parentHandle, sim_handle_all, 3);
            childrenMonitorCallback_in in;
            in.childrenHandles = children;
            childrenMonitorCallback_out out;
            childrenMonitorCallback(scriptID, callback.c_str(), &in, &out);
        };

        auto childrenChangedCallback = [parentHandle, notifyChildrenChanged] (Probe *probe, const json &eventData)
        {
            vector<int> children = sim::getObjectsInTree(parentHandle, sim_handle_all, 3);
            delete probe->setCondition(new conditions::ChildrenMonitor(parentHandle, children));
            notifyChildrenChanged();
        };

        vector<int> children = sim::getObjectsInTree(parentHandle, sim_handle_all, 3);
        auto probe = new Probe(childrenChangedCallback, new conditions::ChildrenMonitor(parentHandle, children));

        // XXX: calling notifyChildrenChanged() here is also required to let
        //      scripts properly react to undo:
        notifyChildrenChanged();

        out->probeHandle = probeHandles.add(probe, in->_.scriptID);
    }

private:
    sim::Handles<Probe*> probeHandles{"simEvents.Probe"};
};

SIM_PLUGIN(Plugin)
#include "stubsPlusPlus.cpp"
