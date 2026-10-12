#include "platform.h"
namespace cui {} using namespace cui; // for code outside the namespace (ObjC blocks, main)
#include "web_request.h"

namespace cui {
std::list<WebRequest*> webRequests_private; // TODO: (useless if we use smartptrs, no?)(unless we sometimes need to cancel them?) add a mutex; always check if the webrequest is in the list before doing wr->OnSuccess (that way we're sure the pointer hasn't been deleted) / note: if wr is always destroyed by answer or timeout it's not a problem; -> so we could go back to the simple pointer in userdata? (no need for this list?)
std::mutex webRequestMutex;

int webRequestGuidCount = 0;

#if  defined(CUICUI_WEB)
static void downloadSucceeded(emscripten_fetch_t *fetch) {
	webRequestMutex.lock();
	WebRequest *_fthis = nullptr;
	for (WebRequest *wr : webRequests_private) // we already have the pointer (in fetch->userData) but we must check in the list that it hasn't been deleted in the meantime // TODO/tothink: what if by bad luck the same address is used twice? maybe use a mutexed id++ instead?
	{
		if (wr == (WebRequest*)fetch->userData)
		{
			_fthis = wr;
			break;
		}
	}
	webRequestMutex.unlock();

	std::string result;
	if (fetch->numBytes > 0)
		result.assign((const char*)fetch->data, fetch->numBytes);
	emscripten_fetch_close(fetch); // Free data associated with the fetch.


	if (_fthis == nullptr) {
		Log("ERROR 4545"); // should never happen				
		return;
	}

	unsigned short status = fetch->status;
	_fthis->Success(result, (int)status);
}

static void downloadFailed(emscripten_fetch_t *fetch) {
	webRequestMutex.lock();
	WebRequest *_fthis = nullptr;
	for (WebRequest *wr : webRequests_private) // we already have the pointer (in fetch->userData) but we must check in the list that it hasn't been deleted in the meantime
	{
		if (wr == (WebRequest*)fetch->userData)
		{
			_fthis = wr;
			break;
		}
	}
	webRequestMutex.unlock();

		
	if (_fthis == nullptr) {
		Log("ERROR 4546"); // should never happen
		return;
	}

	// note: onError() is called when status is not 200, it's not how Cuicui::WebRequest works, we "emulate" it :
	unsigned short status = fetch->status;
	if (status == 0 || status == 404)
	{
		// status==0 : timeout / abort / network unreachable (no HTTP response at all)
		// status==404 : http error treated as no network by historical convention
		// In both cases, route to Failed() so OnNoNetwork() callbacks fire and any
		// in-progress flags (e.g. refreshUsersAndLineInProgress) get released.
		_fthis->Failed();
	}
	else
	{
		std::string result;
		if (fetch->numBytes > 0)
			result.assign((const char*)fetch->data, fetch->numBytes);

		_fthis->Success(result, (int)status);
	}

	emscripten_fetch_close(fetch); // Also free data on failure.
}
#endif

#ifdef CUICUI_UWP
task<void> WebRequest::DoIt_Uwp()
{
	AssertNotOnUiThread(); // it's important that this is not run on ui thread (or UI will be locked until the answser arrives)

	try
	{
		filter = ref new HttpBaseProtocolFilter(); 
		httpClient = ref new HttpClient(filter); 		
		cancellationTokenSource = cancellation_token_source(); // value type (ref-counted): no leak

		using namespace Windows::Storage::Streams;

		auto data = ref new HttpStringContent(StdStringToUwpString(body), UnicodeEncoding::Utf8, "text/plain;charset=UTF-8");		
		for (auto h : headers)
		{
			auto a = StdStringToUwpString(h.first);
			auto b = StdStringToUwpString(h.second);
			httpClient->DefaultRequestHeaders->Append(a,b); // is this ok? won't it be applied to subsequent requests?
		}
		auto pa = co_await httpClient->PostAsync(ref new Uri(StdStringToUwpString(url)), data);

		

		auto c = pa->Content->ToString();
		int statusCode = (int)pa->StatusCode;

		RunOnUiThread([this, c, statusCode]
		{
			AssertOnUiThread();
			Success(UwpStringToStdString(c), statusCode);
		});
	}
	catch (const task_canceled&)
	{
		RunOnUiThread([this] {Failed(); });
	}
	catch (Exception ^ex)
	{
		RunOnUiThread([this] {Failed(); });
	}
}
#endif

void WebRequest::DoIt(const std::string & _verb, const std::string & domain, const std::string & pp, const std::string & _body, bool ssl) // TODO: content-type:xxx
{
	AssertOnUiThread();

	verb = _verb;

	if (verb == "GET") assert(false); // not tested on android/web/ios; not coded on UWP

	assert(verb == "GET" || verb == "POST");

	assert(onSuccess != nullptr); // onSuccess is mandatory
	body = _body;

#ifdef CUICUI_DEBUG
	//ssl = false; // TODO: rather than having to do this, webrequest should be modified so we pass it a complete url (https://blabla or http://blabla)
#endif
	

	if (ssl)
		url = "https://";
	else
		url = "http://";
	url += domain + pp;

	CUICUI_NOP

#if defined(CUICUI_ANDROID)
    
	std::string headersAsString; // format : "name1###value1###name2###value2"
	int i=0;
	for (auto h : headers)
	{
		headersAsString += h.first + std::string("###");
		headersAsString += h.second;
		if (i<headers.size()-1)
			headersAsString += std::string("###");
		i++;
	}

	JNICall_Void_IntStringStringStringString("SendWebRequest", guid, verb, url, body, headersAsString); // note: this is a synchronous call 
	return;
	// TODO: should return a specific error code? a webmethod could return "" without it being an error
#elif defined(CUICUI_UWP)    
	create_async([this]
	{
		create_task(DoIt_Uwp());
	});
	return;
#elif  defined(__EMSCRIPTEN__)
   
    
	//static const char* custom_headers[3] = { "content-type", "text/plain;charset=UTF-8", nullptr }; // WARNING TODO: empscripten doesn't seem to handle this yet, check/update

	assert((headers.size()+1)*2+1 < 128);

	const char* custom_headers[128];
	custom_headers[0] = "content-type";
	custom_headers[1] = "text/plain;charset=UTF-8";

	int idx = 2;
	for (auto &h : headers)
	{
		custom_headers[idx+0] = h.first.c_str();
		custom_headers[idx+1] = h.second.c_str();
		idx += 2;
	}
	custom_headers[idx] = nullptr;

	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, verb.c_str());
	attr.requestHeaders = custom_headers;
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY; // to make a synchronous call: | EMSCRIPTEN_FETCH_SYNCHRONOUS
	attr.onsuccess = downloadSucceeded;
	attr.onerror = downloadFailed;
	// Safety net: without a timeout, a fetch can hang forever (tab backgrounded,
	// connection stalled, etc.) and never invoke onsuccess/onerror, leaving
	// in-progress flags stuck (root cause of the "F5 needed to see new messages" bug).
	// 30s is well above normal latency for our endpoints; on timeout, onerror fires
	// with status==0, which downloadFailed() now routes to Failed() / OnNoNetwork().
	attr.timeoutMSecs = 30000;
	attr.userData = (void*)this;
	attr.requestData = (char *)body.c_str();
	attr.requestDataSize = body.size();
	emscripten_fetch(&attr, url.c_str());

