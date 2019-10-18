------------------------------------------------------------------------------
--                                  Class                                   --
------------------------------------------------------------------------------

local Field = {
    -- Table column type
    __type__ = "varchar",

    -- Validator handler
    validator = function (self, value)
        return true
    end,

    -- Default parser
    as = function (value)
        return value
    end,

    to_type = Type.to.str,

    pureKeepOrgType = false,

    -- Call when create new field in some table
    register = function (self, args)
        if not args then
            args = {}
        end

        -- New field type
        -------------------------------------------
        -- @args {table}
        -- Table can have parametrs:
        --    @args.__type__ {string} some sql valid type
        --    @args.validator {function} type validator
        --    @args.as {function} return parse value
        -------------------------------------------
        new_field_type = {
            -- some sql valid type
            __type__ = args.__type__ or self.__type__,

            -- Validator handler
            validator = args.validator or self.validator,

            -- Parse variable for equation
            as = args.as or self.as,

            -- Cast values to correct type
            to_type = args.to_type or self.to_type,

            -- puredata KeepOrgType
            pureKeepOrgType = args.pureKeepOrgType or self.pureKeepOrgType,

            -- Default settings for type
            settings = args.settings or {},

            -- Get new table column instance
            new = function (this, args)
                if not args then
                    args = {}
                end

                local new_self = {
                    -- link to field instance
                    field = this,

                    -- Column name
                    name = nil,

                    -- Parent table
                    __table__ = nil,

                    -- table column settings
                    settings = {
                        default = nil,
                        null = true,
                        unique = false, 
                        max_length = nil,
                        primary_key = false,
                        index = false,
                    },

                    -- Return string for colmn type create
                    _create_type = function (this, fromAlter)
                        local _type = this.field.__type__

                        if this.settings.max_length and this.settings.max_length > 0 then
                            _type = _type .. "(" .. this.settings.max_length .. ")"
                        end

                        if this.settings.primary_key then
                            _type = _type .. " PRIMARY KEY"
                        end

                        if this.settings.auto_increment then
                            _type = _type .. " AUTO_INCREMENT"
                        end

                        if this.settings.unique then
                            _type = _type .. " UNIQUE"
                        end

                        if not fromAlter then
                            _type = _type .. (this.settings.null and " NULL"
                                                             or " NOT NULL")
                        end
                        
                        return _type
                    end
                }

                -- Set Default settings
                -- new_self.settings = new_self.field.settings
                for k,v in pairs(new_self.field.settings) do
                    new_self.settings[k] = v
                end

                -- Set settings for column
                if args.max_length then
                    new_self.settings.max_length = args.max_length
                end

                if args.null then
                    new_self.settings.null = args.null
                end

                if args.index then
                    new_self.settings.index = true;
                end

                if args.primary_key then
                    new_self.settings.primary_key = true;
                end

                if new_self.settings.foreign_key and args.to then
                    new_self.settings.to = args.to
                end

                if args.to and args.foreign_key then
                    new_self.settings.foreign_key = true
                    new_self.settings.to = args.to
                end

                return new_self
            end
        }

        setmetatable(new_field_type, {__call = new_field_type.new})

        return new_field_type
    end
}

return Field