package com.wrbug.dumpdex;

import java.io.File;

import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;

/**
 * Dump
 *
 * @author suanlafen
 * @since 2018/3/23
 */
public class Dump {
    public static void log(String txt) {
        if (!BuildConfig.DEBUG) {
            return;
        }
        XposedBridge.log("dumpdex-> " + txt);
    }

    public static void dumpDex(String packageName, Class<?> aClass) {
        Object dexCache = XposedHelpers.getObjectField(aClass, "dexCache");
        log("decCache=" + dexCache);
        Object o = XposedHelpers.callMethod(dexCache, "getDex");
        byte[] bytes = (byte[]) XposedHelpers.callMethod(o, "getBytes");
        String path = "/data/data/" + packageName + "/dump";
        File parent = new File(path);
        if (!parent.exists() || !parent.isDirectory()) {
            parent.mkdirs();
        }
        File file = new File(path,"source-" + bytes.length + ".dex");
        if (file.exists()) {
            log(file.getName() + " exists");
            return;
        }
        FileUtils.writeByteToFile(bytes, file.getAbsolutePath());
    }
}
