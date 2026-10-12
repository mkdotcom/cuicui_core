#include "cuicui.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "wImage.h"



#ifdef CUICUI_IOS

@implementation CuiUIImageView {
    wWidget *widgetPtr;
}

// note: this class is useless for now as the touches will be handled by the "uiview" which embed the uiimage; this may change in the future
-(void)setPtr:(wWidget*)_ptr
{
    widgetPtr = _ptr;
}

@end

#endif

#ifdef CUICUI_ANDROID
namespace cui {
extern "C"
void
JNICALL
cuijni_wImageOnLoaded(
	JNIEnv *env,
	jobject obj/* this */,
	jlong widgetPtr,
	int state,
	int imageWidth,
	int imageHeight)
{
	wImage *w = (wImage*)widgetPtr;
	if (!CheckDefWidgetAlive(w)) return;

	w->LoadImageCompleted_Private(state, imageWidth, imageHeight);
}
} // namespace cui
#endif

#ifdef CUICUI_WEB

namespace cui {
extern "C" {
void EMSCRIPTEN_KEEPALIVE webOnLoadCompleted(unsigned int ptr, int state, int imageWidth, int imageHeight)
{
	wImage *w = (wImage*)ptr;
	if (!CheckDefWidgetAlive(w)) return;
	w->LoadImageCompleted_Private(state, imageWidth, imageHeight);
}
}
} // namespace cui
#endif

namespace cui {
void wImage::LoadImageCompleted_Private(int state, int imageWidth, int imageHeight)
{
    timerManager.Create(0.001, [this, imageWidth, imageHeight, state](Timer *t)
    // deferred to the next tick: handling it synchronously crashed in one app with one specific image (cause not found)
    {
        if (!CheckDefWidgetAlive(this)) return;

        if (state == 1)
        {

            this->imagePhysicalWidthInPixels = imageWidth;
            this->imagePhysicalHeightInPixels = imageHeight;

            reInit.InitWithSize(imageWidth, imageHeight);
            re = reInit;

            
    #ifdef CUICUI_IOS
    #ifdef WIMAGE_IOS_WITHDRAWRECT
            this->Reset_setPosHw_prev_values();
            this->SetPos_HW2();

            [this->uiView setNeedsDisplay];
    #else
           CUICUI_NOP
    #endif
    #endif
            
            Opacity(1);

           loadState = eLoadState::Success;

    #ifdef CUICUI_UWP
           if (bitmapImage!=nullptr)
              this->isAnimatedImage = bitmapImage->IsAnimatedBitmap;
    #endif

            // important: if wImage is already layouted, we must refresh it so transform will be applied
            if (this->ax[X].size != -666 && this->ax[Y].size != -666)
            {
                this->Reset_setPosHw_prev_values();
                this->SetPos_HW2();
            }


            // note: we don't send that right now, we need to wait untile the widget is layouted (because user may need the actual widget size in the callback)
        }
        if (state == 0)
        {
            loadState = eLoadState::Fail;
            // note: we don't send that right now, we need to wait until the widget is layouted (because user may need the actual widget size in the callback)

            // important: if wImage is already layouted, we must refresh it so the onLoadFail callback will be called
            if (this->ax[X].size != -666 && this->ax[Y].size != -666)
            {
                this->Reset_setPosHw_prev_values();
                this->SetPos_HW2();
            }
        }
            
    });
}

wImage* wImage::Grayscale()
{
	assert(false); // not tested + web only
#if defined(CUICUI_ANDROID)
	
#elif defined(CUICUI_UWP)
	
#elif defined(CUICUI_WEB)
	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("ERROR wImage()::SourceFromBase64() -> elem not found : " + num.toString());
				return;
			}			
			var theImage = elem.firstElementChild;

			theImage.style["filter"] = "grayscale(1);";
		}, htmlId);
#elif defined(CUICUI_IOS)
	
#elif defined(CUICUI_MACOS)
	
#else
	non_implemented_platform
#endif
	return this;
}


wImage* wImage::Tint(const cColor& col)
{
	EnsureNative();

#if defined(CUICUI_ANDROID)
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiImage_SetTint", "(II)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, theId, col.GetARGB());
#elif defined(CUICUI_UWP)
	// not available on UWP (the UWP sdk doesn't provide this feature)
