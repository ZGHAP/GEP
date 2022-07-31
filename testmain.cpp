
#include "gp.h"
#include <fstream>

// N(umber)f(itness)c(ase)
#define NFC 600
#define Tmax 10.0
std::vector<double> xvals;
std::vector<double> vvals;
std::vector<double> fullsnapx;
std::vector<double> fullsnapv;
double MaxFitness = 0;

std::fstream fo("Out.txt", std::ios::out);

double fitness_function(S_Expression *s, int *hits)
{
	double total = 0; // total raw fitness
	*hits = 0;
	for (unsigned int k = 0; k < xvals.size(); ++k) {
		double x = xvals[k]; // get position for this test case
		double v = vvals[k]; // get velocity for this test case

		Tset.modify("x", x);
		//Tset.modify("v", v);

		if (fabs(v - s->eval()) < 0.001)
			*hits += 1;

		total += fabs(v - s->eval());
	}
	return total;
}

//double fitness_function(S_Expression *s, int *hits)
//{
//	const double tau = 0.02; // time step used for the simulation
//	double total = 0; // total raw fitness
//	double diff = 0;
//
//	int status = 0;
//	double entry = 0;
//	int trades = 0;
//
//	*hits = 0;
//	for (unsigned int k = 0; k < xvals.size(); ++k) {
//		double x = xvals[k]; // get ratio for this test case
//		double v = vvals[k]; // get median for this test case
//
//		Tset.modify("x", x);
//		Tset.modify("v", v);
//
//		if (status == 0) {
//			if (s->eval() > 0
//				&& k != xvals.size() - 1) {
//				entry -= x;
//				status = 1;
//				++trades;
//			}
//			else if (s->eval() < 0
//				&& k != xvals.size() - 1) {
//				entry += x;
//				status = -1;
//				++trades;
//			}
//		}
//		else{
//			if (status > 0) {
//				if (k == xvals.size() - 1) {
//					entry += x;
//					status = 0;
//					++trades;
//				}
//				else if (x < v) {
//					entry += x;
//					status = 0;
//					++trades;
//				}
//			}
//			if (status < 0) {
//				if (k == xvals.size() - 1) {
//					entry -= x;
//					status = 0;
//					++trades;
//				}
//				else if (x > v) {
//					entry -= x;
//					status = 0;
//					++trades;
//				}
//
//			}
//		}
//
//		total = entry;
//	}
//	return  total;
//}

double standardize_fitness(double r) {

	return r ;

	//return pow(0.5,r);
}

