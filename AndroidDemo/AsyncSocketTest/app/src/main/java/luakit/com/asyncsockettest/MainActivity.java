package luakit.com.asyncsockettest;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.common.luakit.LuaHelper;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        LuaHelper.startLuaKit(this);
        LuaHelper.callLuaFunction("async_socket_test","test");
    }
}
