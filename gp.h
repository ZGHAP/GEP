#pragma once

#include <iostream>
#include <time.h>
#include <random>
#include <string>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

#define N 999


class S_Expression;
class GP;
typedef double(*impfunc)(S_Expression **);
typedef S_Expression* (*editfunc)(S_Expression *);
typedef int(*CONDITION)(GP *); 
typedef double(*FLOATFUNC)(double); 
typedef double(*EPHEMERAL)(void);
// Types of S_Expression nodes
enum SEXP_TYPE { STnone, STconstant, STterminal, STfunction };
enum GenerativeMethod { GROW, FULL, RAMPED_HALF_AND_HALF };
#define MAX_SEXP_ARGS 4
typedef double(*FITNESSFUNC)(S_Expression *s, int *hits);
enum SelectionMethod {
	UNIFORM, FITNESS_PROPORTIONATE,
	TOURNAMENT, RANK
};
enum FitnessMeasure { RAW, ADJUSTED };

enum GPVerbosity {
	QUIET = 0,
	TELL_INITIALIZE = 1,
	LIST_INITIAL_EXPRESSIONS = 2,
	GENERATION_UPDATE = 4,
	LIST_GENERATIONAL_FITNESSES = 8,
	SHOW_EDITED_BEST = 16,
	END_REPORT = 32,
	LIST_PARAMETERS = 64,
	DEBUG = -1
};

extern double run_encapsulated_program(int);
extern S_Expression *random_sexpression(GenerativeMethod, int, int);

class TerminalSet {
private:
	struct Terminal {
		char *name; 
		double val; 
	};
	int maxn;
	Terminal *terminals;
public:
	int n; 
public:
	TerminalSet(void);
	~TerminalSet(void);
	void add(const char *name, double val = 0);
	void modify(const char *name, double val);
	void modify(const int index, double val) { terminals[index].val = val; }
	double get(const char *name);
	double get(int ind) { return terminals[ind].val; }
	int index(const char *name);
	double lookup(int ind) { return terminals[ind].val; }
	char *getname(int ind) { return terminals[ind].name; }
	void print(void);
};

extern TerminalSet Tset;

class FunctionSet {
private:
	struct SFunction {
		char *name; 
		int nargs; 
		impfunc func;
		editfunc edit; 
		int active; 
		int side_effects; 
		S_Expression *s; 
	};
	int maxn;
	SFunction *functions; 
	int nencapsulated;
public:
	int n; 
	FunctionSet(void);
	~FunctionSet(void);
	void add(const char *name, int nargs,
		impfunc implementation,
		editfunc edit_function = NULL,
		int has_sides = 0);
	int encapsulate(S_Expression *s, int nargs = 0);
	void remove(const char *name) {
		int i = index(name);
		if (i >= 0)
			functions[i].active = 0;
	}
	void remove(int ind) {
		if (ind >= 0 || ind < n)
			functions[ind].active = 0;
	}
	int index(const char *name);
	char *getname(int ind);
	int nargs(int ind) { return functions[ind].nargs; }
	int has_sideeffects(int ind)
	{
		return functions[ind].side_effects;
	}
	impfunc lookup_implementation(int ind)
	{
		return functions[ind].func;
	}
	S_Expression *lookup_encapsulation(int ind)
	{
		return functions[ind].s;
	}
	int is_encapsulated(int ind)
	{
		return (functions[ind].s ? 1 : 0);
	}
	editfunc editop(int ind) { return functions[ind].edit; }
	void print(void);
};

// Declare the globally visible function set
//extern FunctionSet Fset;
extern FunctionSet Fset;
//////////////////////////////////////////////////////////
// S_Expression class
//////////////////////////////////////////////////////////
class S_Expression {
public:
	SEXP_TYPE type;
	double val; // value if type == STconstant
	int which; // index of terminal or function
	S_Expression *args[MAX_SEXP_ARGS];
	// Constructor
	S_Expression(void);
	// new and delete operators
	void* operator new (size_t size);
	void operator delete (void *);
	// Runs an encapsulated program, specified by index
	friend double run_encapsulated_program(int ind);
	// Evaluate this S_Expression
	double eval(void)
	{
		//std::cerr << "called eval\n";
		double f;
		if (type == STfunction) {
			if (Fset.is_encapsulated(which))
				f = run_encapsulated_program(which);
			else {
				impfunc func = (impfunc)Fset.lookup_implementation(which);
				f = (*func)(args);
			}
		}
		else if (type == STterminal) f = Tset.lookup(which);
		else if (type == STconstant) f = val;
		else {
			std::cerr << "Error: bad case in S_Expression::eval\n";
			char buffer[2048];
			write(buffer);
			std::cerr << buffer << '\n';
			f = 0;
		}
		return f;
	}
	// Edit the tree (destructively), return ptr to new root
	friend S_Expression *edit(S_Expression *s);
	// Permute the arguments of this functional S-Expression
	void permute(void);
	// Make another copy
	S_Expression *copy(void);
	// Test for equivalence between two S_Expressions
	friend int equiv(S_Expression *s1, S_Expression *s2);
	// Is the S_Expression a numerical constant?
	int is_numerical(double& f);
	// Count stuff
	void characterize(int *depth, int *total, int *internal,
		int *external);
	// Does the tree below have functions with side effects?
	int side_effects(void);
	// Select points
	S_Expression * selectany(int, int *, S_Expression ***ptr);
	S_Expression * selectinternal(int, int *, S_Expression ***ptr);
	S_Expression * selectexternal(int, int *, S_Expression ***ptr);
	S_Expression * select(double pip, S_Expression ***ptr);
	// Perform crossover operation
	friend void crossover(S_Expression **s1,
		S_Expression **s2, double pip);
	// Make a random tree
	friend S_Expression *random_sexpression(
		GenerativeMethod strategy, int maxdepth = 6, int depth = 0);
	// Chop off below a certain depth
	void restrict_depth(int maxdepth = 17, int depth = 0);
	// Write the lisp code into a string
	void write(char *, int level = 0);
	// Output to a stream
	friend std::ostream& operator<< (std::ostream& out, S_Expression *s);
	// Make an S_Expression from a LISP string
	friend S_Expression *sexify(char **s);
	friend S_Expression *sexify(char *s);
};

