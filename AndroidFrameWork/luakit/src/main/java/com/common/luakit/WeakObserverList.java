package com.common.luakit;

import android.os.Handler;
import android.util.Log;

import org.chromium.base.ObserverList;

import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import java.util.Iterator;

public class WeakObserverList<E> {
    private ObserverList<WeakReference<E>> mObservers = new ObserverList<WeakReference<E>>();

    public void addObserver(E observer) {
    	
    	Iterator<WeakReference<E>> iterator = mObservers.iterator();
        while (iterator.hasNext()) {
            WeakReference<E> weak = iterator.next();
        	if(weak == null)
        		continue;
        	if(observer == weak.get())
        		return;
        }
    	
        mObservers.addObserver(new WeakReference<E>(observer));
        
    	
    }
    
    public Iterator<WeakReference<E>> getIterator() {
		
    	return mObservers.iterator();
	}

    public void removeObserver(E observer) {
        
            WeakReference<E> ref = null;

            Iterator<WeakReference<E>> iterator = mObservers.iterator();
            while (iterator.hasNext()) {
                WeakReference<E> obs = iterator.next();
                if (obs != null && obs.get() == observer) {
                    ref = obs;
                    mObservers.removeObserver(ref);
                    break;
                }
            }
        
    }

    public boolean isEmpty() {
        return !mObservers.iterator().hasNext();
    }

    private void purge() {
        Iterator<WeakReference<E>> iterator = mObservers.iterator();
        while (iterator.hasNext()) {
            WeakReference<E> obs = iterator.next();
            if (obs != null && obs.get() == null) {
                mObservers.removeObserver(obs);

                //use new iterator
                iterator = mObservers.iterator();
            }
        }
    }

	private static Class<?>[] getArgsClasses(Object[] args, String methodName) {
		Class<?>[] argsClass = (Class<?>[]) null;
		if (args != null) {
			argsClass = new Class<?>[args.length];

			for (int i = 0, j = args.length; i < j; i++) {
				if (args[i] != null) {
					argsClass[i] = args[i].getClass();
				} else {
					argsClass[i] = String.class;
				}
				if (argsClass[i] == Integer.class) {
					argsClass[i] = int.class;
				} else if (argsClass[i] == Boolean.class) {
					argsClass[i] = boolean.class;
				} else if (argsClass[i] == Long.class) {
					argsClass[i] = long.class;
				}  else if (argsClass[i] == Short.class) {
					argsClass[i] = short.class;
				} else if (argsClass[i] == Byte.class) {
					argsClass[i] = byte.class;
				} else if (argsClass[i] == Float.class) {
					argsClass[i] = float.class;
				} else if (argsClass[i] == Double.class) {
					argsClass[i] = double.class;
				} else if (argsClass[i] == Character.class) {
					argsClass[i] = char.class;
				} else if ( args[i]  instanceof Object ) {
                    argsClass[i] = Object.class;
                }
			}
		}
		return argsClass;
	}
	
    public void Notify(final String methodName, final Object... params) {
        Iterator< WeakReference<E> > iterator = mObservers.iterator();
        if (!iterator.hasNext()) {
            return;
        }

        Class<?>[] clss = null;

        if (params.length > 0) {
            clss = getArgsClasses(params,methodName);
        }
        Handler handler = new Handler();
        while (iterator.hasNext()) {
            WeakReference<E> obs = iterator.next();
            final E tempObj = obs.get();
            final Class<?>[] clss1 = clss;
			if (tempObj != null) {

				handler.post(new Runnable() {

					@Override
					public void run() {
						try {
							final Method method = tempObj.getClass().getDeclaredMethod(methodName, clss1);
							method.setAccessible(true);
							method.invoke(tempObj, params);
						} catch (Throwable e) {
                            StackTraceElement[] trace =  e.getStackTrace();
                            Log.w("business", trace[0].toString());
						}
					}
				});
			}
        }
    }

    public void NotifyImmediately(final String methodName, final Object... params) {
        Iterator< WeakReference<E> > iterator = mObservers.iterator();
        if (!iterator.hasNext()) {
            return;
        }

        Class<?>[] clss = null;

        if (params.length > 0) {
            clss = getArgsClasses(params,methodName);
        }
        while (iterator.hasNext()) {
            WeakReference<E> obs = iterator.next();
            final E tempObj = obs.get();
            final Class<?>[] clss1 = clss;
            if (tempObj != null) {
                try {
                    final Method method = tempObj.getClass().getDeclaredMethod(methodName, clss1);
                    method.setAccessible(true);
                    method.invoke(tempObj, params);
                } catch (Throwable e) {
                }
            }
        }
    }

}
