#pragma once


#ifdef CUICUI_IOS
#import "../projects/ios/Pods/SDWebImage/SDWebImage/Core/UIImageView+WebCache.h"

#endif

#include "3rdParty/linalg.h"

//#define WIMAGE_IOS_WITHDRAWRECT // (deprecated) working version but less optimal than the other one (at least on the simulator)(and image url not handled for now)

#include "cuicui.h"

#ifdef CUICUI_IOS
#ifdef WIMAGE_IOS_WITHDRAWRECT
#else
@interface CuiUIImageView : UIImageView <UIGestureRecognizerDelegate>
-(void)setPtr:(cui::wWidget*)_ptr;
@end
#endif
#endif



namespace cui {
class wImage : public wWidget
{
	friend class wWidget;

public:

	class cRect
	{
	public:
		linalg::vec<double, 2> p7, p9, p3, p1, center;
		double diagonalRef;
		bool initedWithSize = false;

		cRect()
		{
		}

		void InitWithSize(double w, double h)
		{
			initedWithSize = true;
			// "1"
			p1.x = 0;
			p1.y = 0;

			// "3"
			p3.x = w;
			p3.y = 0;

			// "9"
			p9.x = w;
			p9.y = h;

			// "7"
			p7.x = 0;
			p7.y = h;

			center.x = w / 2.0;
			center.y = h / 2.0;

			diagonalRef = linalg::length(p9 - p1);
		}


		void RotateAroundCenter(double radians)
		{
			if (!initedWithSize) return; // image not loaded yet

			p1 -= center;
			p1 = linalg::rot(radians, p1);
			p1 += center;

			p7 -= center;
			p7 = linalg::rot(radians, p7);
			p7 += center;

			p3 -= center;
			p3 = linalg::rot(radians, p3);
			p3 += center;

			p9 -= center;
			p9 = linalg::rot(radians, p9);
			p9 += center;
		}

		void ScaleOnPoint(double scale, double onX, double onY)
		{
			// we want a scale AND the "point" to stay in the same position in the viewport
			linalg::vec<double, 2> globalPointPosBeforeScale(onX, onY);
			linalg::vec<double, 2> globalPointPosAfterScale = globalPointPosBeforeScale;

			{
				linalg::vec<double, 2> vectorRadial;
				vectorRadial.x = center.x - globalPointPosBeforeScale.x;
				vectorRadial.y = center.y - globalPointPosBeforeScale.y;
				double dist = linalg::length(vectorRadial);
				double toAdd = (dist * scale) - dist;
				//toAdd /= 2.0; // "vectorRadial" is from center to border, so it's half the size of the rect, we don't want to again divide by 2

				linalg::vec<double, 2> norm = linalg::normalize(vectorRadial);
				globalPointPosAfterScale -= (norm * toAdd);
			}

			linalg::vec<double, 2> translate = globalPointPosAfterScale - globalPointPosBeforeScale;
			ScaleOnCenter(scale);
			Translate(-translate.x, -translate.y);
		}

		void ScaleOnCenter(double scale)
		{
			if (!initedWithSize) return; // image not loaded yet

			{
				linalg::vec<double, 2> vectorDist = p3 - p7;
				double dist = linalg::length(vectorDist);
				double toAdd = (dist * scale) - dist;
				toAdd /= 2.0;

				linalg::vec<double, 2> norm = linalg::normalize(vectorDist);
				norm *= toAdd;
				p7 -= norm;
				p3 += norm;
			}

			{
				linalg::vec<double, 2> vectorDist = p9 - p1;
				double dist = linalg::length(vectorDist);
				double toAdd = (dist * scale) - dist;
				toAdd /= 2.0;

				linalg::vec<double, 2> norm = linalg::normalize(vectorDist);
				norm *= toAdd;
				p1 -= norm;
				p9 += norm;
			}
		}

		void Translate(double dx, double dy)
		{
			if (!initedWithSize) return; // image not loaded yet

			p1 += linalg::vec<double, 2>(dx, dy);
			p7 += linalg::vec<double, 2>(dx, dy);
			p3 += linalg::vec<double, 2>(dx, dy);
			p9 += linalg::vec<double, 2>(dx, dy);

			center += linalg::vec<double, 2>(dx, dy);
		}

