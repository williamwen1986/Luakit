IDE AndroidStudio
-----------------------------

AndroidStudio->Preference..->Plugins->Browse reprositories...

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/1.jpeg)

Search Luakit and install Luakit plugin and restart androidstudio

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/2.jpeg)

Open Project Struture window

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/3.jpeg)

Select Modules and Mark as Sources

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/4.jpeg)

Select Edit Configurations ...

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/5.jpeg)

Select plus

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/6.jpeg)

Add Lua Remote(Mobdebug)

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/7.png)

Before you debug in androidstudio, we need add the follow code, there are two params , first is your computer ip address, second is the connecting port ,default is 8172.

```lua
require("mobdebug").start("172.25.129.165", 8172)
```

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/9.jpeg)

Here are some tips for debuging on the device , if your device can not locate in the same network segment, you can open your device's hotspot and connect your computer to your device's hotspot, now you can debug your device 

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/10.jpeg)

![image](https://raw.githubusercontent.com/williamwen1986/Luakit/master/image/11.jpeg)



