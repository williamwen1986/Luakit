------------------------------------------------------------------------------
--                          dbData.lua                                  --
------------------------------------------------------------------------------
local _data = {}

local _tableParams = {
	user = {
		__dbname__ = "test.db",
	    __tablename__ = "user",
	    username = {"CharField",{max_length = 100, unique = true, primary_key = true}},
	    password = {"CharField",{max_length = 50, unique = true}},
	    age = {"IntegerField",{null = true}},
	    job = {"CharField",{max_length = 50, null = true}},
	    des = {"TextField",{null = true}},
	    time_create = {"DateTimeField",{null = true}}
	},

	news = {
		__dbname__ = "test.db",
	    __tablename__ = "news",
	    title = {"CharField",{max_length = 100, unique = false, null = false, primary_key = true}},
	    text = {"TextField",{null = true}},
	    create_user_id = {"CharField",{max_length = 100, unique = true, foreign_key = true, to = "test.db_user",}}
	},

	weather = {
		__dbname__ = "test.db",
	    __tablename__ = "weather",
	    id = {"IntegerField",{unique = true, null = false , primary_key = true}},
	    wind = {"TextField",{}},
	    wind_direction = {"TextField",{}},
	    sun_info = {"TextField",{}},
	    low = {"IntegerField",{}},
	    high = {"IntegerField",{}},
	    city =  {"TextField",{}},
	 },

}

local dbData = {
    set = function (k,v)
        _data[k] = v
    end,

    get = function (k)
    	return _data[k]
    end,

    tableParams = function (k)
    	return _tableParams[k]
    end,
}
return dbData