#elif defined(CUICUI_WEB)
	// not yet available on WEB : not possible in css without tricks ?
#elif defined(CUICUI_IOS)
    assert(false); // missing (easy to implement)
#elif defined(CUICUI_MACOS)
	missing
#else
	non_implemented_platform
#endif

	return this;
}

wImage::wImage(AutoResize a_autoResize) : wWidget()
{
	coreType = eCoreType::Image;

	resizeMode = a_autoResize;


#if defined(CUICUI_IOS)
#ifdef WIMAGE_IOS_WITHDRAWRECT
#else
    // the uiimage is inside an uiview (without that we have issues; it's not optimum though : would be nice to fix it someday)
    Native();
    uiImageView = [[CuiUIImageView alloc] initWithFrame:CGRectZero];
    [uiView addSubview : uiImageView];

    [uiImageView setContentMode : UIViewContentModeTopLeft];

#endif

    
  
    
	
#endif
#if defined(CUICUI_UWP)
	Canvas ^c = ref new Canvas(); // this canvas prevents the image from being clipped when using Transforms, very important (check Test_Transform and Test_Images if this is changed)
	im = ref new Image();
	im->Clip = nullptr;
	c->Clip = nullptr;
	c->Children->Append(im);
	b = c;

	hd = ref new cCuiElement_uwp(this);
	
	im->ImageOpened += ref new RoutedEventHandler([this](Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^args)
	{
		if (!CheckDefWidgetAlive(this)) return;
		BitmapSource ^bs = (BitmapSource^)im->Source;				
		LoadImageCompleted_Private(1, bs->PixelWidth, bs->PixelHeight);		
	});
	im->ImageFailed += ref new ExceptionRoutedEventHandler([this](Platform::Object ^sender, Windows::UI::Xaml::ExceptionRoutedEventArgs^args)
	{
		Platform::String^ hr = args->ErrorMessage;
		if (!CheckDefWidgetAlive(this)) return;		
		LoadImageCompleted_Private(0, -1,-1);
	});
	

	im->RenderTransformOrigin = Point(0.5, 0.5);
	

	

	im->Stretch = Stretch::None; // do not change this
	im->HorizontalAlignment = HorizontalAlignment::Left;
	im->VerticalAlignment = VerticalAlignment::Top;


	// note: we do this for a slider in an app, but I think it's ok all the time
	// note2: I don't really know why we must do this (BUT it's very important to do it, otherwise the image always catches a touch meant for a touchable located behind it)
	b->IsHitTestVisible = false;



#endif
#if defined(CUICUI_ANDROID)
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiImage_CreateAndAddToElement", "(II)I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, theId, (int)resizeMode);
	imageTheId = (int)v;
#endif
#if defined(CUICUI_WEB)	

	htmlId = htmlIdCount;
	htmlIdCount++;

	EM_ASM_(
	{
		var num = $0;
		var elem = document.createElement('div'); // a "container" for the image; needed for our transform system to work
		var num = $0;		
		elem.id = num.toString();		
		elem.className = "b";
		elem.style["pointer-events"] = "none"; // TODO P5: put in the css

		addUnattachedElements(elem, num.toString());

		

		// the image is a child of "elem"
		var theImage = document.createElement('img');	
		theImage.style["object-fit"] = "none"; 
		theImage.draggable = false;
		elem.appendChild(theImage);
	}, htmlId);
#endif
}


#if defined(CUICUI_UWP)
task<void> UwpSourceFromBase64_Step2(wImage *wImage, Image ^im, std::string base64)
{
	try {
		Windows::Storage::Streams::IBuffer ^bytes = Windows::Security::Cryptography::CryptographicBuffer::DecodeFromBase64String(StdStringToUwpString(base64));
		if (bytes == nullptr)
		{
			LogError("UwpSourceFromBase64_Step2: error 5816");
			return;
		}
		Windows::Storage::Streams::InMemoryRandomAccessStream ^stream = ref new Windows::Storage::Streams::InMemoryRandomAccessStream();
		co_await stream->WriteAsync(bytes);
		stream->Seek(0);

		Windows::Graphics::Imaging::BitmapDecoder ^decoder = co_await Windows::Graphics::Imaging::BitmapDecoder::CreateAsync(stream);
		WriteableBitmap ^writeableBitmap = ref new WriteableBitmap((int)decoder->PixelWidth, (int)decoder->PixelHeight);
		co_await writeableBitmap->SetSourceAsync(stream);				

		im->Source = writeableBitmap;
		wImage->LoadImageCompleted_Private(1, writeableBitmap->PixelWidth, writeableBitmap->PixelHeight);
	}
	catch (...)
	{
		wImage->LoadImageCompleted_Private(0/*fail*/, 0, 0);
	}
}
#endif

