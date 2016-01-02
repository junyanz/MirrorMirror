#include "SVMSolver.h"

void print_null(const char *s) {}

CSVMSolver::CSVMSolver() {
	Clear(); 
    Init();
	svm_set_print_string_function(&print_null); 
}

CSVMSolver::~CSVMSolver() {
	svm_destroy_param(m_param); 
	if (m_model != NULL)
		svm_free_model_content(m_model); 
	Clear(); 
}

void CSVMSolver::Clear() {
	m_param = NULL; 
	m_model = NULL; 
}

void CSVMSolver::Init() {
	m_param = new svm_parameter();
	m_param->probability = 1; 
    m_param->svm_type = EPSILON_SVR;
    m_param->kernel_type = 0;
    m_param->C = 0.1;
	m_param->shrinking = 0; 
	m_param->cache_size = 1000; 
    m_param->eps = 1e-3;
    m_param->p = 0.1;
	m_param->coef0 = 0; 
}

void CSVMSolver::Train( vector<float*> _data, vectord _score, int _ndims ) {
	// set data
	svm_problem* prob = SetData(_data, _score, _ndims); 
	CheckParam(prob, m_param); 
	svm_model* model = svm_train(prob, m_param); 
    DEBUG_INFO("saving model (%s)", m_modelPath.c_str());
	svm_save_model(m_modelPath.c_str(), model); 
	svm_free_model_content(model); 
	DELETE_OBJECT(prob); 
}

vectord CSVMSolver::Predict( vector<float*> _data, int _ndims ) {
	if (m_model == NULL)
		LoadModel(); 
	
	int nData = (int)_data.size(); 
	vectord score(nData, 0.0f); 
	svm_problem* prob = SetData(_data, score, _ndims);
	CheckParam(prob, m_param); 
	FOR (i, nData)
		score[i] = (float)svm_predict(m_model, prob->x[i]); 
	//svm_free_model_content(m_model); 
	DELETE_OBJECT(prob); 
	//score.clear(); 
	return score; 
}

svm_problem* CSVMSolver::SetData( vector<float*> _data, vectord _score, int _ndims ) {
	svm_problem* prob = new svm_problem(); 
	prob->l = (int)_data.size(); 
	int nData = (int)_data.size(); 
	double* y = new double[nData]; 
	svm_node** x = new svm_node*[nData]; 
	FOR (i, nData)
		x[i] = new svm_node[_ndims+1];

	FOR (i, nData) {
		float* f = _data[i]; 
		y[i] = (double)_score[i];

		FOR (j, _ndims) { 
			svm_node node; 
			node.index = j+1; 
			node.value = (double)f[j]; 
			x[i][j] = node; 
		}

		svm_node end_node; 
		end_node.index = -1; 
		end_node.value = 0.0; 
		x[i][_ndims] = end_node;
	}

	prob->y = y; 
	prob->x = x; 
	return prob; 
}

void CSVMSolver::CheckParam( svm_problem* _data, svm_parameter* _param ) {
	if (svm_check_parameter(_data, _param) != NULL)
		cout << svm_check_parameter(_data, _param) << endl; 
}

void CSVMSolver::LoadModel() {
	m_model = svm_load_model(m_modelPath.c_str()); 
	if (m_model == NULL)
		DEBUG_ERROR("Fail to load the model.");
}

void CSVMSolver::TestSVM() {
	int nData = 5; 
	int ndims = 6; 
	vector<float*> data(nData, NULL);
	vectord score(nData, 0.0f); 
	FOR (i, nData) {
		float* f = new float[ndims];
		FOR (j, 6) {
			f[j] = 1000* rand() / (float)INT_MAX;
			cout << f[j] << " "; 
		}
		cout << endl; 
		data[i] = f; 
		score[i] = 1000*rand() / (double)INT_MAX; 
	}
	//Train(data, score, ndims); 
	svm_problem* prob = SetData(data, score, ndims); 
	CheckParam(prob, m_param); 
	svm_model* model = svm_train(prob, m_param); 
	//svm_save_model(m_modelPath.c_str(), model); 
	svm_free_model_content(model); 
	DELETE_OBJECT(prob); 
}

void CSVMSolver::CrossValidation( vector<float*> _data, vectord _score, int _ndims, int _nFold) {
	svm_problem* prob = SetData(_data, _score, _ndims); 
	CheckParam(prob, m_param); 
	int nData = (int)_data.size(); 
	double* pred = new double[nData]; 
	svm_cross_validation(prob, m_param, _nFold, pred); 
	//SHOW_ARRAY(tgt, nData); 
	double sum = 0.0f; 
	double* y = prob->y; 
	FOR (i, nData)
		sum += (pred[i] - y[i]) * (pred[i] - y[i]); 
	sum /= nData; 
	DEBUG_INFO("(%d) cross validation. Square errors %f = %f /^ 2.", _nFold, sum, sqrt(sum)); 
	DELETE_OBJECT(prob);
	DELETE_ARRAY(pred); 
}

double* CSVMSolver::GetW() {
	if (m_model == NULL)
		LoadModel(); 
	int nSV = svm_get_nr_sv(m_model);
	int ndims = 0;
	svm_node* sv = m_model->SV[0]; 
	while (1) {
		if (sv->index != -1) {
			sv++; 
			ndims++; 
		} else
			break; 
	}

	double* w = new double[ndims]; 
	cout << "#support vectors = " << nSV << endl; 
	FOR (i, ndims) {
		w[i] = 0; 
		FOR (j, nSV)
			w[i] += m_model->SV[j][i].value * m_model->sv_coef[0][j]; 			
	}
	return w; 
}

double CSVMSolver::GetB() {
	if (m_model == NULL)
		LoadModel(); 
	return m_model->rho[0]; 
}
