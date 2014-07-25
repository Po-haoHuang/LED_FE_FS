#include"../include/Lars.h"

Lars_Regression::Lars_Regression(){
	//c_tor
}
Lars_Regression::~Lars_Regression(){
	//d_tor
}
bool Lars_Regression::useLarsRegression(arma::mat& data,
					   arma::vec& responses,
					   double lamda1,
					   double lamda2,
					   vector<int>& result){
	vec beta;
	  // Make sure y is oriented the right way.
	//cout<<responses.n_elem<<","<<data.n_rows;
	if (responses.n_elem != data.n_cols)
		Log::Fatal << "Number of responses must be equal to number of rows of X!"
			<< endl;
	LARS lars(false, lamda1, lamda2);
	lars.Regress(data, responses, beta, true );
	#ifdef Detail_Output
	cout<<beta;
	#endif
	uvec temp = sort_index(beta,"d");
	for(int i = 0;i < data.n_rows ;i++)
		result.push_back(temp[i]);
	return 0;
}
