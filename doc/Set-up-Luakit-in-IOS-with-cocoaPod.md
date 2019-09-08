Create a new project with xcode
-----------------------------
If you have your own project , skip this step

Add dependency to your app pod file
-----------------------------


```	
source 'https://github.com/williamwen1986/curl.git'
source 'https://github.com/williamwen1986/LuakitPod.git'

target 'demo' do
  pod 'curl', '~> 1.0.0'
  pod 'LuakitPod', '~> 1.0.12'
end
```

Add the lua source to your project
-----------------------------
Our demo lua source code is in the [luaSrc folder](https://github.com/williamwen1986/Luakit/tree/master/LuaKitProject/src/Projects/LuaSrc), src/Projects/LuaSrc

Initialization Luakit
-----------------------------
Add below code to your entrance of your app. In most cases , you can do this in main function, modify your main file name from main.m to main.mm and add below code

```c++
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#import <LuakitPod/oc_helpers.h>

int main(int argc, char * argv[]) {
    startLuakit(argc, argv);
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
```