#include <iostream>
#include <vector>
#include <math.h>
#include <fstream>
#include <string>
using namespace std;

void printMatrix(vector<vector<double>> matrix) {
	cout << endl << "[" << endl;
	for (auto i = matrix.begin(); i < matrix.end(); ++i) {
		for (auto k = (*i).begin(); k < (*i).end(); ++k) {
			cout << "" << *k << "  ";
		}
		cout << endl;
	}
	cout << "]" << endl;
}

void printVector(vector<double> vec) {
	cout << "[  ";
	for (auto i = vec.begin(); i < vec.end(); ++i) {
		cout << "" << (*i) << "  ";
	}
	cout << "]" << endl;
}

void printVector(vector<int> vec) {
	cout << "[  ";
	for (auto i = vec.begin(); i < vec.end(); ++i) {
		cout << "" << (*i) << "  ";
	}
	cout << "]" << endl;
}

double getErrorInput() {
	double error = NAN;
	cout << "Please input your desired maximum error for the solution." << endl;
	string input;
	while (cin >> input) {
		try {error=stod(input);} catch (exception e) {error = NAN;}
		if (error != NAN) break;
	}
	return error;
}

vector<double> getStartGuess(int n) {
	vector<double> guess(n, NAN);
	for (int i = 0; i < n; ++i) {
		cout << "Please input the guess value for x_" << i << endl;
		string input;
		while (cin >> input) {
			try {guess[i]=stod(input);} catch (exception e) {guess[i] = NAN;}
			if (guess[i] != NAN) break;
		}
	}
	return guess;
}

vector<vector<double>> getInput() {
	string in = "";
	//get n
	cout << "Enter the number of equations in the linear system." << endl;
	
	int n = -1;
	while (cin >> in) {
		try { n = stoi(in); } catch(exception e) { n = -1; }
		if (n >= 2)  break;
		cout << "Enter an integer greater than 1." << endl;
		if (cin.fail()) cin.clear();
	}

	//choose console or file for data entry
	cout << "Please enter the coefficients and constants of the linear system." << endl;
	cout << "Enter (1) to use console input, or (2) to enter a filepath for input." << endl;
	while (cin >> in) {
		if (in == "1" || in == "2")  break;
		cout << "Enter either 1 or 2" << endl;
		if (cin.fail()) { cin.clear(); }
	}
	bool console = (in == "1");
	

	//get the matrix for the system
	vector<vector<double>> system;
	vector<string> input;
	if (console) {
		for (int i = 0; i < n; ++i) {
			for (int k = 0; k < n + 1; ++k) {
				if (k == n) cout << "Input the constant for equation " << (i+1) << endl;
				else cout << "Input the coefficient of x_" << k << " in equation " << (i+1) << endl;
				cin >> in;
				input.push_back(in);
			}
		}
	} else {
		ifstream fin;
		cout << "Enter the filepath for you input file." << endl;
		while (cin >> in) {
			try { fin.open(in); } catch(exception e) { cout << "Please input a valid filepath." << endl; }
			if (fin.is_open()) break;
		}

		while (getline(fin, in, '\n')) {
			in += '\n';
			string str = "";
			bool firstDot = true;
			for (auto ci = in.begin(); ci < in.end(); ++ci) {

				if ((48 <= *ci && *ci <= 57) || (*ci == '.' && firstDot)) {
					str += *ci;
					if (*ci == '.') firstDot = false;
				} else {
					bool v = (str != "");
					for (auto si = str.begin(); si < str.end() && v; ++si) {
						if (!(48 <= *si && *si <= 57)) v = false;
					}

					if (v) input.push_back(str);
					str = "";
				}
			}
			bool v = (str != "");
			for (auto si = str.begin(); si < str.end() && v; ++si) {
				if (!(48 <= *si && *si <= 57)) v = false;
			}
			if (v) input.push_back(str);

		}
		fin.close();
	}

	for (int i = 0; i < input.size(); ++i) cout << input[i] << ", ";
	cout << endl;

	int c = 0;
	for (int i = 0; i < n; ++i) {
		vector<double> temp;
		for (int k = 0; k < n + 1; ++k) {
			temp.push_back(stod(input[c]));
			++c;
		}
		system.push_back(temp);
	}

	cout << "INPUT MATRIX: ";
	printMatrix(system);

	return system;
}