void wImage::Animated_Play()
{
	if (!isAnimatedImage) return;

#if defined(CUICUI_ANDROID)
	//missing
#elif defined(CUICUI_UWP)	
	bitmapImage->Play();
#elif defined(CUICUI_WEB)
	//missing
#elif defined(CUICUI_IOS)
	//missing
#elif defined(CUICUI_MACOS)
	//missing
#else
	//missing
#endif
	
}
void wImage::Animated_Stop()
{
	if (!isAnimatedImage) return;

#if defined(CUICUI_ANDROID)
	//missing
#elif defined(CUICUI_UWP)	
	bitmapImage->Stop();
#elif defined(CUICUI_WEB)
	//missing
#elif defined(CUICUI_IOS)
	//missing
#elif defined(CUICUI_MACOS)
	//missing
#else
	//missing
#endif
	
}

void wImage::OnLoadSuccess(std::function<void(wImage *self)> f)
{
    assert(sourceSet==false); // OnLoadCompleted() must be called BEFORE Source()
    // note: we could possibly make these 2 callbacks required as parameters of Source("file.png", [](){...},  [](){...},) ? at least the user sees the problem at compile time rather than at runtime
    
    onLoadSuccess = f;
}

void wImage::OnLoadFail(std::function<void(wImage *self)> f)
{
    assert(sourceSet==false); // OnLoadFail() must be called BEFORE Source()
    
    onLoadFail = f;
}

wImage* wImage::Source(std::string str)
{
	str = trim(str);
	assert(str != "");
	source = str;
	loadState = eLoadState::Loading;
    sourceSet=true;
	Opacity(0); // need to wait until image is loaded and drawn before actually showing it
				// Note: we dont use Visibility() because on android the wImage are not displayed anymore (using INVISIBLE instead of GONE works, but it's not ok, as GONE is way faster; that would be ok on wImage, but Visibility() applies to every kind of widget)
				// Issue: if the user sets an opacity to the image, it will be lost (TODO!)
                // TODO IMPORTANT! the bgcolor must stay visible !!! need a native call to hide the native image, and not the whole widget
#if defined(CUICUI_IOS)
	NSString *imageUrl; /// @"http://www.foo.com/myImage.jpg";
	if (str.find("http") == 0)
	{
		imageUrl = StdStringToNSString(str);
#ifdef WIMAGE_IOS_WITHDRAWRECT
#else
		[uiImageView
            sd_setImageWithURL : [NSURL URLWithString : imageUrl]
			placeholderImage : [UIImage imageNamed : @"placeholder.png"]
             completed:^(UIImage *image, NSError *error, SDImageCacheType cacheType, NSURL *imageURL)
            {
                if (error==nil)
#ifdef WIMAGE_IOS_WITHDRAWRECT
                    [setImage:image];
#else
                    this->LoadImageCompleted_Private(1, image.size.width, image.size.height);
#endif
                else
                    this->LoadImageCompleted_Private(0, -1, -1);
             }
         ];
#endif
    }
	else
	{
		UIImage * myImage = [UIImage imageNamed : StdStringToNSString("./assets/" + str)];
        
        if (myImage==nullptr)
            this->LoadImageCompleted_Private(0, -1, -1);
        else
        {
#ifdef WIMAGE_IOS_WITHDRAWRECT
            [cuiImageView setImage: myImage];
#else
            [uiImageView setImage: myImage];
            
                this->LoadImageCompleted_Private(1, myImage.size.width, myImage.size.height);
          
#endif
        }
	}



	/*
	// TODO_3443: deprecated method (in ios9?), but there's another very similar one if we don't want to use the SDWebImage component (but we won't have cache management)
	[NSURLConnection sendAsynchronousRequest : [NSURLRequest requestWithURL : [NSURL URLWithString : imageUrl]] queue : [NSOperationQueue mainQueue] completionHandler : ^ (NSURLResponse *response, NSData *data, NSError *error)
	{
		uiImageView.image = [UIImage imageWithData : data];
	}];*/
#endif
#if defined(CUICUI_UWP)		
	auto str3 = StdStringToUwpString(str);

	Uri ^u;
	if (str.find("http") == 0)
		u = ref new Uri(str3); // remote image		
	else
		u = ref new Uri("ms-appx:///mkw_assets/" + str3);  // local image		
	
	bitmapImage = ref new BitmapImage(u); // TODO optim: shouldn't we destroy the bitmapImage once onLoadSuccess or onLoadFail ?

	// load progress : https://docs.microsoft.com/hi-in/uwp/api/windows.ui.xaml.media.imaging.bitmapimage.downloadprogress
		
	
	im->Source = bitmapImage;

#endif
#if defined(CUICUI_ANDROID)
	jstring jstr = globalCuiJniEnv->NewStringUTF(str.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, "cuiImage_SetSource", "(ILjava/lang/String;)I");
	globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, this->theId, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
	// ? -> http://square.github.io/picasso/
// or glide, fresco, universal image loader
#endif
#if defined(__EMSCRIPTEN__)		
	EM_ASM_(
	{
		var num = $0;
		var elem = getUnattachedElementsFromId(num.toString());
		if (elem == null)
		{
			Module.print("ERROR wImage()::Source() -> elem not found : " + num.toString());
			return;
		}
		var url = UTF8ToString($1);
		if (url.indexOf("http") != 0)
			url = "./mkw_assets/" + url; // local image
		var theImage = elem.firstElementChild;
		

		theImage.addEventListener('load', function()
		{
			var imageWidth = this.naturalWidth;
			var imageHeight = this.naturalHeight;
			Module.ccall('webOnLoadCompleted', 'void', ['number', 'number', 'number', 'number'], [$2, 1, imageWidth, imageHeight]);
		});
		theImage.addEventListener('error', function()
		{
			Module.ccall('webOnLoadCompleted', 'void', ['number', 'number', 'number', 'number'], [$2, 0, -1, -1]);
		});

		theImage.src = url;

	}, htmlId, str.c_str(), this);

#endif
	return this;
	}

