------------------------------------------------------------------------------
--                                  Libs                                   --
------------------------------------------------------------------------------

Type = require('orm.class.type')
Field = require('orm.class.fields')



------------------------------------------------------------------------------
--                                Field Types                               --
------------------------------------------------------------------------------
local function save_as_str(str)
    return str
end

local field = {}

field.PrimaryField = Field:register({
    __type__ = "integer",
    validator = Type.is.int,
    settings = {
        null = true,
        primary_key = true,
        auto_increment = true
    },
    to_type = Type.to.number
})

field.IntegerField = Field:register({
    __type__ = "integer",
    validator = Type.is.int,
    to_type = Type.to.number
})

field.RealField = Field:register({
    __type__ = "real",
    validator = Type.is.number,
    to_type = Type.to.number
})

field.BlobField = Field:register({
    __type__ = "blob",
    validator = Type.is.str,
    to_type = Type.to.str
})

field.CharField = Field:register({
    __type__ = "varchar",
    validator = Type.is.str,
    as = save_as_str
})

field.TextField = Field:register({
    __type__ = "text",
    validator = Type.is.str,
    as = save_as_str
})

field.BooleandField = Field:register({
    __type__ = "bool"
})

field.DateTimeField = Field:register({
    __type__ = "integer",
    validator = function (value)
        if (Type.is.table(value) and value.isdst ~= nil)
        or Type.is.int(value) then
            return true
        end
    end,
    as = function (value)
        return Type.is.int(value) and value or os.time(value) 
    end,
    to_type = function (value)
        return os.date("*t", Type.to.number(value))
    end
})

-- field.ForeignKey = Field:register({
--     __type__ = "integer",
--     settings = {
--         null = true,
--         foreign_key = true
--     },
--     to_type = Type.to.number
-- })

field.register = Field.register

return field