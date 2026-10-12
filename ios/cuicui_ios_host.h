#pragma once

// What cuicui expects from the iOS app that hosts it.
//
// The app's root view controller (the one that calls central.Init() in viewDidLoad) must adopt
// this protocol: cuicui calls these methods to show the system pickers, the camera and to change
// the status bar style. See the iOS project of cuicui_project for a complete implementation.

#import <UIKit/UIKit.h>

@protocol CuiHostViewController <NSObject>
- (void)openPickerImages;      // photo library picker; the result goes to cui::onFileReceived
- (void)openPickerDocuments;   // document picker
- (void)openCamera;            // camera; the result goes to cui::onFileReceived
- (void)setCurrentStatusBarStyle:(UIStatusBarStyle)style;
@end
