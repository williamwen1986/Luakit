Create a new project with Android Studio
-----------------------------
If you have your own project , skip this step

Add dependency to your app build.gradle
-----------------------------

```	
repositories {

    maven { url "https://jitpack.io" }

}

dependencies {
    implementation fileTree(dir: 'libs', include: ['*.jar'])
    implementation 'com.github.williamwen1986:LuakitJitpack:1.0.9'
}

```

Copy your lua source code to android assets/lua folder
-----------------------------
Our demo lua source code is in the [luaSrc folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/LuaSrc), you need to copy the source code your need to assets/lua folder, you can also add your own lua file to assets/lua folder.

Initialization Luakit
-----------------------------
Add below code to your entrance of your app

```java
LuaHelper.startLuaKit(this);
```