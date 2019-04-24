------------------------------------------------------------------------------
--                               Require                                    --
------------------------------------------------------------------------------
require ("orm.class.global")
require ("orm.tools.func")
------------------------------------------------------------------------------
--                          Required classes                                --
------------------------------------------------------------------------------

local Select = require('orm.class.select')
local Query, QueryList = require('orm.class.query')
local fields = require('orm.tools.fields')
local _DBDataThreadId = lua_thread.createThread(BusinessThreadLOGIC,"DB_DATA")

------------------------------------------------------------------------------
--                               Table                                      --
------------------------------------------------------------------------------

Table = {
    DBDataThreadId = _DBDataThreadId,

    -- database table name
    __tablename__ = nil,

    -- Foreign Keys list
    foreign_keys = {},
}

function  Table:isTableExist(tableName)
    local sql = "SELECT tbl_name FROM sqlite_master WHERE type='table' AND tbl_name=?"
    local result = lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","query",sql,{tableName})
    if result and #result > 0 then
        return true
    else 
        return false
    end
end

function Table:existColumnsForTable(tableName)
    local sql = "PRAGMA table_info(" .. tableName .. ")"
    local result = lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","query",sql,{},"name")
    return result
end

function Table:existIndexesForTable(tableName)
    local sql = "PRAGMA index_list(" .. tableName .. ")" 
    local result = lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","query",sql,{},"name")
    local ret = {}
    for _,v in ipairs(result) do
        if not v:startswith("sqlite_autoindex") then
            table.insert(ret,v)
        end
    end
    return ret
end


function Table:updateTableSchema(table_instance)
    local existColumns = self:existColumnsForTable(table_instance.__tablename__)

    local newColumns = {}
    for i,colType in ipairs(table_instance.__colnames) do
        table.insert(newColumns, colType.name)
    end

    for i,v in ipairs(existColumns) do
        if not  table.has_value(newColumns, v) then 
            error("DB Error: No mapping for existing column :" .. v)
        end
    end
    
    for i,colType in ipairs(table_instance.__colnames) do
        if not table.has_value(existColumns, colType.name) then
            local sql  = "ALTER TABLE " .. table_instance.__tablename__ .." ADD COLUMN " .. colType.name  .. " " .. colType:_create_type(true)
            lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","execute",sql)
        end
    end
end

function Table:createIndex(table_instance,colType)
    local sql = "CREATE INDEX IF NOT EXISTS " .. table_instance.__tablename__ .. "_".. colType.name .. " ON " .. table_instance.__tablename__ .. "(" .. colType.name .. ")"
    return lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","execute",sql)
end

function Table:createIndexes(table_instance)
    for i,colType in ipairs(table_instance.__colnames) do
        if colType.settings.index then
            self:createIndex(table_instance,colType)
        end
    end
end

function Table:updateTableIndex(table_instance)
    local existingIndexes = self:existIndexesForTable(table_instance.__tablename__)
    local existingIndexesMap = {}
    for i,v in ipairs(existingIndexes) do
        existingIndexesMap[v] = v
    end
    for i,colType in ipairs(table_instance.__colnames) do
        if colType.settings.index  then
            if table.has_key(existingIndexesMap,table_instance.__tablename__ .. "_" .. colType.name) then
                self:createIndex(table_instance,colType)
            else
                existingIndexesMap[colType.name] = nil
            end
        end
    end
    for k,v in pairs(existingIndexesMap) do
        if v ~= nil then
            local sql = "DROP INDEX "..v
            lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","execute",sql)
        end 
    end

end


