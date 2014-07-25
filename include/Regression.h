#ifndef Regression_H //Linear Regression
#define Regression_H
#include <mlpack/core.hpp>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include"../include/LinearRegression.h"
#include"../include/Lars.h"


using namespace mlpack;
using namespace mlpack::regression;
using namespace std;
using namespace arma;

class Regression{

	public:
		Regression();
		virtual ~Regression();
		bool init(vector<vector<double > > input,vector<double > response );
		bool doLinearRegression(int lamda ,vector<int >& result);
		bool doLarsRegression(int lamda1 ,int lamda2,vector<int >& result);
		

	private:
		mat input_mod;
		vec response_mod;

		




};


#endif