int main(int argc, char **argv)
{
	srand((unsigned long)time(NULL));

	int popsize = 500;
	int ngens = 100;

	FILE *fp_in;
	int ID_num = 0;
	char InputBuff[100];

	//if ((fp_in = fopen("T.csv", "r")) == NULL)
	//{
	//	printf("Data File Open Failed\n");
	//	printf("Get and key to exit\n");
	//	getchar();
	//	exit(0);
	//}
	//else
	//{
	//	while (fgets(InputBuff, 100, fp_in) != NULL)
	//	{
	//		std::string tempStr = InputBuff;
	//		std::string full_tempStr = InputBuff;
	//		std::string Substr;
	//		std::string full_Substr;

	//		int pos = tempStr.find(",");
	//		if (pos != -1) {
	//			Substr = tempStr.substr(0, pos);
	//			if (xvals.size() < NFC)
	//				xvals.push_back(atof(Substr.c_str()));
	//			//std::cerr << atof(Substr.c_str());
	//			tempStr.erase(0, pos + 1);
	//			if (vvals.size() < NFC)
	//				vvals.push_back(atof(tempStr.c_str()));
	//			//std::cerr << " " << atof(tempStr.c_str()) << std::endl;
	//		}
	//		else {
	//			printf("Can not seperate data\n");
	//			getchar();
	//			exit(0);
	//		}

	//		int pos_full = full_tempStr.find(",");
	//		if (pos_full != -1) {
	//			full_Substr = full_tempStr.substr(0, pos_full);
	//			fullsnapx.push_back(atof(full_Substr.c_str()));
	//			full_tempStr.erase(0, pos_full + 1);
	//			fullsnapv.push_back(atof(full_tempStr.c_str()));
	//		}
	//		else {
	//			printf("Can not seperate data\n");
	//			getchar();
	//			exit(0);
	//		}
	//	}
	//	fclose(fp_in);
	//}

	if (argc > 2) {
		popsize = atoi(argv[1]);
		ngens = atoi(argv[2]);
		std::cout << "Running " << popsize << " individuals for " << ngens << " generations\n\n";
	}
	if (argc > 3)
		srand(atoi(argv[3]));

	Tset.add("x");
	//Tset.add("v");
	Tset.add("1", 1);

	use_addition();
	use_subtraction();
	use_multiplication();
	//use_gt();
	use_abs();
	use_sin();
	use_exp();
	use_rlog();

	// Testing Data Gen
	for (double i = -2; i < 2; i+=0.01) {
		xvals.push_back(i);
		vvals.push_back(sin(3 * i)+i);
	}


	GP gp(fitness_function, popsize);
	// output controller
	gp.verbose = (GPVerbosity)(TELL_INITIALIZE | GENERATION_UPDATE | END_REPORT);
	gp.standardize_fitness = standardize_fitness;
	gp.go(ngens);
	S_Expression *c = gp.best_of_run.s->copy();
	char buffer[2000];
	c = edit(c);
	c->write(buffer);
	FILE *file = fopen("best_out", "w");
	fprintf(file, "%s\n", buffer);
	//double entry = 0;
	//int status = 0;
	//FILE *file_output = fopen("fullyScenes.csv", "w");
	//fprintf(file_output, "SeriesNo,Indicator Val, Entry, @, Total result\n");
	//for (unsigned int k = 0; k < fullsnapx.size(); ++k) {
	//	double x = fullsnapx[k]; // get position for this test case
	//	double v = fullsnapv[k];

	//	Tset.modify("x", x);
	//	Tset.modify("v", v);

	//	if (status == 0) {
	//		if (gp.best_of_run.s->eval() > 0
	//			&& k != fullsnapx.size() - 1) {
	//			entry -= x;
	//			status = 1;
	//			fprintf(file_output, "%d,%f,%d,%f\n", k, gp.best_of_run.s->eval(), 1, x);
	//		}
	//		else if (gp.best_of_run.s->eval() < 0
	//			&& k != fullsnapx.size() - 1) {
	//			entry += x;
	//			status = -1;
	//			fprintf(file_output, "%d,%f,%d,%f\n", k, gp.best_of_run.s->eval(), -1, x);
	//		}
	//	}
	//	else {
	//		if (status > 0) {
	//			if (k == fullsnapx.size() - 1) {
	//				entry += x;
	//				status = 0;
	//				fprintf(file_output, "%d,%f,%d,%f,%f\n", k, gp.best_of_run.s->eval(), -1, x, entry);
	//			}
	//			else if (x < v) {
	//				entry += x;
	//				status = 0;
	//				fprintf(file_output, "%d,%f,%d,%f,%f\n", k, gp.best_of_run.s->eval(), -1, x, entry);
	//			}
	//		}
	//		if (status < 0) {
	//			if (k == fullsnapx.size() - 1) {
	//				entry -= x;
	//				status = 0;
	//				fprintf(file_output, "%d,%f,%d,%f,%f\n", k, gp.best_of_run.s->eval(), 1, x, entry);
	//			}
	//			else if (x > v) {
	//				entry -= x;
	//				status = 0;
	//				fprintf(file_output, "%d,%f,%d,%f,%f\n", k, gp.best_of_run.s->eval(), 1, x, entry);
	//			}
	//		}
	//	}
	//}
	fclose(file);
	return 0;
}
