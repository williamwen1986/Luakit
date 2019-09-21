-- Function for create column functions
local function Property(args)
    return function (colname)
        local column_func = {
            -- class type
            __classtype__ = AGGREGATOR,

            -- Asc column name
            colname = colname,

            -- concatinate methods
            __concat = function (left_part, right_part)
                return tostring(left_part) .. tostring(right_part)
            end,

            __tostring = args.parse or self.parse
        }

        setmetatable(column_func, {__tostring = column_func.__tostring,
                                   __concat = column_func.__concat})
        return column_func
    end
end

return Property