local simEvents = loadPlugin 'simEvents';
(require 'simEvents-typecheck')(simEvents)

simEvents.ChildObjectMonitor = {}

function simEvents.ChildObjectMonitor:__getChildren()
    return sim.getObjectsInTree(self.__parentHandle, sim.handle_all, 3)
end

function simEvents.ChildObjectMonitor:__removeEventProbe()
    if self.__probeHandle then
        simEvents.removeProbe(self.__probeHandle)
        self.__probeHandle = nil
    end
end

function simEvents.ChildObjectMonitor:__setupEventProbe()
    self:__removeEventProbe()

    local conditions = {'or'}

    table.insert(conditions,
        {'and',
            {'or',
                {'event', 'objectAdded'},
                {'event', 'objectChanged'},
            },
            {'eq', 'parentHandle', self.__parentHandle},
        }
    )

    local children = self:__getChildren()
    table.insert(conditions,
        {'and',
            {'event', 'objectRemoved'},
            {'handles', children},
        }
    )

    table.insert(conditions,
        {'and',
            {'event', 'objectChanged'},
            {'handles', children},
            {'has', 'parentHandle'},
        }
    )

    self.__probeHandle = simEvents.addProbe(
        reify(
            function(data)
                self:__triggerCallback()
                self:__setupEventProbe()
            end
        ),
        conditions
    )
end

function simEvents.ChildObjectMonitor:__triggerCallback()
    if type(self.__callback) == 'function' then
        self.__callback(self:__getChildren())
    end
end

function simEvents.ChildObjectMonitor:__index(k)
    return simEvents.ChildObjectMonitor[k]
end

setmetatable(
    simEvents.ChildObjectMonitor, {
        --@fun ChildObjectMonitor create a child object monitor object
        --@arg int parentHandle handle of the object to monitor
        --@arg func callback function to be called when children of the object change
        --@ret table object an instance of simEvents.ChildObjectMonitor
        __call = function(self, parentHandle, callback)
            local obj = setmetatable(
                {
                    __parentHandle = parentHandle,
                    __callback = callback,
                },
                self
            )
            obj:__setupEventProbe()
            obj:__triggerCallback()
            return obj
        end,
    }
)

return simEvents
