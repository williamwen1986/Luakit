------------------------------------------------------------------------------
--                          query_list.lua                                  --
------------------------------------------------------------------------------

function QueryList(own_table, rows)
    local current_query
    local _query_list = {
        ------------------------------------------------
        --          Table info varibles               --
        ------------------------------------------------

        --class name
        __classname__ = QUERY_LIST,

        -- Own Table
        own_table = own_table,

        -- Stack of data rows
        _stack = {},

        ------------------------------------------------
        --             Metamethods                    --
        ------------------------------------------------

        -- Get n-th position value from Query stack
        ------------------------------------------------
        -- @position {integer} position element is stack
        --
        -- @return {Query Instance} Table row instance
        -- in n-th position
        ------------------------------------------------
        __index = function (self, position)
            if Type.is.int(position) and position >= 1 then
                return self._stack[position]
            end
        end,

        __call = function (self)
            return pairs(self._stack)
        end,

        getPureData = function (self)
            local ret = {}
            for _,v in pairs(self._stack) do
                table.insert(ret, v:getPureData())
            end
            return ret
        end,

        ------------------------------------------------
        --             User methods                   --
        ------------------------------------------------

        -- Add new Query Instance to stack
        add = function (self, QueryInstance)
            table.insert(self._stack, QueryInstance)
        end,

        -- Get count of values in stack
        count = function (self)
            return #self._stack
        end,

        update = function (self)
            for _, query in pairs(self._stack) do
                query:update()
            end
        end,

        -- Remove from database all elements from stack
        delete = function (self)
            for _, query in pairs(self._stack) do
                query:delete()
            end

            self._stack = {}
        end
    }

    setmetatable(_query_list, {__index = _query_list.__index,
                               __len = _query_list.count,
                               __call = _query_list.__call})

    for _, row in pairs(rows) do
        _query_list:add(Query(own_table, row))
    end
    return _query_list
end

return QueryList