wImage* wImage::SourceFromBase64(std::string base64, std::string mimeType)
{
	source = "";
	assert(base64.length() != 0);
#if defined(CUICUI_ANDROID)	
	JNICall_Void_IntString("cuiImage_SetSourceFromBase64", theId, base64);
#elif defined(CUICUI_UWP) 
	UwpSourceFromBase64_Step2(this, im, base64);
#elif defined(CUICUI_WEB)
	
	base64 = "data:" + mimeType + ";base64," + base64;

	EM_ASM_(
		{
			var num = $0;
			var elem = getUnattachedElementsFromId(num.toString());
			if (elem == null)
			{
				Module.print("ERROR wImage()::SourceFromBase64() -> elem not found : " + num.toString());
				return;
			}
			var theBase64 = UTF8ToString($1);
			var theImage = elem.firstElementChild;

			theImage.addEventListener('load', function()
			{
				var imageWidth = this.naturalWidth;
				var imageHeight = this.naturalHeight;
				Module.ccall('webOnLoadCompleted', 'void', ['number', 'number', 'number', 'number'], [$2, 1, imageWidth, imageHeight]);
});
			theImage.addEventListener('error', function()
			{
				Module.ccall('webOnLoadCompleted', 'void', ['number', 'number', 'number', 'number'], [$2, 0, -1, -1]);
			});

			theImage.setAttribute('src', theBase64);
			
		}, htmlId, base64.c_str(), this); // base64.c_str() -> ok because base64 is not utf (note: actually maybe it's always ok, no?)

#elif defined(CUICUI_IOS)
	NSString *s = StdStringToNSString(base64);
	int debugs = (int)[s length];

	NSData *data = [[NSData alloc]initWithBase64EncodedString:s options : 0];
  

	UIImage *image = [UIImage imageWithData : data];
    if (image==nullptr)
        this->LoadImageCompleted_Private(0, -1, -1);
    else
    {
#ifdef WIMAGE_IOS_WITHDRAWRECT
        [cuiImageView setImage: image];
#else
        [uiImageView setImage: image];
#endif
        this->LoadImageCompleted_Private(1, image.size.width, image.size.height);
    }
    
#else
	missing
#endif

		return this;
}