double L2N(vector<double> vec) { //L2-Norm
	double sum = 0;
	for (int i = 0; i < vec.size(); ++i) sum += (vec[i] * vec[i]);
	return sqrt(sum);
}

bool checkError(vector<double> vec, double &prev, double error) {
	bool converged = false;
	double curr = L2N(vec);

	if (abs(prev - curr) <= error) converged = true;
	prev = curr;
	return converged;
}

vector<double> jacobi(vector<vector<double>> system, double error, vector<double> guess) {
	//components
	//At system[i][k]
	//U if i < k
	//D if i == k
	//L if i > k
	//x[i](k+1) = (1/a[i][i]) * (b[i] - sum(a[i][j] * x[i](k) exc.: i==j))
	//i=1,2,3,...,n
	//a[i][j]=system[i][j]
	//b[i]   =system[i][n]

	int n = system.size();
	vector<double> x = guess;
	vector<double> prevx;
	double prevL2N = 0;

	int k = 0;

	while(!checkError(x, prevL2N, error) && k < 50) {
		prevx = x; //can skip this at finale
		for (int i = 0; i < x.size(); ++i) {

			double sumsysij = 0;
			for (int j = 0; j < system[i].size(); ++j) {
				if (i == j) continue;
				sumsysij += system[i][j] * prevx[j];
			}
			x[i] = (1 / system[i][i]) * (system[i][n] - sumsysij);
		}

		printVector(x);
		++k;
	}
	return x;
}

vector<double> gauss_seidel(vector<vector<double>> system, double error, vector<double> guess) {
	//equation
	/*
	a = coefficients matrix, b = constants vector
	sum1 = sum of ( a[i][j]*new_x[j] ) from j=1 to j=i-1
	sum2 = sum of ( a[i][j]*old_x[j] ) from j=i+1 to n
	x[i](k+1) = (1 / a[i][i]) * (b[i] - sum1 - sum2)
	in reality, sum1 - sum2 can equal
	sum of ( a[i][j] * x[j] ) from j=1 to n, j != i
	if x[i](k) is overwritten by x[i](k+1) as new value is found
	*/

	int n = system.size();
	vector<double> x = guess;
	double prevL2N = 0;
	int k = 0;

	while(!checkError(x, prevL2N, error) && k < 50) {
		for (int i = 0; i < x.size(); ++i) {

			double sumsysij = 0;
			for (int j = 0; j < system[i].size(); ++j) {
				if (i == j) continue;
				sumsysij += system[i][j] * x[j];
			}
			x[i] = (1 / system[i][i]) * (system[i][n] - sumsysij);
		}

		printVector(x);
		++k;
	}
	return x;

}

int main(int argc, char *argv[]) {
	vector<vector<double>> system = getInput();
	double error = getErrorInput();
	vector<double> startGuess = getStartGuess(system.size());

	bool sdd = true;
	for (int i = 0; i < system.size() && sdd; ++i) {
		for (int k = 0; k < system[i].size() - 1 && sdd; ++k) {
			if (system[i][i] < system[i][k]) {
				cout << "System is not strictly diagonally dominant.\nThis may result in a failure to converge to the correct solution." << endl;
				sdd = false;
			}
		}
	}

	vector<double> solutions = jacobi(system, error, startGuess);

	cout << "Jacobi Solutions( " << solutions.size() << " ):" << endl;
	for (int i = 0; i < solutions.size(); ++i) {
		cout << "x_" << i << "\t=\t" << solutions[i] << endl;
	}

	solutions = gauss_seidel(system, error, startGuess);

	cout << "G-Seidel Solutions( " << solutions.size() << " ):" << endl;
	for (int i = 0; i < solutions.size(); ++i) {
		cout << "x_" << i << "\t=\t" << solutions[i] << endl;
	}

	if (!sdd) cout << "THIS SOLUTION MAY BE INCORRECT!" << endl;
}