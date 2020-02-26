package com.example.l02_mk_cmake;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        nativeTest();
    }
//    System.loadLibrary只用来加载动态库
    static {
//        System.loadLibrary("Test");
//        使用android.mk时6.0之后会自动引入依赖的动态库，但是电脑位置
//        1、所以6.0以后不需要提前引入依赖的库
//        2、使用android.mk不可以动态库依赖动态库 换cmake
        System.loadLibrary("hello-jni");
    }
    native void nativeTest();
}
