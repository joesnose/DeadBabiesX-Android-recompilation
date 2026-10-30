package com.joesnose.deadbabiesx;
import org.libsdl.app.SDLActivity;
import android.os.Bundle;
import android.app.AlertDialog;
import android.content.res.AssetManager;
import java.io.*;
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
 private void copyAsset(String path,File target)throws IOException{
  String[] list=getAssets().list(path);
  if(list!=null&&list.length>0){if(!target.isDirectory()&&!target.mkdirs())throw new IOException("Cannot create game folder");for(String name:list)copyAsset(path.isEmpty()?name:path+"/"+name,new File(target,name));}
  else {File tmp=new File(target.getPath()+".tmp");try(InputStream in=getAssets().open(path);OutputStream out=new FileOutputStream(tmp)){byte[]b=new byte[65536];int n;while((n=in.read(b))!=-1)out.write(b,0,n);}if(!tmp.renameTo(target))throw new IOException("Cannot install "+path);}
 }
}
