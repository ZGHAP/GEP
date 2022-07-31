#include "./gp.h"
#include <cstring>

// Constructor
FunctionSet::FunctionSet(void)
{
	n = 0; // Currently no functions in the list
	maxn = 16; // Allocate room for 16 terminals to start
	functions = (SFunction *)malloc(maxn * sizeof(SFunction));
	nencapsulated = 0; // No encapsulated functions
}
// Destructor
FunctionSet::~FunctionSet(void)
{
	if (functions) {
		for (int i = 0; i < n; ++i)
			free(functions[i].name);
		free(functions);
	}
}
// Add a new function to the set
//
void FunctionSet::add(const char *name, int nargs,
	impfunc implementation,
	editfunc edit_function,
	int has_sides)
{
	for (int i = 0; i < n; ++i)
		if (!strcmp(functions[i].name, name)) {
			std::cerr << "FunctionSet Error: attempt to add existing function: "
				<< functions[i].name << '\n';
			return;
		}

	if (nargs > MAX_SEXP_ARGS) {
		std::cerr << "Error! Function " << name
			<< " declared with " << nargs
			<< "arguments.\nYou have to change MAX_SEXP_ARGS and recompile.\n";
	}

	if (n == maxn - 1) {
		maxn *= 2;
		functions = (SFunction *)realloc(functions,
			maxn * sizeof(SFunction));
	}

	functions[n].name = strdup(name);
	functions[n].nargs = nargs;
	functions[n].func = implementation;
	functions[n].edit = edit_function;
	functions[n].active = 1;
	functions[n].side_effects = has_sides;
	functions[n++].s = NULL;
}
// Add a new function to the set
int FunctionSet::encapsulate(S_Expression *s, int nargs)
{
	char buffer[16];
	sprintf(buffer, "(E%d)", nencapsulated);
	++nencapsulated;

	if (n == maxn - 1) {
		maxn *= 2;
		functions = (SFunction *)
			realloc(functions, maxn * sizeof(SFunction));
	}
	for (int i = 0; i < n; ++i) {
		if (functions[i].s && equiv(s, functions[i].s))
			return i;
	}

	functions[n].name = strdup(buffer);
	functions[n].nargs = nargs;
	functions[n].func = NULL;
	functions[n].edit = NULL;
	functions[n].active = 1;
	functions[n].side_effects = s->side_effects();
	functions[n].s = s->copy();
	std::cerr << "\nEncapsulating " << buffer << " = " <<
		functions[n].s << '\n';
	std::cerr.flush();
	n++;
	return (n - 1);
}

int FunctionSet::index(const char *name)
{
	for (int i = 0; i < n; ++i)
		if (!strcmp(functions[i].name, name))
			return i;
	std::cerr << "FunctionSet Error: could not index function: " << name << '\n';
	return -1;
}

char * FunctionSet::getname(int ind)
{
	if (ind < 0 || ind >= n) {
		std::cerr << "FunctionSet Error: attempted to getname with bad index(" << ind << ")\n";
		return NULL;
	}
	else return functions[ind].name;
}

void FunctionSet::print(void)
{
	std::cout << "\nFunction set listing:\n"
		<< "---------------------\n";
	for (int i = 0; i < n; ++i) {
		std::cout << '\"' << functions[i].name << "\": "
			<< functions[i].nargs << " parameters ";
		if (!functions[i].active)
			std::cout << " (deleted) ";
		if (functions[i].s)
			std::cout << functions[i].s;
		std::cout << '\n';
	}
	std::cout << '\n';
}

FunctionSet Fset;