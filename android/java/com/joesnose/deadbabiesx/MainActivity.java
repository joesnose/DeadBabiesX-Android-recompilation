package com.joesnose.deadbabiesx;
import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import android.app.AlertDialog;
import android.content.res.AssetManager;
import java.io.*;
import android.os.Handler;
import android.os.Looper;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.LinearLayout;
import android.text.InputType;
import java.net.NetworkInterface;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.Enumeration;
public class MainActivity extends SDLActivity {
 @Override protected String[] getLibraries(){return new String[]{"SDL2","main"};}
 @Override protected void onCreate(Bundle state){
  try {
   // Only packaged assets are replaced on upgrade. Player saves remain intact.
   File marker=new File(getFilesDir(),"assets-v2.ready");
   if(!marker.exists()){copyAsset("",getFilesDir());try(FileOutputStream o=new FileOutputStream(marker)){o.write(1);}}
  } catch(IOException e){super.onCreate(state);new AlertDialog.Builder(this).setMessage("Unable to prepare game files: "+e.getMessage()).setPositiveButton("Close",(d,w)->finish()).show();return;}
  super.onCreate(state);
 }
 private static native void lanHost(boolean mega);
 private static native void lanJoin(String ip);
 private static native void lanStart();
 private static native void lanCancel();
 private static native String lanStatus();
 private AlertDialog lanDialog;
 private final Handler lanHandler=new Handler(Looper.getMainLooper());
 private Runnable lanPoll;
 private boolean lanLoaded=false;
 public void showLanSetup(final boolean mega){runOnUiThread(()->{
  if(isFinishing()||isDestroyed())return;
  lanLoaded=true;
  new AlertDialog.Builder(this).setTitle("LAN multiplayer")
   .setMessage("Connect every phone to the same Wi-Fi and install the same game version. One phone hosts; the others join its Wi-Fi IP. Up to four phones. A guest Wi-Fi network may block phones from connecting to each other.")
   .setPositiveButton("Host",(d,w)->{lanHost(mega);showLanLobby(true);})
   .setNeutralButton("Join",(d,w)->{
    EditText ip=new EditText(this);ip.setSingleLine(true);ip.setInputType(InputType.TYPE_CLASS_PHONE);ip.setHint("Host IP, e.g. 192.168.1.20");
    new AlertDialog.Builder(this).setTitle("Join LAN game").setView(ip).setPositiveButton("Connect",(a,b)->{lanJoin(ip.getText().toString().trim());showLanLobby(false);}).setNegativeButton("Cancel",null).show();
   }).setNegativeButton("Cancel",null).show();
 });}
 private String wifiAddresses(){
  StringBuilder result=new StringBuilder();
  try{Enumeration<NetworkInterface> interfaces=NetworkInterface.getNetworkInterfaces();while(interfaces.hasMoreElements()){NetworkInterface n=interfaces.nextElement();if(!n.isUp()||n.isLoopback())continue;Enumeration<InetAddress> addrs=n.getInetAddresses();while(addrs.hasMoreElements()){InetAddress a=addrs.nextElement();if(a instanceof Inet4Address&&a.isSiteLocalAddress()){if(result.length()>0)result.append(" / ");result.append(a.getHostAddress());}}}}catch(Exception ignored){}
  return result.length()>0?result.toString():"Check Wi-Fi network details in Android Settings";
 }
 private void showLanLobby(final boolean host){
  if(lanPoll!=null)lanHandler.removeCallbacks(lanPoll);
  final TextView status=new TextView(this);status.setPadding(24,20,24,20);status.setTextSize(17);
  LinearLayout layout=new LinearLayout(this);layout.setOrientation(LinearLayout.VERTICAL);
  if(host){TextView addr=new TextView(this);addr.setPadding(24,16,24,0);addr.setText("Your Wi-Fi IP: "+wifiAddresses()+"\nPort: 39828 (TCP). Guests enter your IP. The host's Mega Weapons setting applies to everyone.");layout.addView(addr);}layout.addView(status);
  AlertDialog.Builder builder=new AlertDialog.Builder(this).setTitle(host?"Host LAN room":"Join LAN room").setView(layout).setNegativeButton("Cancel",(d,w)->lanCancel()).setOnCancelListener(d->lanCancel());
  if(host)builder.setPositiveButton("Start match",(d,w)->{});
  lanDialog=builder.create();lanDialog.show();
  if(host)lanDialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener(v->lanStart());
  lanPoll=new Runnable(){public void run(){if(lanDialog==null||!lanDialog.isShowing())return;String text=lanStatus();if(text.equals("PLAYING")){lanDialog.dismiss();lanDialog=null;return;}status.setText(text);if(host)lanDialog.getButton(AlertDialog.BUTTON_POSITIVE).setEnabled(text.startsWith("HOST:")&&!text.startsWith("HOST: 1 "));lanHandler.postDelayed(this,200);}};
  lanHandler.post(lanPoll);
 }
 @Override protected void onPause(){if(lanLoaded)lanCancel();super.onPause();}
 @Override protected void onDestroy(){if(lanLoaded)lanCancel();if(lanPoll!=null)lanHandler.removeCallbacks(lanPoll);if(lanDialog!=null){lanDialog.dismiss();lanDialog=null;}super.onDestroy();}
 private void copyAsset(String path,File target)throws IOException{
  String[] list=getAssets().list(path);
  if(list!=null&&list.length>0){if(!target.isDirectory()&&!target.mkdirs())throw new IOException("Cannot create game folder");for(String name:list)copyAsset(path.isEmpty()?name:path+"/"+name,new File(target,name));}
  else {File tmp=new File(target.getPath()+".tmp");try(InputStream in=getAssets().open(path);OutputStream out=new FileOutputStream(tmp)){byte[]b=new byte[65536];int n;while((n=in.read(b))!=-1)out.write(b,0,n);}if(!tmp.renameTo(target))throw new IOException("Cannot install "+path);}
 }
}
