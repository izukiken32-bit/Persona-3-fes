package com.izukiken.p3fes
import android.app.Activity
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.widget.*
class MainActivity : Activity() {
 companion object { init { System.loadLibrary("p3fes_runtime") } }
 private external fun nativeRuntimeStatus(): String
 private val prefs by lazy { getSharedPreferences("p3fes_settings", MODE_PRIVATE) }
 override fun onCreate(savedInstanceState: Bundle?) { super.onCreate(savedInstanceState)
  window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
  val root=LinearLayout(this).apply { orientation=LinearLayout.VERTICAL; gravity=Gravity.CENTER; setPadding(28,24,28,24); setBackgroundColor(0xFF101522.toInt()) }
  fun label(s:String,sz:Float=16f)=TextView(this).apply { text=s; textSize=sz; setTextColor(0xFFF4F6FA.toInt()); gravity=Gravity.CENTER; setPadding(0,8,0,8) }
  root.addView(label("PERSONA 3 FES",30f)); root.addView(label("ANDROID • EARLY BUILD",13f)); root.addView(label("Graphics target: ARM Mali GPUs | Android")); root.addView(label(nativeRuntimeStatus(),12f))
  val scale=Spinner(this); scale.adapter=ArrayAdapter(this,android.R.layout.simple_spinner_dropdown_item,listOf("Render scale: 1.0x","Render scale: 0.85x","Render scale: 0.70x")); scale.setSelection(prefs.getInt("scale",0).coerceIn(0,2)); scale.onItemSelectedListener=object:AdapterView.OnItemSelectedListener { override fun onItemSelected(p:AdapterView<*>?,v:View?,pos:Int,id:Long){prefs.edit().putInt("scale",pos).apply()}; override fun onNothingSelected(p:AdapterView<*>?){} }; root.addView(scale)
  val fps=Spinner(this); fps.adapter=ArrayAdapter(this,android.R.layout.simple_spinner_dropdown_item,listOf("Frame limit: 30 FPS","Frame limit: 45 FPS","Frame limit: 60 FPS")); fps.setSelection(prefs.getInt("fps",0).coerceIn(0,2)); fps.onItemSelectedListener=object:AdapterView.OnItemSelectedListener { override fun onItemSelected(p:AdapterView<*>?,v:View?,pos:Int,id:Long){prefs.edit().putInt("fps",pos).apply()}; override fun onNothingSelected(p:AdapterView<*>?){} }; root.addView(fps)
  root.addView(label("Settings are saved locally; they take effect only after runtime integration.",12f)); root.addView(label("Builds via GitHub Actions")); setContentView(root)
 }
}
