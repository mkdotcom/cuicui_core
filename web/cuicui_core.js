var unattachedElements = new Map();

document.addEventListener("visibilitychange", function() {
  if (document.visibilityState === 'visible') {
    Module.ccall('visibility_event', 'number', ['number',], [1]);	
  } else {
    Module.ccall('visibility_event', 'number', ['number',], [0]);	
  }
});

window.addEventListener("resize", function()
{ 
	 resizedOccurredLetsWarnCui();				 
}, true);

	 function unicodeStringToTypedArray(s) {
			var escstr = encodeURIComponent(s);
			var binstr = escstr.replace(/%([0-9A-F]{2})/g, function(match, p1) {
				return String.fromCharCode('0x' + p1);
			});
			var ua = new Uint8Array(binstr.length);
			Array.prototype.forEach.call(binstr, function (ch, i) {
				ua[i] = ch.charCodeAt(0);
			});
			return ua;
		}

function resizedOccurredLetsWarnCui()
		{
			 var w = document.documentElement.clientWidth;
			 var h = document.documentElement.clientHeight;
			 Module.ccall('resize_event', 'number', ['number','number'], [w,h]);				 			
		}


		 function webOnDrop(event)
		 {
			console.log("webOnDrop");
			
			if (event.dataTransfer.files.length==0) return;
			
			var file = event.dataTransfer.files[0]; // we only handle the first one for now (same in uwp)
			console.log("filename: " + file.name);
			
			ManageFileReceived(file);
			
			
		 }
 
 function ManageFileReceived(file)
 {
		var fileName = file.name;
 
		var reader = new FileReader();
							
				reader.addEventListener("load", function () {
					
					// PERF: all this is not optimal, do the PERF below to limit conversions?
					
					// "this.result" contains the base64data with "data:image/jpg;base64," at the beginning, we will extract mimetype from this and remove it before sending the data to c++ (because, by design, the onFileReceived in Cuicui required this "tag" to not be in the base64data)
					var pos = this.result.indexOf(";base64,");
					var realData = this.result.substring(pos+8);
					var mimeType = this.result.substring(5, pos); 
					
					// Note: the mimetype may be fake (the browser just use the file extension to determine it, i think)
					
					var myTypedArray = unicodeStringToTypedArray(realData); // have to convert the string to a typedarray with this function
					var buf = Module._malloc(myTypedArray.length);
					Module.HEAPU8.set(myTypedArray, buf);
					
					
					console.log("upload file mime type : " + mimeType);
					console.log("upload file data size : " + myTypedArray.length);
					
					Module.ccall('fileinput_event', 'number', ['string', 'number', 'number', 'string'], [fileName, buf, myTypedArray.length, mimeType]);
					Module._free(buf);
					

					
				  }, false);
				reader.readAsDataURL(file); // PERF: use readAsArrayBuffer? since we send directly into a c++ buffer it would be more optimal (the user can convert to base64 on the c++ side if they want)
 }

// called from main()
        function onLoaded() 
		{
            console.log('Module is loaded');		
			resizedOccurredLetsWarnCui();

			console.log('scrollbar size : ' + getScrollbarWidth());
			
			// TODO: remove this now that we handle it via span and CallSpanTouchEvent?
			// click event on "wMultilineText" give click position
			/*document.onselectionchange = function() {
			  console.log('New selection made');
			  //var selection1 = document.getSelection(); // deprecated?
			  //console.log(selection1);
			  
			  // WARNING: not ok if there is a <br> in the span !!
			  var selection2 = window.getSelection();
			  var parentElement = selection2.baseNode.parentElement.parentElement;
			  console.log("parentElement id : " + parentElement.id);
			  console.log("position : " + selection2.baseOffset);
			  console.log(selection2);
			  
			  
			  if (selection2.baseNode.parentElement.tagName == "SPAN")
				Module.ccall('ontouchableevent', 'number',['number', 'number', 'number', 'number'],[parentElement.id, 77, selection2.baseOffset, -1]);  // 77= special for text clicked
				
			};*/
			
			
			/// global file picker code
			var fileInput = document.getElementById("webFilePickerInputElement");			
			fileInput.addEventListener('change', function(e) 
			{
				var filesList = this.files;
				console.log("selected filesList : " + JSON.stringify(filesList));
				if (filesList==null || filesList==0) 
					return; // canceled by user
					
				var file = filesList[0];
				
				// check file size to avoid files that are too big?
				
				ManageFileReceived(file);
																			
				fileInput.value = ""; // allow user to select again the same file (without that "change" is not triggered again)
				
			}, false);
        }

function cuiLocalStorageWrite(label, value)
		{
			if (typeof(Storage) !== "undefined") 
			{
				localStorage.setItem(label,value);
				return 1;
			} 
			else 
			{
				console.log('cuiLocalStorageWrite... ERROR');		
				return 0; // localstorage is not available
			}
		}
		
		function cuiLocalStorageRead(label, outMemoryArea)
		{
			if (typeof(Storage) !== "undefined") 
			{
				var result = localStorage.getItem(label);
				if (result==null) return 0; // this label is not defined
				var resultLength = lengthBytesUTF8(result);
				stringToUTF8(result, outMemoryArea, 8192); 
				return 1;
			} 
			else 
			{
				console.log("cuiLocalStorageRead : ERROR!");
				return 0; // localstorage is not available
			}
		}

