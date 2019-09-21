package luakit.com.notificationtest;

import android.content.Context;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.common.luakit.INotificationObserver;
import com.common.luakit.LuaHelper;
import com.common.luakit.LuaNotificationListener;
import com.common.luakit.NotificationHelper;

import java.util.HashMap;
import java.util.Map;

public class MainActivity extends AppCompatActivity {
    private  MyAdapter adapter;
    private LuaNotificationListener listener;
    private INotificationObserver observer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        LuaHelper.startLuaKit(this);
        LuaHelper.callLuaFunction("notification_test","test");
        ListView lv=(ListView) findViewById(R.id.lv);
        adapter = new MyAdapter(this);
        lv.setOnItemClickListener( new MyOnItemClickListener());
        lv.setAdapter(adapter);

        listener = new LuaNotificationListener();
        observer = new INotificationObserver() {
            @Override
            public void onObserve(int type, Object info) {
                HashMap<String, Integer> map = (HashMap<String, Integer>)info;
                for (Map.Entry<String, Integer> entry : map.entrySet()) {
                    Log.i("business", "android onObserve");
                    Log.i("business", entry.getKey());
                    Log.i("business",""+entry.getValue());
                }
            }
        };
        listener.addObserver(3, observer);
    }

    private class MyOnItemClickListener implements AdapterView.OnItemClickListener{

        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position,
                                long id) {
            HashMap<String, Integer> map = new HashMap<String, Integer>();
            map.put("row", new Integer(position));
            NotificationHelper.postNotification(3, map);
        }
    }



    private class MyAdapter extends BaseAdapter {
        private LayoutInflater mInflater;

        public MyAdapter(Context context) {
            this.mInflater = LayoutInflater.from(context);
        }

        @Override
        public int getCount() {
            return  10;
        }

        @Override
        public View getView(final int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.list_item, null);
            }
            TextView title = (TextView) convertView.findViewById(R.id.title);
            String titleText = "number : " + position;
            title.setText(titleText);
            return convertView;
        }

        @Override
        public Object getItem(int position) {
            // TODO Auto-generated method stub
            return null;
        }

        @Override
        public long getItemId(int position) {
            // TODO Auto-generated method stub
            return 0;
        }

    }
}
