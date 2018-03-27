# dumpDex-Android脱壳神器

> 插件需要在xposed环境中使用,支持市面上大多数加密壳,软件仅供学习用，请勿用于其他用途

### 编译环境

Android Studio 3.0

jdk1.8

**无法脱壳，请在 [XposedInit.java](https://github.com/WrBug/dumpDex/blob/master/app/src/main/java/com/wrbug/dumpdex/XposedInit.java#L31)文件中，将应用的包名加到packages字段里，编译安装即可,欢迎提交Pull Requests，让软件更加全面**

### 支持设备

大多数xposed环境的手机

### 分支

[develop](https://github.com/WrBug/dumpDex/tree/develop) 开发分支

[master](https://github.com/WrBug/dumpDex/tree/master) 稳定分支

### 使用方式

下载源码编译或者下载apk包并安装，应用xposed模块后重启，运行加固的应用后，插件会自动将dex文件dump到 **/data/data/包名/dump** 目录

**apk文件不会实时更新，获取最新apk请自行编译源码**

### apk 下载

apk文件为master分支打包应用，develop分支请自行编译

[https://doc.wrbug.com/](https://doc.wrbug.com/)

[https://pan.baidu.com/s/1DTR-XjWnniFHx-cB1Iib5Q](https://pan.baidu.com/s/1DTR-XjWnniFHx-cB1Iib5Q)

### 源码编译

将源码下载或者clone到本地，使用android studio打开，编译成功后，安装apk，将 **libnativeDump.so** 复制到 */data/local/tmp* 目录，权限 设置为777，可以通过文件管理器操作，也可以使用如下adb shell命令

```bash
adb shell
su
cp /data/data/com.wrbug.dumpdex/lib/libnativeDump.so /data/local/tmp
chmod 777 /data/local/tmp/libnativeDump.so

```

配置完成后激活xposed重启即可

### 更多精彩内容请关注博客

[https://www.wrbug.com/](https://www.wrbug.com/)


### 支持开发，欢迎打赏

![](/pay.png)


