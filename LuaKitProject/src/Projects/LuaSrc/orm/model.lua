------------------------------------------------------------------------------
--                               Require                                    --
------------------------------------------------------------------------------
require ("orm.class.global")
require ("orm.tools.func")
------------------------------------------------------------------------------
--                                Constants                                 --
------------------------------------------------------------------------------

local _db = require("DB")

local _dbName = ""

local _needCommitTransaction = false

local _timer
-- Database settings
local db
db = {
    -- Execute SQL query
    init = function (name)
        if _dbName ~= "" and _dbName ~= name then
            error(_dbName..","..name.."two db use the same thread")
        end
        _dbName = name
        _db.init(_dbName)

        _timer = lua_timer.createTimer(1)

        local beginTransation = function( )
            db.execute("begin exclusive transaction")
        end

        local commitTransation = function( )
            db.execute("commit transaction")
        end
        beginTransation()
        _timer:start(2000,function ()
            if _needCommitTransaction then
                commitTransation()
                beginTransation()
                _needCommitTransaction = false
            end
        end)
    end,

    execute = function (query, params, needLastInsertId)
        params = params or {}
        local result, rowId = _db.update(query,params,needLastInsertId)
        _needCommitTransaction = true
        if result then
            return result, rowId
        else
            BACKTRACE(WARNING, "Wrong SQL query")
        end
    end,

    -- Return insert query id
    insert = function (query, params, needLastInsertId)
        local result, rowId = db.execute(query, params, needLastInsertId)
        return result, rowId
    end,

    query = function (query, params, name)
        params = params or {}
        local data = _db.query(query,params,name)
        _needCommitTransaction = true
        return data
    end,

    -- get parced data
    rows = function (query, params)
        params = params or {}
        local data = _db.query(query,params)
        _needCommitTransaction = true
        return data
    end
}

return db