//double run_encapsulated_program(int ind){
//	S_Expression *s = Fset.lookup_encapsulation(ind);
//	return s->eval();
//}

extern EPHEMERAL ephemeral_constant;

class Individual {
public:
	S_Expression *s; 
	double rfit, sfit; 
	double afit, nfit; 
	double sumnfit; 
	int hits; 
	int recalc_needed; 
	Individual(void) { s = NULL; recalc_needed = 1; }
	~Individual(void) { if (s) delete s; }
	void operator= (Individual& i) {
		if (s) delete s;
		if (i.s) s = i.s->copy();
		else s = NULL;
		rfit = i.rfit;
		sfit = i.sfit;
		afit = i.afit;
		nfit = i.nfit;
		sumnfit = i.sumnfit;
		hits = i.hits;
		recalc_needed = i.recalc_needed;
	}
};

class GP {
public:
	int M; // The population size
	int G; // The number of generations to run
	double pc; // Probability of crossover
	double pr; // Probability of reproduction
	double pip; // Probability of internal crossover
	int Dcreated; // Max depth of S-Expressions
	int Dinitial; // Max depth of initial population
	double pm; // Probability of mutation
	double pp; // Probability of permutation
	int fed; // Frequency of performing editing
	double pen; // Probability of encapsulation
	CONDITION dec_cond; // Condition for decimation
	double pd; // Decimation percentage
	GenerativeMethod generative_method;
	SelectionMethod reproduction_selection;
	SelectionMethod second_parent_selection;
	int use_greedy_overselection;
	double overselection_boundary;
	int use_elitist_strategy;
	int tournament_size;
	int initialized;
	int gen; // Current generation number
	Individual *pop; // The actual population
	Individual *newpop; // Population we're building
	Individual best_of_run; // Housekeeping information
	int bestofrun_gen; // Gen when best_so_far was found
	int bestofgen_index; // index of this generation's best
	int bestofgen_hits; // hits of this generation's best
	double bestofgen_sfit; // sfitness of this gen's best
	double worstofgen_sfit; // sfitness of this gen's worst
	double avgofgen_sfit; // average sfitness of this gen
						 // User-defined funcs for controlling the GP run and I/O
	int verbose;
	FITNESSFUNC fitness_function; // The fitness evaluation function
	CONDITION termination_criteria; // When do we terminate?
	FLOATFUNC standardize_fitness; // Fitness standardization
	double sfit_dontreport; // Don't report fitness >= this
	CONDITION generation_callback; // Call me after each gen
	int bestworst_freq; // How often to report best/worst?
	char *stat_filename;
	FILE *stat_file;
public:
	GP(FITNESSFUNC fitness_function, int popsize = 500);
	~GP(void);
	void go(int maxgens = 50);
	void print_population(void);
private:
	void init(void);
	void create_population(void);
	void nextgen(void);
	void sort_fitness(void);
	void eval_fitnesses(void);
	void report_on_run(void);
	void list_parameters(void);
};

double default_ephemeral_generator(void);

void use_addition(void);
void use_subtraction(void);
void use_multiplication(void);
void use_protected_division(void);
void use_sin(void);
void use_cos(void);
void use_atg(void);
void use_exp(void);
void use_rlog(void);
void use_gt(void);
void use_abs(void);
void use_sig(void);
void use_and(void);
void use_or(void);
void use_not(void);
void use_if(void);
void use_eq(void);
void use_progn2(void);
void use_progn3(void);
void use_srt(void);
void use_sq(void);
void use_cub(void);
void use_ifltz(void);
void use_iflte(void);
void use_srexpt(void);
//void use_pow(void);
void use_du(void);

extern int max_du_iterations;
//void use_setsv(void);


