------------------------------------------------------------------------------
--                           type.lua                                       --
------------------------------------------------------------------------------

local Type = {
    -- Check value for correct type
    ----------------------------------
    -- @value {any type} checked value
    --
    -- @return {boolean} get true if type is correct
    ----------------------------------
    is = {
        int = function (value)
            if type(value) == "number" then
                integer, fractional = math.modf(value)
                return fractional == 0
            end
        end,

        number = function (value)
            return type(value) == "number"
        end,

        str = function (value)
            return type(value) == "string"
        end,

        table = function (value)
            return type(value) == "table"
        end,
    },

    to = {
        number = function (value)
            return tonumber(value)
        end,

        str = function (value)
            return tostring(value)
        end
    }
}

return Type