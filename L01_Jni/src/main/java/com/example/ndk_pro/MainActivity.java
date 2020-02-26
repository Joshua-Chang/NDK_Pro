package com.example.ndk_pro;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Looper;
import android.util.Log;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
// 1、传递数组

//        int i[] = {11, 22, 33, 44, 55};
//        String j[] = {"哈哈", "哦家哦"};
//        test(i, j);
//        Log.e("MYJNI", "int数组：" + Arrays.toString(i));
        //  2、传递引用类型
        Bean bean = new Bean();
        passObject(bean,"tom");

//        invokeBean2Method();
//        invokeBean2Method();

        invokeBean2Method2();
        invokeBean2Method2();

        dynamicNative();
        Log.e("MYJNI",dynamicNative(88));

        List list=new ArrayList();
        for (int i = 0; i < 10; i++) {
//            list.add(new Bean(i));
            list.add("测试"+i);
        }
        testList(list);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native String test(int[] i, String[] j);
    native void passObject(Bean bean,String str);

    native void invokeBean2Method();

    native void invokeBean2Method2();

    native void dynamicNative();
    native String dynamicNative(int i);
    native void testThread();
    native void testList(List list);
    public void updateUI(){
        if (Looper.myLooper() == Looper.getMainLooper()){
            Toast.makeText(this,"更新UI",Toast.LENGTH_SHORT).show();
        }else{
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(MainActivity.this,"更新UI",Toast.LENGTH_SHORT).show();
                }
            });
        }
    }

}
