
require("sqlite3")
local _dbPath 
print(BASE_DOCUMENT_PATH)
local _db 

local _shouldCacheStatements = true;
local _cacheStmts = {};
local _cacheIndexColumNameMap = {};
local DB = {}
function DB.init(path)
	_dbPath = BASE_DOCUMENT_PATH.."/"..path;
	_db = sqlite3.open(_dbPath);
end

local function _getColumIndex(sql,stmt,name)
	if (not name) or (not stmt)  then
		return nil
	end 
	if _cacheIndexColumNameMap[sql] then
		 return _cacheIndexColumNameMap[sql][name]
	else
		local names = stmt:get_names()
		local cache = {}
		for i,v in ipairs(names) do
			cache[v] = i-1
		end
		_cacheIndexColumNameMap[sql] = cache
		return _cacheIndexColumNameMap[sql][name]
	end
	return nil
end

local function _databaseExists()
	if _db then
		return true;
	else
		return false;
	end
end

function DB.query(sql,params,columName)
	if not _databaseExists() then
		return nil;
	end

	local stmt;
	if _shouldCacheStatements then
		stmt = _cacheStmts[sql];
		if stmt then
			stmt:reset();
		end
	end
	if not stmt then
		local ret, code = _db:prepare(sql);
		if not ret then
			error("db:prepare error code = ".. code)
		else
			stmt = ret;
		end
	end
    local bindCount = stmt:bind_parameter_count();
    local idx = 0;
    local obj;
    while (idx < bindCount) 
    do
    	if params and idx < #params then
    		obj = params[idx + 1];
    	else 
    		break;
    	end 
    	idx = idx+1;
    	local ret = stmt:bind(idx,obj);
    	assert(ret == sqlite3.OK);
    end

    if idx ~= bindCount then
    	stmt:finalize();
    	return nil;
    end 
    if _shouldCacheStatements and  sql then
    	_cacheStmts[sql] = stmt;
    end

    local r = stmt:step()
    local result = {};
    while (r == sqlite3.ROW) do
    	local tem
    	if columName then
    		local index = _getColumIndex(sql,stmt,columName)
    		if index ~= nil then
    			tem = stmt:get_value(index)
    		else
    			error("query columName not exist")
    		end
    	else
    		tem = {stmt:get_uvalues()};
    	end
        table.insert(result,tem);
        r = stmt:step()
    end
    assert(r == sqlite3.DONE)
    if not _shouldCacheStatements then 
    	assert(stmt:finalize() == sqlite3.OK)
    end
    return result;
end

function DB.update(sql,params,needLastInsertId)
	if not  _databaseExists() then
		return nil;
	end

	local stmt;
	if _shouldCacheStatements then
		stmt = _cacheStmts[sql];
		if stmt then
			stmt:reset();
		end
	end
	if not stmt then
		local ret, code = _db:prepare(sql);
		if not ret then
			error("db:prepare error code = ".. "unknon error ",2)
		else
			stmt = ret;
		end
	end
    local bindCount = stmt:bind_parameter_count();
    local idx = 0;
    local obj;
    while (idx < bindCount) 
    do
    	if params and idx < #params then
    		obj = params[idx + 1];
    	else
    		break;
    	end 
    	idx = idx+1;
    	local ret = stmt:bind(idx,obj);
    	assert(ret == sqlite3.OK);
    end
    if idx ~= bindCount then
    	stmt:finalize();
    	return nil;
    end 
    local r = stmt:step();
    if r == sqlite3.DONE then
    else
    error("update error errorcode:"..r);
	end

	local lastInsertId
	if needLastInsertId then
		lastInsertId = stmt:last_insert_rowid()
		if lastInsertId then
			lastInsertId = tonumber(lastInsertId)
		end
	end
	local closeCode;
	if _shouldCacheStatements and r == sqlite3.DONE then
		_cacheStmts[sql] = stmt;
		closeCode = stmt:reset();
	else 
		closeCode = stmt:finalize();
		_cacheStmts[sql] = nil;
	end

	if closeCode ~= sqlite3.OK then
		error("Unknown error finalizing or resetting statement"..sql.."("..closeErrorCode..")");
	end

	return r == sqlite3.DONE,lastInsertId
end

function DB.close()
	if _cacheStmts then
		for k, v in pairs(_cacheStmts) do  
			if v then
				v:finalize();
			end  
		end 
	end
	if _db then
		_db:close();
	end
	_cacheStmts = {};
	_db = nil;
end

return DB;
