#pragma once



// various helpers functions and other stuff

#include "platform.h"

#include <iostream>
#include <sstream>
#include <random>
#include <string>
#include <iostream>
#include <iomanip>

#if defined(WINAPI_FAMILY)

namespace cui {
using namespace concurrency;

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Filters;

} // namespace cui
#endif


namespace cui {
typedef unsigned char BYTE;

static std::string trim(std::string s, char option = 0)
{
// https://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string/33438386

	// convert all whitespace characters to a standard space
	std::replace_if(s.begin(), s.end(), (std::function<int(BYTE)>)::isspace, ' ');

	// remove leading and trailing spaces
	size_t f = s.find_first_not_of(' ');
	if (f == std::string::npos) return "";
	s = s.substr(f, s.find_last_not_of(' ') - f + 1);

	// remove consecutive spaces
	s = std::string(s.begin(), std::unique(s.begin(), s.end(),
		[](BYTE l, BYTE r) { return l == ' ' && r == ' '; }));

	switch (option)
	{
	case 'l':  // convert to lowercase
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		return s;
	case 'U':  // convert to uppercase
		std::transform(s.begin(), s.end(), s.begin(), ::toupper);
		return s;
	case 'n':  // remove all spaces
		s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
		return s;
	default: // just trim
		return s;
	}
}

static std::string trim_deprecated(const std::string &s) // sometimes crashes (when unicode?)
{
	return s; // TODO: temporarily disabled
}


// WARNING: you own the returned pointer, remember to free it at some point !
static unsigned char* base64_decode(const char *data, size_t input_length, size_t *output_length)
{
    char *decoding_table = NULL;

    char encoding_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                              'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                              'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                              'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                              'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                              'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                              'w', 'x', 'y', 'z', '0', '1', '2', '3',
                              '4', '5', '6', '7', '8', '9', '+', '/' };

    decoding_table = (char *)malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char)encoding_table[i]] = i;

    int mod_table[] = { 0, 2, 1 };

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = (unsigned char *)malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
                          + (sextet_b << 2 * 6)
                          + (sextet_c << 1 * 6)
                          + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }
    free(decoding_table);
    return decoded_data;
}


class Length // cuicui base unit
{
protected:
	enum class eType
	{
		unset,
		pixels,
		fill,
		ems,
		min,
		sameAsX,
		rems // like "ems" but not affected by ChangeEmRatio; useful for standard paddings and gaps
	};

public: //protected:
	eType type;
	double v = -DBL_MAX; 

public:
	Length()
	{
		type = eType::unset;
	}

	inline void Unset()
	{
		type = eType::unset;
	}

#ifdef CUICUI_DEBUG
	std::string ToString()
	{
		if (type == eType::unset) return "unset";
		if (type == eType::pixels) return "pixels(" + std::to_string(v) + ")";
		if (type == eType::fill) return "fill(" + std::to_string(v) + ")";
		if (type == eType::ems) return "ems(" + std::to_string(v) + ")";
		if (type == eType::rems) return "rems(" + std::to_string(v) + ")";
		if (type == eType::min) return "min";
		if (type == eType::sameAsX) return "sameAsX";		
		assert(false);
		return "err879";
	}
#endif

	inline bool operator==(const Length& a) const
	{
		return (this->v == a.v && this->type == a.type);
	}

	inline bool IsPixels() { return type == eType::pixels; }
	inline bool IsEms() { return type == eType::ems; }	
	inline bool IsRems() { return type == eType::rems; }
	inline bool IsFill() { return type == eType::fill; }
	inline bool IsMin() { return type == eType::min; }
	inline bool IsUnset() { return type == eType::unset; }
	inline bool IsSameAsX() { return type == eType::sameAsX; }
};

class unset : public Length
{
public:
	unset()
	{
		type = eType::unset;
	}
};

class sameAsX : public Length
{
public:
	sameAsX()
	{
		type = eType::sameAsX;
	}
};

class fill : public Length
{
public:
	//int ratio; // TODO: not yet implemented (always 1)

public:
	fill()
	{
		type = eType::fill;
		v = 1;
	}
	explicit fill(int r)
	{
		type = eType::fill;
		v = r;
	}
};

class ems_or_pixels : public Length
{
public:

};

class ems : public ems_or_pixels
{
public:
	ems() // TODO: is it normal that this is allowed? for the user it's an error
	{
		type = eType::ems;
		v = DBL_MAX;
	}
	explicit ems(double _v)
	{
		type = eType::ems;
		v = _v;
	}

