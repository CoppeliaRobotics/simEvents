# Events plugin for CoppeliaSim

A plugin to filter events.

Usage: call `simEvents.addProbe(callback, condition)` to setup an event probe.

Condition can be one of:

| Condition   | Description |
| ----------- | ----------- |
| `{'and', cond-1, cond-2, ...}` | Boolean "and" of the nested conditions |
| `{'or',  cond-1, cond-2, ...}` | Boolean "or" of the nested conditions |
| `{'not', cond-1}` | Boolean "not" of the nested condition |
| `{'event', eventType}` | Filter on event type (e.g. objectAdded, objectChanged, etc...) |
| `{'handles', {handle-1, ...}}` | Filter on handle field |
| `{'uids', {uid-1, ...}}` | Filter on handle field |
| `{'has', fieldName}` | Filter on the presence of the specified field |
| `{'eq', fieldName, fieldValue}` | Filter on the value of the specified field |

Use `simEvents.removeProbe(probeHandle)` to later remove the probe and stop monitoring those events.

Note: to monitor the direct children of an object, use `simEvents.addChildrenMonitor(callback, handle)`, which provides a self-modifying event probe that simplifies the handles bookkeeping.

Example:

```lua
sim = require 'sim'
simEvents = require 'simEvents'

function callback(data)
    print('callback', data)
end

function sysCall_init()
    dummy = sim.getObject('/dummy')
    probeHandle = simEvents.addProbe('callback', {'and',
        {'event', 'objectChanged'},
        {'handles', {dummy}},
        {'or',
            {'has', 'collapsed'},      -- filter on field presence
            {'eq',  'selected', true}, -- filter on field value
        },
    })
end
```

### Compiling

1. Install required packages for simStubsGen: see simStubsGen's [README](https://github.com/CoppeliaRobotics/include/blob/master/simStubsGen/README.md)
2. Checkout, compile and install into CoppeliaSim:
```sh
$ git clone https://github.com/CoppeliaRobotics/simEvents.git
$ cd simEvents
$ git checkout coppeliasim-v4.5.0-rev0
$ mkdir -p build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build .
$ cmake --install .
```

NOTE: replace `coppeliasim-v4.5.0-rev0` with the actual CoppeliaSim version you have.
