#include "../include/Regression.h"
#include"../include/LinearRegression.h"
#include"../include/Lars.h"
Regression::Regression(){
	
}

Regression::~Regression(){
	
}
bool Regression::init(vector<vector<double > > input,vector<double > response ){

	//convert vector to vec
	response_mod = vec(response);
	//vector<vector> to mat make it 1-D vector
	//and initialize a mat, reshape to 2-D
	vector<double> temp;
	temp.reserve(input.size()*input[0].size());
	for(int i = 0;i < input.size();i++)
		temp.insert(temp.end(), input[i].begin(), input[i].end());
	input_mod = mat(temp);
	input_mod.reshape(input[0].size(),input.size());
	//normalization
	response_mod -= mean(response_mod);
	response_mod /= stddev(response_mod);
	for(int i = 0;i < input_mod.n_rows;i++){
		input_mod.row(i) -= mean(input_mod.row(i));
		input_mod.row(i) /= stddev(input_mod.row(i));
	}
	#ifdef Output_Normalize
	data::Save("ouputnormal.csv",input_mod,true);
	data::Save("ouputnormalres.csv",response_mod,true);
	#endif

	return 0;
	
}
bool Regression::doLinearRegression(int lambda,vector<int >& result){
	Linear_Regression lr;
	lr.useLinearRegression(input_mod,response_mod,lambda,result);
}

bool Regression::doLarsRegression(int lambda1 ,int lambda2,vector<int >& result){
	Lars_Regression lsr;
	lsr.useLarsRegression(input_mod,response_mod,lambda1,lambda2,result);
}