	inline double AsDouble()
	{
		return v;
	}
};

class rems : public ems_or_pixels
{
public:
	rems()
	{
		type = eType::rems;
		v = DBL_MAX;
	}
	explicit rems(double _v)
	{
		type = eType::rems;
		v = _v;
	}

	inline double AsDouble()
	{
		return v;
	}
};

class pixels : public ems_or_pixels
{
protected:

public:
	/* no! we don't want implicit conversion (to avoid mistakes); use "setFromDouble" to do that*/
	explicit pixels(double _v)
	{
		v = _v;
		type = eType::pixels;
	}
	explicit pixels(int _v)
	{
		v = _v;
		type = eType::pixels;
	}
	inline pixels()
	{
		Unset();
		type = eType::pixels;
	}
	inline pixels operator+(const pixels& b) const {
		pixels p;
		p.v = this->v + b.v;
		return p;
	}
	inline void operator+=(const pixels& b) {		
		this->v += b.v;		
	}
	inline pixels operator-(const pixels& b) const {
		pixels p;
		p.v = this->v - b.v;
		return p;
	}
	inline pixels operator*(const pixels& b) const {
		pixels p;
		p.v = this->v * b.v;
		return p;
	}
	inline bool operator==(const pixels &a) const
	{
		return (this->v == a.v);
	}
	inline bool operator<(const pixels &a) const
	{
		return (this->v < a.v);
	}
	inline bool operator>(const pixels &a) const
	{
		return (this->v > a.v);
	}
	inline bool operator<=(const pixels &a) const
	{
		return (this->v <= a.v);
	}
	inline bool operator>=(const pixels &a) const
	{
		return (this->v >= a.v);
	}
	inline pixels operator/(const pixels& b) const {
		pixels p;
		p.v = this->v / b.v; // handle b->v =0 and set v=Nan (and an IsNan())? IsValid() should return "false" in that case? not sure, IsValid actually means "unset" (rename IsValid to IsSet() maybe?)
		return p;
	}
	inline pixels operator*(double m) const {
		pixels p;
		p.v = this->v * m;
		return p;
	}
	inline pixels operator/(double m) const {
		pixels p;
		p.v = this->v / m;
		return p;
	}
	inline bool operator!=(const pixels &b) const {
		return (this->v != b.v);
	}
	inline void setFromDouble(double _v) { SetFromDouble(_v); } // deprecated
	inline void SetFromDouble(double _v) // note: you should probably do "pixels(666)" instead of using this method
	{
		v = _v;
	}
	inline double asDouble() { return AsDouble(); } // deprecated
	inline double AsDouble()
	{
		return v;
	}
	inline std::string AsString()
	{
		if (v == -DBL_MAX)
			return "unset";
		return std::to_string(v);
	}
	inline bool IsValid()
	{
		return (v != -DBL_MAX);
	}
	inline void Unset()
	{
		v = -DBL_MAX;
	}

	// DEPRECATED! use this : pixels(0), pixels(1.0), pixels(41.25)
	static pixels CreateOne() // warning: don't call mypix.CreateOne(x)! it obvisouly won't work; CLEANUP: we should extract this method from the pixels class ?
	{
		return pixels(1);
	}
	static pixels CreateZero() // warning: don't call mypix.CreateZero(x)! it obvisouly won't work; CLEANUP: we should extract this method from the pixels class ?
	{
		return pixels(0);
	}
	static pixels CreateFromDouble(double v) // warning: don't call mypix.CreateFromDouble(x)! it obvisouly won't work; CLEANUP: we should extract this method from the pixels class ?
	{
		return pixels(v);
	}
};

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class min : public Length
{
public:
	min()
	{
		type = eType::min;
	}
};

#if defined(CUICUI_UWP)
static std::string make_string(const std::wstring& wstring)
{
	auto wideData = wstring.c_str();
	int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideData, -1, nullptr, 0, NULL, NULL);
	auto utf8 = std::make_unique<char[]>(bufferSize);
	if (0 == WideCharToMultiByte(CP_UTF8, 0, wideData, -1, utf8.get(), bufferSize, NULL, NULL))
		throw std::exception("Can't convert string to UTF8");

	return std::string(utf8.get());
}

