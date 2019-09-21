package com.common.luakit;

import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;

public class TimerUtil {

    static Object token = new Object();

    static HashMap<Timer, Integer> threadIds = new HashMap<Timer, Integer>();

    static HashMap<Timer, Long> nativeRefs = new HashMap<Timer, Long>();

    private static native void timerCall(long t, int i);

    static Timer createTimer () {
        Timer timer = new Timer(true);
        return timer;
    }

    static void setNativeRefs (Timer t, long ref, int threadId) {
        nativeRefs.put(t,new Long(ref));
        threadIds.put(t,new Integer(threadId));
    }

    static void startTimer (final Timer timer, int type, long period) {
        if(type == 0){
            //one shot
            timer.schedule(new TimerTask() {
                @Override
                public void run() {
                    synchronized (token){
                        if (nativeRefs.containsKey(timer)){
                            Handler h = new Handler(Looper.getMainLooper());
                            h.post(new Runnable() {
                                @Override
                                public void run() {
                                    synchronized (token) {
                                        if (nativeRefs.containsKey(timer) && threadIds.containsKey(timer)) {
                                            Long v = nativeRefs.get(timer);
                                            Integer i = threadIds.get(timer);
                                            timerCall(v.longValue(),i.intValue());
                                        }
                                    }
                                }
                            });
                        }
                    }
                }
            }, period);
        } else {
            //repeat
            timer.schedule(new TimerTask() {
                @Override
                public void run() {
                    synchronized (token) {
                        if (nativeRefs.containsKey(timer)) {
                            Handler h = new Handler(Looper.getMainLooper());
                            h.post(new Runnable() {
                                @Override
                                public void run() {
                                    synchronized (token) {
                                        if (nativeRefs.containsKey(timer) && threadIds.containsKey(timer)) {
                                            Long v = nativeRefs.get(timer);
                                            Integer i = threadIds.get(timer);
                                            timerCall(v.longValue(),i.intValue());
                                        }
                                    }
                                }
                            });
                        }
                    }
                }
            }, 0, period);

        }
    }

    static void stopTimer (Timer timer) {
        timer.cancel();
    }

    static void releaseTimer (Timer timer) {
        synchronized (token) {
            threadIds.remove(timer);
            nativeRefs.remove(timer);
        }
    }
}
