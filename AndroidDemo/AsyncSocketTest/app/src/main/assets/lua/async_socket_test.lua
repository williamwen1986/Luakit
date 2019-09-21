local test = {}

test.test = function ()
<<<<<<< HEAD
    print("Test...")
=======
>>>>>>> Merge "build-macos" branch with William
    lua_thread.postToThread(BusinessThreadIO,"async_socket_test","testOnIOThread")
end

local socket
local timer
test.testOnIOThread = function ()
<<<<<<< HEAD
    print("testOnIOThread")
    socket = lua_asyncSocket.create("88.190.98.37",80)
=======
    socket = lua_asyncSocket.create("www.google.com",443)
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
<<<<<<< HEAD
                        print("Timer")
=======
>>>>>>> Merge "build-macos" branch with William
                        socket:read()
                end

        )
        socket:read()
    end

    socket.writeCallback = function (rv)
        print("write: " .. rv)
    end
<<<<<<< HEAD

    socket:connect()

end

return test
=======
    socket:connect()
    print ('socket:connect()')

end

return test
>>>>>>> Merge "build-macos" branch with William
