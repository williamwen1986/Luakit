local test = {}

test.fun1 = function ()
	local function callback(p1,c1)
		c1(p1 ,function (p2)
				print("fun1 callback p1:"..p1.." p2:"..p2)
			end)		
	end
	print("fun1")
	local newThreadId1 = lua_thread.createThread(BusinessThreadLOGIC,"newThread1")
	lua_thread.postToThreadSync(newThreadId1,"thread_test","fun2","params:111-222",callback)
	print("fun1 end")
end

test.fun2 = function (p1,callback)
	print("fun2:"..p1)
	local newThreadId2 = lua_thread.createThread(BusinessThreadLOGIC,"newThread2")
	callback(p1,function(p,c)
		print("fun2 callback p:"..p)
		c(p..p)
	end)
	lua_thread.postToThread(newThreadId2,"thread_test","fun3","params:222-333",callback)
	print("fun2 end")
end

test.fun3 = function (p1,callback)
	print("fun3:"..p1)
	callback(p1,function(p,c)
		print("fun3 callback p:"..p)
		c(p..p)
	end)
end

return test
