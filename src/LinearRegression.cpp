#include"..\include\LinearRegression.h"

Linear_Regression::Linear_Regression(){
	//c_tor
}
Linear_Regression::~Linear_Regression(){
	//d_tor
}

bool Linear_Regression::useLinearRegression(arma::mat& data,
								  			arma::vec& responses,
								  			double lambda,
  						  					vector<int>& result,
											vector<double>& cof){

	//check if x , y matches
	if (responses.n_elem != data.n_cols)
		Log::Fatal << "Number of responses must be equal to number of rows of X!"
		<< endl;
	//usage of linear regression(Lambda = 0)
	//and ridge regression(set lambda = 1 or 2 or 3)
	lr = LinearRegression(data,responses,lambda);
	vec tempPara = lr.Parameters();
	tempPara.shed_row(0);//remove the B0 of linear regression(constant beta 0)
	cof = conv_to< std::vector<double> >::from(sort(tempPara,"d"));
	//sort the beta coefficient
	uvec temp = sort_index(tempPara,"d");
	#ifdef Detail_Output
	cout<<tempPara;
	data::Save("LinearCof.csv",tempPara,true);
	#endif
	for(int i = 0;i < data.n_rows ;i++)
		result.push_back(temp[i]);

	return 0;
}
