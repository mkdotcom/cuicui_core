#include "platform.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "local_storage.h"
#include "cuicui.h"

namespace cui {
	LocalStorage* LocalStorage::_instance = nullptr;

	void LocalStorage::Write(const std::string & label, const std::string & value) // return false if error
	{
		//AssertOnUiThread(); // NO! Check INFO_54487
		assert(CheckLabel(label));
		
		if (value.size() > MaxValueSize) { assert(false); LogError("LocalStorage error 831: value is too large."); return; } // TODO: would deserve a unit test (on all platforms)
		if (label.size() > MaxTagSize) { assert(false); LogError("LocalStorage error 245"); return; } // same
		
#ifdef CuicuiLocalStorage_useCache
		if (!cacheDisabled)
			cache[label] = value; // note: this will overwrite if already exists
#endif

#if defined(CUICUI_IOS)
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		[defaults setValue : StdStringToNSString(value) forKey : StdStringToNSString(label)];
		[defaults synchronize];
		return;
#endif
#if defined(WINAPI_FAMILY)
		try {
			Windows::Storage::ApplicationDataContainer^ localSettings = Windows::Storage::ApplicationData::Current->LocalSettings;
			if (localSettings == nullptr)
			{
				LogError("LocalStorage error 683");
				return;
			}
			auto values = localSettings->Values;
			auto sss = StdStringToUwpString(value);			
			bool ret = values->Insert(StdStringToUwpString(label), dynamic_cast<PropertyValue^>(PropertyValue::CreateString(sss)));
			if (ret == false)
			{
				LogError("LocalStorage error 684");
				return;
			}
		}
		catch (Exception ^ex)
		{
			LogError("LocalStorage error 124"); 
			return;
		}
		return;
#endif
#if defined(__EMSCRIPTEN__) 		

		unsigned int retVal = EM_ASM_INT({
			var str1 = UTF8ToString($0);
		var str2 = UTF8ToString($1);
		var result = cuiLocalStorageWrite(str1, str2);
		return result;
			}, label.c_str(), value.c_str());

		if (retVal == 1) return;
		LogError("LocalStorage error 547");
		return;
#endif
#if defined(CUICUI_ANDROID)
		JniCall cc(globalCuiJniEnv, "io/github/mkdotcom/cuicui/LocalStorage", "localStorage_Write");
		cc.str(label);
		cc.str(value);
		std::string ret = cc.CallStaticStringMethod();
		cc.CleanUp();

		if (ret == "ok") return;
		LogError("LocalStorage error 327");
		return;
#endif

		assert(false); // missing platform implementation
		return;
	}

	void LocalStorage::Read(const std::string & label, std::string * result) // returns "" if error, returns "" if this key doesn't exists 
	{
		//AssertOnUiThread(); // NO! Check INFO_54487
		assert(CheckLabel(label));		

		if (result == nullptr) { assert(false); LogError("LocalStorage error 681"); return; }
		*result = ""; // if an error orccurs, the result will be null (though you should check the returned boolean of course)
		if (label.size() >= 255) { assert(false); LogError("LocalStorage error 398"); return; }

#ifdef CuicuiLocalStorage_useCache
		if (!cacheDisabled)
		{
			auto cached = cache.find(label);
			if (cached != cache.end())
			{
				// found it
				*result = (*cached).second;
				return;
			}
		}
#endif

#if defined(CUICUI_IOS)
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSString *rr = [defaults stringForKey : StdStringToNSString(label)];
		if (rr != nil)
			*result = NSStringToStdString(rr);		
#endif
#if defined(WINAPI_FAMILY)
		Windows::Storage::ApplicationDataContainer^ localSettings = Windows::Storage::ApplicationData::Current->LocalSettings;
		if (localSettings == nullptr)
		{
			LogError("LocalStorage error 951");
			*result = ""; // error
		}

		auto values = localSettings->Values;
		String^ value = safe_cast<String^>(values->Lookup(StdStringToUwpString(label)));
		if (value == nullptr)
		{
			*result = ""; // error
		}
		else
			*result = UwpStringToStdString(value);
#endif
#if defined(CUICUI_ANDROID)
        JniCall cc(globalCuiJniEnv, "io/github/mkdotcom/cuicui/LocalStorage", "localStorage_Read");
        cc.str(label);
        *result = cc.CallStaticStringMethod();
        cc.CleanUp();

		if (*result == "android_local_storage_read_error")
		{
			*result = "";
		}
#endif
#if defined(__EMSCRIPTEN__) 		

		char outMemoryArea[8192+1]; // should define max size of value, no? TODO
		outMemoryArea[0] = '\0';

		unsigned int retVal = EM_ASM_INT({
			var str1 = UTF8ToString($0);
			var result = cuiLocalStorageRead(str1, $1);
			return result;
		}, label.c_str(), outMemoryArea);


		if (retVal == 0 /* error*/ ) *result = "";
		(*result).assign(outMemoryArea);
		
#endif

#ifdef CuicuiLocalStorage_useCache
		if (!cacheDisabled)
			cache[label] = *result; // note: this will overwrite if already exists
#endif
		
		return;
	}

