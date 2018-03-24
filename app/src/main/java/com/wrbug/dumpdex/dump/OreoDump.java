package com.wrbug.dumpdex.dump;

import com.wrbug.dumpdex.BuildConfig;
import com.wrbug.dumpdex.Native;

import java.io.File;

import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

/**
 * OreoDump
 *
 * @author suanlafen
 * @since 2018/3/23
 */
public class OreoDump {

    public static void log(String txt) {
        if (!BuildConfig.DEBUG) {
            return;
        }
        XposedBridge.log("dumpdex-> " + txt);
    }

    public static void init(XC_LoadPackage.LoadPackageParam lpparam) {
        Native.dump(lpparam.packageName);
    }
}
