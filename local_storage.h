#pragma once


// Notes:
// Maybe the name should be changed to "LocalSettings" or another name ? (it's not really a storage as data size is not very high)
// LocalStorage can be slow (depending on platform/devices), you shouldn't use it too often.
// Size of data for each tag is limited to few kbytes (check MaxValueSize below)


// Dev note:
// Notes about cache:
// There is a "CuicuiLocalStorage_useCache" define that enable a cache system.
// It's currently disabled because there is one big side effect:
//  -->  if another native code/module write something in localstorage with NATIVE CODE (without using this class) the cache will not be synced! 
//#define CuicuiLocalStorage_useCache // see "Notes about cache"




// INFO_54487: Localstorage must be accessed outside of the ui thread because on android (and maybe on other platform) the app can be launched without UI.
//             When receiving an alarm event, a reboot event, a notification : the CuicuiActivity is not created, so there is not UI. 
//			   User must be able to write things in the local storage (so the app can use them next time it actually starts).


namespace cui {
	class LocalStorage  		// Note: this is a singleton (TODO: remove singleton and add a "LocalStorage localStorage" in cCentral ?
	{
	public:
		static LocalStorage *_instance;		

#ifdef CuicuiLocalStorage_useCache
	protected:
		std::map<std::string, std::string> cache;  
		bool cacheDisabled = false; 
#endif

	protected:
		bool CheckLabel(const std::string &label);

	public:
		static LocalStorage* GetInstance();
		static bool IsAvailable();

		static const int MaxTagSize = 254;		// 254 chars (254*2 bytes) is ok on uwp for label
		static const int MaxValueSize = 4095;   // note: uwp limits is 8k bytes, but Platform::String encodes in UTF16, so limits for "value" is 4096

	public:
		void ClearAll();
		void Write(const std::string &label, const std::string &value);
		void Read(const std::string &label, std::string *result);

#ifdef CuicuiLocalStorage_useCache
		void DisableCache(); // Use this if you have components or code that change the native local storage without using this class (the cache would be out of sync in this case)
#endif
	};


} // namespace cui