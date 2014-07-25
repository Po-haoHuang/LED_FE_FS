#include"..\include\LinearRegression.h"

Linear_Regression::Linear_Regression(){
	//c_tor
}
Linear_Regression::~Linear_Regression(){
	//d_tor
}

bool Linear_Regression::useLinearRegression(arma::mat& data,
								  			arma::vec& responses,
								  			double lamda,
  						  					vector<int>& result){


	if (responses.n_elem != data.n_cols)
		Log::Fatal << "Number of responses must be equal to number of rows of X!"
		<< endl;
	lr = LinearRegression(data,responses,lamda);
	vec tempPara = lr.Parameters();
	tempPara.shed_row(tempPara.n_rows-1);
	uvec temp = sort_index(tempPara,"d");
	#ifdef Detail_Output
	cout<<tempPara;
	#endif
	for(int i = 0;i < data.n_rows ;i++)
		result.push_back(temp[i]);

	return 0;
}
