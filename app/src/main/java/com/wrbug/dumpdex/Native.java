package com.wrbug.dumpdex;

/**
 * Native
 *
 * @author suanlafen
 * @since 2018/3/23
 */
public class Native {

    static {
        try {
            System.load("/data/local/tmp/libnativeDump.so");
        } catch (Throwable t) {
            System.load("/data/local/tmp/libnativeDump64.so");
        }
//        System.loadLibrary("nativeDump");
    }

    public static native void dump(String packageName);
}