		void Extract(double& rotation, double& scale, double& translationX, double& translationY)
		{
			assert(initedWithSize);

			auto axis = p3 - p1;
			auto ref = linalg::vec<double, 2>(1, 0);
			rotation = linalg::angle(axis, ref);

			if (rotation == NAN || rotation == -NAN)
				assert(false);

			if (p3.y < p1.y)
				rotation = -rotation;

			cRect tmp = *this;
			tmp.RotateAroundCenter(-rotation);

			double diagonal = linalg::length(tmp.p9 - tmp.p1);

			scale = diagonal / diagonalRef;
			tmp.ScaleOnCenter(1.0 / scale);

			translationX = tmp.p1.x;
			translationY = tmp.p1.y;
		}
};

public: // protected
#if defined(CUICUI_IOS)
#ifdef WIMAGE_IOS_WITHDRAWRECT
	cCuiImageView *cuiImageView;
#else
    CuiUIImageView *uiImageView;
#endif
#endif
#if defined(WINAPI_FAMILY)
	Image ^im;
	BitmapImage^ bitmapImage = nullptr;
#endif
#if defined(CUICUI_ANDROID)
	int imageTheId;
#endif
#if  defined(__EMSCRIPTEN__)
	int imageTheId;
#endif

	std::string source; // url or filepath
    bool sourceSet=false;
	cRect re;
	cRect reInit;

	double currentAngle =0;
	int imagePhysicalWidthInPixels=-1, imagePhysicalHeightInPixels=-1;
	bool isAnimatedImage = false; // is animated gif (WARNING: valid ONLY after going through OnLoadSuccess) // TODO: it would probably be good to receive this info in the onLoadSucess, no? that way the user knows when the info is valid (since it's asynchronous)

protected:
	std::function<void(wImage *self)> onLoadSuccess = nullptr;
	std::function<void(wImage *self)> onLoadFail = nullptr;

public:
	enum class AutoResize : unsigned char {
		Contain = 0, // default
		Cover = 1,
		//Fill = 2, // warning: this will distord the image  (NOT AVAILABLE)
		None = 3, // use this one if you want to use Transform_...() methods (this prevents cuicui to auto fit the image when layout changes)
	};
	// DEV warning: never change these values (or modify web and android code accordingly)
	
	AutoResize resizeMode;

protected:
	enum class eLoadState : unsigned char {
		Success=0,
		Fail=1,
		None=2,
		Loading=3,
	};
	eLoadState loadState = eLoadState::None;


public: //protected:
	void LoadImageCompleted_Private(int state, int imageWidth, int imageHeight);

public:
	wImage(AutoResize autoResize);
	void Transform_Reset();
	void Transform_Translate(double x, double y, bool usePixelDensity=true);
	void Transform_Scale(double s); 	
	void Transform_ScaleOnPoint(double s, double pointX, double pointY);
	void Transform_Rotate(double radians); // RADIANS!
	void Transform_FitContain(double *outFinalWidth=NULL, double *outFinalHeight=NULL); // make the image fit entirely in the space without deformations (note: this cancels every previous transforms)
	void Transform_FitCover(); // make the image fill the space without deformations  (note: this cancels every previous transforms) 
	double Transform_GetFinalWidth() { return re.p9.x - re.p7.x; }
	double Transform_GetFinalHeight() { return re.p9.y - re.p3.y; }
	wImage* SourceFromBase64(std::string base64, std::string mimeType);
	wImage* Source(std::string str);
    void OnLoadSuccess(std::function<void(wImage *self)> f); // note: it's actually called when the image is loaded AND layouted
    void OnLoadFail(std::function<void(wImage *self)> f); // note: it's actually called when the image is loaded AND layouted

	wImage* Grayscale(); // WEB only for now -> deprecated in favor of TintColor ? if we know how to do tint on web, yes :)
	wImage* Tint(const cColor& col); // ANDROID and IOS only 

	void Animated_Play(); // for animated image (gif) : no effect if the image is not animated; WARNING: UWP only
	void Animated_Stop(); // for animated image (gif) : no effect if the image is not animated; WARNING: UWP only
	/* we don't handle MIN on images (see pb_657)
	virtual bool ComputeMinSize(double *w, double *h)
	{
		assert(!(ax[X].realEm == MIN && ax[Y].realEm == MIN)); // wImage can only have one MIN
		if (ax[X].realEm == MIN)
		{
			assert(false); // NYI
			return true;
		}
		if (ax[Y].realEm == MIN)
		{

#ifdef CUICUI_UWP

#else
			CUICUI_NYI
#endif
			return true;
		}
		return false;
	}*/

protected:
	void InitMatrix();
	void ApplyMatrix();

public:
	void ApplyMatrix_HW();

public:		
	static wImage *Create(Length xem, Length yem, wImage::AutoResize rm = wImage::AutoResize::Contain)
	{
		assert(xem.IsMin()==false && yem.IsMin()==false); // wImage cannot use MIN (see pb_657)
		auto c = new wImage(rm);
		c->Xem(xem)->Yem(yem);
		return c;
	}
};

} // namespace cui