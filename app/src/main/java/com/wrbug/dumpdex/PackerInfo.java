package com.wrbug.dumpdex;

import android.support.annotation.NonNull;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

/**
 * PackerInfo
 *
 * @author WrBug
 * @since 2018/3/29
 * <p>
 * 加壳类型
 */
public class PackerInfo {

    private static List<String> sPackageName = new ArrayList<>();
    private static Map<String, Type> sTypeMap = new HashMap<>();


    /**
     * 加固应用包含的包名，如果无法脱壳，请将application的包名，加到相应数组
     */

    /**
     * 60加固
     */
    private static final String[] QI_HOO = {"com.stub.StubApp"};
    /**
     * 爱加密
     */
    private static final String[] AI_JIA_MI = {"s.h.e.l.l.S"};
    /**
     * 梆梆加固
     */
    private static final String[] BANG_BANG = {"com.secneo.apkwrapper.ApplicationWrapper"};
    /**
     * 腾讯加固
     */
    private static final String[] TENCENT = {"com.tencent.StubShell.TxAppEntry"};
    /**
     * 百度加固
     */
    private static final String[] BAI_DU = {"com.baidu.protect.StubApplication"};


    static {
        sPackageName.addAll(Arrays.asList(QI_HOO));
        sPackageName.addAll(Arrays.asList(AI_JIA_MI));
        sPackageName.addAll(Arrays.asList(BANG_BANG));
        sPackageName.addAll(Arrays.asList(TENCENT));
        sPackageName.addAll(Arrays.asList(BAI_DU));

        for (String s : QI_HOO) {
            sTypeMap.put(s, Type.QI_HOO);
        }
        for (String s : AI_JIA_MI) {
            sTypeMap.put(s, Type.AI_JIA_MI);
        }
        for (String s : BANG_BANG) {
            sTypeMap.put(s, Type.BANG_BANG);
        }
        for (String s : TENCENT) {
            sTypeMap.put(s, Type.TENCENT);
        }
        for (String s : BAI_DU) {
            sTypeMap.put(s, Type.BAI_DU);
        }

    }

    public static void log(String txt) {
        XposedBridge.log("dumpdex.PackerInfo-> " + txt);
    }

    public static Type find(final XC_LoadPackage.LoadPackageParam lpparam) {
        for (String s : sPackageName) {
            Class clazz = XposedHelpers.findClassIfExists(s, lpparam.classLoader);
            if (clazz != null) {
                log("find class:" + s);
                Type type = getType(s);
                log("find packerType :" + type.getName());
                return type;
            }
        }
        return null;
    }


    private static Type getType(String packageName) {
        return sTypeMap.get(packageName);
    }

    public enum Type {

        QI_HOO("360加固"),
        AI_JIA_MI("爱加密"),
        BANG_BANG("梆梆加固"),
        TENCENT("腾讯加固"),
        BAI_DU("百度加固");

        String name;

        Type(String s) {
            name = s;
        }

        public String getName() {
            return name;
        }
    }

}
