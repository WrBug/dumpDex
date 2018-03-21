package com.wrbug.dumpdex;

import java.io.File;
import java.lang.reflect.Method;

import dalvik.system.DexFile;
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
    private String[] packages = {"com.stub.StubApp", "s.h.e.l.l.S",
            "com.secneo.apkwrapper.ApplicationWrapper", "com.tencent.StubShell.TxAppEntry"};

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
        XposedBridge.log(packageName);
        try {
            initDexMethod();
        } catch (Throwable t) {
            log(t);
            return;
        }
        XposedHelpers.findAndHookMethod("java.lang.ClassLoader", lpparam.classLoader, "loadClass", String.class, boolean.class, new XC_MethodHook() {
            @Override
            protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                Class c = (Class) param.getResult();
                if (c == null) {
                    return;
                }
                Object object = getDexMethod.invoke(c);
                byte[] array = (byte[]) getBytesMethod.invoke(object);
                if (array == null) {
                    return;
                }
                saveData(packageName, array);
            }
        });
    }

    private void saveData(String packageName, byte[] array) {
        String path = "/data/data/" + packageName + "/dump";
        File parent = new File(path);
        if (!parent.exists() || !parent.isDirectory()) {
            parent.mkdirs();
        }
        final File file = new File(path, "source-" + array.length + ".dex");
        if (!file.exists()) {
            FileUtils.writeByteToFile(array, file.getAbsolutePath());
            log("dump dex :" + file.getAbsolutePath());
        }
    }

    public void initDexMethod() throws ClassNotFoundException, NoSuchMethodException {
        Class dex = Class.forName("com.android.dex.Dex");
        this.getBytesMethod = dex.getDeclaredMethod("getBytes");
        this.getDexMethod = Class.forName("java.lang.Class").getDeclaredMethod("getDex");

    }
}
