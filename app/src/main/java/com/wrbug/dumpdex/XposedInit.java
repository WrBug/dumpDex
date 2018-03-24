package com.wrbug.dumpdex;

import android.app.Application;
import android.content.Context;
import android.content.ContextWrapper;
import android.os.Build;

import java.io.File;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
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

    private Method getBytesMethod;
    private Method getDexMethod;


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
            XposedHelpers.findAndHookMethod("android.app.Instrumentation", lpparam.classLoader, "newApplication", ClassLoader.class, String.class, Context.class, new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                    log("Application=" + param.getResult());
                    dumpDex(packageName, param.getResult().getClass());
                    attachBaseContextHook(lpparam, ((Application) param.getResult()));
                }
            });
        }
    }

    private void dumpDex(String packageName, Class<?> aClass) {
        if (Build.VERSION.PREVIEW_SDK_INT >= 26) {
            DumpOreo.dumpDex(packageName, aClass);
        } else {
            Dump.dumpDex(packageName, aClass);
        }
    }

    private void attachBaseContextHook(final XC_LoadPackage.LoadPackageParam lpparam, final Application application) {
        ClassLoader classLoader = application.getClassLoader();
        XposedHelpers.findAndHookMethod(ClassLoader.class, "loadClass", String.class, boolean.class, new XC_MethodHook() {
            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                log("loadClass->" + param.args[0]);
                Class result = (Class) param.getResult();
                if (result != null) {
                    dumpDex(lpparam.packageName, result);
                }
            }
        });
        XposedHelpers.findAndHookMethod("java.lang.ClassLoader", classLoader, "loadClass", String.class, boolean.class, new XC_MethodHook() {
            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                log("loadClassWithclassLoader->" + param.args[0]);
                Class result = (Class) param.getResult();
                if (result != null) {
                    dumpDex(lpparam.packageName, result);
                }
            }
        });
    }
}
