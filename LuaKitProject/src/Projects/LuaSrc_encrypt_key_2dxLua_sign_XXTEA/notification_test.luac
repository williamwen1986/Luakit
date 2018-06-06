local notification = {}

notification.test = function ()
    lua.thread.postToThread(BusinessThreadLOGIC,"notification_test","testOnLOGICThread")
end

local listener

notification.testOnLOGICThread = function ()
    lua.notification.createListener(function (l)
        listener = l
        listener:AddObserver(3,
            function (data)
                print("lua Observer")
                if data then
                    for k,v in pairs(data) do
                        print("lua Observer"..k..v)
                    end
                end
            end
        )
    end);
end

notification.testPostNotification = function ()
    lua.thread.postToThread(BusinessThreadLOGIC,"notification_test","postNotificationOnIOThread")
end

notification.postNotificationOnIOThread = function ()
    lua.notification.postNotification(3,
        {
            lua1 = "lua123",
            lua2 = "lua234"
        })
end

return notification