-- This method create new table
-------------------------------------------
-- @table_instance {table} class Table instance
-- 
-- @table_instance.__tablename__ {string} table name
-- @table_instance.__colnames {table} list of column instances
-- @table_instance.__foreign_keys {table} list of foreign key
--                                        column instances
-------------------------------------------
function Table:create_table(table_instance)
    -- table information
    local tablename = table_instance.__tablename__
    local columns = table_instance.__colnames
    local foreign_keys = table_instance.__foreign_keys

    BACKTRACE(INFO, "Start create table: " .. tablename)

    -- other variables
    local create_query = "CREATE TABLE IF NOT EXISTS `" .. tablename .. "` \n("
    local counter = 0
    local column_query
    local result

    for _, coltype in pairs(columns) do
        column_query = "\n     `" .. coltype.name .. "` " .. coltype:_create_type()

        if counter ~= 0 then
            column_query = "," .. column_query
        end

        create_query = create_query .. column_query
        counter = counter + 1
    end

    for _, coltype in pairs(foreign_keys) do
        if not coltype.settings.to.__primary_key then
            error("FOREIGN KEY to table no primary_key" )
        end
        local toTablePrimaryParam = coltype.settings.to.__tablename__
        create_query = create_query .. ",\n     FOREIGN KEY(`" ..
                       coltype.name .. "`)" .. " REFERENCES `" ..
                       coltype.settings.to.__tablename__ ..
                       "`(`" ..coltype.settings.to.__primary_key.name.."`)"
    end

    create_query = create_query .. "\n)"

    lua_thread.postToThreadSync(self.cacheThreadId,"orm.cache","execute",create_query)
end

function Table.addTableInfo(params,callback)
    local function c()
        callback(nil)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","addTableInfoOnLogicThread",params, c);
end

function Table.addTableInfoOnLogicThread(params,callback)
    local tableName = params.name
    local info = params.args
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"DB_DATA")
    lua_thread.postToThreadSync(threadId,"orm.dbData","setDBParams",tableName, info)
    Table(tableName)
    callback();
end

function Table.saveOrm(params,callback)
    local function c()
        callback(nil)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","saveOrmOnLogicThread",params, c);
end

function Table.saveOrmOnLogicThread(params,callback)
    local tableName = params.name
    local orm = params.args
    local t = Table(tableName)
    local data = t(orm)
    data:save();
    callback();
end

function Table.batchSaveOrms(params,callback)
    local function c()
        callback(nil)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","batchSaveOrmsOnLogicThread",params, c);
end

function Table.batchSaveOrmsOnLogicThread(params,callback)
    local tableName = params.name
    local data = params.args
    local t = Table(tableName)
    if data ~= nil then
        for _,orm in ipairs(data) do
            local data = t(orm)
            data:save();
        end
    end
    callback();
end

function Table.prepareQuery(t,params)
    local tableGet =  t.get
    for _,v in ipairs(params) do
        local type = v.type
        local value = v.value
        if type == "WHERE_COLUMS" then
            local wt = {}
            for _,p in ipairs(value) do
                local columName = p.columName;
                local pv = p.value;
                local ptype = p.type;
                local key;
                if ptype == "LESS_THEN" then
                    key = columName.."__lt"
                elseif ptype == "EQ_OR_LESS_THEN" then
                    key = columName.."__lte"
                elseif ptype == "MORE_THEN" then
                    key = columName.."__gt"
                elseif ptype == "EQ_OR_MORE_THEN" then
                    key = columName.."__gte"
                elseif ptype == "IN" then
                    key = columName.."__in"
                elseif ptype == "NOT_IN" then
                    key = columName.."__notin"
                elseif ptype == "IS_NULL" then
                    key = columName.."__null"
                elseif ptype == "LIKE" then
                    key = columName.."__like"
                end
                wt[key] = pv
            end
            tableGet:where(wt)
        elseif type == "WHERE_SQL" then
            local sql = value.sql
            local args = value.args
            tableGet:where(args,sql)
        elseif type == "PRIMARY_KEY" then
            tableGet:primaryKey(value)
        elseif type == "LIMIT" then
            tableGet:limit(value)
        elseif type == "OFFSET" then
            tableGet:offset(value)
        elseif type == "ORDER_BY" then
            tableGet:order_by(value)
        elseif type == "GROUP_BY" then
            tableGet:group_by(value)
        elseif type == "HAVING" then
            tableGet:having(value)
        elseif type == "HAVING_BY_BINDS" then
            local sql = value.sql
            local args = value.args
            tableGet:having(args, sql)
        elseif type == "NEED_COLUMNS" then
            tableGet:needColumns(value)
        elseif type == "JOIN" then
            local tableName = value.tableName
            local joinType = value.type
            local where = value.where
            local whereBindingValues = value.whereBindingValues
            local needColumns = value.needColumns
            local matchColumns = value.matchColumns
            local t = Table(tableName)
            if joinType == "INNER" then
                tableGet:join(t,where,whereBindingValues,needColumns,matchColumns)
            elseif joinType == "FULL" then
                tableGet:full_join(t,where,whereBindingValues,needColumns,matchColumns)
            elseif joinType == "LEFT" then
                tableGet:left_join(t,where,whereBindingValues,needColumns,matchColumns)
            elseif joinType == "RIGHT" then
                tableGet:right_join(t,where,whereBindingValues,needColumns,matchColumns)
            end
        end
    end
    return tableGet
