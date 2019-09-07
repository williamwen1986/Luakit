------------------------------------------------------------------------------
--                                Constants                                 --
------------------------------------------------------------------------------

-- For WHERE equations ends
local LESS_THEN = "__lt"
local EQ_OR_LESS_THEN = "__lte"
local MORE_THEN = "__gt"
local EQ_OR_MORE_THEN = "__gte"
local IN = "__in"
local NOT_IN = "__notin"
local IS_NULL = '__null'
local LIKE = "__like"

-- Joining types
local JOIN = {
    INNER = 'i',
    LEFT = 'l',
    RIGHT = 'r',
    FULL = 'f'
}

------------------------------------------------------------------------------
--                                  Class                                   --
------------------------------------------------------------------------------

local Select = function(own_table)
    return {
        ------------------------------------------------
        --          Table info varibles               --
        ------------------------------------------------
        -- Link for table instance
        own_table = own_table,

        -- Create select rules
        _rules = {
            primaryKey = {},

            -- Where equation rules where whereStr 二选一 
            where = {},

            _bindValuse = {},

            whereStr = "",
            whereBindValues = {},

            needColumns = {},

            selectColumns = {},
            
            -- Having equation rules having havingStr 二选一
            having = {},
            havingStr = "",
            havingBindValues = {},
            -- limit
            limit = nil,
            -- offset
            offset = nil,
            -- order columns list
            order = {},
            -- group columns list
            group = {},
            --Columns rules
            columns = {
                needColumns = {},

                where = "",
                -- Joining tables rules
                join = {},
                -- including columns list
                include = {},
            }
        },

        ------------------------------------------------
        --          Private methods                   --
        ------------------------------------------------

        -- Build correctly equation for SQL searching
        _build_equation = function (self, colname, value)
            local result = ""
            local table_column
            local rule
            local _in

            if colname:endswith(LESS_THEN) and Type.is.number(value) then
                colname = string.cutend(colname, LESS_THEN)
                table_column = self.own_table:get_column(colname)
                result =  " < ?"
                table.insert(self._rules._bindValuse,value)
            elseif colname:endswith(MORE_THEN) and Type.is.number(value) then
                colname = string.cutend(colname, MORE_THEN)
                table_column = self.own_table:get_column(colname)
                result = " > ?" 
                table.insert(self._rules._bindValuse,value)
            elseif colname:endswith(EQ_OR_LESS_THEN) and Type.is.number(value) then
                colname = string.cutend(colname, EQ_OR_LESS_THEN)
                table_column = self.own_table:get_column(colname)
                result = " <= ?" 
                table.insert(self._rules._bindValuse,value)
            elseif colname:endswith(EQ_OR_MORE_THEN) and Type.is.number(value) then
                colname = string.cutend(colname, EQ_OR_MORE_THEN)
                table_column = self.own_table:get_column(colname)
                result = " >= ?" 
                table.insert(self._rules._bindValuse,value)
            elseif colname:endswith(IS_NULL) then
                colname = string.cutend(colname, IS_NULL)

                if value then
                    result = " IS NULL"
                else
                    result = " NOT NULL"
                end
            elseif colname:endswith(LIKE) then
                result = " LIKE "..value
            elseif colname:endswith(IN) or colname:endswith(NOT_IN) then
                rule = colname:endswith(IN) and IN or NOT_IN

                if type(value) == "table" and table.getn(value) > 0 then
                    colname = string.cutend(colname, rule)
                    table_column = self.own_table:get_column(colname)
                    _in = {}

                    for counter, val in pairs(value) do
                        table.insert(_in, "?")
                        table.insert(self._rules._bindValuse,val)
                    end

                    if rule == IN then
                        result = " IN (" .. table.join(_in) .. ")"
                    elseif rule == NOT_IN then
                        result = " NOT IN (" .. table.join(_in) .. ")"
                    end
                end

            else
                table_column = self.own_table:get_column(colname)
                result = " = ?" 
                table.insert(self._rules._bindValuse,value)
            end

            if self.own_table:has_column(colname) then
                local parse_column, _ = self.own_table:column(colname)
                result = parse_column .. result
            end

            return result
        end,

        -- Need for ASC and DESC columns
        _update_col_names = function (self, list_of_cols)
            local tablename = self.own_table.__tablename__
            local result = {}
            local parsed_column

            for _, col in pairs(list_of_cols) do
                if Type.is.table(col) and col.__classtype__ == AGGREGATOR then
                    col.__table__ = self.own_table.__tablename__
                    table.insert(result, col)

                else
                    parsed_column, _ = self.own_table:column(col)
                    table.insert(result, parsed_column)
                end
            end

            return result
        end,

        _buildPrimaryKey = function (self)
            if self._rules.primaryKey and #self._rules.primaryKey > 0 then
                if #self._rules.primaryKey == 1 then
                    local primaryKeyStr = "\nWHERE "..self.own_table.__primary_key.name.." = ?"
                    return primaryKeyStr
                else
                    local sqlKey = "primary_"..own_table.__tablename__..#self._rules.primaryKey
                    local cache = lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","getCacheSql", self.own_table.__tablename__ , sqlKey)
                    if cache then
                        return cache
                    else
                        local keys = {}
                        for _,key in ipairs(self._rules.primaryKey) do
                           table.insert(keys,"?")
                        end
                        local primaryKeyStr = "\nWHERE ".. self.own_table.__primary_key.name.." IN("..table.join(keys)..") "
                        lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","setCacheSql", self.own_table.__tablename__ , sqlKey, primaryKeyStr)
                        return primaryKeyStr
                    end
                end
            end
        end,

        -- Build condition for equation rules
        ---------------------------------------------------
        -- @rules {table} list of columns
        -- @start_with {string} WHERE or HAVING
        --
        -- @retrun {string} parsed string for select equation
        ---------------------------------------------------
        _condition = function (self, rules, start_with)
            local counter = 0
            local condition = ""
            local _equation

            condition = condition .. start_with

            for colname, value in pairs(rules) do
                _equation = self:_build_equation(colname, value)

                if counter ~= 0 then
                     _equation = "AND " .. _equation
                end

                condition = condition .. " " .. _equation
                counter = counter + 1
            end

            return condition
        end,

        -- BUild join tables rules
        _build_join = function (self)
            local result_join = ""
            local unique_tables = {}
            local left_table, right_table, mode ,matchColumns, where, whereParams
            local join_mode, colname
            local parsed_column, _
            local tablename

            for _, value in pairs(self._rules.columns.join) do
                left_table = value[1]
                right_table = value[2]
                mode = value[3]
                where = value[4]
                whereParams = value[5] or {}
                matchColumns = value[6] or {}
                tablename = left_table.__tablename__

                if mode == JOIN.INNER then
                    join_mode = "INNER JOIN"

                elseif mode == JOIN.LEFT then
                    join_mode = "LEFT OUTER JOIN"

                elseif mode == JOIN.RIGHT then
                    join_mode = "RIGHT OUTER JOIN"

                elseif mode == JOIN.FULL then
                    join_mode = "FULL OUTER JOIN"

                else
                    BACKTRACE(WARNING, "Not valid join mode " .. mode)
                end

                if where and #where > 0 then
                    result_join = result_join .. " \n" .. join_mode .. " " ..
                                          tablename .. " ON " .. where
                    for _,v in ipairs(whereParams) do
                        table.insert(self._rules._bindValuse,v)
                    end
                elseif next(matchColumns) ~= nil then
                    local index = 0
                    for rightParam, leftParam  in pairs(matchColumns) do
                        if index == 0 then
                            result_join = result_join .. " \n" .. join_mode .. " " ..
                                      tablename .. " ON "
                        else 
                            result_join = result_join .. " AND "
                        end
                        index = index + 1
                        parsed_column, _ = left_table:column(leftParam)
                        result_join = result_join .. parsed_column
                        parsed_column, _ = right_table:column(rightParam)
                        result_join = result_join .. " = " .. parsed_column
                    end
                else
                    for _, key in pairs(right_table.__foreign_keys) do
                        if key.settings.to == left_table then
                            colname = key.name

                            result_join = result_join .. " \n" .. join_mode .. " " ..
                                          tablename .. " ON "

                            parsed_column, _ = right_table:column(colname)
                            result_join = result_join .. parsed_column

                            parsed_column, _ = left_table:column(left_table.__primary_key.name)
                            result_join = result_join .. " = " .. parsed_column

                            break
                        end
                    end
                end
            end
            return result_join
        end,

        -- String with includin data in select
        --------------------------------------------
        -- @own_table {table|nill} Table instance
        --
        -- @return {string} comma separeted fields
        --------------------------------------------
        _build_including = function (self, own_table, needColumns)
            if not own_table then
                own_table = self.own_table
            end
            local sqlKey = "include_"..own_table.__tablename__
            if needColumns and table.getn(needColumns) > 0 then
                sqlKey = sqlKey..table.concat(needColumns,"_")
            end
            local cache = lua_thread.postToThreadSync(own_table.cacheThreadId,"orm.cache","getCacheSql",own_table.__tablename__, sqlKey)
            if cache then
                for _,v in ipairs(cache.needColumns) do
                   table.insert(self._rules.selectColumns,v)
                end
                return cache.include
            end

            local include = {}
            local colname_as, colname
            local cacheNeedColumns = {}
            -- get current column 
            for _, column in pairs(own_table.__colnames) do
                local addIncluding = function ()
                   local selectColumn = {}
                    selectColumn.tableName = own_table.__tablename__
                    selectColumn.columnName = column.name
                    table.insert(self._rules.selectColumns,selectColumn)
                    table.insert(cacheNeedColumns,selectColumn)
                    colname, colname_as = own_table:column(column.name)
                    table.insert(include, colname .. " AS " .. colname_as)
                end
                if needColumns and table.getn(needColumns) > 0 then
                    if table.has_value(needColumns,column.name) then
                        addIncluding()
                    end
                else
                    addIncluding()
                end
            end

            include = table.join(include)

            lua_thread.postToThreadSync(own_table.cacheThreadId,"orm.cache","setCacheSql", own_table.__tablename__, sqlKey, {
                include = include,
                needColumns = cacheNeedColumns,
            })
            return include
        end,

        -- Method for build select with rules
        _select = function (self)
            self._rules.selectColumns = {}
            local needColumns = {}
            needColumns[self.own_table.__tablename__] = self._rules.needColumns or {}
            local including = self:_build_including(self.own_table,self._rules.needColumns)
            local joining = ""
            local _select
            local tablename
            local condition
            local where
            local rule
            local join

            --------------------- Include Columns To Select ------------------
            _select = "SELECT " .. including

            -- Add join rules
            if table.getn(self._rules.columns.join) > 0 then
                local unique_tables = {}
                table.insert(unique_tables, self.own_table)
                local join_tables = {}
                local left_table, right_table

                for _, values in pairs(self._rules.columns.join) do
                    left_table = values[1]
                    right_table = values[2]
                  
                    if not table.has_value(unique_tables, left_table) then
                        table.insert(unique_tables, left_table)
                        needColumns[left_table.__tablename__] = self._rules.columns.needColumns or {}
                        _select = _select .. ", " .. self:_build_including(left_table,self._rules.columns.needColumns)
                    end
                  
                    if not table.has_value(unique_tables, right_table) then
                        table.insert(unique_tables, right_table)
                        needColumns[right_table.__tablename__] = self._rules.columns.needColumns or {}
                        _select = _select .. ", " .. self:_build_including(right_table,self._rules.columns.needColumns)
                    end
                end

                join = self:_build_join()
            end

            -- Check agregators in select
            if table.getn(self._rules.columns.include) > 0 then
                local aggregators = {}
                local aggregator, as

                for _, value in pairs(self._rules.columns.include) do
                    _, as = own_table:column(value.as)
                    table.insert(aggregators, value[1] .. " AS " .. as)
                end

                _select = _select .. ", " .. table.join(aggregators)
            end
            ------------------- End Include Columns To Select ----------------

            _select = _select .. " FROM " .. self.own_table.__tablename__

            if join then
                _select = _select .. " " .. join
            end

            -- Build WHERE
            if self._rules.primaryKey and #self._rules.primaryKey>0 then
                condition = self:_buildPrimaryKey()
                for _,v in ipairs(self._rules.primaryKey) do
                    table.insert(self._rules._bindValuse,v)
                end
                _select = _select .. " " .. condition
            elseif self._rules.whereStr and self._rules.whereStr ~= "" then
                condition = "\nWHERE ".. self._rules.whereStr
                for _,v in ipairs(self._rules.whereBindValues) do
                    table.insert(self._rules._bindValuse,v)
                end
                _select = _select .. " " .. condition
            else
                if next(self._rules.where) then
                    condition = self:_condition(self._rules.where, "\nWHERE")
                    _select = _select .. " " .. condition
                end
            end

            -- Build GROUP BY
            if table.getn(self._rules.group) > 0 then
                rule = self:_update_col_names(self._rules.group)
                rule = table.join(rule)
                _select = _select .. " \nGROUP BY " .. rule
            end

            -- Build HAVING
            if self._rules.havingStr then
                condition = "\nHAVING".. self._rules.havingStr
                for _,v in ipairs(self._rules.havingBindValues) do
                    table.insert(self._rules._bindValuse,v)
                end
            else
                if next(self._rules.having) and self._rules.group then
                    condition = self:_condition(self._rules.having, "\nHAVING")
                    _select = _select .. " " .. condition
                end
            end
            

            -- Build ORDER BY
            if table.getn(self._rules.order) > 0 then
                rule = self:_update_col_names(self._rules.order)
                rule = table.join(rule)
                _select = _select .. " \nORDER BY " .. rule
            end

            -- Build LIMIT
            if self._rules.limit then
                _select = _select .. " \nLIMIT " .. self._rules.limit
            end

            -- Build OFFSET
            if self._rules.offset then
                _select = _select .. " \nOFFSET " .. self._rules.offset
            end
            return lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","rows",self.own_table.__tablename__,_select,self._rules._bindValuse,needColumns,self._rules.primaryKey,self._rules.selectColumns)
        end,

        -- Add column to table
        -------------------------------------------------
        -- @col_table {table} table with column names
        -- @colname {string/table} column name or list of column names
        -------------------------------------------------
        _add_col_to_table = function (self, col_table, colname)
            if Type.is.str(colname) and self.own_table:has_column(colname) then
                table.insert(col_table, colname)

            elseif Type.is.table(colname) then
                for _, column in pairs(colname) do
                    if (Type.is.table(column) and column.__classtype__ == AGGREGATOR
                    and self.own_table:has_column(column.colname))
                    or self.own_table:has_column(column) then
                        table.insert(col_table, column)
                    end
                end

            else
                BACKTRACE(WARNING, "Not a string and not a table (" ..
                                   tostring(colname) .. ")")
            end
        end,

        --------------------------------------------------------
        --                   Column filters                   --
        --------------------------------------------------------

        needColumns = function (self, needColumns)
            self._rules.needColumns = needColumns
            return self
        end,

        -- Including columns to select query
        include = function (self, column_list)
            if Type.is.table(column_list) then
                for _, value in pairs(column_list) do
                    if Type.is.table(value) and value.as and value[1]
                    and value[1].__classtype__ == AGGREGATOR then
                        table.insert(self._rules.columns.include, value)
                    else
                        BACKTRACE(WARNING, "Not valid aggregator syntax")
                    end
                end
            else
                BACKTRACE(WARNING, "You can include only table type data")
            end

            return self
        end,

        --------------------------------------------------------
        --              Joining tables methods                --
        --------------------------------------------------------

        -- By default, join is INNER JOIN command
        _join = function (self, MODE, left_table, where , whereParams, needColumns, matchColumns)
            local right_table = self.own_table
            if left_table.__tablename__ then
                self._rules.columns.needColumns = needColumns
                table.insert(self._rules.columns.join,
                            {left_table, right_table, MODE, where, whereParams, matchColumns})
            else
                BACKTRACE(WARNING, "Not table in join")
            end

            return self
        end,

        join = function (self, left_table, where, whereParams, needColumns, matchColumns)
            self:_join(JOIN.INNER, left_table, where, whereParams, needColumns, matchColumns)
            return self
        end,

        -- left outer joining command
        left_join = function (self, left_table, where, whereParams, needColumns, matchColumns)
            self:_join(JOIN.LEFT, left_table, where, whereParams, needColumns, matchColumns)
            return self
        end,

        -- right outer joining command
        right_join = function (self, left_table, where, whereParams, needColumns, matchColumns)
            self:_join(JOIN.RIGHT, left_table, where, whereParams, needColumns, matchColumns)
            return self
        end,

        -- full outer joining command
        full_join = function (self, left_table, where, whereParams, needColumns, matchColumns)
            self:_join(JOIN.FULL, left_table, where, whereParams, needColumns, matchColumns)
            return self
        end,

        --------------------------------------------------------
        --              Select building methods               --
        --------------------------------------------------------

        primaryKey = function (self,values)
            if not self.own_table.__primary_key then
                error(self.own_table.__tablename__.. "do not define primary_key, can not select by primary_key")
            end
            self._rules.where = {}
            self._rules.whereStr = ""
            self._rules.whereBindValues = {}
            self._rules.primaryKey = values
            return self
        end,

        -- SQL Where query rules
        where = function (self, args ,whereStr)
            self._rules.primaryKey ={}
            if whereStr then
                self._rules.whereStr = whereStr
                self._rules.whereBindValues = args
            else
                self._rules.whereStr = ""
                self._rules.whereBindValues = {}
                for col, value in pairs(args) do
                    self._rules.where[col] = value
                end
            end

            return self
        end,

        -- Set returned data limit
        limit = function (self, count)
            if Type.is.int(count) then
                self._rules.limit = count
            else
                BACKTRACE(WARNING, "You try set limit to not integer value")
            end

            return self
        end,

        -- From which position start get data
        offset = function (self, count)
            if Type.is.int(count) then
                self._rules.offset = count
            else
                BACKTRACE(WARNING, "You try set offset to not integer value")
            end

            return self
        end,

        -- Order table
        order_by = function (self, colname)
            self:_add_col_to_table(self._rules.order, colname)
            return self
        end,

        -- Group table
        group_by = function (self, colname)
            self:_add_col_to_table(self._rules.group, colname)
            return self
        end,

        -- Having
        having = function (self, args, havingStr)
            if havingStr then
                self._rules.havingStr = havingStr
                self._rules.havingBindValues = args
            end
            for col, value in pairs(args) do
                self._rules.having[col] = value
            end

            return self
        end,

        --------------------------------------------------------
        --                 Update data methods                --
        --------------------------------------------------------

        update = function (self, data)
            if Type.is.table(data) then
                if self._rules.primaryKey and #self._rules.primaryKey>0 then
                    lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","updateWithPrimaryKey",self.own_table.__tablename__,self._rules.primaryKey,data)
                else
                    -- -- Build WHERE
                    local _where = ""
                    if self._rules.whereStr and #self._rules.whereStr>0 then
                        _where = "\nWHERE "..self._rules.whereStr
                        for _,v in ipairs(self._rules.whereBindValues) do
                            table.insert(self._rules._bindValuse,v)
                        end
                    else
                        if next(self._rules.where) then
                            _where = self:_condition(self._rules.where, "\nWHERE")
                        else
                            BACKTRACE(INFO, "No 'where' statement. All data update!")
                        end
                    end
                    lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","update",self.own_table.__tablename__,_where,self._rules._bindValuse,data)
                end
            else
                BACKTRACE(WARNING, "No data for global update")
            end
        end,

        --------------------------------------------------------
        --                 Delete data methods                --
        --------------------------------------------------------

        delete = function (self)
            local _delete = ""
            -- Build WHERE
            local deleteWithPrimary = false
            if self._rules.primaryKey and #self._rules.primaryKey>0 then
                deleteWithPrimary = true
            elseif self._rules.whereStr and #self._rules.whereStr>0 then
                _delete = _delete.."\nWHERE "..self._rules.whereStr
                for _,v in ipairs(self._rules.whereBindValues) do
                    table.insert(self._rules._bindValuse,v)
                end
            else
                if next(self._rules.where) then
                    _delete = _delete .. self:_condition(self._rules.where, "\nWHERE")
                else
                    BACKTRACE(WARNING, "Try delete all values")
                end
            end
            if deleteWithPrimary then
                lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","deleteWithPrimaryKey",self.own_table.__tablename__,self._rules.primaryKey)
            else
                lua_thread.postToThreadSync(self.own_table.cacheThreadId,"orm.cache","delete",self.own_table.__tablename__,_delete,self._rules._bindValuse)
            end
        end,

        --------------------------------------------------------
        --              Get select data methods               --
        --------------------------------------------------------

        -- Return one value
        first = function (self) 
            self._rules.limit = 1
            local data = self:all()

            if data:count() == 1 then
                return data[1]
            end
        end,

        -- Return list of values
        all = function (self)
            local data = self:_select()
            local result = {}
            local tableName = self.own_table.__tablename__
            for _,val in ipairs(data) do
                local current_row = {}
                for i,valueRow in pairs(val) do
                    if self._rules.selectColumns[i].tableName == tableName then
                        current_row[self._rules.selectColumns[i].columnName] = valueRow
                    else
                        if  not current_row[self._rules.selectColumns[i].tableName] then
                            current_row[self._rules.selectColumns[i].tableName] = {}
                        end
                        current_row[self._rules.selectColumns[i].tableName][self._rules.selectColumns[i].columnName] = valueRow
                    end
                end
                table.insert(result, current_row)
            end
            return QueryList(self.own_table, result)
        end
    }
end

return Select 
