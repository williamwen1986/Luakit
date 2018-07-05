package com.common.luakit;

import android.content.Context;
import android.util.Log;
import org.chromium.base.PathUtils;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class LuaHelper {

    private static native void startLuaKitNative(Context c);

    public static native Object callLuaFunction(String moduleName, String methodName);

    public static native Object callLuaFunction(String moduleName, String methodName, Object p1);

    public static native Object callLuaFunction(String moduleName, String methodName, Object p1, Object p2);

    public static native Object callLuaFunction(String moduleName, String methodName, Object p1, Object p2, Object p3);

    public static native Object callLuaFunction(String moduleName, String methodName, Object p1, Object p2, Object p3, Object p4);

    public static native Object callLuaFunction(String moduleName, String methodName, Object p1, Object p2, Object p3, Object p4, Object p5);

    static { System.loadLibrary("luaFramework");}

    public static void startLuaKit(Context c){
        String toPath = PathUtils.getDataDirectory(c)+"/lua";
        File toFolder = new File(toPath);
        if (toFolder.exists()){
            deleteDirection(toFolder);
        }
        toFolder = new File(toPath);
        toFolder.mkdir();
        copyFolderFromAssets(c, "lua",toPath);
        Log.d("copyfile", "copyFolderFromAssets");
        startLuaKitNative(c);
    }

    private static boolean deleteDirection(File dir) {
        if (dir == null || !dir.exists() || dir.isFile()) {
            return false;
        }
        for (File file : dir.listFiles()) {
            if (file.isFile()) {
                file.delete();
            } else if (file.isDirectory()) {
                deleteDirection(file);// 递归
            }
        }
        dir.delete();
        return true;
    }

    private static void copyFolderFromAssets(Context context, String rootDirFullPath, String targetDirFullPath) {
        Log.d("copyfile", "copyFolderFromAssets " + "rootDirFullPath-" + rootDirFullPath + " targetDirFullPath-" + targetDirFullPath);
        try {
            String[] listFiles = context.getAssets().list(rootDirFullPath);
            for (String string : listFiles) {
                Log.d("copyfile", "name-" + rootDirFullPath + "/" + string);
                if (isFileByName(string)) {
                    copyFileFromAssets(context, rootDirFullPath + "/" + string, targetDirFullPath + "/" + string);
                } else {
                    String childRootDirFullPath = rootDirFullPath + "/" + string;
                    String childTargetDirFullPath = targetDirFullPath + "/" + string;
                    new File(childTargetDirFullPath).mkdirs();
                    copyFolderFromAssets(context, childRootDirFullPath, childTargetDirFullPath);
                }
            }
        } catch (IOException e) {
            Log.d("copyfile", "copyFolderFromAssets " + "IOException-" + e.getMessage());
            Log.d("copyfile", "copyFolderFromAssets " + "IOException-" + e.getLocalizedMessage());
            e.printStackTrace();
        }
    }

    private static boolean isFileByName(String string) {
        if (string.contains(".")) {
            return true;
        }
        return false;
    }

    private static void copyFileFromAssets(Context context, String assetsFilePath, String targetFileFullPath) {
        Log.d("copyfile", "copyFileFromAssets ");
        InputStream assestsFileImputStream;
        try {
            assestsFileImputStream = context.getAssets().open(assetsFilePath);

            FileOutputStream fos = new FileOutputStream(new File(targetFileFullPath));
            byte[] buffer = new byte[1024];
            int byteCount=0;
            while((byteCount=assestsFileImputStream.read(buffer))!=-1) {
                fos.write(buffer, 0, byteCount);
            }
            fos.flush();
            assestsFileImputStream.close();
            fos.close();
        } catch (IOException e) {
            Log.d("copyfile", "copyFileFromAssets " + "IOException-" + e.getMessage());
            e.printStackTrace();
        }
    }

}
