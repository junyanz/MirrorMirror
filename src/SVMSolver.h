#ifndef SVM_SOLVER_H_
#define SVM_SOLVER_H_
#include "common.h"
#include "../svm/svm.h"

class CSVMSolver {
public: 
    CSVMSolver();
	~CSVMSolver();
	void Train(vector<float*> _data, vectord _score, int _ndims); 
	void SetModelPath(string _modelPath) { m_modelPath = _modelPath; }
	vectord Predict(vector<float*> _data, int _ndims); 
	void LoadModel(); 
	void CrossValidation(vector<float*> _data, vectord _score, int _ndims, int _nFold); 
	double* GetW(); 
	double GetB();

private: 
	void Clear(); 
    void Init();
	svm_problem* SetData(vector<float*> _data, vectord _score, int _ndims); 
	void CheckParam(svm_problem* _data, svm_parameter* _param); 
	void TestSVM(); 

private: 
	// svm 
	svm_parameter* m_param; 
	svm_model* m_model; 
	// dir
	string m_modelPath; 
};

#endif //SVM_SOLVER_H_
