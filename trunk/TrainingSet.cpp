#include "precomp.h"
#include "constants.h"
#include "TrainingSample.h"
#include "TrainingSet.h"

TrainingSet::TrainingSet(void) {
    posSampleCount = 0;
    negSampleCount = 0;
	motionSampleCount = 0;
    rangeSampleCount = 0;
}

TrainingSet::~TrainingSet(void) {
	ClearSamples();
}

HIMAGELIST TrainingSet::GetImageList() {
    // TODO: store and maintain imagelist here instead of in videomarkup?
    return NULL;
}

void TrainingSet::AddSample(TrainingSample *sample) {
    if (sample->iGroupId == GROUPID_POSSAMPLES) posSampleCount++;
    else if (sample->iGroupId == GROUPID_NEGSAMPLES) negSampleCount++;
    else if (sample->iGroupId == GROUPID_MOTIONSAMPLES) motionSampleCount++;
    else if (sample->iGroupId == GROUPID_RANGESAMPLES) rangeSampleCount++;
    sampleMap[sample->id] = sample;
}

void  TrainingSet::SetSampleGroup(UINT sampleId, int groupId) {
    map<UINT, TrainingSample*>::iterator i = sampleMap.find(sampleId);
    if (i != sampleMap.end()) {
        TrainingSample *sample = i->second;

        // reduce count of old group id
        int oldGroupId = sample->iGroupId;
        if (oldGroupId == GROUPID_POSSAMPLES) posSampleCount--;
        else if (oldGroupId == GROUPID_NEGSAMPLES) negSampleCount--;
        else if (oldGroupId == GROUPID_MOTIONSAMPLES) motionSampleCount--;
        else if (oldGroupId == GROUPID_RANGESAMPLES) rangeSampleCount--;

        // increase count of new group id
        if (groupId == GROUPID_POSSAMPLES) posSampleCount++;
        else if (groupId == GROUPID_NEGSAMPLES) negSampleCount++;
        else if (groupId == GROUPID_MOTIONSAMPLES) motionSampleCount++;
        else if (groupId == GROUPID_RANGESAMPLES) rangeSampleCount++;

        // set new group id
        sample->iGroupId = groupId;
    }
}

int TrainingSet::GetOriginalSampleGroup(UINT sampleId) {
    map<UINT, TrainingSample*>::iterator i = sampleMap.find(sampleId);
    if (i != sampleMap.end()) {
        TrainingSample *sample = i->second;
		return sample->iOrigId;
	}
	return -1;
}

void TrainingSet::RemoveSample(UINT sampleId) {
    map<UINT, TrainingSample*>::iterator i = sampleMap.find(sampleId);
    if (i != sampleMap.end()) {
        TrainingSample *sample = i->second;
        delete sample;
        sampleMap.erase(i);
    }
}

void TrainingSet::CopyTo(TrainingSet *target) {
	target->ClearSamples();
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = i->second;
		TrainingSample *sampleCopy = new TrainingSample(sample);
		target->AddSample(sampleCopy);
	}
}

void TrainingSet::ClearSamples() {
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        delete sample;
    }
    sampleMap.clear();
}

void TrainingSet::Save(WCHAR *directory) {
	int posindex = 0, negindex = 0, motindex = 0, rngindex = 0;
	// Write out all the samples
	for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
		TrainingSample *sample = (*i).second;
	    if (sample->iGroupId == GROUPID_POSSAMPLES) { // positive sample
			sample->Save(directory, posindex++);
		} else if (sample->iGroupId == GROUPID_NEGSAMPLES) { // negative sample
			sample->Save(directory, negindex++);
		} else if (sample->iGroupId == GROUPID_MOTIONSAMPLES) { // negative sample
			sample->Save(directory, motindex++);
		} else if (sample->iGroupId == GROUPID_RANGESAMPLES) { // range sample
			sample->Save(directory, rngindex++);
		}
    }
}

void TrainingSet::ShowSamples() {
    cvNamedWindow("Samples");

    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 0.7, 0.7, 0, 1, 8);

    // draw the positive samples
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_POSSAMPLES) {
            IplImage *copy = cvCloneImage(sample->fullImageCopy);
            cvPutText(copy, "Positive Sample", cvPoint(10,10), &font, CV_RGB(0,255,0));
            cvShowImage("Samples", copy);
            cvReleaseImage(&copy);
            cvWaitKey(0);
        }
    }

    // draw the negative samples
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_NEGSAMPLES) {
            IplImage *copy = cvCloneImage(sample->fullImageCopy);
            cvPutText(copy, "Negative Sample", cvPoint(10,10), &font, CV_RGB(255,0,0));
            cvShowImage("Samples", copy);
            cvReleaseImage(&copy);
            cvWaitKey(0);
        }
    }

    // draw the trash
    for (map<UINT, TrainingSample*>::iterator i = sampleMap.begin(); i != sampleMap.end(); i++) {
        TrainingSample *sample = (*i).second;
        if (sample->iGroupId == GROUPID_TRASH) {
            IplImage *copy = cvCloneImage(sample->fullImageCopy);
            cvPutText(copy, "Trash", cvPoint(10,10), &font, CV_RGB(150,150,150));
            cvShowImage("Samples", copy);
            cvReleaseImage(&copy);
            cvWaitKey(0);
        }
    }

    cvDestroyWindow("Samples");
}