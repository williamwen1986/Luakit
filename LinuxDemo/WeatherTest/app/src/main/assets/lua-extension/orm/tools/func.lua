
local Property = require('orm.class.property')

_G.asc = Property({
    parse = function (self)
        return self.__table__ .. "." .. self.colname .. " ASC"
    end
})

_G.desc = Property({
    parse = function (self)
        return  self.__table__ .. "." .. self.colname .. " DESC"
    end
})

_G.MAX = Property({
    parse = function (self)
        return "MAX(" .. self.__table__ .. "." .. self.colname .. ")"
    end
})

_G.MIN = Property({
    parse = function (self)
        return "MIN(" .. self.__table__ .. "." .. self.colname .. ")"
    end
})

_G.COUNT = Property({
    parse = function (self)
        return "COUNT(" .. self.__table__ .. "." .. self.colname .. ")"
    end
})

_G.SUM = Property({
    parse = function (self)
        return "SUM(" .. self.colname .. ")"
    end
})