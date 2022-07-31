#include "gp.h"
#include <cstring>

TerminalSet::TerminalSet(void)
{
	n = 0; // Currently no terminals in the list
	maxn = 16; // Allocate room for 16 terminals to start
	terminals = (Terminal *)malloc(maxn * sizeof(Terminal));
}

TerminalSet::~TerminalSet(void)
{
	if (terminals) {
		for (int i = 0; i < n; ++i)
			free(terminals[i].name);
		free(terminals);
	}
}
// Add a new terminal to the set
void TerminalSet::add(const char *name, double val)
{

	for (int i = 0; i < n; ++i)
		if (!strcmp(terminals[i].name, name)) {
			std::cerr << "TerminalSet Error: attempt to add existing terminal: "
				<< terminals[i].name << '\n';
			return;
		}

	if (n == maxn - 1) {
		maxn *= 2;
		terminals = (Terminal *)realloc(terminals,
			maxn * sizeof(Terminal));
	}

	terminals[n].name = strdup(name);
	terminals[n++].val = val;
}

void TerminalSet::modify(const char *name, double val)
{
	for (int i = 0; i < n; ++i)
		if (!strcmp(terminals[i].name, name)) {
			terminals[i].val = val;
			return;
		}

	std::cerr << "TerminalSet Error: attempt to modify non-existing terminal: "
		<< name << '\n';
}

double TerminalSet::get(const char *name)
{
	for (int i = 0; i < n; ++i)
		if (!strcmp(terminals[i].name, name))
			return terminals[i].val;

	std::cerr << "TerminalSet Error: could not find terminal: " << name << '\n';
	return 0;
}

int TerminalSet::index(const char *name)
{
	for (int i = 0; i < n; ++i)
		if (!strcmp(terminals[i].name, name))
			return i;

	std::cerr << "TerminalSet Error: could not index terminal: " <<
		name << '\n';
	return -1;
}

void TerminalSet::print(void)
{
	std::cout << "\nTerminal set listing:";
	std::cout << "\n---------------------\n";
	for (int i = 0; i < n; ++i)
		std::cout << '\"' << terminals[i].name << "\" = " <<
		terminals[i].val << '\n';
	std::cout << '\n';
}

TerminalSet Tset;