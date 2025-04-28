local simEvents = loadPlugin 'simEvents';
(require 'simEvents-typecheck')(simEvents)

simEvents.ChildObjectMonitor = {}

function simEvents.ChildObjectMonitor:getChildren()
    return sim.getObjectsInTree(self.__parentHandle, sim.handle_all, 3)
end

function simEvents.ChildObjectMonitor:removeEventProbe()
    if self.__probeHandle then
        simEvents.removeProbe(self.__probeHandle)
        self.__probeHandle = nil
    end
end

function simEvents.ChildObjectMonitor:setupEventProbe()
    self:removeEventProbe()

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

    local children = self:getChildren()
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
                self:triggerCallback()
                self:setupEventProbe()
            end
        ),
        conditions
    )
end

function simEvents.ChildObjectMonitor:triggerCallback()
    if type(self.__callback) == 'function' then
        self.__callback(self:getChildren())
    end
end

function simEvents.ChildObjectMonitor:__index(k)
    return simEvents.ChildObjectMonitor[k]
end

setmetatable(
    simEvents.ChildObjectMonitor, {
        __call = function(self, parentHandle, callback)
            local obj = setmetatable(
                {
                    __parentHandle = parentHandle,
                    __callback = callback,
                },
                self
            )
            obj:setupEventProbe()
            obj:triggerCallback()
            return obj
        end,
    }
)

return simEvents
