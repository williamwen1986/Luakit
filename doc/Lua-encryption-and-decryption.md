Luakit provide lua encryption and decryption refer to cocos2d-x 
-----------------------------

You can use the encryption tool in [cocos2d-x](http://www.cocos.com/download) to encrypt your lua code , download the cocos2d-x project, go to the folder cocos2d-x-3.17/tools/cocos2d-console/bin , run the shell as below to encrypt lua codes

```	
./cocos luacompile -e -s LuaSrc -d LuaSrc_encrypt_key_2dxLua_sign_XXTEA -k 2dxLua -b XXTEA --disable-compile
```	
Here the ENCRYPTKEY is 2dxLua ande the ENCRYPTSIGN is XXTEA, you can modify it to your own ENCRYPTKEY and ENCRYPTSIGN

Android decryption
-----------------------------

Refer to jni [demo code](https://github.com/williamwen1986/Luakit/blob/master/LuaKitProject/src/Projects/jni/com_common_luakit_LuaHelper.cpp) , add below code at the entrance of your jni code

```	c++
setXXTEAKeyAndSign("2dxLua", strlen("2dxLua"), "XXTEA", strlen("XXTEA"));
```	

IOS decryption
-----------------------------

Refer to [demo code](https://github.com/williamwen1986/Luakit/blob/master/LuaKitProject/IOS%20Demo/WeatherTest/WeatherTest/main.mm) , add below code at the entrance of your app

```	c++
setXXTEAKeyAndSign("2dxLua", strlen("2dxLua"), "XXTEA", strlen("XXTEA"));
```	