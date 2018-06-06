local db = {}

local Table = require('orm.class.table')

db.test = function ()
    print("test_create=========================")
    db.test_create();
    print("test_update=========================")
    db.test_update();
    print("test_delete=========================")
    db.test_delete();
    print("test_update_many=========================")
    db.test_update_many();
    print("test_delete_many=========================")
    db.test_delete_many();
    print("test_join=========================")
    db.test_join();
end

db.test_create = function ()
	local userTable = Table("user")
	local user = userTable({
			username = "user1",
            password = "abc",
            time_create = os.time()
		})
	print("username:"..user.username)
	print("password:"..user.password)
	print("time_create:"..os.time(user.time_create))
	user:save()
	lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
	local users = userTable.get:all()
	for i,v in ipairs(users) do
		print(i.."---------------------")
		print(v.username)
		print(v.password)
		print(os.time(user.time_create))
	end
	lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
end

db.test_update = function ()
	local userTable = Table("user")
	local user = userTable.get:primaryKey({"user1"}):first()
	user.password = "efg"
	user.time_create = os.time()
	print("username:"..user.username)
	print("password:"..user.password)
	print("time_create:"..os.time(user.time_create))
	user:save()
	lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
	local users = userTable.get:all()
	for i,v in ipairs(users) do
		print(i.."---------------------")
		print(v.username)
		print(v.password)
		print(os.time(user.time_create))
	end
	lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")

end

db.test_delete = function ()
	local userTable = Table("user")
	local user = userTable.get:primaryKey({"user1"}):first()
	user:delete()
	lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
	local users = userTable.get:all()
	print("users count :"..#users)
	for i,v in ipairs(users) do
		print(i.."---------------------")
		print(v.username)
		print(v.password)
		print(os.time(user.time_create))
	end
	lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
end

db.test_update_many = function ()
	local userTable = Table("user")
	local usernames = {"first", "second", "third", "operator",
                           "creator", "randomusername"}
    local passwords = {"secret_one", "scrt_tw", "hello",
                       "world", "testpasswd", "new"}
    local age = {33, 12, 22, 44, 44, 44}
    local user

    for i = 1, #usernames do
        user = userTable({username = usernames[i],
                     password = passwords[i],
                     age = age[i]})
        user:save()
    end
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    userTable.get:where({age__gt = 40}):update({age = 45})
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    local users = userTable.get:all()
	print("users count :"..#users)
	for i,v in ipairs(users) do
		print(i.."---------------------")
		print(v.username)
		print(v.password)
		print(v.age)
	end

end

db.test_delete_many = function ()
	local userTable = Table("user")
	local usernames = {"first", "second", "third", "operator",
                           "creator", "randomusername"}
    local passwords = {"secret_one", "scrt_tw", "hello",
                       "world", "testpasswd", "new"}
    local age = {33, 12, 22, 44, 44, 44}
    local user

    for i = 1, #usernames do
        user = userTable({username = usernames[i],
                     password = passwords[i],
                     age = age[i]})
        user:save()
    end
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    userTable.get:where({age__gt = 40}):delete()
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    local users = userTable.get:all()
	print("users count :"..#users)
	for i,v in ipairs(users) do
		print(i.."---------------------")
		print(v.username)
		print(v.password)
		print(v.age)
	end
end

db.test_select = function ()
	local userTable = Table("user")
	local usernames = {"first", "second", "third", "operator",
                           "creator", "randomusername"}
    local passwords = {"secret_one", "scrt_tw", "hello",
                       "world", "testpasswd", "new"}
    local age = {33, 12, 22, 44, 44, 44}
    local user

    for i = 1, #usernames do
        user = userTable({username = usernames[i],
                     password = passwords[i],
                     age = age[i]})
        user:save()
    end

    local users = userTable.get:all()
    print("select all -----------")
    print("users count :"..#users)

    user = userTable.get:first()
    print("select first -----------")
    print(user.username)

    users = userTable.get:limit(3):offset(2):all()
    print("select limit offset -----------")
    for _, u in pairs(users) do
        print(u.username .. "  " .. u.password .. "  " .. u.age)
    end

    print("select order_by -----------")
    users = userTable.get:order_by({desc('age'), asc('username')}):all()
    for _, u in pairs(users) do
        print(u.username .. "  " .. u.age)
    end

    print("select where -----------")
    users = userTable.get:where({ age__lt = 30,
	                         age__lte = 30,
	                         age__gt = 10,
	                         age__gte = 10,
	                         username__in = {"first", "second", "creator"},
	                         password__notin = {"testpasswd", "new", "hello"},
	                         username__null = false
                              }):all()
    for _, u in pairs(users) do
        print(u.username .. "  " .. u.password .. "  " .. u.age)
    end

    print("select where customs -----------")
    users = userTable.get:where({"scrt_tw",30},"password = ? AND age < ?"):all()
    for _, u in pairs(users) do
        print(u.username .. "  " .. u.password .. "  " .. u.age)
    end

    print("select primaryKey -----------")
	users = userTable.get:primaryKey({"first","randomusername"}):all()
	for _, u in pairs(users) do
        print(u.username .. "  " .. u.password .. "  " .. u.age)
    end
end

db.test_join = function ()
	local userTable = Table("user")
	local usernames = {"first", "second", "third", "operator",
                           "creator", "randomusername"}
    local passwords = {"secret_one", "scrt_tw", "hello",
                       "world", "testpasswd", "new"}
    local age = {33, 12, 22, 44, 44, 44}
    local user

    for i = 1, #usernames do
        user = userTable({username = usernames[i],
                     password = passwords[i],
                     age = age[i]})
        user:save()
    end

    local newsTable = Table("news")
    newsTable.get:delete()
    local news = newsTable({title = "first some news", create_user_id = "first"})
    news:save()
    news = newsTable({title = "second", create_user_id = "second"})
    news:save()

    print("join foreign_key")
    local user_group = newsTable.get:join(userTable):all()
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    for _,v in pairs(user_group) do
    	print(v.title.." ".. v.create_user_id.. " " .. v.user.username .. "  " .. v.user.password .. "  " .. v.user.age)
    end

    print("join where ")
    user_group = newsTable.get:join(userTable,"news.create_user_id = user.username AND user.age < ?", {20}):all()
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    for _,v in pairs(user_group) do
    	print(v.title.." ".. v.create_user_id.. " " .. v.user.username .. "  " .. v.user.password .. "  " .. v.user.age)
    end

    print("join matchColumns ")
    user_group = newsTable.get:join(userTable,nil,nil,nil,{create_user_id = "username", title = "username"}):all()
    lua.thread.postToThreadSync(userTable.cacheThreadId,"orm.cache","printInstanceCache")
    for _,v in pairs(user_group) do
    	print(v.title.." ".. v.create_user_id.. " " .. v.user.username .. "  " .. v.user.password .. "  " .. v.user.age)
    end
end


return db