end

function Table.getAllByParams(params,callback)
    local function c(d)
        callback(d)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","getAllByParamsOnLogicThread",params, c);
end

function Table.getAllByParamsOnLogicThread(params,callback)
    local tableName = params.name
    local data = params.args
    local t = Table(tableName)
    local selectItem = Table.prepareQuery(t,data)
    local d = selectItem:all():getPureData()
    callback(d)
end

function Table.getFirstByParams(params,callback)
    local function c(d)
        callback(d)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","getFirstOnLogicThread",params, c);
end

function Table.getFirstOnLogicThread(params,callback)
    local tableName = params.name
    local data = params.args
    local t = Table(tableName)
    local selectItem = Table.prepareQuery(t,data)
    callback(selectItem:first():getPureData())
end

function Table.deleteByParams(params,callback)
    local function c()
        callback(nil)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","deleteByParamsOnLogicThread",params, c);

end

function Table.deleteByParamsOnLogicThread(params,callback)
    local tableName = params.name
    local data = params.args
    local t = Table(tableName)
    local selectItem = Table.prepareQuery(t,data)
    selectItem:delete()
    callback();
end

function Table.updateByParams(params,callback)
    local function c()
        callback(nil)
    end
    local threadId = lua_thread.createThread(BusinessThreadLOGIC,"OrmsThread")
    lua_thread.postToThread(threadId,"orm.class.table","updateByParamsOnLogicThread",params, c);

end

function Table.updateByParamsOnLogicThread(params,callback)
    local tableName = params.name
    local data = params.args
    local updateValue = params.updateValue
    local t = Table(tableName)
    local selectItem = Table.prepareQuery(t,data)
    selectItem:update(updateValue);
    callback();
end



