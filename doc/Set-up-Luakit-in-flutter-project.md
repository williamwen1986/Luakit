## 如何接入flutter\_luakit\_plugin

经过了几个月磨合实践，我们团队已经把接入flutter\_luakit\_plugin的成本降到最低，可以说是非常方便接入了。我们已经把flutter\_luakit\_plugin发布到flutter官方的插件仓库。首先，要像其他flutter插件一样，在pubspec.yaml里面加上依赖，可参考[demo配置](https://github.com/williamwen1986/flutter_luakit_demo/blob/master/pubspec.yaml)

```
flutter_luakit_plugin: ^1.0.8
```

然后在ios项目的podfile加上ios的依赖，可参考[demo配置](https://github.com/williamwen1986/flutter_luakit_demo/blob/master/ios/Podfile)

```
source 'https://github.com/williamwen1986/LuakitPod.git'
source 'https://github.com/williamwen1986/curl.git'
pod 'curl', '~> 1.0.0'
pod 'LuakitPod', '~> 1.0.17'
```

然后在android项目app的build.gradle文件加上android的依赖，可参考[demo配置](https://github.com/williamwen1986/flutter_luakit_demo/blob/master/android/app/build.gradle)


```
repositories {
    maven { url "https://jitpack.io" }
}

dependencies {
    implementation 'com.github.williamwen1986:LuakitJitpack:1.0.9'
}

```
最后，在需要使用的地方加上import就可以使用lua脚本了

```
import 'package:flutter_luakit_plugin/flutter_luakit_plugin.dart';
```

lua脚本我们默认的执行根路径在android是 assets/lua，ios默认的执行根路径是Bundle路径。