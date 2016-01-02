#ifndef PARAMETER_H_
#define PARAMETER_H_
#include "common.h"

enum EWorkMode {
	WORKMODE_TRAIN, 
	WORKMODE_UI,
};


class CParameter {
public:
    CParameter(int _argc, char* _argv[]);
	~CParameter();
    void PrintHelp();
    void CheckParams();
    void PrintParams();
    void ParseCommandLine(int _argc, char* _argv[]);
	// get functions 
	string DataDir() const { return m_dataDir; }
    string VideoFile() const { return m_videoFile; }
	int MaxNumFrames() const { return m_maxNumFrames; }
	int NumFrameAlign() const {return m_numFrameAlign; }
	int NumFrameH() const {return m_numFrameH; }
	int WaitTime() const { return m_waitTime; }
	string SaveDir() const {return m_saveDir; }
    string ModelFile() const { return m_modelFile; }
	EWorkMode  WorkMode() const { return m_workMode; }

    bool IsVerbose() const {return m_isVerbose; }
	vectord Ratios() const {return m_ratios;}

private:
	void Clear(); 
    void PrintLine();
private: 
	// directory
	string m_dataDir;
	string m_saveDir; 
    string m_videoFile;
    string m_modelFile;
	// parameters
	int m_maxNumFrames; 
	int m_waitTime;
	int m_numFrameAlign;
	int m_numFrameH;
	// mode 
	EWorkMode m_workMode; 
	//flag
    bool m_isVerbose;
    bool m_isDebug;
	// ratios
	vectord m_ratios;
};
#endif //PARAMETERS_H_
