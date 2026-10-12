package io.github.mkdotcom.cuicui;

import android.content.Context;
import android.content.SharedPreferences;
import android.util.Log;

public class LocalStorage {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static SharedPreferences preferences;
    public static LocalStorage instance=null;

    // This must be called before using LocalStorage.
    // Cuicui calls it in CuicuiActivity::onCreate() so user doesn't have to do it
    // But! User must call it inside a BroadcastReceiver if he uses one (as the BroadcastReceiver won't start the CuicuiActivity)
    public static void EnsureReady(Context context)
    {
        if (instance==null) { // to think: can a Context expired or be deleted? if yes maybe we should "recreate" the instance with "context" if it changed from previous EnsureReady() ?
            instance = new LocalStorage();
            try {
                instance.preferences = context.getSharedPreferences("cuiSharedPreferences", Context.MODE_PRIVATE);
            } catch (Exception ex)
            {
                Log.e("cuicui", "android : LocalStorage::GetInstance() exception :" + ex.getMessage());
            }
        }
    }

    public static LocalStorage GetInstance()
    {
       return instance;
    }

    public static void localStorage_ClearAll()
    {
        LocalStorage ls = LocalStorage.GetInstance();
        try
        {
            SharedPreferences.Editor editor = ls.preferences.edit();
            editor.clear();
            editor.commit();
        }
        catch (Exception ex)
        {
            Log.d("cuicui","localStorage_ClearAll: error 1");
            return;
        }
    }


    public static String localStorage_Write(String tag, String value)
    {
        LocalStorage ls = LocalStorage.GetInstance();
        try {
            SharedPreferences.Editor editor = ls.preferences.edit();
            editor.putString(tag, value);
            editor.commit();
            return "ok"; // TODO: boolean would be better
        } catch (Exception ex)
        {
            return "error"; // errir
        }
    }


    public static String localStorage_Read(String tag) {
        LocalStorage ls = LocalStorage.GetInstance();
        try {
            if (ls.preferences.contains(tag)==false)
                return "android_local_storage_read_error";
            return ls.preferences.getString(tag, "");
        } catch (Exception ex)
        {
            Log.e("cuicui", "android localStorage_Read exception :" + ex.getMessage());
            return "android_local_storage_read_error";
        }
    }


}
