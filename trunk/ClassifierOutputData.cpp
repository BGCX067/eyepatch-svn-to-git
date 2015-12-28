#include "precomp.h"
#include "ClassifierOutputData.h"


ClassifierOutputData::ClassifierOutputData() {
}

ClassifierOutputData::~ClassifierOutputData() {
}

void ClassifierOutputData::AddVariable(ClassifierOutputVariable var) {
	if (HasVariable(var.GetName())) {	// this variable already exists
		SetVariable(var);
	} else {
		data.push_back(var);
	}
}

void ClassifierOutputData::AddVariable(string name, int value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

void ClassifierOutputData::AddVariable(string name, float value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

void ClassifierOutputData::AddVariable(string name, Point value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

void ClassifierOutputData::AddVariable(string name, string value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

void ClassifierOutputData::AddVariable(string name, IplImage* value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

void ClassifierOutputData::AddVariable(string name, CvSeq* value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

void ClassifierOutputData::AddVariable(string name, vector<Rect>* value, bool state) {
	ClassifierOutputVariable var(name, value, state);
	AddVariable(var);
}

ClassifierOutputVariable ClassifierOutputData::GetVariable(string name) {
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			return var;
		}
	}
	ClassifierOutputVariable error;
	return error;
}

void ClassifierOutputData::SetVariable(ClassifierOutputVariable newvar) {
	string name = newvar.GetName();
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			assert(newvar.GetType() == var.GetType());
			// preserve the old state; this should only be changed with the SetVariableState function
			newvar.SetState(var.GetState());
			data[i] = newvar;
		}
	}
}

bool ClassifierOutputData::GetVariableState(string name) {
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			return var.GetState();
		}
	}
	return false;
}

void ClassifierOutputData::SetVariableState(string name, bool state) {
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			var.SetState(state);
			data[i] = var;
		}
	}
}

bool ClassifierOutputData::HasVariable(string name) {
	for (int i=0; i<data.size(); i++) {
		ClassifierOutputVariable var = data[i];
		if (name.compare(var.GetName()) == 0) {
			return true;
		}
	}
	return false;
}

int ClassifierOutputData::GetIntData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetIntData();
}

float ClassifierOutputData::GetFloatData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetFloatData();
}

Point ClassifierOutputData::GetPointData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetPointData();
}

string ClassifierOutputData::GetStringData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetStringData();
}

IplImage* ClassifierOutputData::GetImageData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetImageData();
}

CvSeq* ClassifierOutputData::GetSequenceData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetSequenceData();
}

vector<Rect>* ClassifierOutputData::GetBoundingBoxData(string name) {
	ClassifierOutputVariable var = GetVariable(name);
	return var.GetBoundingBoxData();
}


void ClassifierOutputData::SetVariable(string name, int value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

void ClassifierOutputData::SetVariable(string name, float value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

void ClassifierOutputData::SetVariable(string name, Point value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

void ClassifierOutputData::SetVariable(string name, string value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

void ClassifierOutputData::SetVariable(string name, IplImage* value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

void ClassifierOutputData::SetVariable(string name, CvSeq* value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

void ClassifierOutputData::SetVariable(string name, vector<Rect>* value) {
	ClassifierOutputVariable var(name, value);
	SetVariable(var);
}

int ClassifierOutputData::NumVariables() {
	return data.size();
}

bool ClassifierOutputData::GetStateOfIndex(int i) {
	return data[i].GetState();
}

string ClassifierOutputData::GetNameOfIndex(int i) {
	return data[i].GetName();
}

ClassifierVariableType ClassifierOutputData::GetTypeOfIndex(int i) {
	return data[i].GetType();
}

void ClassifierOutputData::MergeWith(ClassifierOutputData mergeData) {
	for (int i=0; i<mergeData.data.size(); i++) {
		ClassifierOutputVariable var = mergeData.data[i];
		if (this->HasVariable(var.GetName())) {	// this variable is already present, so we'll overwrite it
			this->SetVariable(var);
		} else {	// it's missing, so we'll add it
			this->AddVariable(var);
		}
	}
}