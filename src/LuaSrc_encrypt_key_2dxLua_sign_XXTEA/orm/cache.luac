------------------------------------------------------------------------------
--                          cache.lua                                  --
------------------------------------------------------------------------------
local Table = require('orm.class.table')
local _instanceCache = {}

local _sqlCache = {}

local _dbThreadId = -1 
local _cache
_cache = {
    
    printInstanceCache = function ()
        for k,v in pairs(_instanceCache) do
            print("print cache "..k.."------------------")
            for k2,v2 in pairs(v) do
                print(k2.."------------------")
                for k3,v3 in pairs(v2) do
                    print(k3,v3)
                end
            end
            print("end print cache "..k.."------------------")
        end
    end,

    initWithDB = function (dbname)
        if _dbThreadId == -1 then
            local cacheThreadId = lua_thread.createThread(BusinessThreadLOGIC,"db_"..dbname)
            _dbThreadId = cacheThreadId
            lua_thread.postToThreadSync(_dbThreadId,"orm.model","init",dbname)
        end
    end,

    query = function (query, params, name)
        params = params or {}
        local data = lua_thread.postToThreadSync(_dbThreadId,"orm.model","query",query,params,name)
        return data
    end,

    execute = function (query, params)
        params = params or {}
        local data = lua_thread.postToThreadSync(_dbThreadId,"orm.model","execute",query,params)
        return data
    end,

    rows = function (tablename, sql, bindValues, needColumns, whereIds, selectColums)
        _instanceCache[tablename] =  _instanceCache[tablename] or {}
        local results = {}
        local needDBData = false
        if whereIds and #whereIds > 0 then
            for i,v in ipairs(whereIds) do
                if not _instanceCache[tablename][v] then
                    needDBData = true
                    break
                end
            end
        else
            needDBData = true
        end

        if not needDBData then
            for _,v in ipairs(whereIds) do
                local t = _instanceCache[tablename][v]
                local result = {}
                for i,selectColum in ipairs(selectColums) do
                    result[i] = t[selectColum.columnName]
                end
                table.insert(results,result)
            end
            return results
        end
        results = lua_thread.postToThreadSync(_dbThreadId,"orm.model","rows",sql,bindValues)

        local needCacheTable = {}

        for k,v in pairs(needColumns) do
            local t = Table(k)
            if #v == 0 and t.__primary_key then
                needCacheTable[k] = {}
            end
        end

        for i,result in ipairs(results) do
            for _,v in pairs(needCacheTable) do
                v[i] = {}
            end
            for index,selectColum in ipairs(selectColums) do
                if needCacheTable[selectColum.tableName] then
                    needCacheTable[selectColum.tableName][i][selectColum.columnName] = result[index]
                end
            end
        end
        
        for k,v in pairs(needCacheTable) do
            local t = Table(k)
            for _,instance in ipairs(v) do
                _instanceCache[k] = _instanceCache[k] or {}
                _instanceCache[k][instance[t.__primary_key.name]] = instance
            end
        end

        -- for k,v in pairs(_instanceCache[tablename]) do
        --      print("cache5:",k)
        --      for k2,v2 in pairs(v) do
        --          print(k2,v2)
        --      end
        --  end
        return results
    end,


    insert = function (tablename, kv, needPrimaryKey)
        local t = Table(tablename)
        local hasCacheSql = false
        local needPrimaryKeyNum = 0
        if needPrimaryKey then
            needPrimaryKeyNum = 1
        end
        local sqlKey = "insert_"..tablename..needPrimaryKeyNum
        local cache = _cache.getCacheSql(tablename,sqlKey)
        local insert
        if cache then
            insert = cache
            hasCacheSql = true
        else
            insert = "REPLACE INTO " .. tablename .. " ("
        end
        local counter = 0
        local values = ""
        local bindValues = {}
        for _, table_column in pairs(t.__colnames) do
            if needPrimaryKey and table_column.name == self.own_table.__primary_key.name then
            else
                local colname = table_column.name
                local value = kv[colname]
                if value then
                    table.insert(bindValues,value)
                    value = "?"
                else
                    value = "NULL"
                end

                if counter ~= 0 then
                    if not hasCacheSql then
                        colname = ", " .. colname
                    end
                    value = ", " .. value
                end
                values = values .. value
                if not hasCacheSql then
                    insert = insert .. colname
                end
                counter = counter + 1
            end
        end

        if not hasCacheSql then
            insert = insert .. ") \n\t    VALUES (" 
            _cache.setCacheSql(tablename, sqlKey, insert)
        end
        insert = insert .. values .. ")"

        
        if not needPrimaryKey then
            lua_thread.postToThread(_dbThreadId,"orm.model","insert",insert,bindValues)
            if t.__primary_key and  kv[t.__primary_key.name] ~= nil then
                if not _instanceCache[tablename] then
                    _instanceCache[tablename] = {}
                end
                _instanceCache[tablename][kv[t.__primary_key.name]] = kv
            end
        else
            local result,rowId
            result,rowId = lua_thread.postToThreadSync(_dbThreadId,"orm.model","insert",insert,bindValues,needPrimaryKey)
            return  
        end
    end,

    update = function (tablename, whereSql, whereBindingValues, data)
        local t = Table(tablename)
        local _update = "UPDATE " .. tablename .. ""
        local _set = ""
        local coltype
        local _set_tbl = {}
        local setBindingValue = {}
        local i=1

        for colname, new_value in pairs(data) do
            coltype = t:get_column(colname)
            if coltype and coltype.field.validator(new_value) then
                _set = _set .. " " .. colname .. " = ?"
                _set_tbl[i] = " " .. colname .. " = ?"
                i=i+1
                table.insert(setBindingValue, new_value)
            else
                BACKTRACE(WARNING, "Can't update value for column `" ..
                                    Type.to.str(colname) .. "`")
            end
        end
        if t.__primary_key then
            local selectSql = "SELECT "..t.__primary_key.name.." FROM "..tablename.." "..whereSql
            local results = lua_thread.postToThreadSync(_dbThreadId,"orm.model","rows",selectSql,whereBindingValues)
            local ids = {}
            for _,v in ipairs(results) do
                table.insert(ids, v[1])
            end
            if #ids > 0 then
                local s = require('orm.class.select')(t)
                s:primaryKey(ids)
                local where = s:_buildPrimaryKey()
                if _set ~= "" then
                    if #_set_tbl<2 then
                        _update = _update .. " SET " .. _set .. " " .. where
                    else
                        _update = _update .. " SET " .. table.concat(_set_tbl,",") .. " " .. where
                    end
                else
                    BACKTRACE(WARNING, "No table columns for update")
                end
                for _,v in ipairs(ids) do
                    table.insert(setBindingValue,v)
                end
                lua_thread.postToThread(_dbThreadId,"orm.model","execute",_update,setBindingValue)
                -- remove cache
                if _instanceCache[tablename] then
                    for _,v in ipairs(ids) do
                        if _instanceCache[tablename][v] then
                            for key,value in pairs(data) do
                                _instanceCache[tablename][v][key] = value
                            end
                        end
                    end
                end
            end
        else
            if _set ~= "" then
                if #_set_tbl<2 then
                    _update = _update .. " SET " .. _set .. " " .. _where
                else
                    _update = _update .. " SET " .. table.concat(_set_tbl,",") .. " " .. _where
                end
                for _,v in ipairs(whereBindingValues) do
                    table.insert(setBindingValue,v)
                end
                lua_thread.postToThread(_dbThreadId,"orm.model","execute",_update,setBindingValue)
            else
                BACKTRACE(WARNING, "No table columns for update")
            end
        end

        -- for k,v in pairs(_instanceCache[tablename]) do
        --      print("cache4:",k)
        --      for k2,v2 in pairs(v) do
        --          print(k2,v2)
        --      end
        -- end
    end,

    updateWithPrimaryKey = function (tablename,ids,data)
         if #ids > 0 then
            local _update = "UPDATE " .. tablename .. ""
            local t = Table(tablename)
            local _set = ""
            local coltype
            local _set_tbl = {}
            local setBindingValue = {}
            local i=1

            for colname, new_value in pairs(data) do
                coltype = t:get_column(colname)
                if coltype and coltype.field.validator(new_value) then
                    _set = _set .. " " .. colname .. " = ?"
                    _set_tbl[i] = " " .. colname .. " = ?"
                    i=i+1
                    table.insert(setBindingValue, new_value)
                else
                    BACKTRACE(WARNING, "Can't update value for column `" ..
                                        Type.to.str(colname) .. "`")
                end
            end
           
            local s = require('orm.class.select')(t)
            s:primaryKey(ids)
            local where = s:_buildPrimaryKey()
            if _set ~= "" then
                if #_set_tbl<2 then
                    _update = _update .. " SET " .. _set .. " " .. where
                else
                    _update = _update .. " SET " .. table.concat(_set_tbl,",") .. " " .. where
                end
            else
                BACKTRACE(WARNING, "No table columns for update")
            end
            for _,v in ipairs(ids) do
                table.insert(setBindingValue,v)
            end
            lua_thread.postToThread(_dbThreadId,"orm.model","execute",_update,setBindingValue)
            -- remove cache
            if _instanceCache[tablename] then
                for _,v in ipairs(ids) do
                    if _instanceCache[tablename][v] then
                        for key,value in pairs(data) do
                            _instanceCache[tablename][v][key] = value

                        end
                    end
                end
            end

            -- for k,v in pairs(_instanceCache[tablename]) do
            --      print("cache2:",k)
            --      for k2,v2 in pairs(v) do
            --          print(k2,v2)
            --      end
            -- end
        end
    end,



    delete = function (tablename, whereSql, bindValues)
        local t = Table(tablename)
        if t.__primary_key then
            local selectSql = "SELECT "..t.__primary_key.name.." from "..tablename.." "..whereSql
            local results = lua_thread.postToThreadSync(_dbThreadId,"orm.model","rows",selectSql,bindValues)
            local ids = {}
            for _,v in ipairs(results) do
                table.insert(ids, v[1])
            end
            if #ids > 0 then
                local s = require('orm.class.select')(t)
                s:primaryKey(ids)
                local whereSql = s:_buildPrimaryKey()
                local sql = "DELETE from "..tablename.." "..whereSql
                lua_thread.postToThread(_dbThreadId,"orm.model","execute",sql,ids)

                -- remove cache
                if _instanceCache[tablename] then
                    for _,v in ipairs(ids) do
                        _instanceCache[tablename][v] = nil
                    end
                end
            end
        else
            local sql = "DELETE from "..tablename.." "..whereSql
            lua_thread.postToThread(_dbThreadId,"orm.model","execute",sql,bindValues)
        end
    end,

    deleteWithPrimaryKey = function (tablename,ids)
        local t = Table(tablename)
        if #ids > 0 then
            local s = require('orm.class.select')(t)
            s:primaryKey(ids)
            local whereSql = s:_buildPrimaryKey()
            local sql = "DELETE from "..tablename.." "..whereSql
            lua_thread.postToThread(_dbThreadId,"orm.model","execute",sql,ids)

            -- remove cache
            if _instanceCache[tablename] then
                for _,v in ipairs(ids) do
                    _instanceCache[tablename][v] = nil
                end
            end
        end
    end,


    getCacheSql = function (tableName,key)
        if _sqlCache[tableName] then
            return _sqlCache[tableName][key]
        end
    end,

    setCacheSql = function (tableName, k , v)
        if not _sqlCache[tableName] then
            _sqlCache[tableName] = {}
        end
        _sqlCache[tableName][k] = v
    end,
}

return _cache