-- Create new table instance
--------------------------------------
-- @args {table} must have __tablename__ key
-- and other must be a column names
--------------------------------------
function Table.new(self, tableName)
    local args = lua_thread.postToThreadSync(self.DBDataThreadId,"orm.dbData","tableParams",tableName)

    local colnames = {}
    local create_query

    self.__tablename__ = args.__tablename__
    self.__dbname__ = args.__dbname__
    lua_thread.synchronized(function ()
        local cacheName = "cache_"..self.__dbname__
        local dbInfo = lua_thread.postToThreadSync(self.DBDataThreadId,"orm.dbData","get",self.__dbname__)
        if  not dbInfo then
            local cacheThreadId = lua_thread.createThread(BusinessThreadLOGIC,cacheName)
            dbInfo = {
                threadName = cacheName,
                threadId = cacheThreadId
            }
            lua_thread.postToThreadSync(self.DBDataThreadId,"orm.dbData","set", self.__dbname__,dbInfo)
        end 
        lua_thread.postToThreadSync(dbInfo.threadId,"orm.cache","initWithDB",self.__dbname__)
        self.cacheThreadId = dbInfo.threadId
    end)

    if _G.All_Tables[self.__dbname__.."_"..self.__tablename__] then
        return _G.All_Tables[self.__dbname__.."_"..self.__tablename__]
    end

    args.__tablename__ = nil
    args.__dbname__ = nil

    local Table_instance = {
        ------------------------------------------------
        --             Table info varibles            --
        ------------------------------------------------

        -- SQL table name
        __tablename__ = self.__tablename__,

        -- SQL db name
        __dbname__ = self.__dbname__,

        -- list of column names
        __colnames = {},

        -- Foreign keys list
        __foreign_keys = {},

        __primary_key = nil,

        ------------------------------------------------
        --                Metamethods                 --
        ------------------------------------------------

        -- If try get value by name "get" it return Select class instance
        __index = function (self, key)
            if key == "get" then
                return Select(self)
            end

            local old_index = self.__index
            setmetatable(self, {__index = nil})

            key = self[key]

            setmetatable(self, {__index = old_index, __call = self.create})

            return key
        end,

        -- Create new row instance
        -----------------------------------------
        -- @data {table} parsed query answer data
        --
        -- @retrun {table} Query instance
        -----------------------------------------
        create = function (self, data)
            return Query(self, data)
        end,

        ------------------------------------------------
        --          Methods which using               --
        ------------------------------------------------

        -- parse column in correct types
        column = function (self, column)
            local tablename = self.__tablename__

            if Type.is.table(column) and column.__classtype__ == AGGREGATOR then
                column.colname = tablename .. column.colname
                column = column .. ""
            end
            return tablename .. "." .. column,
                   tablename .. "_" .. column
        end,

        -- Check column in table
        -----------------------------------------
        -- @colname {string} column name
        --
        -- @return {boolean} get true if column exist
        -----------------------------------------
        has_column = function (self, colname)
            for _, table_column in pairs(self.__colnames) do
                if table_column.name == colname then
                    return true
                end
            end
        end,

        -- get column instance by name
        -----------------------------------------
        -- @colname {string} column name
        --
        -- @return {table} get column instance if column exist
        -----------------------------------------
        get_column = function (self, colname)
            for _, table_column in pairs(self.__colnames) do
                if table_column.name == colname then
                    return table_column
                end
            end

            BACKTRACE(WARNING, "Can't find column '" .. tostring(column) ..
                               "' in table '" .. self.__tablename__ .. "'")
        end
    }

    Table_instance.cacheThreadId = self.cacheThreadId
    -- copy column arguments to new table instance
    for colname, coltype in pairs(args) do
        if coltype[2].to then
            coltype[2].to = _G.All_Tables[coltype[2].to]
        end
        coltype = fields[coltype[1]](coltype[2])
        coltype.name = colname
        coltype.__table__ = Table_instance

        table.insert(Table_instance.__colnames, coltype)
        if coltype.settings.foreign_key then
            table.insert(Table_instance.__foreign_keys, coltype)
        end
        if coltype.settings.primary_key then
            Table_instance.__primary_key = coltype
        end
    end

    Table_instance.args = args
    Table_instance.args.__tablename__ =self.__tablename__ 
    Table_instance.args.__dbname__ =self.__dbname__ 

    setmetatable(Table_instance, {
        __call = Table_instance.create,
        __index = Table_instance.__index
    })

    _G.All_Tables[self.__dbname__.."_"..self.__tablename__] = Table_instance

    -- Create ne table if need
    if self:isTableExist(self.__tablename__) then
        self:updateTableSchema(Table_instance)
        self:updateTableIndex(Table_instance)
    else     
        self:create_table(Table_instance)
        self:createIndexes(Table_instance)
    end

    return Table_instance
end

setmetatable(Table, {__call = Table.new})

return Table