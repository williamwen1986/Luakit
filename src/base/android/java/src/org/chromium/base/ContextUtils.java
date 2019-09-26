// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.base;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.os.Build;
import android.os.Process;
import android.preference.PreferenceManager;

import androidx.annotation.Nullable;

import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.MainDex;
import org.chromium.base.annotations.VerifiesOnP;

/**
 * This class provides Android application context related utility methods.
 */
@JNINamespace("base::android")
public class ContextUtils {
    private static final String TAG = "ContextUtils";
    private static Context sApplicationContext;

    /**
     * Initialization-on-demand holder. This exists for thread-safe lazy initialization.
     */
    private static class Holder {
        // Not final for tests.
        private static SharedPreferences sSharedPreferences = fetchAppSharedPreferences();
    }

    /**
     * Get the Android application context.
     *
     * Under normal circumstances there is only one application context in a process, so it's safe
     * to treat this as a global. In WebView it's possible for more than one app using WebView to be
     * running in a single process, but this mechanism is rarely used and this is not the only
     * problem in that scenario, so we don't currently forbid using it as a global.
     *
     * Do not downcast the context returned by this method to Application (or any subclass). It may
     * not be an Application object; it may be wrapped in a ContextWrapper. The only assumption you
     * may make is that it is a Context whose lifetime is the same as the lifetime of the process.
     */
    public static Context getApplicationContext() {
        return sApplicationContext;
    }

    /**
     * Initializes the java application context.
     *
     * This should be called exactly once early on during startup, before native is loaded and
     * before any other clients make use of the application context through this class.
     *
     * @param appContext The application context.
     */
    @MainDex // TODO(agrieve): Could add to whole class if not for ApplicationStatus.initialize().
    public static void initApplicationContext(Context appContext) {
        // Conceding that occasionally in tests, native is loaded before the browser process is
        // started, in which case the browser process re-sets the application context.
        assert sApplicationContext == null || sApplicationContext == appContext
                || ((ContextWrapper) sApplicationContext).getBaseContext() == appContext;
        initJavaSideApplicationContext(appContext);
    }

    /**
     * Only called by the static holder class and tests.
     *
     * @return The application-wide shared preferences.
     */
    private static SharedPreferences fetchAppSharedPreferences() {
        return PreferenceManager.getDefaultSharedPreferences(sApplicationContext);
    }

    /**
     * This is used to ensure that we always use the application context to fetch the default shared
     * preferences. This avoids needless I/O for android N and above. It also makes it clear that
     * the app-wide shared preference is desired, rather than the potentially context-specific one.
     *
     * @return application-wide shared preferences.
     */
    public static SharedPreferences getAppSharedPreferences() {
        return Holder.sSharedPreferences;
    }

    /**
     * Occasionally tests cannot ensure the application context doesn't change between tests (junit)
     * and sometimes specific tests has its own special needs, initApplicationContext should be used
     * as much as possible, but this method can be used to override it.
     *
     * @param appContext The new application context.
     */
    @VisibleForTesting
    public static void initApplicationContextForTests(Context appContext) {
        initJavaSideApplicationContext(appContext);
        Holder.sSharedPreferences = fetchAppSharedPreferences();
    }

    private static void initJavaSideApplicationContext(Context appContext) {
        assert appContext != null;
        // Guard against anyone trying to downcast.
        if (BuildConfig.DCHECK_IS_ON && appContext instanceof Application) {
            appContext = new ContextWrapper(appContext);
        }
        sApplicationContext = appContext;
    }

    /**
     * In most cases, {@link Context#getAssets()} can be used directly. Modified resources are
     * used downstream and are set up on application startup, and this method provides access to
     * regular assets before that initialization is complete.
     *
     * This method should ONLY be used for accessing files within the assets folder.
     *
     * @return Application assets.
     */
    public static AssetManager getApplicationAssets() {
        Context context = getApplicationContext();
        while (context instanceof ContextWrapper) {
            context = ((ContextWrapper) context).getBaseContext();
        }
        return context.getAssets();
    }

    /**
     * @return Whether the process is isolated.
     */
    @SuppressWarnings("NewApi")
    public static boolean isIsolatedProcess() {
        // Was not made visible until Android P, but the method has always been there.
        return Process.isIsolated();
    }

    /** @return The name of the current process. E.g. "org.chromium.chrome:privileged_process0". */
    @VerifiesOnP
    public static String getProcessName() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            return Application.getProcessName();
        }
        try {
            Class<?> activityThreadClazz = Class.forName("android.app.ActivityThread");
            return (String) activityThreadClazz.getMethod("currentProcessName").invoke(null);
        } catch (Exception e) {
            // If fallback logic is ever needed, refer to:
            // https://chromium-review.googlesource.com/c/chromium/src/+/905563/1
            throw new RuntimeException(e);
        }
    }

    /**
     * Extract the {@link Activity} if the given {@link Context} either is or wraps one.
     *
     * @param context The context to check.
     * @return Extracted activity if it exists, otherwise null.
     */
    public static @Nullable Activity activityFromContext(@Nullable Context context) {
        // Only retrieves the base context if the supplied context is a ContextWrapper but not an
        // Activity, because Activity is a subclass of ContextWrapper.
        while (context instanceof ContextWrapper) {
            if (context instanceof Activity) return (Activity) context;

            context = ((ContextWrapper) context).getBaseContext();
        }

        return null;
    }
}
