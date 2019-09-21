local test = {}

test.test = function ()
    lua_thread.postToThread(BusinessThreadIO,"async_socket_test","testOnIOThread")
end

local socket
local timer
test.testOnIOThread = function ()
<<<<<<< HEAD
    socket = lua_asyncSocket.create("88.190.98.37",80)
=======
    socket = lua_asyncSocket.create("127.0.0.1",80)
>>>>>>> Merge "build-macos" branch with William

    socket.connectCallback = function (rv)
        if rv >= 0 then
            print("Connected")
            socket:read()
            socket:write('La plume de ma tante\n')
        else
            print("NOT Connected")
        end
    end

    socket.readCallback = function (str)
        print("read: " .. str)
        timer = lua_timer.createTimer(0)
        timer:start (
                2000,
                function ()
                        socket:read()
                end

        )
        socket:read()
    end

    socket.writeCallback = function (rv)
        print("write: " .. rv)
    end

    socket:connect()

end

return test