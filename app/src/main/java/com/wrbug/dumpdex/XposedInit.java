package com.wrbug.dumpdex;

import android.os.Build;

import com.wrbug.dumpdex.dump.LowSdkDump;
import com.wrbug.dumpdex.dump.OreoDump;

import java.io.File;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

/**
 * XposedInit
 *
 * @author wrbug
 * @since 2018/3/20
 */
public class XposedInit implements IXposedHookLoadPackage {

    /**
     * 加固应用包含的包名，如果无法脱壳，请将application的包名，加到此数组
     * com.stub.StubApp 360加固
     * s.h.e.l.l.S 爱加密
     * com.secneo.apkwrapper.ApplicationWrapper 梆梆加固
     * com.tencent.StubShell.TxAppEntry 腾讯加固
     */
    private String[] packages = {"com.stub.StubApp", "s.h.e.l.l.S",
            "com.secneo.apkwrapper.ApplicationWrapper", "com.tencent.StubShell.TxAppEntry"
            , "com.baidu.protect.StubApplication"};

    public static void log(String txt) {
        if (!BuildConfig.DEBUG) {
            return;
        }
        XposedBridge.log("dumpdex-> " + txt);
    }

    public static void log(Throwable t) {
        if (!BuildConfig.DEBUG) {
            return;
        }
        XposedBridge.log(t);
    }

    @Override
    public void handleLoadPackage(final XC_LoadPackage.LoadPackageParam lpparam) {
        Class<?> clazz = null;
        for (String aPackage : packages) {
            clazz = XposedHelpers.findClassIfExists(aPackage, lpparam.classLoader);
            if (clazz != null) {
                log("find class:" + aPackage);
                break;
            }
        }
        if (clazz == null) {
            return;
        }
        final String packageName = lpparam.packageName;
        if (lpparam.packageName.equals(packageName)) {
            String path = "/data/data/" + packageName + "/dump";
            File parent = new File(path);
            if (!parent.exists() || !parent.isDirectory()) {
                parent.mkdirs();
            }
            if (Build.VERSION.SDK_INT >= 26) {
                OreoDump.init(lpparam);
            } else {
                LowSdkDump.init(lpparam);
            }

        }
    }
}
