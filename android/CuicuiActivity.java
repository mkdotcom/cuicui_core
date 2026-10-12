package io.github.mkdotcom.cuicui;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Point;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.provider.MediaStore;
import android.provider.OpenableColumns;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.FileProvider;
import androidx.core.view.MotionEventCompat;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.exifinterface.media.ExifInterface;
import androidx.fragment.app.FragmentActivity;
import androidx.lifecycle.Lifecycle;
import androidx.lifecycle.LifecycleObserver;
import androidx.lifecycle.OnLifecycleEvent;
import androidx.lifecycle.ProcessLifecycleOwner;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.provider.Settings;
import android.text.Editable;
import android.text.InputType;
import android.text.Layout;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.method.PasswordTransformationMethod;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Display;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.ViewTreeObserver;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.CalendarView;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.DataSource;
import com.bumptech.glide.load.engine.GlideException;
import com.bumptech.glide.load.resource.bitmap.DownsampleStrategy;
import com.bumptech.glide.load.resource.gif.GifDrawable;
import com.bumptech.glide.request.RequestListener;
import com.bumptech.glide.request.target.Target;
import com.bumptech.glide.request.transition.Transition;
// (formerly com.wshunli.assets.* / android-copy-assets lib: replaced by a vanilla copy via AssetManager, see cuicuiCopyAssetDir below)


import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import android.util.Base64;


import io.github.mkdotcom.cuicui.LocalStorage;

import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;


// PERF : GradientDrawable -> replace with PaintDrawable maybe


import static android.util.TypedValue.COMPLEX_UNIT_PX;
import static android.view.MotionEvent.INVALID_POINTER_ID;
import static android.view.ViewGroup.LayoutParams.MATCH_PARENT;
import static android.view.ViewGroup.LayoutParams.WRAP_CONTENT;
import static java.security.AccessController.getContext;


public class CuicuiActivity extends FragmentActivity implements LifecycleObserver   {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    public static String fakeStaticMethodForTestOnly(String tag) {
        Log.d("cuicui","fakeStaticMethod_testonly called");
        return "ok";
    }

    private static final int READ_REQUEST_CODE = 42;
    private static final int REQUEST_TAKE_PHOTO = 43;


    private int myIds=10000; // we use that instead of "View.generateViewId();" because this method may returns many times the same number

    FrameLayout masterFrameLayout;

    private int lastMasterFrameWidth=-1;
    private int lastMasterFrameHeight=-1;

    /////////////////
    // upload image
    // TODO: move all this to the app's MainActivity


