package com.common.luakit;

public class Demo {

    private static native Object[] getWeatherNative();
    private static native void loadWeatherNative(ILuaCallback callback);
    private static native void threadTestNative();
    private static native void notificationTestNative();
    private static native void dbTestNative();
    private static native void asynSocketTestNative();

    public static  Object[] getWeather(){
        return getWeatherNative();
    }

    public static void loadWeather(ILuaCallback callback)
    {
        loadWeatherNative(callback);
    }

    public static void asynSocketTest(){
        asynSocketTestNative();
    }

    public static void threadTest(){
        threadTestNative();
    }

    public static void notificationTest(){
        notificationTestNative();
    }

    public static void dbTest(){
        dbTestNative();
    }

}