void wImage::InitMatrix()
{
#ifdef CUICUI_UWP
	
	return;
#endif

}

void wImage::ApplyMatrix()
{
	ApplyMatrix_HW();
}

void wImage::ApplyMatrix_HW()
{
	if (imagePhysicalWidthInPixels == -1 || imagePhysicalHeightInPixels == -1)
		return; // image not yet loaded
	double rotation, scale, translationX, translationY;
	re.Extract(rotation, scale, translationX, translationY);

	// preventing problems
	if (isnan(rotation)) return;	
	if (isnan(scale)) return;
	if (isnan(translationX)) return;
	if (isnan(translationY)) return;

#if defined(CUICUI_ANDROID)
	JNICall_Void_IntFloatFloatFloatFloat("cuiImage_SetRotationScaleTranslation", theId, (rotation*180.0) / 3.1415, scale, translationX, translationY);
#elif defined(CUICUI_UWP)
	auto transformGroup = ref new TransformGroup(); // TODO: should not create all these each time, juste once 

	auto rotationTransform = ref new RotateTransform();
	rotationTransform->Angle = (rotation*180.0) / 3.1415;

	auto scaleTransform = ref new ScaleTransform();
	scaleTransform->ScaleX = scale;
	scaleTransform->ScaleY = scale;

	auto translationTransform = ref new TranslateTransform();
	translationTransform->X = translationX;
	translationTransform->Y = translationY;

	transformGroup->Children->Append(rotationTransform);
	transformGroup->Children->Append(scaleTransform);
	transformGroup->Children->Append(translationTransform);

	im->RenderTransform = transformGroup;
#elif defined(CUICUI_WEB)
	EM_ASM_({			
			var theimage = gu_child($0);
			var tr = "";
			tr += " translate(" + $3 + "px, " + $4 + "px)";
			tr += " scale(" + $2 + "," + $2 + ")";
			tr += " rotate(" + $1 + "rad)";
			theimage.style["transform"] = tr;			
	}, htmlId, rotation, scale, translationX, translationY );
#elif defined(CUICUI_IOS)
#ifdef WIMAGE_IOS_WITHDRAWRECT
    [cuiImageView setNeedsDisplay];
#else
    
    // version 2:  same hack as in "drawRect" here (not necessarily possible?)
    //need to set "aspectfit"
    // problem: rotation does a scale... (+ over doesn't work)
    /*
    cRect re2 = re;
    re2.RotateAroundCenter(-rotation); // remove the rotation from "re"
    
    CGRect tt= CGRectMake(re2.p1.x, re2.p1.y, re2.p3.x-re2.p1.x, re2.p7.y-re2.p1.y); // get bounding box named "tt"
    
   
    CGAffineTransform tmp = CGAffineTransformIdentity;
    //tmp = CGAffineTransformConcat(tmp, CGAffineTransformMakeTranslation(re2.center.x, re2.center.y));
    tmp = CGAffineTransformConcat(tmp, CGAffineTransformMakeRotation(rotation));
    //tmp = CGAffineTransformConcat(tmp, CGAffineTransformMakeTranslation(-re2.center.x, -re2.center.y));
   // CGContextTranslateCTM(context, re2.center.x, re2.center.y); // go to 0,0
    //CGContextRotateCTM(context, rotation); // apply rotation
    //CGContextTranslateCTM(context, -re2.center.x, -re2.center.y); // go back to real pos
    
    
    uiImageView.transform = tmp;
    
    [uiImageView setFrame : tt];
    //CGRectMake(0, 0, width, height)];
    */


    // version 1
    // setting "transform" modifies these things, so we absolutely need to set them back each time:
    [uiImageView setFrame: CGRectMake(0, 0, imagePhysicalWidthInPixels, imagePhysicalHeightInPixels)];
    [uiImageView setBounds: CGRectMake(0, 0, imagePhysicalWidthInPixels, imagePhysicalHeightInPixels)];
    uiImageView.center = CGPointMake(imagePhysicalWidthInPixels/2, imagePhysicalHeightInPixels/2);
    
    // apply the transform
    CGAffineTransform tmp = CGAffineTransformIdentity;
    tmp = CGAffineTransformConcat(tmp, CGAffineTransformMakeRotation(rotation));
    tmp = CGAffineTransformConcat(tmp, CGAffineTransformMakeScale(scale,scale));
    tmp = CGAffineTransformConcat(tmp, CGAffineTransformMakeTranslation(translationX, translationY));

    uiImageView.transform = tmp;
#endif

#elif defined(CUICUI_MACOS)
	missing
#else
	missing
#endif
}