static std::wstring make_wstring(const std::string& string)
{
	auto utf8Data = string.c_str();
	int bufferSize = MultiByteToWideChar(CP_UTF8, 0, utf8Data, -1, nullptr, 0);
	auto wide = std::make_unique<wchar_t[]>(bufferSize);
	if (0 == MultiByteToWideChar(CP_UTF8, 0, utf8Data, -1, wide.get(), bufferSize))
		throw std::exception("Can't convert string to Unicode");

	return std::wstring(wide.get());
}
#else
static std::wstring make_wstring(const std::string& str)  // convert UTF-8 string to wstring
{

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
	return convert.from_bytes(str);
}

static std::string make_string(const std::wstring& wstr)  // convert wstring to UTF-8 string
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;
	return convert.to_bytes(wstr);
}
#endif

#ifdef CUICUI_IOS
static std::string NSStringToStdString(NSString *foo)
{
    if (foo==nil) return "";
	std::string bar([foo UTF8String]);
	return bar;
}
static NSString* StdStringToNSString(const std::string &str)
{
	NSString *ttt = [NSString stringWithUTF8String : str.c_str()];
	return ttt; // autoreleased: no leak
}
#endif

#ifdef CUICUI_UWP
static String^ StdStringToUwpString(const std::string &str)
{
	auto ws = make_wstring(str);
	Platform::String^ retVal = ref new Platform::String(ws.c_str());
	return retVal;
}

static std::string UwpStringToStdString(String^ inputStr)
{
	std::wstring wsstr(inputStr->Data());
	auto utf8Str = make_string(wsstr);
	return utf8Str;
}
#endif



static unsigned char random_char() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 255);
	return static_cast<unsigned char>(dis(gen));
}

static std::string generate_hex(const unsigned int len) {
	std::stringstream ss;
	for (unsigned int i = 0; i < len; i++) {
		auto rc = random_char();
		std::stringstream hexstream;
		hexstream << std::hex << int(rc);
		auto hex = hexstream.str();
		ss << (hex.length() < 2 ? '0' + hex : hex);
	}
	return ss.str();
}

static std::string GenFakeJpgFileName()
{
	return generate_hex(10) + ".jpg";
}
static std::string GenFakeBmpFileName()
{
	return generate_hex(10) + ".bmp";
}

extern double GetEmSize(double);

class axis
{
public:
	Length realEm; // user's choice

	std::vector<double> history;

	// working variables
	double size; // should be DEPRECATED (since the value is in em as pixel(123)
	pixels pos; // native pos in the real dom

protected:
	Length em;

public:
	void DontUseMePrivateSetEm(Length _em)
	{
		em = _em;
	}
	Length DontUseMePrivateGetEm()
	{
		return em;
	}

	double maxSizeInMin; //MaxSizeInMin // these are em

	void ResetTempValues(bool butPos = false)
	{
		if (realEm.IsPixels())
			return;

		if (butPos == false)
			pos.setFromDouble(-666);
		size = -666;
		em.Unset();
		history.clear();
		maxSizeInMin = 0;
	}
	axis()
	{
		realEm.Unset();
		ResetTempValues();
	}

	void SetPixels(pixels p)
	{
		em = p;
		size = p.AsDouble();
	}
	void SetPixels(int p)
	{		
		em = pixels(p); 
		size = p;
	}

	int GetPixels()
	{
		assert(0); // deprecated
		return -1;
	}
};

class cRect
{
public:
    double left=0,top=0,right=0,bottom=0;
public:
    void Set(double _left, double _top, double _right, double _bottom) {
        left=_left;
        right=_right;
        top=_top;
        bottom=_bottom;
    }
};

