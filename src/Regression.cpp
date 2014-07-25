#include "../include/Regression.h"
#include"../include/LinearRegression.h"
#include"../include/Lars.h"
Regression::Regression(){
	
}

Regression::~Regression(){
	
}
bool Regression::init(vector<vector<double > > input,vector<double > response ){
	response_mod = vec(response);
	vector<double> temp;
	temp.reserve(input.size()*input[0].size());
	for(int i = 0;i < input.size();i++)
		temp.insert(temp.end(), input[i].begin(), input[i].end());
	input_mod = mat(temp);
	input_mod.reshape(input[0].size(),input.size());
	return 0;
	
}
bool Regression::doLinearRegression(int lamda,vector<int >& result){
	Linear_Regression lr;
	lr.useLinearRegression(input_mod,response_mod,lamda,result);
}

bool Regression::doLarsRegression(int lamda1 ,int lamda2,vector<int >& result){
	Lars_Regression lsr;
	lsr.useLarsRegression(input_mod,response_mod,lamda1,lamda2,result);
}
