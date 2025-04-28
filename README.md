# Events plugin for CoppeliaSim

A plugin to filter events.

Usage:

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
            {'has', 'collapsed'},     -- filter on field presence
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