function getScrollbarWidth() 
		 {
			// https://gist.github.com/kflorence/3086552
			var inner = document.createElement('p');
			inner.style.width = "100%";
			inner.style.height = "200px";

			var outer = document.createElement('div');
			outer.style.position = "absolute";
			outer.style.top = "0px";
			outer.style.left = "0px";
			outer.style.visibility = "hidden";
			outer.style.width = "200px";
			outer.style.height = "150px";
			outer.style.overflow = "hidden";
			outer.appendChild(inner);

			document.body.appendChild(outer);
			var w1 = inner.offsetWidth;
			outer.style.overflow = 'scroll';
			var w2 = inner.offsetWidth;

			if (w1 == w2) {
				w2 = outer.clientWidth;
			}

			document.body.removeChild(outer);

			return (w1 - w2);
		}

function cuiTimerEvent(timerUid)
		{
			Module.ccall('cui_timer_event', 'number', ['number'], [timerUid]);	
		}

		function computeSingleLineTextWidth(str)
		{
			var iSpan = document.createElement('span');					
			iSpan.style["white-space"] = "nowrap";
			iSpan.innerHTML = str;
			
			var cuiRootDiv = document.getElementById("cuiRootDiv");
			cuiRootDiv.appendChild(iSpan);
			
			var www = iSpan.offsetWidth;

			cuiRootDiv.removeChild(iSpan);

			return www;
		}
		
		function computeTextHeight(str, requiredWidth, wrap) // wrap: 0=signeline, 1=multiline
		{
			var iDiv = document.createElement('div');
			//iDiv.style["left"] = "0px"; // TODO: remove if useless
			//iDiv.style["top"] = "0px"; // TODO: remove if useless
			iDiv.style["width"] = requiredWidth + "px";
			iDiv.style["word-wrap"] = "break-word"; // DUP_565
			iDiv.innerHTML = str;
			
			var cuiRootDiv = document.getElementById("cuiRootDiv");
			cuiRootDiv.appendChild(iDiv);
			
			var hhh = iDiv.offsetHeight;

			cuiRootDiv.removeChild(iDiv);

			return hhh;
		}

function addUnattachedElements(element, htmlId)
{
	unattachedElements.set(htmlId.toString(), element); 
}

function removeFromUnattachedElementsAndFromDom(htmlId)
{
	var e = unattachedElements.get(htmlId.toString());
	if (e!=null && e.parentNode!=null)
		e.parentNode.removeChild(e);
	if (e!=null) 
		unattachedElements.delete(htmlId.toString());			
}

function getUnattachedElementsFromId(htmlId) // TODO: cleaning: must remove it from unattachedElements when it is put in the dom (but that means we have to search with getElementById (which may not be faster?)(it seems getElementById is very fast)
// possibility: get rid of addunattachedelement and put them in a hidden "special" div? anyway an unatttachedelement is meant to be used right away
// meh, apparently we can still go faster than getElementById
// (simple array or sorted array or some hashmap, -> we should be able to go faster)
// TODO: we keep "unattachedElements" (renamed) AND we remember to delete (with the children) when deleteme
// in any case when an elem is deleteme we must remove it, and all its children, from unattachedElements (VERY IMPORTANT)
{
	var e = unattachedElements.get(htmlId.toString());
	if (e==null) {
		console.log("ERROR : unattachedElementsFromId -> htmlId not found : " + htmlId);
		return null;
	}
	return e;
}



function gu(htmlId)
{
	return getUnattachedElementsFromId(htmlId.toString());
}

function gu_child(htmlId)
{
	var p = getUnattachedElementsFromId(htmlId.toString());
	if (p==null) {console.log("gu_child: error 57861_A"); console.trace(); return null; }
	var child = p.firstElementChild;			
	if (child==null) {console.log("gu_child: error 57861_B, htmlId = " + htmlId); console.trace(); return null; }
	return child;
}

// Rich text spans (wText): called from the HTML generated for touchable spans
function CallSpanTouchEvent_hover(event, widgetPtr, riId)
{
	event.stopPropagation();
	Module.ccall('onspantouchableevent_hover', 'number', ['number', 'number', 'number', 'number'], [widgetPtr, riId, -1, -1]);
}

function CallSpanTouchEvent_click(event, widgetPtr, riId)
{
	event.stopPropagation();
	Module.ccall('onspantouchableevent_click', 'number', ['number', 'number', 'number', 'number'], [widgetPtr, riId, -1, -1]);
}

function CallSpanTouchEvent_leave(event, widgetPtr, riId)
{
	event.stopPropagation();
	Module.ccall('onspantouchableevent_leave', 'number', ['number', 'number', 'number', 'number'], [widgetPtr, riId, -1, -1]);
}
