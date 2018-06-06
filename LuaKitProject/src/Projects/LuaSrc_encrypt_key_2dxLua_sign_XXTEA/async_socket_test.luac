local test = {}

test.test = function ()
    lua.thread.postToThread(BusinessThreadIO,"async_socket_test","testOnIOThread")
end

local socket
local timer
test.testOnIOThread = function ()
    socket = lua.asyncSocket.create("127.0.0.1",4001)

    socket.connectCallback = function (rv)
        if rv >= 0 then
            print("Connected")
            socket:read()
        end
    end
    
    socket.readCallback = function (str)
        print(str)
        timer = lua.timer.createTimer(0)
        timer:start(2000,function ()
            socket:write(str)
        end)
        socket:read()
    end

    socket.writeCallback = function (rv)
        print("write" .. rv)
    end

    socket:connect()

end

return test