    private boolean isDeviceInPortrait=true;

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE)
        {
            Log.d("cuicui", "onConfigurationChanged() -> ORIENTATION_LANDSCAPE");
            isDeviceInPortrait = false;
        } else if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT)
        {
            Log.d("cuicui", "onConfigurationChanged() -> ORIENTATION_PORTRAIT");
            isDeviceInPortrait = true;
        }
        else
        {
            Log.d("cuicui","onConfigurationChanged() : error 5645 -> undefined orientation? ");
            isDeviceInPortrait = true;
        }
    }

    public boolean cui_isDeviceInPortrait()
    {
        return isDeviceInPortrait;
    }

    public void OpenFilePicker(int unused, String filter)
    {
        if (filter.compareTo("images")==0) {
            Intent intent = new Intent(Intent.ACTION_PICK,
                    android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            startActivityForResult(intent, READ_REQUEST_CODE);
        }
        else
        {
            // ACTION_OPEN_DOCUMENT is the intent to choose a file via the system's file
            // browser.
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            // Filter to only show results that can be "opened", such as a
            // file (as opposed to a list of contacts or timezones)
            intent.addCategory(Intent.CATEGORY_OPENABLE);

            // Filter to show only images, using the image MIME data type.
            // If one wanted to search for ogg vorbis files, the type would be "audio/ogg".
            // To search for all documents available via installed storage providers,
            // it would be "*/*".
               intent.setType("*/*");

            startActivityForResult(intent, READ_REQUEST_CODE);
        }
    }

    private String queryName(ContentResolver resolver, Uri uri) {
        try {
            Cursor returnCursor =
                    resolver.query(uri, null, null, null, null);
            if (returnCursor ==null) return ""; // error
            int nameIndex = returnCursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
            returnCursor.moveToFirst();
            String name = returnCursor.getString(nameIndex);
            returnCursor.close();
            return name;
        }
        catch (Exception ex)
        {
            Log.d("cuicui", "queryName exception");
        }
        return ""; // error
    }

    public int orientationInDegrees(Uri selectedImageUri){
        Uri uri = selectedImageUri; // the URI you've received from the other app
        InputStream in = null;
        try {
            in = getContentResolver().openInputStream(uri);
            ExifInterface exifInterface = new ExifInterface(in);
            int rotation = 0;
            int orientation = exifInterface.getAttributeInt(
                    ExifInterface.TAG_ORIENTATION,
                    ExifInterface.ORIENTATION_NORMAL);
            switch (orientation) {
                case ExifInterface.ORIENTATION_ROTATE_90:
                    rotation = 90;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_180:
                    rotation = 180;
                    break;
                case ExifInterface.ORIENTATION_ROTATE_270:
                    rotation = 270;
                    break;
            }
            // Now you can extract any Exif tag you want
            // Assuming the image is a JPEG or supported raw format
            return rotation;
        } catch (IOException e) {
            // Handle any errors
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException ignored) {}
            }
        }
        return -1;
    }

    public byte[] readBytes(InputStream inputStream, int chunkSize) throws IOException {
        // this dynamically extends to take the bytes you read
        ByteArrayOutputStream byteBuffer = new ByteArrayOutputStream();

        // this is storage overwritten on each iteration with bytes
        int bufferSize = chunkSize;
        byte[] buffer = new byte[bufferSize];

        // we need to know how may bytes were read to write them to the byteBuffer
        int len = 0;
        while ((len = inputStream.read(buffer)) != -1) {
            byteBuffer.write(buffer, 0, len);
        }

        // and then we can return your byte array.
        return byteBuffer.toByteArray();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode,
                                 Intent resultData) {

        // The ACTION_OPEN_DOCUMENT intent was sent with the request code
        // READ_REQUEST_CODE. If the request code seen here doesn't match, it's the
        // response to some other intent, and the code below shouldn't run at all.

        boolean receivedSomethingThatWeCanHandle=false; // CLEANUP: can be removed
        Uri receivedSomethingThatWeCanHandle_uri = null;
        if (requestCode == REQUEST_TAKE_PHOTO  && resultCode == Activity.RESULT_OK) // receiving an image from device's camera
        {
            if (mCurrentPhotoUri!=null)
            {
                receivedSomethingThatWeCanHandle=true;
                receivedSomethingThatWeCanHandle_uri = mCurrentPhotoUri;
            }
        }

        if (requestCode == READ_REQUEST_CODE && resultCode == Activity.RESULT_OK ) // receiving something from a share
        {
            if (resultData != null) {
                receivedSomethingThatWeCanHandle = true;
                receivedSomethingThatWeCanHandle_uri = resultData.getData();
            }
        }

        if (receivedSomethingThatWeCanHandle==true && receivedSomethingThatWeCanHandle_uri!=null)
        {
            // The document selected by the user won't be returned in the intent.
            // Instead, a URI to that document will be contained in the return intent
            // provided to this method as a parameter.
            // Pull that URI using resultData.getData().
            Uri uri = null;
            uri = receivedSomethingThatWeCanHandle_uri;

            try { // DUPLICATE_5647

                int orientation = orientationInDegrees(uri);

                InputStream finput = getContentResolver().openInputStream(uri);
                String fileName = queryName(getContentResolver(), uri);
                String mimeType = getContentResolver().getType(uri);

                // old
                //int finputLength = finput.available();
                //byte[] imageBytes = new byte[finputLength];
                //finput.read(imageBytes, 0, finputLength);

                // PERF (medium) android upload: we can probably use old actually (I assume that in this case the stream is fully ready whatever the image size)

                // new
                byte[] imageBytes = readBytes(finput, 500*1024); // will read by chunk of 500kb (most images are bigger so that's fine)

                int length = imageBytes.length;
                finput.close();
                String imageStr =  Base64.encodeToString(imageBytes, Base64.NO_WRAP);
                Log.e("cuicui", "base64: " + imageStr);
                Log.e("cuicui", "base64 size: " + imageStr.length());
                cuiAndroidOnFileInputSelection(fileName, imageStr, mimeType, orientation, "from camera or gallery");
            } catch (Exception ex )
            {
                Log.e("cuicui", "error 65741 : " + ex.toString());
            }

        }
    }

    public void ShowSimpleMessageBoxYesNo(String message)
    { // TODO: not finished
        AlertDialog.Builder builder1 = new AlertDialog.Builder(this);
        //builder1.setTitle("testtitle"); // cCentral::ShowSimpleMessageBox() doesn't use title
        builder1.setMessage(message);
        builder1.setCancelable(false);

        builder1.setPositiveButton(
                "Ok",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        cuicuiShowSimpleMessageBoxYesNo(1); // ok=yes=1=true
                    }
                });

        builder1.setNegativeButton(
                "Cancel",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        cuicuiShowSimpleMessageBoxYesNo(0); // cancel=no=0=false
                    }
                });

        AlertDialog alert11 = builder1.create();
        alert11.show();
    }

    public void ShowSimpleMessageBox(String message)
    {
        AlertDialog.Builder builder1 = new AlertDialog.Builder(this);
        //builder1.setTitle("testtitle"); // cCentral::ShowSimpleMessageBox() doesn't use title
        builder1.setMessage(message);
        builder1.setCancelable(true);

        builder1.setPositiveButton(
                "Ok",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                    }
                });

        AlertDialog alert11 = builder1.create();
        alert11.show();
    }

    public String cFileReceived_ResizeAndJpegize(String base64, int maxWidth, int maxHeight, int jpegCompression, int imageOrientation)
    {
        try
        {
            byte[] decodedString = Base64.decode(base64, Base64.NO_WRAP);
            Bitmap bitmap = BitmapFactory.decodeByteArray(decodedString, 0, decodedString.length);

            float scale = Math.min(((float) maxHeight / bitmap.getWidth()), ((float) maxWidth / bitmap.getHeight()));
            scale = Math.min(scale, 1.0f); // never enlarge: we only shrink (otherwise an image smaller than the bound would be upscaled)

            Matrix matrix = new Matrix();
            matrix.postScale(scale, scale);

            if (imageOrientation!=-1)
            {
                // note: doing this process will remove exif information, so we rotate the image if needed
                matrix.postRotate(imageOrientation);
            }

            bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);

            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
            bitmap.compress(Bitmap.CompressFormat.JPEG, jpegCompression, byteArrayOutputStream);
            byte[] byteArray = byteArrayOutputStream.toByteArray();
            String encoded = Base64.encodeToString(byteArray, Base64.NO_WRAP); // NO_WRAP: no '\n' (consistent with decoding, avoids breaking a possible JSON/URL transport)

            return encoded;
        }
        catch (Exception ex)
        {
            Log.e("cuicui", "cFileReceived_ResizeAndJpegize exception : " + ex.toString());
            return "";
        }
    }


    // upload image
    /////////////////


    // headers = "name1###value###name2###value2";
    public void SendWebRequest(int webRequestId, String mode /*GET/POST/PATCH/...*/, String url, String payload, String headers) {

        try {
            AsyncWebRequest awr = new AsyncWebRequest();
            awr.mode = new String(mode);
            awr.url = new String(url);
            awr.payload = new String(payload);
            awr.webRequestId = webRequestId;

            if (headers.isEmpty()==false) {
                String s[] = headers.split("###");
                for (int i = 0; i < s.length; i += 2) {
                    if (s[i + 0].isEmpty()) break;
                    awr.headers.put(s[i + 0], s[i + 1]);
                }
            }

            awr.execute();
        } catch (Exception ex) {
            cuiWebRequestEvent(webRequestId, "", false, -1);
        }
    }

    private class AsyncWebRequest extends AsyncTask<Void, Void, String> {

        public String mode, url, payload;
        public int webRequestId;
        public Map<String,String> headers = new HashMap<String,String>();


        @Override
        protected void onPostExecute(String s) {
            super.onPostExecute(s);

            String[] sss = s.split("###", 2);
            int httpStatusCode = Integer.parseInt(sss[0]);

            if (sss[0].compareTo("-1")==0)
                cuiWebRequestEvent(webRequestId, "", false, -1); // network error or exception
            else
                cuiWebRequestEvent(webRequestId, sss[1], true, httpStatusCode);

            // Success() ?
        }

        @Override
        protected String doInBackground(Void... params) {

            // this code is synchronous, that's why we put it in an AsyncTask
            {
                try {
                    URL reqURL = new URL(url);
                    HttpURLConnection request = (HttpURLConnection)(reqURL.openConnection());

                    request.setDoOutput(true);
                    request.addRequestProperty("Content-Length", Integer.toString(payload.getBytes().length)); //add the content length of the post data
                    request.addRequestProperty("Content-Type", "text/plain;charset=UTF-8"); // "application/json; charset=UTF-8"); //add the content type of the request, most post data is of this type
                    for (Map.Entry<String,String> entry : headers.entrySet())
                    {
                        request.addRequestProperty(entry.getKey(), entry.getValue());
                    }
                    request.setRequestMethod(mode);
                    request.setFixedLengthStreamingMode(payload.getBytes().length);

                    request.connect();

                    OutputStreamWriter writer = new OutputStreamWriter(request.getOutputStream(), StandardCharsets.UTF_8); //we will write our request data here
                    writer.write(payload);
                    writer.flush();

                    BufferedReader br = new BufferedReader(new InputStreamReader( request.getInputStream(),StandardCharsets.UTF_8));
                    String line = null;
                    StringBuilder response = new StringBuilder("");
                    while ((line = br.readLine()) != null) {
                        response.append(line + "\n");
                    }
                    br.close();

                    return Integer.toString(request.getResponseCode()) + "###" +  response.toString();
                }
                catch (Exception ex)
                {
                    Log.d("cuicui", "AsyncWebRequest::doInBackground exception");
                    return "-1###";
                }
            }

        }
    }



    protected void ManageSendIntent(Intent intent, String action, String type)
    {
        if (Intent.ACTION_SEND.equals(action) && type != null) {
            Log.d("cuicui", "share received, type : " + type);

            if (type.startsWith("text"))
            {
                Log.d("cuicui", "share received: text");
                String sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);
                if (sharedText != null) {
                    Log.d("cuicui", "share received: text = " + sharedText);
                    cuiAndroidOnFileInputSelection("", sharedText, type, -1, "from_share_intent");
                }
                else
                    Log.d("cuicui", "share received: text is NULL");
            }
            if (type.startsWith("image"))
            {

                Uri uri = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
                Log.d("cuicui", "share received: image, uri : " + uri.toString());
                if (uri != null) {
                    // Update UI to reflect image being shared
                    try { // DUPLICATE_5647
                        int orientation = orientationInDegrees(uri);

                        InputStream finput = getContentResolver().openInputStream(uri);
                        String fileName = queryName(getContentResolver(), uri);
                        String mimeType = getContentResolver().getType(uri);
                        int length = finput.available();
                        byte[] imageBytes = new byte[length];
                        finput.read(imageBytes, 0, imageBytes.length);
                        finput.close();
                        String imageStr = Base64.encodeToString(imageBytes, Base64.NO_WRAP);
                        Log.d("cuicui", "filereceived : " + fileName + " / mimetype : " + mimeType);
                        cuiAndroidOnFileInputSelection(fileName, imageStr, mimeType, orientation, "from_share_intent");
                    } catch (Exception ex) {
                        Log.e("cuicui", "error 65741 : " + ex.toString());
                    }
                }
            }
        }
    }



    @Override
    protected void onNewIntent(Intent intent) {

        super.onNewIntent(intent);

        Bundle extras = intent.getExtras();

        String action = intent.getAction();
        String type = intent.getType();
        if (Intent.ACTION_SEND.equals(action) && type != null)
        {
            ManageSendIntent(intent, action, type);
        }
    }

    public void hideSoftKeyboard()
    {
        try {
            InputMethodManager inputMethodManager = (InputMethodManager) this.getSystemService(Activity.INPUT_METHOD_SERVICE);
            inputMethodManager.hideSoftInputFromWindow(this.getCurrentFocus().getWindowToken(), 0);
        }
        catch (Exception ex)
        {
            Log.d("cuicui", "hideSoftKeyboard() exception : " + ex.toString());
        }
    }




    @OnLifecycleEvent(Lifecycle.Event.ON_CREATE)
    public void created() {
        Log.d("cuicui", "LifecycleObserver: ON_CREATE");
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_START)
    public void started() {
        Log.d("cuicui", "LifecycleObserver: ON_START");
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_RESUME)
    public void resumed() {
        Log.d("cuicui", "LifecycleObserver: ON_RESUME");
        cuicuiOnAppStateChanged(0);
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_PAUSE)
    public void paused() {
        Log.d("cuicui", "LifecycleObserver: ON_PAUSE");
    }

    @OnLifecycleEvent(Lifecycle.Event.ON_STOP) // note: this is not "destroy": ON_STOP is more like "app not visible to the user"
    public void stopped() {
        Log.d("cuicui", "LifecycleObserver: ON_STOP");
        cuicuiOnAppStateChanged(1);
    }

    private int insetB, insetT, insetR, insetL;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        Log.d("cuicui", "CuicuiActivity::onCreate()");

        LocalStorage.EnsureReady(this);

        // we do that here because this code must run on UI Thread, and we don't want to force user to so "RunOnUiThread" each time he needs this
        try {
            directoryTemp = getApplicationContext().getCacheDir().toString() + "/";
            directoryAssets = directoryTemp + "cuicui_assets_copied_here/";
            // persistent "documents" folder (equivalent of iOS NSDocumentDirectory): app's private internal storage
            directoryDocuments = getApplicationContext().getFilesDir().toString() + "/";

            // note: the assets are in compressed apk, we copy them to a regular place where fopen and other will work
            // (replaces the old android-copy-assets lib with a vanilla recursive copy via AssetManager)
            // only when the app was installed / updated (or when the system cleared the cache)
            String assetsVersion = String.valueOf(getPackageManager().getPackageInfo(getPackageName(), 0).lastUpdateTime);
            File assetsMarker = new File(directoryAssets + ".cuicui_assets_version");
            if (!assetsVersion.equals(cuicuiReadSmallFile(assetsMarker))) {
                cuicuiCopyAssetDir("", directoryAssets);
                FileOutputStream fos = new FileOutputStream(assetsMarker);
                fos.write(assetsVersion.getBytes(StandardCharsets.UTF_8));
                fos.close();
                Log.d("cuicui", "assets copied to " + directoryAssets);
            }



        } catch(Exception ex)
        {
            Log.e("cuicui", "FATAL: could not prepare the app directories / assets", ex);
        }


        Intent intent = getIntent();
        String action = intent.getAction();
        String type = intent.getType();
        if (Intent.ACTION_SEND.equals(action) && type != null)
        {
            ManageSendIntent(intent, action, type);
        }

        super.onCreate(savedInstanceState);
        // root view of the cuicui UI, created in code so that cuicui doesn't depend on the app's resources (R)
        masterFrameLayout = new FrameLayout(this);
        masterFrameLayout.setId(cuiNewViewId());
        masterFrameLayout.setFocusableInTouchMode(true); // prevents the keyboard from popping up when the app starts
        masterFrameLayout.setBackgroundColor(Color.BLACK);
        setContentView(masterFrameLayout, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        // explicit edge-to-edge: the app draws behind the system bars on all versions
        // (on Android 15 / API 35 it's enforced anyway). cuicui handles safe areas itself
        // via wSafeZone + central.safeAreaRect ; so there must NOT be android:fitsSystemWindows
        // (otherwise insets are counted twice and content is pushed off screen).
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);

        ProcessLifecycleOwner.get().getLifecycle().addObserver(this);

        // the activity will be "fullscreen" but we still have the system top and bottom bar (cuicui is always like that, user use central.safeAreaRect to be fine)
        // WARNING: this break the "resize" event when keyboard open... so i desactivated this (note_78552)
        // TODO: android sdk 11 (30?) brings a "keyboard opening" event I think: check if a version for android <11 is available or not; that would fix all this once and for all
        /*if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {

            getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                    WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
        }*/

        // TODO: rename to "androidRoot..." to avoid confusion with "master" (which is the real master of myapp)

        cuiMain(masterFrameLayout.getId());

        // Modern insets reading via androidx (WindowInsetsCompat): backward compatible down to minSdk 24
        // and robust on Android 15+ (forced edge-to-edge). Replaces the deprecated getStableInset*().
        // - safe-area    = system bars + notch  (status bar / nav bar / display cutout)
        // - keyboard     = Type.ime()
        // - final bottom = max(bars, keyboard) so that the input bar moves up above the keyboard.
        // The relayout when an inset changes (without window resize) is triggered on the C++ side in
        // cuiSetCentralSafeAreaRect (cf. note_safearea_relayout).
        View root = findViewById(android.R.id.content).getRootView();
        ViewCompat.setOnApplyWindowInsetsListener(root, new OnApplyWindowInsetsListener() {
            @Override
            public WindowInsetsCompat onApplyWindowInsets(View v, WindowInsetsCompat insets) {

                androidx.core.graphics.Insets bars = insets.getInsets(
                        WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.displayCutout());
                androidx.core.graphics.Insets ime = insets.getInsets(WindowInsetsCompat.Type.ime());

                insetL = bars.left;
                insetT = bars.top;
                insetR = bars.right;
                insetB = Math.max(bars.bottom, ime.bottom); // keyboard open -> move up above the keyboard

                cuiSetCentralSafeAreaRect(insetL, insetT, insetR, insetB);

                return insets;
            }
        });


        // TODO android P5: this is called as soon as something changes in the ui, it "does nothing" since if the size
        // hasn't changed we do nothing, but it's still not great
        masterFrameLayout.addOnLayoutChangeListener(new View.OnLayoutChangeListener()
        {
            @Override
            public void onLayoutChange(View view, int i, int i1, int i2, int i3, int i4, int i5, int i6, int i7) {

                if (lastMasterFrameWidth!=i2 || lastMasterFrameHeight!=i3)
                {
                    Log.d("CUICUI_ANDROID", "master layout size changed, calling the cuiAppSizeChanged() event as size is not the same");
                    Log.d("CUICUI_ANDROID", "insets l,r = " + insetL + "," + insetR);
                    Log.d("CUICUI_ANDROID", "insets t,b = " + insetT + "," + insetB);
                    cuiAppSizeChanged(i2, i3);
                    lastMasterFrameWidth = i2;
                    lastMasterFrameHeight = i3;
                }
            }
        });


        if (false==true) // deprecated
        masterFrameLayout.setOnTouchListener(new View.OnTouchListener() {

            @Override
            public boolean onTouch(View view, MotionEvent motionEvent)
            {

                final int action = motionEvent.getAction();


                if(action == MotionEvent.ACTION_MOVE) {
                    Log.d("cuicui","onTouch on master: ACTION_MOVE, pos:" + motionEvent.getX() + "," + motionEvent.getY());
                    cuiTouchEvent(0 /*deprecated*/,5, motionEvent.getX(), motionEvent.getY(), -1, -1);  // pointer moved
                    return true; // swallow the event
                }
                if(action == MotionEvent.ACTION_DOWN) {
                    Log.d("cuicui","onTouch on master: ACTION_DOWN, pos:" + motionEvent.getX() + "," + motionEvent.getY());
                    cuiTouchEvent(0 /*deprecated*/,3, motionEvent.getX(), motionEvent.getY(), -1, -1);  // pointer moved
                    return true; // swallow the event
                }
                if(action == MotionEvent.ACTION_UP) {
                    Log.d("cuicui","onTouch on master: ACTION_UP, pos:" + motionEvent.getX() + "," + motionEvent.getY());
                    cuiTouchEvent(0 /*deprecated*/,4, motionEvent.getX(), motionEvent.getY(), -1, -1);  // pointer moved
                    return true; // swallow the event
                }

                return false;
            }
        });



        // note: we should do that : android:windowSoftInputMode="adjustResize|stateAlwaysHidden"
        // but it doesn't work
        // doing the same thing in code here works:

        // resize:
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);

        // pan: (doesn't work)
        //getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_PAN | WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);

        //

        // back button / back gesture (predictive back on API 33+, see HandleBackPressed)
        RegisterOnBackInvokedCallback();
    }

    // ============================================================================
    // "Back" button / gesture
    //
    // WARNING: since we target API 36 (Android 16), "predictive back" is enabled by
    // default: the system NO LONGER calls onBackPressed() and NO LONGER dispatches KEYCODE_BACK.
    // Without a registered callback, the system handled back by itself and closed the activity
    // (the app seemed to crash / minimize instead of closing the current cuicui view,
    // e.g. the fullscreen image).
    // -> API 33+ : we register an OnBackInvokedCallback.
    // -> API < 33 : onBackPressed() (still called).
    // Both go through HandleBackPressed().
    // ============================================================================

    private Object backInvokedCallback; // Object: android.window.OnBackInvokedCallback only exists in API 33+

    private void HandleBackPressed() {
        int cuiBackButtonManagerPushedElement = cuiGetBackButtonManagerPushedElementsCount();
        if (cuiBackButtonManagerPushedElement==0)
            moveTaskToBack(true); // this "reduces" the android app (without destroying it)
        else
            cuiBackButtonEvent();
    }

    // the callback is permanently registered (default priority): cuicui decides
    // what to do, including the "moveTaskToBack" when nothing is stacked anymore.
    private void RegisterOnBackInvokedCallback() {
        if (Build.VERSION.SDK_INT < 33 /*Build.VERSION_CODES.TIRAMISU*/) return;
        if (backInvokedCallback != null) return;
        OnBackInvokedCallback cb = new OnBackInvokedCallback() {
            @Override
            public void onBackInvoked() {
                HandleBackPressed();
            }
        };
        backInvokedCallback = cb;
        getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT, cb);
    }

    @Override
    public void onBackPressed() {
        // only called on API < 33 (or if predictive back is disabled)
        HandleBackPressed();
    }

    public void OpenExternalApp(String uri) {
        Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(uri));
        startActivity(browserIntent);
    }

    public void cui_Exit()
    {
        finish();
    }

    public int cuiElement_SetPos_HW(int viewId, int x, int y, int width, int height)
    {
        try {
            CustomFrameLayout fl = createdElements.get(viewId);

            CustomFrameLayout.LayoutParams lp = new CustomFrameLayout.LayoutParams(width, height);
            fl.setLayoutParams(lp);

            fl.setX(x);
            fl.setY(y);

        } catch (Exception ex)
        {
            Log.e("cuicui", "Exception 6514 in cuiElement_SetPos_HW !!! -> " + ex);
        }

        return -1; // TODO: no need for return value
    }




    class CustomTouchListener implements View.OnTouchListener
    {
        public CustomFrameLayout owner;

        // dragging stuff
        private boolean dragging=false;
        private float mLastTouchX, mLastTouchY;
        // port to new cuicui API: gestures now send a cumulative offset (since the start of the gesture) + a phase (0=Start,1=Run,2=End)
        private float mPanStartX, mPanStartY;   // position at the start of the drag (offsetX/Y = current pos - start)
        private boolean panOneStarted=false;     // has the 1-finger pan already emitted its Start event?
        private boolean panTwoStarted=false;     // same for 2 fingers
        float mScaleAccum=1.0f;                   // cumulative absolute scale of the current pinch
        /////


        boolean singleTapConfirmedEnabled=false;
        boolean singleTapEnabled=false;
        boolean doubleTapEnabled=false;
        boolean panOneFingerEnabled=false;
        boolean panTwoFingersEnabled=false;
        boolean scaleEnabled=false; // pinch
        boolean longPressEnabled=false;

        // scale

        private class MyScaleListener
                extends ScaleGestureDetector.SimpleOnScaleGestureListener
        {
            private CustomTouchListener customTouchListener;

            public void SetOwner(CustomTouchListener c)
            {
                customTouchListener = c;
            }


            @Override
            public boolean onScale(ScaleGestureDetector detector) {

                // we don't call super?

                if (customTouchListener.scaleEnabled==false) return false;

                float s = detector.getScaleFactor();
                // new cuicui API: we send the cumulative absolute scale (1.0 at the start) + phase 1=Run
                customTouchListener.mScaleAccum *= s;
                cuiGestureScaleEvent(owner.widgetPtr, customTouchListener.mScaleAccum, detector.getFocusX(), detector.getFocusY(), 1);
                return true;
            }

            @Override
            public boolean onScaleBegin(ScaleGestureDetector detector) {
                boolean b = super.onScaleBegin(detector);
                if (customTouchListener.scaleEnabled==false) return false;
                customTouchListener.scalingInProgress=true;
                customTouchListener.mScaleAccum = 1.0f;
                cuiGestureScaleEvent(owner.widgetPtr, customTouchListener.mScaleAccum, detector.getFocusX(), detector.getFocusY(), 0); // 0=Start
                return b;
            }

            @Override
            public void onScaleEnd(ScaleGestureDetector detector) {
                super.onScaleEnd(detector);
                if (customTouchListener.scaleEnabled==false) return;
                customTouchListener.scalingInProgress=false;
                cuiGestureScaleEvent(owner.widgetPtr, customTouchListener.mScaleAccum, detector.getFocusX(), detector.getFocusY(), 2); // 2=End
            }
        }
        ScaleGestureDetector mScaleDetector = null;

        private class MyGestureListener
                extends GestureDetector.SimpleOnGestureListener 
        {
            public CustomTouchListener customTouchListener;

            public void SetOwner(CustomTouchListener c)
            {
                customTouchListener = c;
            }

            @Override
            public boolean onDoubleTap(MotionEvent e) {
                if (doubleTapEnabled==false) {super.onDoubleTap(e); return false;}
                cuiGestureDoubleTapEvent(owner.widgetPtr, e.getX(), e.getY());
                return true;
            }

            // note: this one is fast as it doesn't wait to be sure it will not be a double tap
            @Override
            public boolean onSingleTapUp(MotionEvent e) {
                if (singleTapEnabled==false) {super.onSingleTapUp(e); return false;}
                cuiGestureSingleTapEvent(owner.widgetPtr, e.getX(), e.getY());
                return true;
            }

            @Override
            public void onLongPress(MotionEvent e) {
                if (longPressEnabled==false) {super.onLongPress(e); return;}
                cuiGestureLongPressEvent(owner.widgetPtr, e.getX(), e.getY());
                return;
            }

            // note: this one is slow as it wait to be sure it will not be a double tap
            @Override
            public boolean onSingleTapConfirmed(MotionEvent e) {
                if (singleTapConfirmedEnabled==false) {super.onSingleTapConfirmed(e); return false;}
                cuiGestureSingleTapEvent(owner.widgetPtr, e.getX(), e.getY());
                return true;
            }
        }
        GestureDetector mGestureDetector = null;

        CustomTouchListener(CuicuiActivity activity, int vid)
        {
            super();

            // TODO: do it only if Touchable(withGestures)
            MyScaleListener sl = new MyScaleListener();
            sl.SetOwner(this);
            mScaleDetector = new ScaleGestureDetector(activity, sl);

            // TODO: do it only if Touchable(withGestures)
            MyGestureListener gl = new MyGestureListener();
            gl.SetOwner(this);
            mGestureDetector = new GestureDetector(activity, gl);
        }

        public boolean scalingInProgress=false;

        /*@Override
        public boolean onTouch(View v, MotionEvent event) {
            Log.d("cuicui", "WEIRD_56564"); // should never pass here now (i think)
            return false;
        }*/


        @Override
        public boolean onTouch(View v, MotionEvent event)
        {
            if (owner.opaqueToTouch)
            {
                return true; // swallow
            }

            final int action = event.getAction();

            if (mScaleDetector!=null)
            {
                mScaleDetector.onTouchEvent(event);

                if (scalingInProgress)
                {
                    dragging=false;
                    return true; // swallow the event
                }
            }
            if (mGestureDetector!=null)
            {
                mGestureDetector.onTouchEvent(event);
            }


            if(action == MotionEvent.ACTION_MOVE){ // there's also ACTION_HOVER_MOVE, useful?

                float x = event.getX();
                float y = event.getY();


                // TODO: is there not a real "pan/drag" gesture in android? that would be simpler

                if (dragging) {
                    // cumulative offset since the start of the drag (new cuicui API)
                    float offsetX = x - mPanStartX;
                    float offsetY = y - mPanStartY;

                    int pointerCount = event.getPointerCount();
                    if (pointerCount==1)
                        if (this.panOneFingerEnabled) {
                            int phase = panOneStarted ? 1 : 0; // 0=Start, 1=Run
                            panOneStarted = true;
                            cuiGesturePanEvent(owner.widgetPtr, x, y, offsetX, offsetY, phase);  // dragging gesture
                        }
                    if (pointerCount==2)
                        if (this.panTwoFingersEnabled)
                        {
                            int phase = panTwoStarted ? 1 : 0;
                            panTwoStarted = true;
                            cuiGesture2FingersPanEvent(owner.widgetPtr, x, y, offsetX, offsetY, phase);  // dragging 2 fingersgesture
                        }
                    mLastTouchX = x;
                    mLastTouchY = y;
                }

                cuiTouchEvent(owner.widgetPtr,5, x, y, -1, -1);  // pointer moved
                return true; // swallow the event
            }

            if(action == MotionEvent.ACTION_UP){
                if (dragging) {
                    // emit the end-of-pan event (phase 2=End) for the new cuicui API
                    float offsetX = event.getX() - mPanStartX;
                    float offsetY = event.getY() - mPanStartY;
                    if (panOneStarted && this.panOneFingerEnabled)
                        cuiGesturePanEvent(owner.widgetPtr, event.getX(), event.getY(), offsetX, offsetY, 2);
                    if (panTwoStarted && this.panTwoFingersEnabled)
                        cuiGesture2FingersPanEvent(owner.widgetPtr, event.getX(), event.getY(), offsetX, offsetY, 2);
                }
                dragging = false;
                panOneStarted = false;
                panTwoStarted = false;
                //v.performClick(); // I think it's needed for some reason I don't know (careful to call it only if there really is a click? hmm...?)

                // shouldn't we skip all this if it's not a rich text containing touchspans? (PERF_P1_854 android)
                try {
                    CustomFrameLayout fl = (CustomFrameLayout)v;
                    TextView tv = (TextView) fl.getChildAt(0);
                    if (tv instanceof TextView) {
                        Layout layout = ((TextView) tv).getLayout();
                        int x = (int) event.getX();
                        int y = (int) event.getY();
                        if (layout != null) {
                            int line = layout.getLineForVertical(y);
                            int offset = layout.getOffsetForHorizontal(line, x);
                            Log.v("cuicui", " character touched: " + offset);

                            int ret = cuiHandleTouchSpanReleased(owner.widgetPtr, offset);
                            if (ret == 1) {
                                return true; // swallow the event
                            }
                            // we don't swallow event and continue running this method, so the events below are sent
                        }
                    }
                } catch (Exception ex)
                {
                    // this is not a wText probably (or other weird issue)
                }
                cuiTouchEvent(owner.widgetPtr,4, event.getX(), event.getY(),-1,-1); // pointer up
                cuiTouchEvent(owner.widgetPtr,2, event.getX(), event.getY(),-1,-1); // pointer exited (useful with wButton in touch mode for example)(I did it on ios too)(maybe wButton should check IsTouchDevice() and handle it itself? QUESTION)
                return true; // swallow the event
            }
            if(action == MotionEvent.ACTION_DOWN){
                mLastTouchX = event.getX();
                mLastTouchY = event.getY();
                mPanStartX = event.getX();
                mPanStartY = event.getY();
                panOneStarted = false;
                panTwoStarted = false;
                dragging = true;

                cuiTouchEvent(owner.widgetPtr,1, event.getX(), event.getY(),-1,-1); // pointer entered (useful with wButton in touch mode for example)(I did it on ios too)(maybe wButton should check IsTouchDevice() and handle it itself? QUESTION)
                cuiTouchEvent(owner.widgetPtr,3, event.getX(), event.getY(),-1,-1); // pointer down
                return true; // swallow the event
            }
            return true; // swallow the event
        }
    }

    public void cuicuiEnableGesture(int viewId, String gestureName)
    {
        CustomFrameLayout fl = createdElements.get(viewId);
        ActivateTouchListenerIfNotAlreadyActivated(fl,viewId); // will activate only if not already done

        // PERF: change strings to int/enum

        if (gestureName.compareTo("pinch")==0) fl.touchListener.scaleEnabled = true;
        if (gestureName.compareTo("longPress")==0) fl.touchListener.longPressEnabled = true;
        if (gestureName.compareTo("panOneFinger")==0) fl.touchListener.panOneFingerEnabled = true;
        if (gestureName.compareTo("panTwoFingers")==0) fl.touchListener.panTwoFingersEnabled = true;
        if (gestureName.compareTo("singleTap")==0) fl.touchListener.singleTapEnabled = true;
        if (gestureName.compareTo("singleTapConfirmed")==0) fl.touchListener.singleTapConfirmedEnabled = true;
        if (gestureName.compareTo("doubleTap")==0) fl.touchListener.doubleTapEnabled = true;
    }

    public void cuiElement_SetOpaqueToTouch(int viewId)
    {
        CustomFrameLayout fl = createdElements.get(viewId);
        ActivateTouchListenerIfNotAlreadyActivated(fl,viewId); // TODO: is there not a better way to do that?
        fl.opaqueToTouch=true;
    }

    public int cuiElement_Touchable(int viewId)
    {
        CustomFrameLayout fl = createdElements.get(viewId);
        ActivateTouchListenerIfNotAlreadyActivated(fl,viewId);
        //fl.touchable=true; // this thing is useful (I think) because currently we put ActivateTouchListener() on ALL views
        return -1;
    }

    public void cuiElement_Opacity(int viewId, double opacity) {
        CustomFrameLayout fl = createdElements.get(viewId);
        if (fl==null)
        {
            Log.e("cuicui", "cuiElement_Opacity : fl is null");
            return;
        }
        fl.setAlpha((float)opacity);
    }

    public void wGradient_Apply(int viewId, String colors)
    {
        CustomFrameLayout fl = createdElements.get(viewId);

        String[] _arrayOfColors = colors.split("#");
        if (_arrayOfColors.length==0) return;
        String[] arrayOfColors = Arrays.copyOfRange(_arrayOfColors, 1, _arrayOfColors.length); // removing first always empty

        int[] gdColors = new int[arrayOfColors.length];
        for (int i=0; i<arrayOfColors.length; i++)
        {
            gdColors[i] = Color.parseColor("#"+ arrayOfColors[i]);
        }

        GradientDrawable gd = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, gdColors);

        fl.setBackground(gd);
    }

    public int cuiElement_Border(int viewId, int thicknessInPixels, int rgb, int cornerRadiusInPixels, int corners/*bitfield*/)
    {
        CustomFrameLayout fl = createdElements.get(viewId);

        int currentBgColor = 0x00000000;

        GradientDrawable shape = null;
        Drawable d = fl.getBackground();
        if (d instanceof ColorDrawable) // set by "BgColor()"
        {
            currentBgColor = ((ColorDrawable)d).getColor();
            shape = new GradientDrawable();
            shape.setColor(currentBgColor);
        }
        if (d instanceof GradientDrawable) // set by "Border"
        {
            shape = (GradientDrawable)d;
            // note: bgcolor already set
        }

        if (shape==null) // set by nobody
        {
            shape = new GradientDrawable();
        }

        // https://stackoverflow.com/questions/44071755/android-how-to-set-corner-radius-programmatically

        shape.setShape(GradientDrawable.RECTANGLE);
        shape.setStroke(thicknessInPixels, 0xff000000 + rgb);
        if (cornerRadiusInPixels>0) {
            shape.setCornerRadii(new float[]{
                    (corners & 1) != 0 ? cornerRadiusInPixels : 0, (corners & 1) != 0 ? cornerRadiusInPixels : 0,    // top left
                    (corners & 2) != 0 ? cornerRadiusInPixels : 0, (corners & 2) != 0 ? cornerRadiusInPixels : 0,    // top right
                    (corners & 4) != 0 ? cornerRadiusInPixels : 0, (corners & 4) != 0 ? cornerRadiusInPixels : 0,    // bottom right
                    (corners & 8) != 0 ? cornerRadiusInPixels : 0, (corners & 8) != 0 ? cornerRadiusInPixels : 0,}); // bottom left
        }



        fl.setBackground(shape);

        fl.setClipToOutline(true); // this will clip content using the background shape as "mask"

        return -1; // useless
    }

    public void cuiCentralSimpleToast(String message, boolean shortDuration)
    {
        Toast.makeText(this, message, shortDuration?Toast.LENGTH_SHORT:Toast.LENGTH_LONG).show();
    }

    public void cuiCentralSendTextToClipboard(String text)
    {
        ClipboardManager clipboard = (ClipboardManager)getSystemService(Context.CLIPBOARD_SERVICE);
        ClipData clip = ClipData.newPlainText("", text);
        clipboard.setPrimaryClip(clip);
    }

    // WARNING: returns "" in this case: If the application is not the default IME or does not have input focus
    // It means if you don't call that from an user input action it won't work
    public String cuiCentralGetTextFromClipboard(int uselessParam)
    {
        try {
            ClipboardManager clipboard = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
            return clipboard.getText().toString();
        } catch(Exception ex)
        {
            return ""; // error
        }
    }

    public void cui_UnsetBgColor(int viewId)
    {
        CustomFrameLayout fl = createdElements.get(viewId);

        Drawable d = fl.getBackground();
        if( d instanceof GradientDrawable) // this widget had a call to Border() previously
        {
            GradientDrawable gd = (GradientDrawable)d;
            gd.setColor(null);
            fl.setBackground(gd);
        }
        else
            fl.setBackground(null);
    }

    public int cuiElement_BackgroundColor(int viewId, int argb, int isTouchable)
    {
        CustomFrameLayout fl = createdElements.get(viewId);
        if (fl==null)
        {
            Log.e("cuicui", "cuiElement_BackgroundColor : fl is null");
            return -1;
        }
        Drawable d = fl.getBackground();
        if( d instanceof GradientDrawable) // this widget had a call to Border() previously
        {
            GradientDrawable gd = (GradientDrawable)d;
            gd.setColor(argb);
            fl.setBackground(gd);
        }
        else
            fl.setBackgroundColor(argb);

        ///// DEPRECATED /// see "important_7521" for information about this
        /*{

            boolean isFullyTransparent = (argb & 0xff000000)==0;
            if (!isFullyTransparent) // we want a non fully transparent view to always swallow the event (so it doesn't go to a view possibly located below)
            if (isTouchable!=1) // unless it's a touchable
            fl.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    Log.d("cuicui","onTouch on : " + v.getId());
                    ((View)v.getParent()).onTouchEvent(event);
                    return true; // swallow the touch event //NO ! the event must bubble to the parent! an opaque must bubble, but not swallow
                }
            });

            // note: views have fully transparent background by default, so doing this only here is ok
        }*/

        return -1; // useless
    }

    public void cui_ChangeVisibility(int viewId, int visibility)
    {
        CustomFrameLayout fl = createdElements.get(viewId);
        if (fl==null)
        {
            Log.e("cuicui", "cui_ChangeVisibility : fl is null");
            return;
        }

        if (visibility==0) {
            fl.setVisibility(FrameLayout.GONE);
        }
        else {
            fl.setVisibility(FrameLayout.VISIBLE);
        }
    }



    public int cuiElement_AddToParentView(int parentViewId, int viewId)
    {
        FrameLayout flParent;
        if (parentViewId==-1000)
            flParent = masterFrameLayout;
        else {
            flParent = createdElements.get(parentViewId);
            /*if (flParent==null)
            {
                // special case used only by wListView ? (CLEANUP...)
                ViewGroup parent = (ViewGroup)findViewById(parentViewId);
                View child = (View)findViewById(viewId);
                parent.addView(child);
                return -1;
            }*/
        }



        //FrameLayout fl = (FrameLayout)findViewById(parentViewId); // TODO: if it's a text it won't be a FrameLayout?
        CustomFrameLayout added = createdElements.get(viewId);

        // CRASH_1781: the view (parent or child) may have been destroyed by cuiElement_Delete before
        // cuicui asks for the attachment (e.g. late HTTP response refreshing an unmounted screen;
        // on the C++ side the wWidget survives ~10ms longer because of the DeleteMe() garbage collector).
        // Without this safeguard, addView() throws an NPE / IllegalArgumentException INSIDE a JNI call, which
        // ART turns into a SIGABRT (hard crash) instead of an exception. The web backend already does the same
        // (WEB_CuiAttachChildInNativeDom: "parent not found" / "child not found" -> log + return), just
        // like the neighboring methods here (cuiImage_SetTint, etc.). We ignore it, like they do.
        if (flParent == null) {
            Log.d("cuicui", "cuiElement_AddToParentView: parent not found (view removed) : " + parentViewId);
            return -1;
        }
        if (added == null) {
            Log.d("cuicui", "cuiElement_AddToParentView: child not found (view removed) : " + viewId);
            return -1;
        }

        flParent.addView(added);
        return -1; // useless
    }

    public void cuiImage_SetTint(int parentViewId, int color)
    {
        CustomFrameLayout fl = createdElements.get(parentViewId);
        if (fl==null) return; // view removed (transition) -> ignore
        ImageView imView = (ImageView)fl.getChildAt(0);
        imView.setColorFilter(0xff000000 | color);
    }

    public void cuiImage_SetRotationScaleTranslation(int parentViewId, float rotationDegrees, float scale, float translationX, float translationY)
    {
        CustomFrameLayout fl = createdElements.get(parentViewId);
        if (fl==null) return; // the view may have been removed (e.g. during a screen transition) -> ignore
        ImageView imView = (ImageView)fl.getChildAt(0);

        if (imView.getDrawable()==null)
             return; // image is not loaded yet


        // android PERF: do it only once
        int width = imView.getDrawable().getIntrinsicWidth();
        int height = imView.getDrawable().getIntrinsicHeight();

        // apply
        Matrix matrix = new Matrix();
        imView.setScaleType(ImageView.ScaleType.MATRIX);   //required
        matrix.postRotate(rotationDegrees, width/2, height/2);
        matrix.postScale(scale, scale, width/2, height/2);
        matrix.postTranslate(translationX,translationY);
        imView.setImageMatrix(matrix);
    }

    public void cuiImage_SetRotation_deprecated(int parentViewId, int degrees)
    {
        CustomFrameLayout fl = createdElements.get(parentViewId);
        ImageView imView = (ImageView)fl.getChildAt(0);

        imView.setRotation(degrees); // TODO: the image is distorted (probably because of "CONTAIN" ?)
    }

    public void cuiImage_SetSourceFromBase64(int parentViewId, String base64)
    {
        // we could do without it (PERF small)
        CustomFrameLayout fl = createdElements.get(parentViewId);
        ImageView imView = (ImageView)fl.getChildAt(0);

        try {
            byte[] decodedString = Base64.decode(base64, Base64.NO_WRAP);

            final CustomFrameLayout f_fl = fl;
            final ImageView f_imView = imView;
            Glide.with(this)
                    .load(decodedString)
                    .downsample(DownsampleStrategy.CENTER_INSIDE) // very important or glide crash with some images ( => an image of 488x36 was decoded by Glide as 33564x2476)
                    .listener(new RequestListener<Drawable>()
                    {
                        // DUPLICATE_5771
                        @Override
                        public boolean onLoadFailed(@Nullable GlideException e, Object model, Target<Drawable> target, boolean isFirstResource)
                        {
                            // log exception
                            Log.e("cuicui", "cuiImage_SetSource: Error loading image", e);
                            wImageOnLoaded(f_fl.widgetPtr, 0,-1,-1);
                            return true; // important to return false so the error placeholder can be placed (we don't use the placeholder so we return true)
                        }
                        @Override
                        public boolean onResourceReady(Drawable resource, Object model, Target<Drawable> target, DataSource dataSource, boolean isFirstResource)
                        {
                            int w = resource.getIntrinsicWidth();
                            int h = resource.getIntrinsicHeight();

                            if (resource instanceof GifDrawable) {
                                GifDrawable gd = (GifDrawable)resource;
                                f_imView.setImageDrawable(gd);
                                gd.setLoopCount(5);
                                gd.start();
                            }
                            else
                                f_imView.setImageDrawable((BitmapDrawable)resource); // I set the image myself here because if I let glide do it, it's not yet ready when wImageOnLoaded is called


                            wImageOnLoaded(f_fl.widgetPtr, 1,w,h);
                            return true; // tell Glide to not set the image
                        }
                    })
                    .into(imView);

        } catch (Exception ex)
        {
            Log.e("cuicui", "cuiImage_SetSourceFromBase64 exception !");
        }
    }

    public int cuiImage_SetSource(int parentViewId, String source)
    {
        CustomFrameLayout fl = createdElements.get(parentViewId);
        ImageView imView = (ImageView)fl.getChildAt(0);

        if (source.indexOf("http")!=0)
            source = "file:///android_asset/" + source;

        final CustomFrameLayout f_fl = fl;
        final ImageView f_imView = imView;

        Glide.with(this)
                .load(source)
                .downsample(DownsampleStrategy.CENTER_INSIDE) // very important or glide crash with some images ( => an image of 488x36 was decoded by Glide as 33564x2476)
                .listener(new RequestListener<Drawable>()
                {
                    // DUPLICATE_5771
                    @Override
                    public boolean onLoadFailed(@Nullable GlideException e, Object model, Target<Drawable> target, boolean isFirstResource)
                    {
                        // log exception
                        Log.e("cuicui", "cuiImage_SetSource: Error loading image", e);
                        wImageOnLoaded(f_fl.widgetPtr, 0,-1,-1);
                        return true; // important to return false so the error placeholder can be placed (we don't use the placeholder so we return true)
                    }
                    @Override
                    public boolean onResourceReady(Drawable resource, Object model, Target<Drawable> target, DataSource dataSource, boolean isFirstResource)
                    {
                        int w = resource.getIntrinsicWidth();
                        int h = resource.getIntrinsicHeight();

                        if (resource instanceof GifDrawable) {
                            GifDrawable gd = (GifDrawable)resource;
                            f_imView.setImageDrawable(gd);
                            if (gd.getFrameCount()>0) {
                                // animated gif
                                wImageIsAnimated(f_fl.widgetPtr);
                                gd.setLoopCount(5);
                                gd.start();
                            }

                        }
                        else
                            f_imView.setImageDrawable((Drawable)resource); // I set the image myself here because if I let glide do it, it's not yet ready when wImageOnLoaded is called


                        wImageOnLoaded(f_fl.widgetPtr, 1,w,h);
                        return true; // tell Glide to not set the image
                    }
                })
                .into(imView);

        return -1; // useless

    }

    // note: this list contains all created cuicui widgets, whether they are in the DOM or not
    // note2: cuiElement_Delete removes the deleted widget's ref from this list
    Map<Integer, CustomFrameLayout> createdElements = new HashMap<>();

    public int cuiImage_CreateAndAddToElement(int elementParentViewId, int resizeMode)
    {
        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        ImageView imView = new ImageView(this);
        int theId = myIds++;
        imView.setId(theId);

        Matrix matrix = new Matrix();
        imView.setScaleType(ImageView.ScaleType.MATRIX);   //required
        imView.setImageMatrix(matrix);


        /*
        if (resizeMode==0) imView.setScaleType(ImageView.ScaleType.FIT_CENTER); // Contain
        if (resizeMode==1) imView.setScaleType(ImageView.ScaleType.CENTER_CROP); // Cover
        if (resizeMode==2) imView.setScaleType(ImageView.ScaleType.FIT_XY); // Fill
        */

//        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(100, 100); // TODO: REMOVE
        //      imView.setLayoutParams(lp); // TODO: REMOVE

        flParent.addView(imView);

        return theId;
    }

    public class CustomFrameLayout extends FrameLayout // public: modules use it (e.g. cuicui_wOpenGL)
    {
        public long widgetPtr=0;
        public boolean opaqueToTouch=false;
        public CustomTouchListener touchListener;
        public CustomFrameLayout(Context context) {
            super(context);
        }
    }

    private void ActivateTouchListenerIfNotAlreadyActivated(CustomFrameLayout c, int theId)
    {
        if (c.touchListener!=null) return; // already set
        c.touchListener = new CustomTouchListener(this, theId);
        c.touchListener.owner = c;
        c.setOnTouchListener(c.touchListener);
    }

    public int cuiElement_Create(int unused)
    {
        CustomFrameLayout c = new CustomFrameLayout(this); // TODO/tothink (very low priority): why FrameLayouts and not Views ?
        int theId = myIds++;
        c.setId(theId);
        c.setBackgroundColor(Color.TRANSPARENT);

       // ActivateTouchListenerIfNotAlreadyActivated(c, theId); // yes, all views must listen (see notes about android touch handling on Cuicui)

        createdElements.put(theId, c);

        return theId;
    }

    public class MyObject { // could maybe be removed and replaced by "int"
        int uid;

        public MyObject(int _uid) {
            uid = _uid;
        }

        //getters & setters
    }

    // TODO wListView: is there a "cell destroyed" event? we must notify cuicui so it deletes the wEmpty in question (and removes it from the listviewcells map)

    // used by wListView
    public class MyViewHolder extends RecyclerView.ViewHolder{

        View itemView;
        RecyclerView recyclerView;

        //itemView is the view corresponding to 1 cell
        public MyViewHolder(View _itemView, RecyclerView _recyclerView) {
            super(_itemView);
            itemView = _itemView;
            recyclerView = _recyclerView;

            //this.setIsRecyclable(false); // blocks recycling (new view created each time) -> for now we do it like that
        }

        //then add a function to fill the cell according to a MyObject
        public void bind(MyObject myObject){
            // TODO: call cuicui

            cuiListViewUpdateCell(((ViewGroup)recyclerView.getParent()).getId(), myObject.uid, itemView.getId());

        }
    }

    public class MyAdapter extends RecyclerView.Adapter<MyViewHolder> {

        List<MyObject> list;
        RecyclerView recyclerView;

        //add a constructor taking a list as input
        public MyAdapter(List<MyObject> list, RecyclerView _recyclerView) {
            recyclerView = _recyclerView;
            this.list = list;
        }

        //this function creates the viewHolders
        //and at the same time specifies the view to inflate (from the xml layouts)
        @Override
        public MyViewHolder onCreateViewHolder(ViewGroup viewGroup, int itemType) {
            CustomFrameLayout view = new CustomFrameLayout(viewGroup.getContext());
            int theId = myIds++;
            view.setId(theId);
            createdElements.put(theId, view);
            Log.d("cuicui", "ListView, onCreateViewHolder with this cellview id: " + theId);
            return new MyViewHolder(view, recyclerView);
        }

        //this is where we fill our cell with the text/image of each MyObjects
        @Override
        public void onBindViewHolder(MyViewHolder myViewHolder, int position) {
            MyObject myObject = list.get(position);
            myViewHolder.bind(myObject);
        }

        @Override
        public int getItemCount() {
            return list.size();
        }

    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_VOLUME_DOWN)) {
            return true; // we always capture the event, even if user doesn't want it : TODO_81412
        } else if ((keyCode == KeyEvent.KEYCODE_VOLUME_UP)) {
            return true; // we always capture the event, even if user doesn't want it : TODO_81412
        } else
            return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_VOLUME_DOWN)) {
            cuiVolumeDownKeyEvent();
            return true; // we always capture the event, even if user doesn't want it : TODO_81412
        } else if ((keyCode == KeyEvent.KEYCODE_VOLUME_UP)) {
            cuiVolumeUpKeyEvent();
            return true; // we always capture the event, even if user doesn't want it : TODO_81412
        } else
            return super.onKeyUp(keyCode, event);
    }

    public int cuiListView_CreateAndAddToElement(int elementParentViewId)
    {
        Log.d("cuicui", "cuiScrollView_CreateAndAddToElement");

        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        RecyclerView recyclerView = new RecyclerView(this);
        recyclerView.getRecycledViewPool().setMaxRecycledViews(0,50); // TODO P2: without this it constantly creates new ViewHolders; check whether 50 is always ok or what
        recyclerView.setLayoutManager(new LinearLayoutManager(this)); // vertical listview
        recyclerView.setHasFixedSize(false); // optim to use if we know the items always have the same size

        List<MyObject> cities = new ArrayList<>();
        for (int i=0; i<100; i++)
            cities.add(new MyObject(i));
        recyclerView.setAdapter(new MyAdapter(cities, recyclerView));

        flParent.addView(recyclerView);

        return -1; // useless
    }

    public void cuiWebView_SetSource(int elementParentViewId, String source)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        WebView wv = (WebView)fl.getChildAt(0);
        wv.loadUrl(source);
    }

    public void cuiWebView_SetHtml(int elementParentViewId, String html)
    {
        try {
            CustomFrameLayout fl = createdElements.get(elementParentViewId);

            WebView wv = (WebView) fl.getChildAt(0);

            // this show scrollbar on the webview if needed
            // Note: if the wWebView is is a wScrollView it's not possible to scroll (the wScrollView catch the gestures...) (TODO?)
            WebSettings mWebSettings = wv.getSettings();
            mWebSettings.setBuiltInZoomControls(true);
            wv.setScrollBarStyle(WebView.SCROLLBARS_OUTSIDE_OVERLAY);
            wv.setScrollbarFadingEnabled(false);


            wv.loadDataWithBaseURL(null, html, "text/html", "UTF-8", null);
        } catch (Exception ex)
        {
            Log.d("cuicui", "cuiWebView_SetHtml exception 21");
        }
    }

    public int cuiWebView_CreateAndAddToElement(int elementParentViewId)
    {
        Log.d("cuicui", "cuiWebView_CreateAndAddToElement");

        CustomFrameLayout flParent = createdElements.get(elementParentViewId);
        if (flParent==null) { /* assert */ return -1; }

        WebView webView = new WebView(this);
        webView.setWebViewClient(new WebViewClient());

        webView.setInitialScale(1);
        webView.getSettings().setUseWideViewPort(true);
        webView.getSettings().setLoadWithOverviewMode(true);

        webView.getSettings().setJavaScriptEnabled(true);
        webView.getSettings().setDomStorageEnabled(true);

        int theId = myIds++;
        webView.setId(theId);
        flParent.addView(webView);

        return theId;
    }

    public void cuiScrollView_ScrollToBottom(int elementParentViewId, int type)
    {
        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        // ScrollToEnd() is deferred (timer ~100ms): the native element may have been destroyed in the meantime
        // (group change, chat refresh, leaving the screen during a resize). So we guard the access.
        if (flParent == null) return;

        if (type==0) // vertical
        {
            final ScrollView scrollView = (ScrollView) flParent.getChildAt(0);

            scrollView.post(new Runnable() {
                @Override
                public void run() {
                    scrollView.fullScroll(ScrollView.FOCUS_DOWN);
                }
            });
        }
        else // horizontal
        {
            final HorizontalScrollView scrollView = (HorizontalScrollView) flParent.getChildAt(0);

            scrollView.post(new Runnable() {
                @Override
                public void run() {
                    scrollView.fullScroll(ScrollView.FOCUS_RIGHT);
                }
            });
        }
    }

    public int cuiScrollView_GetOffset(int elementParentViewId, int type)
    {
        CustomFrameLayout flParent = createdElements.get(elementParentViewId);
        if (type==0) {
            ScrollView scroll = (ScrollView)flParent.getChildAt(0);
            return scroll.getScrollY();
        }
        else {
            HorizontalScrollView scroll = (HorizontalScrollView)flParent.getChildAt(0);
            return scroll.getScrollX();
        }
    }

    public void cuiScrollView_SetContent(int elementParentViewId, int contentId, int type)
    {
        Log.d("cuicui", "cuiScrollView_SetContent");

        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        CustomFrameLayout content = createdElements.get(contentId);

        if (type==0) {
            ScrollView scroll = (ScrollView)flParent.getChildAt(0);
            scroll.addView(content);
        }
        else {
            HorizontalScrollView scroll = (HorizontalScrollView)flParent.getChildAt(0);
            scroll.addView(content);
        }
    }

    // TEMPORARY! if this works it should be done during the creates, to avoid making 2 calls every time (PERF)
    public void cuiSetWidgetPtr(int elementParentViewId, long widgetPtr)
    {
        try {
            CustomFrameLayout flParent = createdElements.get(elementParentViewId);
            flParent.widgetPtr = widgetPtr;
        } catch (Exception ex)
        {
            Log.e("cuicui", "ERROR cuiSetWidgetPtr() exception : " + ex.toString()); // should not happen (saw that on schmuck device)
        }
    }

    public int cuiScrollView_CreateAndAdd(int elementParentViewId,  int type)
    {
        Log.d("cuicui", "cuiScrollView_CreateAndAdd");

        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        // the scrollview needs a FrameLayout that will receive the children (the scrollview content)
        CustomFrameLayout ll = new CustomFrameLayout(this);
        int theId = myIds++;
        ll.setId(theId);
        createdElements.put(theId, ll);



        if (type==0) {
            ScrollView scroll = new ScrollView(this);
            // NOT DONE HERE scroll.addView(ll);
            flParent.addView(scroll);

            scroll.setSmoothScrollingEnabled(false); // default on cuicui

            final CustomFrameLayout fflParent = (CustomFrameLayout)flParent;

            scroll.setOnScrollChangeListener( new ScrollView.OnScrollChangeListener()
            {
                @Override
                public void onScrollChange(View v, int scrollX, int scrollY, int oldScrollX, int oldScrollY) {
                    scrollViewUserScrolled(fflParent.widgetPtr);
                }
            });
        }
        else {
            HorizontalScrollView scroll = new HorizontalScrollView(this);
            // NOT DONE HERE scroll.addView(ll);
            scroll.setSmoothScrollingEnabled(false); // default on cuicui
            flParent.addView(scroll);
            // TODO: add "user scrolled" event like above
        }

        return theId;
    }

    // Extension points for widgets defined in modules (e.g. cuicui_wOpenGL):
    // they give access to the native container view of a widget (created by cuiElement_Create)
    // so that a module can add its own native views inside it.
    public FrameLayout cuiGetElementView(int elementViewId)
    {
        return createdElements.get(elementViewId);
    }

    public long cuiGetElementWidgetPtr(int elementViewId)
    {
        CustomFrameLayout fl = createdElements.get(elementViewId);
        return fl == null ? 0 : fl.widgetPtr;
    }

    // unique id for a native view created by a module (same counter as the core's own views)
    public int cuiNewViewId()
    {
        return myIds++;
    }

    public int cuiTimePicker_CreateAndAddToElement(int elementParentViewId, int hours, int minutes, boolean use24HoursMode) // TODO: should be in a separate file (if we consider it shouldn't be built into Cuicui of course)
    {
        // NOTE_58715:
        // cv.measure doesn't work (always returns a width of 88)
        // it's because (it seems) that TimePicker can have a variable width.
        // to prevent this issue, we fixed it size in the cpp file in wTimePicker::wTimePicker()

        CustomFrameLayout flParent = createdElements.get(elementParentViewId);
        final CustomFrameLayout f_flParent = flParent;




        TimePicker cv = new TimePicker(this);



        int theId = myIds++;
        cv.setId(theId);

        // important (we always want minimal width/height)
        cv.setLayoutParams(new CustomFrameLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT));

        cv.setHour(hours);
        cv.setMinute(minutes);
        if (use24HoursMode)
            cv.setIs24HourView(true);
        else
            cv.setIs24HourView(false);

        cv.setOnTimeChangedListener(new TimePicker.OnTimeChangedListener() {
            @Override
            public void onTimeChanged(TimePicker view, int hourOfDay, int minute) {
                wTimePickerOnDateChanged(f_flParent.widgetPtr, hourOfDay, minute);
            }
        });


        flParent.addView(cv);

        cv.getLayoutParams().width = WRAP_CONTENT;
        cv.getLayoutParams().height = WRAP_CONTENT;

        return theId;
    }

    public int cuiDatePicker_CreateAndAddToElement(int elementParentViewId) // TODO: should be in a separate file (if we consider it shouldn't be built into Cuicui of course)
    {
        DatePicker cv = new DatePicker(this);
        int theId = myIds++;
        cv.setId(theId);

        cv.setCalendarViewShown(false); // force not displaying calendar but spinner instead
                                        // note: deprecated in API>=24

        // important (we always want minimal width/height)
        CustomFrameLayout.LayoutParams lp = new CustomFrameLayout.LayoutParams(WRAP_CONTENT, WRAP_CONTENT);
        cv.setLayoutParams(lp);

        CustomFrameLayout flParent = createdElements.get(elementParentViewId);
        flParent.addView(cv);

        final CustomFrameLayout f_flParent = flParent;

        Calendar currCalendar = Calendar.getInstance();
        int year = currCalendar.get(Calendar.YEAR);
        int month = currCalendar.get(Calendar.MONTH);
        int day = currCalendar.get(Calendar.DAY_OF_MONTH);
        cv.init(year, month, day, new DatePicker.OnDateChangedListener() {
            @Override
            public void onDateChanged(DatePicker datePicker, int i, int i1, int i2) {
                wDatePickerOnDateChanged(f_flParent.widgetPtr, i, i1, i2);
            }
        });

        // setOnDateChangedListener is api 26 (android 8)
        /*cv.setOnDateChangedListener(new DatePicker.OnDateChangedListener() {
            @Override
            public void onDateChanged(DatePicker datePicker, int year, int month, int dayOfMonth) {
                wDatePickerOnDateChanged(f_elementParentViewId, year, month, dayOfMonth);
            }
        });*/

        return theId;
    }

    public void OpenOsAppSettingsPage()
    {
        Intent intent = new Intent();
        intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
        Uri uri = Uri.fromParts("package", getPackageName(), null);
        intent.setData(uri);
        startActivity(intent);
    }



    public int cuiTextInput_CreateAndAddToElement(int elementParentViewId, int type)
    {
        TextView tmp = new TextView(this);
        double df = tmp.getTextSize(); // default system font


        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        EditText editText = new EditText(this);
        int theId = myIds++;
        editText.setId(theId);
        editText.setTextSize(COMPLEX_UNIT_PX, (float)(df)); // important: EditText have bigger font than default system font size (used by TextBlock for example)

        // wTextInput doesn't handle multiline, so we set this:
        editText.setSingleLine(true); // allows having a "done" button on textbox and password box
        //editText.setImeOptions(EditorInfo.IME_ACTION_NEXT); // allows clicking "done" to move to the next control
        editText.setImeOptions(EditorInfo.IME_ACTION_DONE);

        editText.setBackground(null); // note: important, without that Android apply custom "theme" (orange border when selected. vertical padding, ...)

        final CustomFrameLayout f_flParent = flParent;
        editText.setOnEditorActionListener(new EditText.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
                boolean handled = false;
                //if (actionId == EditorInfo.IME_ACTION_SEND) { // send arrives if the keyboard is configured to have send button (instead of default "done")
                 if (actionId == EditorInfo.IME_ACTION_DONE) {
                     cuiTextInputDonePressed(f_flParent.widgetPtr);
                    handled = true;
                }
                return handled;
            }
        });

        if (type==0)
        {
            editText.setInputType(InputType.TYPE_CLASS_TEXT);
            editText.setMaxLines(1);
        }
        if (type==1)
        {
            Log.d("cuicui", "cuiTextInput_CreateAndAddToElement : MULTILINE");
            editText.setInputType(InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE);
            editText.setSingleLine(false);
            editText.setHorizontalScrollBarEnabled(false);
            editText.setVerticalScrollBarEnabled(false);
        }
        if (type==2)
        {
            editText.setTransformationMethod(PasswordTransformationMethod.getInstance());
        }

        editText.setPadding(0,0,0,0);
        flParent.addView(editText);


        editText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                Log.d("cuicui", "onTextChanged");
                String str = "";
                try {
                    if (s != null) str = s.toString();
                    if (str == null) str = "";
                }
                catch (Exception ex)
                {
                    str="";
                }
                cuiTextInputChangedEvent(f_flParent.widgetPtr, str);  // pointer exited
            }

            @Override
            public void afterTextChanged(Editable s) {
            }
        });

        // important (we always want minimal height)
        CustomFrameLayout.LayoutParams lp = new CustomFrameLayout.LayoutParams(MATCH_PARENT, WRAP_CONTENT);
        editText.setLayoutParams(lp);

        return theId;
    }

    public int wText_GetCharacterPositionFromLocalPos(int elementParentViewId, int localX, int localY)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        TextView tv = (TextView)fl.getChildAt(0);

        try {
                Layout layout = ((TextView) tv).getLayout();
                int x = (int) localX;
                int y = (int) localY;
                if (layout != null)
                {
                    int line = layout.getLineForVertical(y);
                    int offset = layout.getOffsetForHorizontal(line, x);

                    return offset;
                }
        }
        catch (Exception ex)
        {
            return 0; // error
        }
        return 0;
    }

    public void wTextInput_Multiline_DisableReturnKey(int elementParentViewId)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText editText = (EditText)fl.getChildAt(0);
        editText.setImeOptions(EditorInfo.IME_ACTION_DONE);
        editText.setRawInputType(InputType.TYPE_CLASS_TEXT);
    }

    public void wTextInput_Enabled(int elementParentViewId, int state /*1=true, 0=false*/)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);
        if (state==1)
            et.setEnabled(true);
        else
            et.setEnabled(false);
    }

    public void cuiTextInput_InsertText(int elementParentViewId, int pos, String text)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);
        int len = et.getText().length();
        if (pos < 0 || pos > len)
        {
            Log.w("cuicui", "cuiTextInput_InsertText: pos " + pos + " out of range (length " + len + "), clamped");
            pos = Math.max(0, Math.min(pos, len)); // otherwise IndexOutOfBoundsException
        }
        et.getText().insert(pos, text);
    }

    public void cuiTextInput_SetFocus(int elementParentViewId)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);

        final EditText f_et = et;
        et.post(new Runnable() {
            public void run() {
                // requestFocusFromTouch() alone is no longer enough since Android 15 (targetSdk 35): when another
                // focusable widget (ScrollView, etc.) holds the focus, it is not transferred to the EditText
                // and the IME stays connected to the old view (mServedView), so typing goes nowhere.
                // We force the EditText to be focusable in touch-mode then explicitly take the focus.
                f_et.setFocusable(true);
                f_et.setFocusableInTouchMode(true);
                f_et.requestFocus();
                InputMethodManager lManager = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
                lManager.showSoftInput(f_et, InputMethodManager.SHOW_IMPLICIT);
            }
        });
    }

    public void cuiTextInput_SetCursorPos(int elementParentViewId, int pos)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);
        int len = et.getText().length();
        if (pos < 0 || pos > len)
        {
            // typically: position computed in utf8 bytes instead of utf16 units
            Log.w("cuicui", "cuiTextInput_SetCursorPos: pos " + pos + " out of range (length " + len + "), clamped");
            pos = Math.max(0, Math.min(pos, len)); // otherwise IndexOutOfBoundsException
        }
        et.setSelection(pos);
    }

    public int cuiTextInput_GetCursorPos(int elementParentViewId)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);
        return et.getSelectionStart();
    }

    public void cuiTextInput_SetPlaceholder(int elementParentViewId, String text)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);
        et.setHint(text);
    }

    public void cuiTextInput_SetText(int elementParentViewId, String text)
    {
        CustomFrameLayout fl = createdElements.get(elementParentViewId);
        EditText et = (EditText)fl.getChildAt(0);
        et.setText(text);
    }

    SpannableStringBuilder globalSpannableStringBuilder = new SpannableStringBuilder(); // note : it's ok as we can't create more than one wMultiLineText at the same time (only on UI thread)
                                                                                                // TODO: this is not true, an user could call cuiTextBlock_RichTextAppend on two differents wText...
    public void cuiTextBlock_RichTextClear()
    {
        globalSpannableStringBuilder.clear();
        globalSpannableStringBuilder.clearSpans();
    }

    public int cuiTextBlock_RichTextAppend(int parentElemViewId, int riId, int flag, int textColor /*0=default*/, String text)
    {
        int start = globalSpannableStringBuilder.length();
        globalSpannableStringBuilder.append(text);

        boolean bold=false;
        if ((flag&1)==1) bold=true;
        boolean italic=false;
        if ((flag&2)==2) italic=true;

        if (bold && !italic)
            globalSpannableStringBuilder.setSpan(new StyleSpan(android.graphics.Typeface.BOLD), start, globalSpannableStringBuilder.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        if (italic && !bold)
            globalSpannableStringBuilder.setSpan(new StyleSpan(Typeface.ITALIC), start, globalSpannableStringBuilder.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        if (italic && bold)
            globalSpannableStringBuilder.setSpan(new StyleSpan(Typeface.BOLD_ITALIC), start, globalSpannableStringBuilder.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        if (textColor!=0) {
            globalSpannableStringBuilder.setSpan(new ForegroundColorSpan(textColor), start, globalSpannableStringBuilder.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }


        return globalSpannableStringBuilder.length(); // not optimal (PERF)
    }

    public void cuiTextBlock_RichTextCommit(int parentElemViewId)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        TextView tv = (TextView)fl.getChildAt(0);
        tv.setText(globalSpannableStringBuilder);

        // needed for touchable spans, but disabled: it may also catch the clicks outside the touchable spans (to test before enabling)
        //tv.setClickable(true);
        //tv.setMovementMethod(LinkMovementMethod.getInstance());
    }

    public void cuiTextBlock_SetEllipsis(int parentElemViewId)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        TextView tv = (TextView)fl.getChildAt(0);
        tv.setEllipsize(TextUtils.TruncateAt.END);
    }

    public int cuiTextBlock_CreateAndAddToElement(int elementParentViewId, int wrap /*1=multiline, 0=singleline*/)
    {
        CustomFrameLayout flParent = createdElements.get(elementParentViewId);

        TextView textView = new TextView(this);
        textView.setTextColor(0xff000000); // note: we must set the wText default color (black) as this can be different if a theme is selected in the manifest file
        if (wrap==1) {
            // DUPLICATE_571
            textView.setSingleLine(false);
            textView.setHorizontallyScrolling(false); //  useful?
            //textView.setEllipsize(TextUtils.TruncateAt.END); //  Ellipsis (default cuicui )not anymore!
            ///
        }
        else {
            textView.setSingleLine(true);
            //textView.setEllipsize(TextUtils.TruncateAt.END); // Ellipsis (default cuicui)not anymore!
        }
        int theId = myIds++;
        textView.setId(theId);
        textView.setBackgroundColor(Color.TRANSPARENT); // yes? or leave default?
        flParent.addView(textView);

        final int f_elementParentViewId = elementParentViewId;

        // note: we don't handle touchspans here because it's too late, we must test on the parent layout

        return theId;
    }

    public int cuiComputeNativeWidgetSize(int parentElemViewId)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        View v = (View)fl.getChildAt(0);

        v.measure(WRAP_CONTENT, WRAP_CONTENT);

        // tested to fix the problem of the timepicker being measured with a width of 88 (height is measured correctly)
        //fl.measure(WRAP_CONTENT, WRAP_CONTENT);

        // tested to fix the problem of the timepicker being measured with a width of 88 (height is measured correctly)
        //int widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(10000, View.MeasureSpec.UNSPECIFIED);
        //int heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(10000, View.MeasureSpec.UNSPECIFIED);
        //v.requestLayout();
        //v.measure(widthMeasureSpec, heightMeasureSpec);

        int w = v.getMeasuredWidth();
        int h =  v.getMeasuredHeight();
        Log.d("cuicui","cuiComputeNativeWidgetSize : " + w + " , " + h);
        int wh = (h<<16) + w;

        return wh;
    }

    public int cuiTextViewComputeWidthHeight(int parentElemViewId, int requiredWidth)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        TextView tv = (TextView)fl.getChildAt(0);

        int widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(requiredWidth, View.MeasureSpec.AT_MOST);
        int heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
        tv.measure(widthMeasureSpec, heightMeasureSpec);

        int wh = (tv.getMeasuredHeight()<<16) + tv.getMeasuredWidth();
        return wh;
    }


    public int cuiTextBlock_MeasureMe_MultiLine(String str, int thisWidth)
    {
        TextView textView = new TextView(this);

        // DUPLICATE_571
        textView.setSingleLine(false);
        textView.setHorizontallyScrolling(false);
        textView.setEllipsize(TextUtils.TruncateAt.END);
        ////

        textView.setText(str);

        int textViewWidth = thisWidth;
        int widthMeasureSpec = View.MeasureSpec.makeMeasureSpec(textViewWidth, View.MeasureSpec.AT_MOST);
        int heightMeasureSpec = View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.UNSPECIFIED);
        textView.measure(widthMeasureSpec, heightMeasureSpec);
        int h = textView.getMeasuredHeight();

        if (textView.getMeasuredWidth() != thisWidth)
            Log.e("cuicui", "error 214: " + textView.getMeasuredWidth() + " should be " + thisWidth);

        Log.d("cuicui", "cuiTextBlock_MeasureMe_MultiLine :  height=" + h);

        return h;
    }



    public void cuiElement_Delete(int viewId)
    {
        CustomFrameLayout fl = createdElements.get(viewId);
        if (fl!=null)
        {
            createdElements.remove(viewId);
            ViewGroup parent = (ViewGroup) fl.getParent();
            if (parent != null)
                parent.removeView(fl);
        }
    }

    double df = -1;

    public int cui_GetDefaultFontSizeInPixels()
    {
        TextView myTextView = new TextView(this);
        return(int) myTextView.getTextSize(); // TODO: return a float
    }

    public int cui_GetDefaultFontLineHeight() {
        TextView myTextView = new TextView(this);
        float f = myTextView.getPaint().getFontMetrics().bottom - myTextView.getPaint().getFontMetrics().top;
        return (int)f; // TODO: return a float
    }

    public void cuiTextInput_SetFontSize(int parentElemViewId, double fontSize)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        EditText tv = (EditText)fl.getChildAt(0);
        tv.setTextSize(COMPLEX_UNIT_PX, (float)fontSize);
    }

    public void cuiTextBlock_SetFontSize(int parentElemViewId, double fontSize)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        TextView tv = (TextView)fl.getChildAt(0);
        tv.setTextSize(COMPLEX_UNIT_PX, (float)fontSize);
    }

    public void cuiTextBlock_SetTextColor(int parentElemViewId, int argb)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        TextView tv = (TextView)fl.getChildAt(0);
        tv.setTextColor(argb);
    }

    public void cuiTextInput_SetTextColor(int parentElemViewId, int argb)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        EditText tv = (EditText)fl.getChildAt(0);
        tv.setTextColor(argb);
        // placeholder: same color, half transparent (otherwise it keeps the theme's color, often invisible)
        tv.setHintTextColor((argb & 0x00ffffff) | ((((argb >>> 24) & 0xff) / 2) << 24));
    }

    public void cuiTextBlock_SetText(int parentElemViewId, String text)
    {
        CustomFrameLayout fl = createdElements.get(parentElemViewId);
        TextView tv = (TextView)fl.getChildAt(0);
        tv.setText(text);
    }

    class CuiAnimationRunnable implements Runnable
    {
        Handler handler;
        long time;
        int durationInMilliseconds;
        boolean firstLoop=true;
        static final int requestedFrameDuration = 8; // with 16ms (60fps) it's not very smooth

        public CuiAnimationRunnable(Handler _handler)
        {
            handler = _handler;
            time = 0;
        }
        public void run()
        {
            handler.postDelayed(this, requestedFrameDuration);

            if (firstLoop==true) {
                firstLoop = false; // it's important that we don't do RunFrame the first time
                // because when doing animation::create, it starts the eventloop
                // if RunFrame is called, it will see that there is no transitions in the transitions list (as it wasn't already created)
                // and then will stop the eventloop, which is not ok
            }
            else {
                cuiAnimationEvent(-1 /*deprecated*/, (int) time);
            }
        }
        public void stop()
        {
            handler.removeCallbacks(this);
            firstLoop=true;
        }
    }

    Handler myTransitionEventLoopHandler = new Handler();
    CuiAnimationRunnable myTransitionEventLoopRunnable = new CuiAnimationRunnable(myTransitionEventLoopHandler);


    public void cuiElement_StopTransitionEventLoop(int viewId, int durationInMilliseconds)
    {
        Log.d("cuicui", "cuiElement_StopTransitionEventLoop");
        myTransitionEventLoopRunnable.stop();
    }

    public void cuiElement_StartTransitionEventLoop(int viewId, int durationInMilliseconds)
    {
        Log.d("cuicui", "cuiElement_StartTransitionEventLoop");
        myTransitionEventLoopRunnable.run();
    }

    static final int REQUEST_IMAGE_CAPTURE = 1;

    String mCurrentPhotoPath=null;
    Uri mCurrentPhotoUri = null;

    private File createImageFile() throws IOException {
        // always use the same file
        String imageFileName = "JPEG_cui_camera_temp_file";

        File storageDir = getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = File.createTempFile(
                imageFileName,  /* prefix */
                ".jpg",         /* suffix */
                storageDir      /* directory */
        );

        // Save a file: path for use with ACTION_VIEW intents
        mCurrentPhotoPath = image.getAbsolutePath();

        return image;
    }

    String directoryTemp, directoryAssets, directoryDocuments;

    public String GetSpecialDirectory(String directoryType)
    {
        if (directoryType.equals("temp")) return directoryTemp;
        else
        if (directoryType.equals("assets")) return directoryAssets;
        else
        if (directoryType.equals("document")) return directoryDocuments;
        else
            return "invalid directoryType";
    }


    private void cuiOpenCamera() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        // Ensure that there's a camera activity to handle the intent
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            // Create the File where the photo should go
            File photoFile = null;
            try {
                photoFile = createImageFile();
            } catch (IOException ex) {
                // Error occurred while creating the File
                Log.e("cuicui", "dispatchTakePictureIntent() error 545: can't create temp file");
            }
            // Continue only if the File was successfully created
            if (photoFile != null) {
                // the app must declare a FileProvider with android:authorities="${applicationId}.fileprovider"
                Uri photoURI = FileProvider.getUriForFile(this,
                        getPackageName() + ".fileprovider",
                        photoFile);
                mCurrentPhotoUri = photoURI;
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
                startActivityForResult(takePictureIntent, REQUEST_TAKE_PHOTO);
            }
        }
    }

    ///////////

    // note: remember that these are Java types, a long in Java is 64 bits ( = jlong or "long long" on c++ side)
    public native void cuiMain(int androidRootViewId);
    public native void cuiBackButtonEvent();
    public native int cuiIsNativeWithTouch(long widgetPtr);
    public native void wDatePickerOnDateChanged(long widgetPtr, int y, int m, int d);
    public native void wTimePickerOnDateChanged(long widgetPtr, int h, int m);
    public native void wImageOnLoaded(long widgetPtr, int state, int imageWidth, int imageHeight);
    public native void cuiTextInputDonePressed(long widgetPtr);
    public native void cuiWebRequestEvent(int webRequestId, String result, boolean success, int httpStatusCode);
    public native void scrollViewUserScrolled(long widgetPtr);
    public native int cuiHandleTouchSpanReleased(long widgetPtr, int characterPos);
    public native void cuiAppSizeChanged(int width, int height);
    public native void cuicuiShowSimpleMessageBoxYesNo(int answer);
    public native void wImageIsAnimated(long widgetPtr);
    // Recursive copy of the apk's "assets" folder to a folder accessible via fopen (replaces the old android-copy-assets lib)
    private static String cuicuiReadSmallFile(File f) {
        if (!f.exists()) return null;
        try (FileInputStream fis = new FileInputStream(f)) {
            byte[] data = new byte[(int) f.length()];
            int n = fis.read(data);
            return new String(data, 0, Math.max(n, 0), StandardCharsets.UTF_8);
        } catch (IOException e) {
            return null;
        }
    }

    private void cuicuiCopyAssetDir(String assetPath, String destPath) {
        try {
            String[] children = getAssets().list(assetPath);
            if (children == null || children.length == 0) {
                // leaf = file
                cuicuiCopyAssetFile(assetPath, destPath);
                return;
            }
            // directory
            new File(destPath).mkdirs();
            for (String child : children) {
                String a = assetPath.isEmpty() ? child : assetPath + "/" + child;
                String d = destPath + "/" + child;
                cuicuiCopyAssetDir(a, d);
            }
        } catch (IOException e) {
            Log.d("INFO_6874", "cuicuiCopyAssetDir(" + assetPath + "): " + e);
        }
    }

    private void cuicuiCopyAssetFile(String assetPath, String destPath) {
        InputStream in = null;
        java.io.OutputStream out = null;
        try {
            in = getAssets().open(assetPath);
            File df = new File(destPath);
            File parent = df.getParentFile();
            if (parent != null) parent.mkdirs();
            out = new java.io.FileOutputStream(df);
            byte[] buf = new byte[8192];
            int n;
            while ((n = in.read(buf)) > 0) out.write(buf, 0, n);
        } catch (IOException e) {
            Log.d("INFO_6874", "cuicuiCopyAssetFile(" + assetPath + "): " + e);
        } finally {
            try { if (in != null) in.close(); } catch (IOException ignore) {}
            try { if (out != null) out.close(); } catch (IOException ignore) {}
        }
    }

    public native void cuiTouchEvent(long widgetPtr, int eventType, float x, float y, float c, float d); // c,d = other info
    public native void cuiGesturePanEvent(long widgetPtr, float x, float y, float offsetX, float offsetY, int phase);
    public native void cuiGesture2FingersPanEvent(long widgetPtr, float x, float y, float offsetX, float offsetY, int phase);
    public native void cuiGestureScaleEvent(long widgetPtr, float scale, float x, float y, int phase);
    public native void cuiGestureDoubleTapEvent(long widgetPtr, float x, float y);
    public native void cuiGestureSingleTapEvent(long widgetPtr, float x, float y);
    public native void cuiGestureLongPressEvent(long widgetPtr, float x, float y);
    public native void cuiTextInputChangedEvent(long widgetPtr, String newText);
    public native void cuiVolumeDownKeyEvent();
    public native void cuiVolumeUpKeyEvent();
    public native void cuiAnimationEvent(int useless, int currentTimeInMilliseconds);
    public native void cuicuiOnAppStateChanged(int state);
    public native void cuiSetCentralSafeAreaRect(int l, int t, int r, int b);
    public native int cuiListViewCreateOneCell();
    public native int cuiGetBackButtonManagerPushedElementsCount();
    public native void cuiListViewUpdateCell(int wListViewAndroidId, int cellId, int viewId); // if we reuse it: remember to use long widgetPtr like everywhere else
    public native void cuiAndroidOnFileInputSelection(String fileName, String base64Data, String mimeType, int imageOrientation, String option);

}