	bool LocalStorage::CheckLabel(const std::string & label)
	{
		// this is to check the label is "simple" and will be accepted on all platforms (note: I don't know if it's really useful, maybe all platforms accepts everything)
		// I think UWP doesn't differentiate upper and lowercase, so I only accepts lowercase on all platforms
		for (int i = 0; i < label.size(); i++)
		{
			bool ok = false;
			char c = label.at(i);
			if (c >= 'a' && c <= 'z') ok = true;
			if (c >= '0' && c <= '9') ok = true;
			if (c == '_') ok = true;
			if (!ok) return false;
		}
		return true;
	}

	LocalStorage* LocalStorage::GetInstance()
	{
		if (_instance == nullptr)
			_instance = new LocalStorage();
		return _instance;
	}

	bool LocalStorage::IsAvailable() // use this to check LocalStorage is available and working
	{
		LocalStorage::GetInstance()->Write("cui_localstorage_test", "42");
		std::string r;
		LocalStorage::GetInstance()->Read("cui_localstorage_test", &r);
		if (r != "42")
			return false;
		return true;
	}

#ifdef CuicuiLocalStorage_useCache
	void LocalStorage::DisableCache()
	{
		cache.clear();
		cacheDisabled = true;
	}
#endif

	void LocalStorage::ClearAll()
	{
		//AssertOnUiThread(); // NO! Check INFO_54487

#ifdef CuicuiLocalStorage_useCache
		if (!cacheDisabled)
			cache.clear();
#endif

#if defined(CUICUI_ANDROID)
        JniCall cc(globalCuiJniEnv, "io/github/mkdotcom/cuicui/LocalStorage", "localStorage_ClearAll");
        cc.CallStaticVoidMethod();
        cc.CleanUp();
#elif defined(CUICUI_UWP)
		Windows::Storage::ApplicationDataContainer^ localSettings = Windows::Storage::ApplicationData::Current->LocalSettings;
		if (localSettings == nullptr)
		{
			LogError("LocalStorage error 683");
			return;
		}
		auto containerSettings = (Windows::Storage::ApplicationDataContainerSettings^)localSettings->Values;
		for (auto keys : containerSettings)
		{
			localSettings->Values->Remove(keys->Key);
		}
#elif defined(CUICUI_WEB)
		EM_ASM({
			localStorage.clear();
			});
#elif defined(CUICUI_IOS)
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSDictionary * dict = [defaults dictionaryRepresentation];
		for (id key in dict) {
			[defaults removeObjectForKey : key];
		}
		[defaults synchronize];
		return;
#elif defined(CUICUI_MACOS)
		missing
#else
		missing
#endif
	}


} // namespace cui