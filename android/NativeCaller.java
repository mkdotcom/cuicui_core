package io.github.mkdotcom.cuicui;

import android.util.Log;

public class NativeCaller {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static void cuiCppMessageBox(String tag, String value)
    {
        try {
            nativecuiCppMessageBox(tag,value);
        }
        catch(java.lang.UnsatisfiedLinkError e)
        {
            // this may happen when MyFirebaseMessagingService receive a notification BEFORE the native lib is loaded
            // Note: i'm 100% sure it's the real reason, but it seems so
            // FIXED: I've added the "System.loadLibrary" which probably fix this issue anyway
            Log.e("cuicui", "NativeCaller: the native lib may not yet loaded yet (if the exception is UnsatisfiedLinkError), message can't be sent. Warning or error?");
        }
        catch(Exception e)
        {
            Log.e("cuicui", "NativeCaller: exception");
        }
    }

    public native static void nativecuiCppMessageBox(String tag, String value);
}
