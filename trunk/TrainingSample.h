#pragma once

class TrainingSample
{
public:
    IplImage *fullImageCopy, *motionHistory;
    LVITEM lvi;
    HBITMAP hbmImage;
    UINT id;
    int iGroupId, iOrigId;
	Rect selectBounds;
    MotionTrack motionTrack;

    TrainingSample(IplImage* srcImage, HWND listControl, HIMAGELIST imageList, Rect selectBounds, int groupId);
    TrainingSample(IplImage* srcImage, IplImage* motionHistory, HWND listControl, HIMAGELIST imageList, Rect selectBounds, int groupId);
    TrainingSample(char *filename, HWND listControl, HIMAGELIST imageList, int groupId);
    TrainingSample(IplImage*, MotionTrack mt, HWND listControl, HIMAGELIST imageList, int groupId);
	TrainingSample(TrainingSample *toClone);
    ~TrainingSample(void);
    void Draw(Graphics*, int x, int y);
	void Save(WCHAR *directory, int index);

private:
    IplImage *resizedImage;
    Bitmap *bmpImage;
    int width, height;
    HWND hwndListControl;
    HIMAGELIST hImageList;
};
