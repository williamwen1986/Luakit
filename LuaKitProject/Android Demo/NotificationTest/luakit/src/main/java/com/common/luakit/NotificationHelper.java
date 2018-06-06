package com.common.luakit;

import org.chromium.base.ThreadUtils;

public class NotificationHelper {

    private static native void postNotificationNative(int type , Object o);

    public static void postNotification( final int type ,final Object o){
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                postNotificationNative(type,o);
            }
        });
    }

}