void wImage::Transform_Reset()
{		
	if (!reInit.initedWithSize) return; // image not loaded yet
	InitMatrix();
	re = reInit;
	ApplyMatrix();
}

void wImage::Transform_Translate(double x, double y, bool usePixelDensity)
{	
	InitMatrix();
    double r=1;
#ifdef CUICUI_IOS
    r *= [UIScreen mainScreen].scale;
    if (usePixelDensity==false)
        r = 1;
    r=1; // weird
#endif
	re.Translate((x*r), (y*r));
	ApplyMatrix();
}

void wImage::Transform_ScaleOnPoint(double s, double pointX, double pointY)
{
	InitMatrix();		
	re.ScaleOnPoint(s, pointX, pointY);	
	ApplyMatrix();
}

void wImage::Transform_Scale(double s)
{	
	InitMatrix();    
	re.ScaleOnCenter(s);
	ApplyMatrix();
}

void wImage::Transform_FitCover()
{
    
	if (imagePhysicalWidthInPixels != -1 && imagePhysicalHeightInPixels != -1) // image loaded?
	{
		double width = this->ax[X].size;
		double height = this->ax[Y].size;
		if (width == -666 || height == -666)
			return; // widget not ready yet		

		double nw, nh;
		Transform_FitContain(&nw, &nh);
		
		double wratio = width / nw;
		double hratio = height / nh;
		double scaleFactor = std::max(wratio, hratio);
		Transform_Scale(scaleFactor);
	}
}

bool IsAlmostEqual(double a, double b)
{	
	double diff = abs(a - b);
	if (diff < 2)
		return true;
	return false;
}

void wImage::Transform_FitContain(double *outFinalWidth, double *outFinalHeight)
{		
	if (imagePhysicalWidthInPixels != -1 && imagePhysicalHeightInPixels != -1) // image loaded?
	{
		double width = this->ax[X].size;
		double height = this->ax[Y].size;
		if (width == -666 || height == -666)
		{
			if (outFinalWidth != nullptr) *outFinalWidth = 0;
			if (outFinalHeight != nullptr) *outFinalHeight = 0;
			return; // widget not ready yet	
		}
		if (width == 0 || height == 0)
		{
			if (outFinalWidth != nullptr) *outFinalWidth = 0;
			if (outFinalHeight != nullptr) *outFinalHeight = 0;
			return; // nothing to do, computations would fail	
		}

#ifdef CUICUI_IOS
#endif
        double pw = ((double)imagePhysicalWidthInPixels);
        double ph = ((double)imagePhysicalHeightInPixels);
        
		Transform_Reset();

		double widgetRatio = width / height;
		double imageRatio = (double)pw / (double)ph;

		double scaleFactor;
		if (widgetRatio > imageRatio)
			scaleFactor = height / (double)ph;
		else
			scaleFactor = width / (double)pw;

		CUICUI_NOP



        Transform_Translate(-pw/2, -ph/2, false);
        Transform_Scale(scaleFactor);
        //is the scale factor really the same uwp/ios ?
        Transform_Translate(width / 2, height / 2); // WEIRD, on ios if we use pw/ph instead of width/height it's ok o_O
       // put width back...
        
       // need to compare the matrices to be sure
       // and otherwise I don't know, it's still very weird

		ApplyMatrix();

        // useful for cover
		double finalWidth = pw * scaleFactor;
		double finalHeight = ph * scaleFactor;
		if (outFinalWidth != nullptr) *outFinalWidth = finalWidth;
		if (outFinalHeight != nullptr) *outFinalHeight = finalHeight;

	}
}

void wImage::Transform_Rotate(double radians)
{
	InitMatrix();
	double va = radians;
	re.RotateAroundCenter(va);
	ApplyMatrix();
	currentAngle = radians;
	return;
}

} // namespace cui