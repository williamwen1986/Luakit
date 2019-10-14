------------------------------------------------------------------------------
--                          query.lua                                       --
------------------------------------------------------------------------------


-- Creates an instance to retrieve and manage a
-- string table with the database
---------------------------------------------------
-- @own_table {table} parent table instace
-- @data {table} data returned by the query to the database
--
-- @return {table} database query instance
---------------------------------------------------
function Query(own_table, data)
    local query = {
        ------------------------------------------------
        --          Table info varibles               --
        ------------------------------------------------

        -- Table instance
        own_table = own_table,

        -- Column data
        -- Structure example of one column
        -- fieldname = {
        --     old = nil,
        --     new = nil
        -- }
        _data = {},

        _pureData = {},

        -- Data only for read mode
        _readonly = {},

        ------------------------------------------------
        --             Metamethods                    --
        ------------------------------------------------

        -- Get column value
        -----------------------------------------
        -- @colname {string} column name in table
        -- 
        -- @return {string|boolean|number|nil} column value
        -----------------------------------------
        _get_col = function (self, colname)
            if self._data[colname] and self._data[colname].new then
                return self._data[colname].new

            elseif self._readonly[colname] then
                return self._readonly[colname]
            elseif self.own_table:has_column(colname) then 
                local colType = self.own_table:get_column(colname)
                if colType.settings.default then
                    return colType.field.as(table_column.settings.default)
                end
            end
        end,

        -- Set column new value
        -----------------------------------------
        -- @colname {string} column name in table
        -- @colvalue {string|number|boolean} new column value
        -----------------------------------------
        _set_col = function (self, colname, colvalue)
            local coltype

            if self._data[colname] and self._data[colname].new then
                coltype = self.own_table:get_column(colname)
                if coltype and coltype.field.validator(colvalue) then
                    local orgValue = colvalue
                    colvalue = coltype.field.to_type(colvalue)
                    self._data[colname].old = self._data[colname].new
                    self._data[colname].new = colvalue
                    if coltype.field.pureKeepOrgType then
                        self._pureData[colname] = orgValue
                    else
                        self._pureData[colname] = colvalue
                    end
                    
                else
                    BACKTRACE(WARNING, "Not valid column value for update")
                end
            elseif self.own_table:has_column(colname) then
                self._data[colname] = {
                    new = colvalue,
                    old = colvalue
                }
                self._pureData[colname] = colvalue
            end
        end,

        ------------------------------------------------
        --             Private methods                --
        ------------------------------------------------

        -- Add new row to table
        _add = function (self)
            local kv,needPrimaryKey =  self:getSaveKv();
            if needPrimaryKey then
                local result,rowId = lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","insert",self.own_table.__tablename__,kv,needPrimaryKey)
                self:setPrimaryKey(needPrimaryKey,rowId)
                return rowId
            else
                lua_thread.postToThread(self.own_table.cacheThreadId,"orm.cache","insert",self.own_table.__tablename__,kv,needPrimaryKey)
                if self.own_table.__primary_key then
                    return self[self.own_table.__primary_key.name]
                end
            end

        end,

        setPrimaryKey = function(self,needPrimaryKey,rowId)
            if needPrimaryKey and rowId ~= nil then
                self._data[self.own_table.__primary_key.name] = {
                    new = rowId,
                    old = rowId
                }
                self._pureData[self.own_table.__primary_key.name] = rowId
            end
        end,

        getSaveKv = function(self)
            local value
            local colname
            local needPrimaryKey = false
            if self.own_table.__primary_key and self.own_table.__primary_key.field.__type__ == "integer" and (not self[self.own_table.__primary_key.name])   then
                needPrimaryKey = true
            end

            local kv = {}

            for _, table_column in pairs(self.own_table.__colnames) do
                if needPrimaryKey and table_column.name == self.own_table.__primary_key.name then
                else
                    colname = table_column.name
                    -- If value exist correct value
                    if self[colname] ~= nil then
                        value = self[colname]

                        if table_column.field.validator(value) then
                            value = table_column.field.as(value)
                        else
                            BACKTRACE(WARNING, "Wrong type for table '" ..
                                    self.own_table.__tablename__ ..
                                    "' in column '" .. tostring(colname) .. "'")
                            return false
                        end

                        -- Set default value
                    elseif table_column.settings.default then
                        value = table_column.field.as(table_column.settings.default)
                    else
                        value = nil
                    end
                    kv[colname] = value
                end
            end
            return kv,needPrimaryKey
        end,

        getPureData = function (self)
            local ret = {}
            for i, v in pairs(self._pureData) do

                ret[i] = v
            end
            for i, v in pairs(self._readonly) do
                ret[i] = v:getPureData()
            end
            return ret
        end ,



        -- save row
        save = function (self)
            return self:_add()
        end,

        -- delete row
        delete = function (self)
            if not self.own_table.__primary_key then
                error("can not query delete without primary_key")
            end
            lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","deleteWithPrimaryKey",self.own_table.__tablename__,{self[self.own_table.__primary_key.name]})
            self._data = {}
        end
    }

    if data then
        local current_table

         for colname, colvalue in pairs(data) do
             if query.own_table:has_column(colname) then
                 local orgValue = colvalue
                 colvalue = query.own_table:get_column(colname)
                                             .field.to_type(colvalue)
                 query._data[colname] = {
                     new = colvalue,
                     old = colvalue
                 }

                 if query.own_table:get_column(colname)
                                             .field.pureKeepOrgType then
                     query._pureData[colname] = orgValue
                 else
                     query._pureData[colname] = colvalue
                 end

             else
                 if _G.All_Tables[query.own_table.__dbname__ .."_"..colname] then
                     current_table = _G.All_Tables[query.own_table.__dbname__.."_"..colname]
                     colvalue = Query(current_table, colvalue)
                     -- query._readonly[colname .. "_all"] = QueryList(current_table, {})
                     -- query._readonly[colname .. "_all"]:add(colvalue)
                 end
                 query._readonly[colname] = colvalue
             end
         end
    else
        BACKTRACE(INFO, "Create empty row instance for table '" ..
                        self.own_table.__tablename__ .. "'")
    end

    setmetatable(query, {__index = query._get_col,
                         __newindex = query._set_col})

    return query
end

local QueryList = require('orm.class.query_list')

return Query, QueryList