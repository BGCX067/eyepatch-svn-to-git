#pragma once

typedef enum ClassifierVariableType {
	CVAR_VOID = 0,
    CVAR_INT,
    CVAR_FLOAT,
	CVAR_POINT,
    CVAR_STRING,
	CVAR_IMAGE,
	CVAR_SEQ,
	CVAR_BBOXES
} ClassifierVariableType;

class ClassifierOutputVariable {
public:
	ClassifierOutputVariable() {
		m_name = "ERROR";
		m_type = CVAR_VOID;
		m_state = false;
	}
	ClassifierOutputVariable(string name, int data, bool state=true) {
		m_name = name;
		m_type = CVAR_INT;
		m_intdata = data;
		m_state = state;
	}
	ClassifierOutputVariable(string name, float data, bool state=true) {
		m_name = name;
		m_type = CVAR_FLOAT;
		m_floatdata = data;
		m_state = state;
	}
	ClassifierOutputVariable(string name, Point data, bool state=true) {
		m_name = name;
		m_type = CVAR_POINT;
		m_pointdata = data;
		m_state = state;
	}
	ClassifierOutputVariable(string name, string data, bool state=true) {
		m_name = name;
		m_type = CVAR_STRING;
		m_stringdata = data;
		m_state = state;
	}
	ClassifierOutputVariable(string name, IplImage *data, bool state=true) {
		m_name = name;
		m_type = CVAR_IMAGE;
		m_imagedata = data;
		m_state = state;
	}
	ClassifierOutputVariable(string name, CvSeq *data, bool state=true) {
		m_name = name;
		m_type = CVAR_SEQ;
		m_sequencedata = data;
		m_state = state;
	}
	ClassifierOutputVariable(string name, vector<Rect> *data, bool state=true) {
		m_name = name;
		m_type = CVAR_BBOXES;
		m_bboxdata = data;
		m_state = state;
	}

	~ClassifierOutputVariable() { }
	string GetName() { return m_name; }
	ClassifierVariableType GetType() { return m_type; }
	bool GetState() { return m_state; }
	void SetState(bool state) { m_state = state; }

	int GetIntData() { assert(m_type==CVAR_INT);	return m_intdata; }
	float GetFloatData() { assert(m_type==CVAR_FLOAT);	return m_floatdata; }
	Point GetPointData() { assert(m_type==CVAR_POINT);	return m_pointdata; }
	string GetStringData() { assert(m_type==CVAR_STRING);	return m_stringdata; }
	IplImage* GetImageData() { assert(m_type==CVAR_IMAGE);	return m_imagedata; }
	CvSeq* GetSequenceData() { assert(m_type==CVAR_SEQ);	return m_sequencedata; }
	vector<Rect>* GetBoundingBoxData() { assert(m_type==CVAR_BBOXES);	return m_bboxdata; }

private:
	string m_name;
	ClassifierVariableType m_type;
	bool m_state;
	int m_intdata;
	float m_floatdata;
	Point m_pointdata;
	string m_stringdata;
	IplImage *m_imagedata;
	CvSeq* m_sequencedata;
	vector<Rect>* m_bboxdata;
};

class ClassifierOutputData {
public:
	ClassifierOutputData();
	~ClassifierOutputData();

	void AddVariable(ClassifierOutputVariable var);
	void AddVariable(string name, int value, bool state=true);
	void AddVariable(string name, float value, bool state=true);
	void AddVariable(string name, Point value, bool state=true);
	void AddVariable(string name, string value, bool state=true);
	void AddVariable(string name, IplImage* value, bool state=true);
	void AddVariable(string name, CvSeq* value, bool state=true);
	void AddVariable(string name, vector<Rect>* value, bool state=true);

	void SetVariable(ClassifierOutputVariable var);
	void SetVariable(string name, int value);
	void SetVariable(string name, float value);
	void SetVariable(string name, Point value);
	void SetVariable(string name, string value);
	void SetVariable(string name, IplImage* value);
	void SetVariable(string name, CvSeq* value);
	void SetVariable(string name, vector<Rect>* value);

	void SetVariableState(string name, bool state);
	bool GetVariableState(string name);

	void MergeWith(ClassifierOutputData mergeData);

	ClassifierOutputVariable GetVariable(string name);
	int GetIntData(string name);
	float GetFloatData(string name);
	Point GetPointData(string name);
	string GetStringData(string name);
	IplImage* GetImageData(string name);
	CvSeq* GetSequenceData(string name);
	vector<Rect>* GetBoundingBoxData(string name);

	bool HasVariable(string name);
	int NumVariables();
	bool GetStateOfIndex(int index);
	string GetNameOfIndex(int index);
	ClassifierVariableType GetTypeOfIndex(int index);

	vector<ClassifierOutputVariable> data;

private:
};
