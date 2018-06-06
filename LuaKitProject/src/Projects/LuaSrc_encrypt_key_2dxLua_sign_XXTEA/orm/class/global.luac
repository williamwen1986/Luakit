
------------------------------------------------------------------------------
--                                Constants                                 --
------------------------------------------------------------------------------
-- Global
ID = "id"
AGGREGATOR = "aggregator"
QUERY_LIST = "query_list"
-- Backtrace types
ERROR = 'e'
WARNING = 'w'
INFO = 'i'
DEBUG = 'd'

All_Tables = {}

------------------------------------------------------------------------------
--                          Helping functions                               --
------------------------------------------------------------------------------

local _pairs = pairs

local _ipairs = ipairs

function pairs(Table)
    if Table.__classname__ == QUERY_LIST then
        return Table()
    else
        return _pairs(Table)
    end
end

function ipairs(Table)
    if Table.__classname__ == QUERY_LIST then
        return Table()
    else
        return _ipairs(Table)
    end
end

function BACKTRACE(tracetype, message)
    -- if DB.backtrace then
        if tracetype == ERROR then
            print("[SQL:Error] " .. message)
            os.exit()

        elseif tracetype == WARNING then
            print("[SQL:Warning] " .. message)

        elseif tracetype == INFO then
            print("[SQL:Info] " .. message)
        end
    -- end

    -- if DB.DEBUG and tracetype == DEBUG then
        print("[SQL:Debug] " .. message)
    -- end
end

function string.startswith(String, start)
    return string.sub(String, 1, string.len(start)) == start
end

function string.endswith(String, End)
    return End == '' or string.sub(String, -string.len(End)) == End
end

function string.cutend(String, End)
    return End == '' and String or string.sub(String, 0, -#End - 1)
end

function string.divided_into(String, separator)
    local separator_pos = string.find(String, separator)
    return string.sub(String, 0, separator_pos - 1),
           string.sub(String, separator_pos + 1, #String)
end

function table.has_key(array, key)
    if Type.is.table(key) and key.colname then
        key = key.colname
    end

    for array_key, _  in pairs(array) do
        if array_key == key then
            return true
        end
    end
end

function table.has_value(array, value)
    if Type.is.table(value) and value.colname then
        value = value.colname
    end

    for _, array_value  in pairs(array) do
        if array_value == value then
            return true
        end
    end
end

function table.join(array, separator)
    local result = ""
    local counter = 0

    if not separator then
        separator = ","
    end

    for _, value in pairs(array) do
        if counter ~= 0 then
            value = separator .. value
        end

        result = result .. value
        counter = counter + 1
    end

    return result
end