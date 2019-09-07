package com.common.luakit;


public class NativeHandleHolder {
    protected static final int HANDLE_TYPE_NOTIFICATION_LISTENER = 0;

    protected long mNativeHandle = 0;

    protected native void nativeFree(long handle, int type);

    public void Free(int type) {
        try {
            if (mNativeHandle != 0) {
                nativeFree(mNativeHandle, type);
                mNativeHandle = 0;
            }
        }catch(Throwable t){
        }
    }
}