class cColor
{
public:
	unsigned int rgb; // [0x00000000,0x00ffffff] (first 8 bits are never set, it's not "argb")
	unsigned char cr, cg, cb, ca; // [0,255];
	double alpha; // [0.0,1.0];
	bool null = true;

public:
	cColor()
	{
		null = true;
	}
	cColor(unsigned int _rgb)
	{
		Set(_rgb);
	}
	cColor(unsigned int _rgb, double _alpha)
	{
		Set(_rgb, _alpha);
	}	
	std::string ToReadableString()
	{
		if (null)
			return "null";
		else
		{
            std::string s = "rgb: ";
            s += std::to_string(cr) + "," + std::to_string(cg) + "," + std::to_string(cb); // TODO: send hex
			s += ", alpha = " + std::to_string(alpha);
			return s;
		}
	}
	void Set(unsigned int _rgb)
	{
		Set(_rgb, 1.0);
	}
	unsigned int GetRGB() const
	{
		return (((unsigned int)0xff << 24) + rgb);
	}
	unsigned int GetARGB() const
	{
		return (((unsigned int)ca << 24) + rgb);
	}
	bool IsPureWhite()
	{
		return (ca == 255 && cr == 255 && cg == 255 && cb == 255);
	}
	std::string ToHtmlColorCode() // ex:  #AA12B5 (no alpha)
	{				
		std::stringstream stream;
		stream << "#";
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)cr;
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)cg;
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)cb;
		std::string result(stream.str());
		return result;
	}
	std::string ToHtmlColorCodeWithAlpha() // ex:  #FFAA12B5 (with alpha)
	{
		std::stringstream stream;
		stream << "#";		
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)cr;
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)cg;
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)cb;
		stream << std::setfill('0') << std::setw(2) << std::hex << (int)ca;
		std::string result(stream.str());
		return result;
	}
	void SetFromHtmlColorCode(const std::string &htmlColorCode) // ex: #AA12B5 (no alpha)
	{
		assert(htmlColorCode.size() == 7 && htmlColorCode[0] == '#');
		const char* str = htmlColorCode.c_str();
		int tmpR, tmpG, tmpB;
		sscanf(str, "#%02x%02x%02x", &tmpR, &tmpG, &tmpB);
		Set(1.0, (double)tmpR / 255.0, (double)tmpG / 255.0, (double)tmpB / 255.0);		
	}
	void Set(double a, double r, double g, double b)
	{		
		unsigned int ir = (unsigned int)(r*255.0);
		unsigned int ig = (unsigned int)(g*255.0);
		unsigned int ib = (unsigned int)(b*255.0);
		unsigned int c = (ir << 16) | (ig << 8) | ib;
		Set(c, a);
	}
	void Set(unsigned int _rgb, double _alpha)
	{
		if (_alpha<0.0) _alpha=0.0;
		if (_alpha>1.0) _alpha=1.0;

		// TODO: assert first 8bits of _rgb is not 0 ? or not

		null = false;

		rgb = _rgb;
		alpha = _alpha;

		cr = (rgb & 0xff0000) >> 16;
		cg = (rgb & 0x00ff00) >> 8;
		cb = (rgb & 0x0000ff);

		ca = (unsigned char)(alpha * 255.0);
	}
	void Nullify()
	{
		null = true;
	}
	bool IsNull()
	{
		return null;
	}
	bool IsNotNull()
	{
		return !null;
	}
	bool IsFullyTransparent() // returns true if the color is 'null' OR alpha is "0" 
	{
		if (null) return true;
		if (alpha == 0) return true;
		return false;
	}
	inline bool operator==(const cColor& rhs)
	{
		if (rgb != rhs.rgb) return false;
		if (alpha != rhs.alpha) return false;
		return true;
	}
#ifdef CUICUI_UWP
	SolidColorBrush^ ToUWPSolidColorBrush() const
	{
		if (null)
			ref new SolidColorBrush(Windows::UI::Colors::White);
		 return ref new SolidColorBrush(Windows::UI::ColorHelper::FromArgb(ca, cr, cg, cb));
	}
#endif
#ifdef CUICUI_IOS
    UIColor* ToUIColor() const
    {
        if (null)
            return [UIColor colorWithWhite:1 alpha:1];        
        return [UIColor colorWithRed : (float)cr / 255.0f green : (float)cg / 255.0f blue : (float)cb / 255.0f alpha : alpha];
    }
#endif
};


static bool StdStringReplaceFirstOccurence(std::string &str /*base string*/, const std::string &str2 /*sub string to find*/, const std::string &str3 /*replacement substring*/)
{
	if (str.find(str2) == -1) return false;
	str.replace(str.find(str2), str2.length(), str3);
	return true; // a replacement occurred
}

// note: there is probably a better way
static void StdStringReplaceAllOccurences(std::string &str /*base string*/, const std::string &str2 /*sub string to find*/, const std::string &str3 /*replacement substring*/)
{
	while (1)
	{
		bool r = StdStringReplaceFirstOccurence(str, str2, str3);
		if (r == false) return;
	}
}


#if defined(CUICUI_ANDROID)
static std::string JstringToStdString(JNIEnv *env, jstring value)
{
	if (value == NULL) // or (*env)->IsSameObject(env, someJObject, NULL)  ? https://stackoverflow.com/questions/2346615/jni-how-can-i-check-if-jobject-is-a-null-object-in-native-c-code
		return "";
	else {
		std::string sValue;
		const char *str;
		str = env->GetStringUTFChars(value, 0);
		sValue.assign(str);
		env->ReleaseStringUTFChars(value, str);
		return sValue;
	}
}
#endif

