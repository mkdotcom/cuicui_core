#pragma once


#if  defined(__EMSCRIPTEN__)
#include <emscripten/fetch.h> 
#endif

#include "cuicui.h"

// TODO optim: "body" is copied many time

namespace cui {
	class WebRequest;

	// TODO: should it be static? but even static isn't ok if this .h is included several times? we could put it in a singleton otherwise? or simply extern and the real ones in a .cpp
	// --> a singleton might be nice:
	//     WebRequest *wr = WebRequestManager.GetInstance().Post("http://...") 
	// currently we do : WebRequest *wr = new WebRequest();
	extern std::list<WebRequest*> webRequests_private; // TODO: (useless if we use smartptrs, no?)(unless we sometimes need to cancel them?) add a mutex; always check if the webrequest is in the list before doing wr->OnSuccess (that way we're sure the pointer hasn't been deleted) / note: if wr is always destroyed by answer or timeout it's not a problem; -> so we could go back to the simple pointer in userdata? (no need for this list?)
	extern std::mutex webRequestMutex;

	extern int webRequestGuidCount;

	class WebRequest
	{
	protected:		
		std::function<void(std::string body, int httpCode)> onSuccess;
		std::function<void()> onFail;

		int guid;
		wWidget* linkedWidget = nullptr;

	public:		
		std::string body;
		std::string url;
		std::string verb;
        std::map<std::string, std::string> headers;

	protected:
#if  defined(__EMSCRIPTEN__)
		emscripten_fetch_attr_t attr;
#endif
#if defined(WINAPI_FAMILY) 
		HttpBaseProtocolFilter ^filter;
		HttpClient ^httpClient;
		cancellation_token_source cancellationTokenSource;
#endif

	public:
		virtual void OnAnswer(std::function<void(std::string body, int httpCode)> f)
		{
			onSuccess = f;
		}
		virtual void OnNoNetwork(std::function<void()> f) // network error
		{
			onFail = f;
		}

		int GetGuid() {
			return guid;
		}

	protected:
		// note: do not delete WebRequest, it's autodeleted
		~WebRequest()
		{
		}

	public: // protected
		WebRequest(wWidget *_linkedWidget=nullptr)
		{
			linkedWidget = _linkedWidget;
			onSuccess = nullptr;
			onFail = nullptr;

			webRequestMutex.lock();
			webRequestGuidCount++;
			guid = webRequestGuidCount;
			webRequests_private.push_back(this);
			webRequestMutex.unlock();
		}
        
        void AddHeader(const std::string &name, const std::string &value)
        {
            headers[name] = value;
        }

	public: //protected:
		void Failed() // network error
		{
			AssertOnUiThread();

			if (onFail != nullptr)
				if (linkedWidget == nullptr || CheckDefWidgetAlive(linkedWidget))				
					onFail();

			webRequestMutex.lock();
			webRequests_private.remove(this);
			delete this;
			webRequestMutex.unlock();
		}
		
		void Success(std::string body, int httpCode)
		{
			AssertOnUiThread();

			if (onSuccess != nullptr)
				if (linkedWidget==nullptr || CheckDefWidgetAlive(linkedWidget))				
					onSuccess(body, httpCode);

			webRequestMutex.lock();
			webRequests_private.remove(this);
			delete this;
			webRequestMutex.unlock();
		}
	public:
#if defined(WINAPI_FAMILY)

#endif



		void Post(const std::string &domain, const std::string &pp, const std::string &_body, bool ssl = true)
		{
			DoIt("POST", domain, pp, _body, ssl);
		}
		void Get(const std::string &domain, const std::string &pp, const std::string &_body, bool ssl = true)
		{
			assert(false); // NYI
		}

#ifdef CUICUI_UWP
protected:
	task<void> DoIt_Uwp();
#endif

public: //protected:
	void DoIt(const std::string &_verb /*"POST"/"GET"*/, const std::string &domain, const std::string &pp, const std::string &_body, bool ssl = true);


	};


} // namespace cui