#pragma once

#define _WIN32_WINNT 0x0600
#define WINVER 0x0600
#define _WIN32_IE 0x0700
#define UNICODE
#define _CRT_SECURE_NO_WARNINGS
#define __MSW32__

// Windows includes
#include <winsock2.h>   // this must come first to prevent errors with MSVC7
#include <windows.h>
#include <windowsx.h>
#include <shfolder.h>
#include <shlobj.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// standard includes
#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <io.h>
#include <time.h>

#include <gdiplus.h>
using namespace Gdiplus;

// STL includes
#include <list>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

// ATL includes
#include <atlbase.h>
#include <atltypes.h>
#include <atlfile.h>
#include <atlwin.h>

// OpenCV Includes
#include "cv.h"
#include "highgui.h"
#include "cvaux.h"

// Haar Training Includes
#include "CVHaar/_cvcommon.h"
#include "CVHaar/_cvhaartraining.h"
#include "CVHaar/cvhaartraining.h"
#include "CVHaar/cvclassifier.h"

// SIFT includes
#include "SIFT/sift.h"
#include "SIFT/imgfeatures.h"
#include "SIFT/kdtree.h"
#include "SIFT/utils.h"
#include "SIFT/xform.h"

// Gesture Tracking includes
#include "Gesture/OneDollar.h"
typedef vector<OneDollarPoint> MotionTrack;
#include "Gesture/FlowTracker.h"

// Tesseract OCR includes
#include "tessdll.h"

// Utility functions
void CopyImageToClipboard (IplImage* img);
void IplToBitmap(IplImage *src, Bitmap *dst);
CvScalar hsv2rgb(float hue);
void DrawArrow(IplImage *img, CvPoint center, double angleDegrees, double magnitude, CvScalar color, int thickness=1);
void DrawTrack(IplImage *img, MotionTrack mt, CvScalar color, int thickness, float squareSize, int maxPointsToDraw=0);
void DrawTrack(Graphics *graphics, MotionTrack mt, float width, float height, float squareSize);
bool DeleteDirectory(LPCTSTR lpszDir, bool useRecycleBin);
void SaveTrackToFile(MotionTrack mt, WCHAR *filename);
MotionTrack ReadTrackFromFile (WCHAR* filename);

// swatch of "nice" colors
#define COLOR_SWATCH_SIZE 16
extern CvScalar colorSwatch[];
extern WCHAR *filterNames[];