	return;
#elif defined(CUICUI_IOS)
	NSString *nssbody = StdStringToNSString(body);
	NSString *nssUrl = StdStringToNSString(url);
	NSString *nssVerb = StdStringToNSString(verb);

	const char *bytes = [nssbody UTF8String];

	NSMutableURLRequest *urlRequest = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString : nssUrl]];

	//create the Method "POST"
	[urlRequest setHTTPMethod : nssVerb];
    [urlRequest setHTTPBody : [NSData dataWithBytes : bytes length : strlen(bytes)]];
    
    [urlRequest addValue : @"text/plain; charset=UTF-8" forHTTPHeaderField:@"content-type"];
    for (auto h : headers)
    {
        NSString *field = StdStringToNSString(h.first);
        NSString *value = StdStringToNSString(h.second);
        [urlRequest addValue : value forHTTPHeaderField:field];
    }

	NSURLSession *session = [NSURLSession sharedSession];

	NSURLSessionDataTask *dataTask = [session dataTaskWithRequest : urlRequest completionHandler : ^ (NSData *data, NSURLResponse *response, NSError *error)
	{
        if (error)
        {
            RunOnUiThread([this]() {
                Failed();
            });
            return;
        }
		NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
		long statusCode = httpResponse.statusCode;
		
		NSString* newStr = [[NSString alloc] initWithData:data encoding : NSUTF8StringEncoding];
		std::string result = NSStringToStdString(newStr);

        NSLog(@"WebRequest: Ok");
		RunOnUiThread([this, result, statusCode]() {
			Success(result, (int)statusCode);
		});

	}];
	[dataTask resume];

	return;
#else
	deprecated
#endif
}

} // namespace cui