#if defined(CUICUI_ANDROID)



static inline void
check_java_exception(JNIEnv *env)
{
    if(env->ExceptionOccurred()) {
        fprintf(stderr, "Java exception at rlpjni.cpp line");
        env->ExceptionDescribe();
        assert(false);
    }
}
#ifdef CUICUI_DEBUG
#define _CHECK_JAVA_EXCEPTION(_env) check_java_exception(_env)
#else
#define _CHECK_JAVA_EXCEPTION(_env) // nop
#endif

// #3 — runtime guard ACTIVE IN RELEASE (assert/cui_assert are silent in release).
// A JNIEnv* is bound to its thread. If the provided env is null or is not the one of the current
// thread (badly positioned cache, call outside the UI thread...), we get the env of the current
// thread via the JavaVM rather than dereferencing a foreign env (UB -> SIGSEGV), and we
// log to make any future regression visible in logcat/Crashlytics.
static inline JNIEnv* SafeJniEnv(JNIEnv* env)
{
    JNIEnv* tls = CurrentThreadJniEnv();
    if (tls != nullptr && tls != env)
    {
        __android_log_print(ANDROID_LOG_WARN, "CUICUI_DEBUG",
            "SafeJniEnv: provided JNIEnv (%p) != current thread env (%p) -> corrected (note: globalCuiJniEnv cross-thread)",
            (void*)env, (void*)tls);
        return tls;
    }
    return env;
}

class JniCall_manager // helpers to improve performances (using cache to avoid some jni calls)
{
protected:
    std::map<std::string, jclass> jclasses;

public:
    jclass FindClass(JNIEnv *env, std::string className)
    {
    	// TODO: add a mutex for the cache

        env = SafeJniEnv(env); // #3 : fixes a possible cross-thread JNIEnv* before the JNI call
        AssertOnUiThread(); // I'm 99% sure FindClass crashes or works badly if not on ui thread (even if in some cases it works)

        auto cached = jclasses.find(className);
        if (cached != jclasses.end())
            return (*cached).second;
        else
        {
            jclass local = env->FindClass(className.c_str());
            if (local==nullptr)
			{
            	assert(false); // call must be on ui thread, You probably forgot a RunOnUiThread() { }
			}
            _CHECK_JAVA_EXCEPTION(env);

            jclass global = (jclass)env->NewGlobalRef(local);
            _CHECK_JAVA_EXCEPTION(env);
            jclasses[className] = global;

            env->DeleteLocalRef(local); // not useful I think?
            _CHECK_JAVA_EXCEPTION(env);

            return global;
        }
    }
};
extern JniCall_manager jniCallManager;

class JniCall // "easy" jni calls
{
	/*
	Common problems :

	1) check you use CallStatic() if the java method is static (and not static if not)

	2) check also this:

	extern "C"
	JNIEXPORT jint
	JNICALL
	Java_your_package_yourmethod(
		JNIEnv * env,
		jobject obj // "this"
		)
	
	This is ok if in the java it's declared like this : public native int yourmethod();
	If it's a static : public native static int yourmethod();
	then use this :

	extern "C"
	JNIEXPORT jint
	JNICALL
	Java_your_package_yourmethod(
		JNIEnv * env,
		jclass a_jclass // not "this"
		)


	*/

protected:
    jvalue jvalues[100];
    int jvalues_count=0;
    jclass classz = nullptr;
    JNIEnv* env = nullptr;
    std::string signature;
    std::string sig;
    std::string methodName;

public:
    JniCall(JNIEnv* _env, const std::string &className, const std::string &_methodName) /* TODO: remove the _env parameter if it's always globalCuiJniEnv */
    {
        //AssertOnUiThread(); // required ?

        assert(_env != nullptr);
        env = SafeJniEnv(_env); // #3 : fixes a possible cross-thread (or null) JNIEnv* before any JNI call

        classz = jniCallManager.FindClass(env, className);
        assert(classz != nullptr);

        sig = "(";
        methodName = _methodName;
    }

    ~JniCall()
    {
        CleanUp();
    }

    jstring str(const char* a_str)
    {
        jstring jstr = env->NewStringUTF(a_str); // note: we don't need to delete it, jvm will do it

        jvalues[jvalues_count].l = jstr;
        jvalues_count++;

        sig += "Ljava/lang/String;";

        return jstr;
    }

    jstring str(const std::string &a_str)
    {
        return str(a_str.c_str());
    }

