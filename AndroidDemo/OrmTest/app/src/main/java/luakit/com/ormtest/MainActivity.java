package luakit.com.ormtest;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import com.common.luakit.LuaHelper;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        System.out.print("first statement. \n");
        setContentView(R.layout.activity_main);
        LuaHelper.startLuaKit(this);
        LuaHelper.callLuaFunction("db_test","test");
    }
}
