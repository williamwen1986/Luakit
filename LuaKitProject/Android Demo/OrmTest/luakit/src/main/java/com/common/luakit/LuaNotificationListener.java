package com.common.luakit;

import android.util.SparseArray;

import org.chromium.base.ThreadUtils;

public class LuaNotificationListener extends NativeHandleHolder {
    public LuaNotificationListener() {
        mNativeHandle = nativeNewNotificationListener();
        assert mNativeHandle != 0;
    }

    private SparseArray<WeakObserverList<INotificationObserver>> mTypedObservers = new SparseArray<WeakObserverList<INotificationObserver>>();
    private WeakObserverList<INotificationObserver> mObservers = new WeakObserverList<INotificationObserver>();

    public void addObserver(int type, INotificationObserver observer) {
        if (mTypedObservers.get(type) == null) {
            mTypedObservers.put(type, new WeakObserverList<INotificationObserver>());
            nativeAddObserver(type);
        }

        WeakObserverList<INotificationObserver> observers = mTypedObservers.get(type);
        observers.addObserver(observer);
    }

    public void removeObserver(int type, INotificationObserver observer) {
        if (mTypedObservers.get(type) != null) {
            WeakObserverList<INotificationObserver> observers = mTypedObservers.get(type);
            observers.removeObserver(observer);

            if (observers.isEmpty()) {
                nativeRemoveObserver(type);
            }
        }
    }

    //called by jni
    private void onObserve(int type, Object info) {
        WeakObserverList<INotificationObserver> observers = mTypedObservers.get(type);
        if (observers != null) {
            observers.Notify("onObserve", type, info);
        }
    }

    protected void finalize() {
        assert mNativeHandle != 0;

        if (mNativeHandle != 0) {
            ThreadUtils.postOnUiThread(new Runnable() {
                @Override
                public void run() {
                    int size = mTypedObservers.size();
                    for (int index = 0; index < size; index ++) {
                        int key = mTypedObservers.keyAt(index);
                        nativeRemoveObserver(key);
                    }

                    Free(HANDLE_TYPE_NOTIFICATION_LISTENER);
                    mNativeHandle = 0;
                }
            });
        }
    }

    private native long nativeNewNotificationListener();
    private native void nativeAddObserver(int type);
    private native void nativeRemoveObserver(int type);
}