    jboolean b(bool v)
    {
        jvalues[jvalues_count].z = v;
        jvalues_count++;

        sig += "Z";
        return v;
    }

    int i(int v)
    {
        jvalues[jvalues_count].i = v;
        jvalues_count++;

        sig += "I";
        return v;
    }

    void CleanUp()
    {
    }

protected:
    jmethodID __IntMethod() // TODO: cache? same for the others
    {
        std::string signature = sig + ")I";
        jmethodID method = env->GetMethodID(classz, methodName.c_str(), signature.c_str());
        return method;
    }

    jmethodID __VoidMethod()
    {
        std::string signature = sig + ")V";
        jmethodID method = env->GetMethodID(classz, methodName.c_str(), signature.c_str());
        return method;
    }

    jmethodID __StaticVoidMethod()
    {
        std::string signature = sig + ")V";
        jmethodID method = env->GetStaticMethodID(classz, methodName.c_str(), signature.c_str());
        return method;
    }

    jmethodID __StaticStringMethod()
    {
        std::string signature = sig + ")Ljava/lang/String;";
        jmethodID method = env->GetStaticMethodID(classz, methodName.c_str(), signature.c_str());
        return method;
    }

    jmethodID __StringMethod()
    {
        std::string signature = sig + ")Ljava/lang/String;";
        jmethodID method = env->GetMethodID(classz, methodName.c_str(), signature.c_str());
        return method;
    }

public:

    void CallStaticVoidMethod()
    {
        env->CallStaticVoidMethodA(classz, this->__StaticVoidMethod(), jvalues);
    }

    std::string CallStaticStringMethod()
    {
        jstring v = (jstring)env->CallStaticObjectMethodA(classz, this->__StaticStringMethod(), jvalues);

        const char* str = globalCuiJniEnv->GetStringUTFChars(v, 0);
        std::string result(str);
        globalCuiJniEnv->ReleaseStringUTFChars(v, str);
        globalCuiJniEnv->DeleteLocalRef(v);
        return result;
    }

    std::string CallStringMethod()
    {
    	jstring v;
    	v = (jstring)env->CallObjectMethodA(classz, this->__StringMethod(), jvalues);

        const char* str = globalCuiJniEnv->GetStringUTFChars(v, 0);
        std::string result(str);
        globalCuiJniEnv->ReleaseStringUTFChars(v, str);
        globalCuiJniEnv->DeleteLocalRef(v);
        return result;
    }

    void CallVoidMethod(jobject classInstance) // if we manage it: then make everything we can protected + globalCuiJniObj in the constructor?
    {
        env->CallVoidMethodA(classInstance, this->__VoidMethod(), jvalues);
    }

    int CallIntMethod(jobject classInstance)
    {
        int ret = env->CallIntMethodA(classInstance, this->__IntMethod(), jvalues);
        return ret;
    }
};

//////////////////
// TODO: all JNICALL_... functions below should be removed and replaced by code using "JniCall" :


static void JNICallWithClassName_Void(const std::string &className, const std::string &methodName)
{
	AssertOnUiThread();
	jclass clazz = globalCuiJniEnv->FindClass(className.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(clazz, methodName.c_str(), "()V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe);
	globalCuiJniEnv->DeleteLocalRef(clazz);
}


// PERF: // jclass clazz = globalCuiJniEnv->FindClass("io/github/mkdotcom/cuicui/CuicuiActivity"); // PERF: need to do this every time?
static void JNICall_Void(const std::string &methodName)
{
	AssertOnUiThread();
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "()V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe);
}
static void JNICall_Void_(const std::string &methodName)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "()V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe);
}



static void JNICall_Void_IntFloatFloatFloatFloat(const std::string &methodName, int a, float b, float c, float d, float e)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(IFFFF)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, b, c, d, e);
}
static void JNICall_Void_IntLong(const std::string &methodName, int a, long b)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(IJ)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, b);
}
static void JNICall_Void_Int(const std::string &methodName, int a)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(I)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a);
}
static void JNICall_Void_IntInt(const std::string &methodName, int a, int b)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(II)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, b);
}
static void JNICall_Void_IntIntInt(const std::string &methodName, int a, int b, int c)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(III)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, b, c);
}
static void JNICall_Void_IntDouble(const std::string &methodName, int a, double b)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(ID)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, b);
}
static void JNICall_Void_IntString(const std::string &methodName, int a, const std::string b)
{
	jstring jstr = globalCuiJniEnv->NewStringUTF(b.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(ILjava/lang/String;)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
};
static void JNICall_Void_IntIntString(const std::string &methodName, int a, int c, const std::string b)
{
	jstring jstr = globalCuiJniEnv->NewStringUTF(b.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(IILjava/lang/String;)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, c, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
};
static void JNICall_Void_IntIntIntString(const std::string &methodName, int a, int c, int d, const std::string b)
{
	jstring jstr = globalCuiJniEnv->NewStringUTF(b.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(IIILjava/lang/String;)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, a, c, d, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
};
static int JNICall_Int_IntIntIntString(const std::string &methodName, int a, int c, int d, const std::string b)
{
	jstring jstr = globalCuiJniEnv->NewStringUTF(b.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(IIILjava/lang/String;)I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, a, c, d, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
	return (int)v;
};


static int JNICallWithClassName_Int(const std::string& className, const std::string& methodName)
{
	AssertOnUiThread();
	jclass clazz = globalCuiJniEnv->FindClass(className.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(clazz, methodName.c_str(), "()I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe);
	globalCuiJniEnv->DeleteLocalRef(clazz);
	return (int)v;
}
static std::string JNICallWithClassName_String(const std::string& className, const std::string& methodName)
{
	jclass clazz = globalCuiJniEnv->FindClass(className.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(clazz, methodName.c_str(), "()Ljava/lang/String;");
	jstring v = (jstring)globalCuiJniEnv->CallObjectMethod(globalCuiJniObj, messageMe);

	const char* str = globalCuiJniEnv->GetStringUTFChars(v, 0);
	std::string result(str);
	globalCuiJniEnv->ReleaseStringUTFChars(v, str);

	globalCuiJniEnv->DeleteLocalRef(v);
    globalCuiJniEnv->DeleteLocalRef(clazz);
	return result;
}


static bool JNICall_Bool_(const std::string &methodName)
{	
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "()Z");
	jboolean v = globalCuiJniEnv->CallBooleanMethod(globalCuiJniObj, messageMe);
	return (bool)v;
}

static int JNICall_Int_IntIntInt(const std::string &methodName, int a, int c, int d)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(III)I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, a, c, d);
	return (int)v;
};
static int JNICall_Int_IntIntIntIntString(const std::string &methodName, int a, int c, int d, int e, const std::string b)
{
	jstring jstr = globalCuiJniEnv->NewStringUTF(b.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(IIIILjava/lang/String;)I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, a, c, d, e, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
	return (int)v;
};


static void JNICall_Void_StringBool(const std::string &methodName, const std::string a, bool b)
{
    jstring jstr = globalCuiJniEnv->NewStringUTF(a.c_str());
    jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(Ljava/lang/String;Z)V");
    globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, jstr, b);
    globalCuiJniEnv->DeleteLocalRef(jstr);
};

static void JNICall_Void_String(const std::string &methodName, const std::string a)
{
	jstring jstr = globalCuiJniEnv->NewStringUTF(a.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(Ljava/lang/String;)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, jstr);
	globalCuiJniEnv->DeleteLocalRef(jstr);
};
static void JNICall_Void_IntStringStringString(const std::string &methodName, int d, const std::string &a, const std::string &b, const std::string &c)
{
	jstring jstrA = globalCuiJniEnv->NewStringUTF(a.c_str());
	jstring jstrB = globalCuiJniEnv->NewStringUTF(b.c_str());
	jstring jstrC = globalCuiJniEnv->NewStringUTF(c.c_str());

	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, d, jstrA, jstrB, jstrC);

	globalCuiJniEnv->DeleteLocalRef(jstrC);
	globalCuiJniEnv->DeleteLocalRef(jstrB);
	globalCuiJniEnv->DeleteLocalRef(jstrA);
}
static void JNICall_Void_IntStringStringStringString(const std::string& methodName, int i, const std::string& a, const std::string& b, const std::string& c, const std::string &d)
{
	jstring jstrA = globalCuiJniEnv->NewStringUTF(a.c_str());
	jstring jstrB = globalCuiJniEnv->NewStringUTF(b.c_str());
	jstring jstrC = globalCuiJniEnv->NewStringUTF(c.c_str());
	jstring jstrD = globalCuiJniEnv->NewStringUTF(d.c_str());

	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	globalCuiJniEnv->CallVoidMethod(globalCuiJniObj, messageMe, i, jstrA, jstrB, jstrC, jstrD);

	globalCuiJniEnv->DeleteLocalRef(jstrD);
	globalCuiJniEnv->DeleteLocalRef(jstrC);
	globalCuiJniEnv->DeleteLocalRef(jstrB);
	globalCuiJniEnv->DeleteLocalRef(jstrA);
}
static int JNICall_Int_Void(const std::string &methodName)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "()I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe);
	return (int)v;
}
static int JNICall_Int_Int(const std::string &methodName, int a)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(I)I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, a);
	return (int)v;
}
static int JNICall_Int_IntInt(const std::string &methodName, int a, int b)
{
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(II)I");
	jint v = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, a, b);
	return (int)v;
}
static std::string JNICall_String_Int(const std::string &methodName, const int &aa)
{
    assert(globalCuiJniEnv!=nullptr);
    jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(I)Ljava/lang/String;");
    jstring v = (jstring)globalCuiJniEnv->CallObjectMethod(globalCuiJniObj, messageMe, aa);

    const char *str = globalCuiJniEnv->GetStringUTFChars(v, 0);
    std::string result(str);
    globalCuiJniEnv->ReleaseStringUTFChars(v, str);

    globalCuiJniEnv->DeleteLocalRef(v);
    return result;
}
static std::string JNICall_String_String(const std::string &methodName, const std::string &a)
{
	assert(globalCuiJniEnv!=nullptr);
	jstring jstrA = globalCuiJniEnv->NewStringUTF(a.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(Ljava/lang/String;)Ljava/lang/String;");
	jstring v = (jstring)globalCuiJniEnv->CallObjectMethod(globalCuiJniObj, messageMe, jstrA);

	const char *str = globalCuiJniEnv->GetStringUTFChars(v, 0);
	std::string result(str);
	globalCuiJniEnv->ReleaseStringUTFChars(v, str);

	globalCuiJniEnv->DeleteLocalRef(jstrA);
	globalCuiJniEnv->DeleteLocalRef(v);
	return result;
}
static std::string JNICall_String_StringIntIntIntInt(const std::string &methodName, const std::string &a, int b, int c, int d, int e)
{
	jstring jstrA = globalCuiJniEnv->NewStringUTF(a.c_str());
	
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(Ljava/lang/String;IIII)Ljava/lang/String;");
	jstring v = (jstring)globalCuiJniEnv->CallObjectMethod(globalCuiJniObj, messageMe, jstrA, b, c, d, e);

	const char *str = globalCuiJniEnv->GetStringUTFChars(v, 0);
	std::string result(str);
	globalCuiJniEnv->ReleaseStringUTFChars(v, str);
	
	globalCuiJniEnv->DeleteLocalRef(jstrA);
	globalCuiJniEnv->DeleteLocalRef(v);
	return result;
}
static std::string JNICall_String_StringStringString(const std::string &methodName, const std::string &a, const std::string &b, const std::string &c)
{
	jstring jstrA = globalCuiJniEnv->NewStringUTF(a.c_str());
	jstring jstrB = globalCuiJniEnv->NewStringUTF(b.c_str());
	jstring jstrC = globalCuiJniEnv->NewStringUTF(c.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	jstring v = (jstring)globalCuiJniEnv->CallObjectMethod(globalCuiJniObj, messageMe, jstrA, jstrB, jstrC);

	const char *str = globalCuiJniEnv->GetStringUTFChars(v, 0);
	std::string result(str);
	globalCuiJniEnv->ReleaseStringUTFChars(v, str);

	globalCuiJniEnv->DeleteLocalRef(jstrA);
	globalCuiJniEnv->DeleteLocalRef(jstrB);
	globalCuiJniEnv->DeleteLocalRef(jstrC);
	globalCuiJniEnv->DeleteLocalRef(v);
	return result;
}

static int JNICall_Int_StringString(const std::string &methodName, const std::string a, const std::string b)
{
	jstring jstrA = globalCuiJniEnv->NewStringUTF(a.c_str());
	jstring jstrB = globalCuiJniEnv->NewStringUTF(b.c_str());
	jmethodID messageMe = globalCuiJniEnv->GetMethodID(globalCuiMainActivityJClass, methodName.c_str(), "(Ljava/lang/String;Ljava/lang/String;)I");
	int ret = globalCuiJniEnv->CallIntMethod(globalCuiJniObj, messageMe, jstrA, jstrB);
	globalCuiJniEnv->DeleteLocalRef(jstrA);
	globalCuiJniEnv->DeleteLocalRef(jstrB);
	return ret;
};
#endif // CUICUI_ANDROID


} // namespace cui