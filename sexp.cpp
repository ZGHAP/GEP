#include "gp.h"
#include "string.h"

static S_Expression *free_list = NULL;
// Constructor
S_Expression::S_Expression(void)
{
	type = STnone;
	val = 0;
	which = 0;
	for (int i = 0; i < MAX_SEXP_ARGS; ++i)
		args[i] = NULL;
}
void* S_Expression::operator new (size_t size)
{
	static long retrieved = 0;
	static long numalloced = 0;
	S_Expression *s;
	if (free_list) {
		s = free_list;
		free_list = s->args[0];
		++retrieved;
	}
	else {
		s = (S_Expression *)malloc(sizeof(S_Expression));
		++numalloced;
		if (!(numalloced % 100000))
			std::cerr << "\nAllocated " << numalloced
			<< " sexp cells (" << retrieved << ")\n";
	}
	return s;
}

void S_Expression::operator delete (void *s)
{
	S_Expression *se = (S_Expression *)s;
	for (int i = 0; i < MAX_SEXP_ARGS; ++i)
		if (se->args[i]) delete se->args[i];
	((S_Expression *)s)->args[0] = free_list;
	free_list = (S_Expression *)s;
}

double run_encapsulated_program(int ind)
{
	S_Expression *s = Fset.lookup_encapsulation(ind);
	return s->eval();
}

S_Expression *edit(S_Expression *s)
{
	if (s && (s->type == STfunction)) {
		for (int i = 0; i < Fset.nargs(s->which); ++i)
			s->args[i] = edit(s->args[i]);
		editfunc e = Fset.editop(s->which);
		return e ? (*e)(s) : s;
	}
	return s;
}

void S_Expression::permute(void)
{
	std::cout << "Permuting:\nold = " << this << "\nnew = ";
	std::cout.flush();
	
	if (type == STfunction && Fset.nargs(which) > 1) {
		for (int i = Fset.nargs(which) - 1; i; --i) {
			double u = (rand() % (N + 1) / (float)(N + 1));
			int k = (int)floor(i * u);
			S_Expression *temp = args[k];
			args[k] = args[i];
			args[i] = temp;
		}
	}
	std::cout << this << "\n";
	std::cout.flush();
}

S_Expression * S_Expression::copy(void)
{
	S_Expression *se = new S_Expression;
	se->type = type;
	se->val = val;
	se->which = which;
	for (int i = 0; i < MAX_SEXP_ARGS; ++i)
		if (args[i]) se->args[i] = args[i]->copy();
		else se->args[i] = NULL;
		return se;
}

int equiv(S_Expression *s1, S_Expression *s2)
{
	if (!s1 && !s2) return 1;
	if (!s1 || !s2) return 0;
	if (s1->type != s2->type) return 0;
	switch (s1->type) {
	case STnone:
		std::cerr << "Error: bad case in S_Expression::==\n";
		return 0;
	case STconstant: return (s1->val == s2->val);
	case STterminal: return (s1->which == s2->which);
	case STfunction:
		if (s1->which != s2->which)
			return 0;
		for (int i = 0; i < MAX_SEXP_ARGS; ++i)
			if (!equiv(s1->args[i], s2->args[i]))
				return 0;
		return 1;
	}
}

int S_Expression::is_numerical(double& f)
{
	if (type == STconstant) {
		f = val;
		return 1;
	}
	else if (type == STterminal) {
			char *name = Tset.getname(which);
		if ((name[0] >= '0' && name[0] <= '9') ||
			name[0] == '-') {
			f = Tset.lookup(which);
			return 1;
		}
	}
	return 0;
}

void S_Expression::characterize(int *depth,
	int *totalnodes, int *internal, int *external)
{
	*depth = 1; *totalnodes = 1;
	if (type == STfunction && Fset.nargs(which) >= 1) {
		int child_depth, child_total, child_internal;
		int child_external;
		*internal = 1;
		*external = 0;
		int nargs = Fset.nargs(which);
		int max_child_depth = 0;
		for (int i = 0; i < nargs; ++i) {
			args[i]->characterize(&child_depth, &child_total,
				&child_internal, &child_external);
			*totalnodes += child_total;
			*internal += child_internal;
			*external += child_external;
			if (child_depth > max_child_depth)
				max_child_depth = child_depth;
		}
		*depth += max_child_depth;
	}
	else {
		*internal = 0;
		*external = 1;
	}
}
// Return if the tree contains functions with side effects
int S_Expression::side_effects(void)
{
	if (type == STfunction) {
		if (Fset.has_sideeffects(which))
			return 1;
		for (int i = 0; i < Fset.nargs(which); ++i)
			if (args[i]->side_effects())
				return 1;
	}
	return 0;
}
S_Expression * S_Expression::selectany(int m, int *n, S_Expression ***ptr)
{
	(*n)++;
	if (*n == m)
		return (this);
	if (type == STfunction) {
		int nargs = Fset.nargs(which);
		for (int i = 0; i < nargs; ++i) {
			S_Expression *s = args[i]->selectany(m, n, ptr);
			if (s) {
				if (args[i] == s)
					*ptr = &(args[i]);
				return s;
			}
		}
	}
	return NULL;
}
S_Expression* S_Expression::selectexternal(int m, int *n,
	S_Expression ***ptr)
{
	if (type == STfunction && Fset.nargs(which) >= 1) {
		int nargs = Fset.nargs(which);
		for (int i = 0; i < nargs; ++i) {
			S_Expression *s = args[i]->selectexternal(m, n, ptr);
			if (s) {
				if (args[i] == s)
					*ptr = &(args[i]);
				return s;
			}
		}
		return NULL;
	}
	else {
		(*n)++;
		if (*n == m) return (this);
		else return NULL;
	}
}

S_Expression* S_Expression::selectinternal(int m, int *n, S_Expression ***ptr)
{
	if (type == STfunction && Fset.nargs(which) >= 1) {
		(*n)++;
		if (*n == m)
			return (this);
		int nargs = Fset.nargs(which);
		for (int i = 0; i < nargs; ++i) {
			S_Expression *s = args[i]->selectinternal(m, n, ptr);
			if (s) {
				if (args[i] == s)
					*ptr = &(args[i]);
				return s;
			}
		}
	}
	return NULL;
}
S_Expression * S_Expression::select(double pip, S_Expression ***ptr)
{
	int depth, total, internal, external;
	int n = -1;
	int which;
	*ptr = NULL;
	
	characterize(&depth, &total, &internal, &external);
	if ((rand() % (N + 1) / (float)(N + 1)) < pip && internal >= 1) {
		which = (int)floor((rand() % (N + 1) / (float)(N + 1)) * internal);
		return selectinternal(which, &n, ptr);
	}
	else if (external >= 1) {
		which = (int)floor((rand() % (N + 1) / (float)(N + 1)) * external);
		return selectexternal(which, &n, ptr);
	}
}
// Perform the crossover between these two S-Expressions
void crossover(S_Expression **s1, S_Expression **s2, double pip)
{
	S_Expression **parent1ptr = NULL, **parent2ptr = NULL;
	S_Expression *fragment1, *fragment2;
	//char buffer[1024];
	fragment1 = (*s1)->select(pip, &parent1ptr);
	if (!parent1ptr)
		parent1ptr = s1;
	fragment2 = (*s2)->select(pip, &parent2ptr);
	if (!parent2ptr)
		parent2ptr = s2;
	std::cout.flush();
	*parent1ptr = fragment2;
	*parent2ptr = fragment1;
}
// Choose a random terminal (possibly including the
// ephemeral constant
static void random_terminal(S_Expression *s)
{
	int i = Tset.n + (ephemeral_constant ? 1 : 0);
	s->which = (int)floor(i * (rand() % (N + 1) / (float)(N + 1)));
	if (s->which >= Tset.n) {
		s->type = STconstant;
		s->val = ephemeral_constant();
	}
	else s->type = STterminal;
}
// Choose a random function
static void random_function(S_Expression *s)
{
	s->type = STfunction;
	s->which = (int)floor(Fset.n * (rand() % (N + 1) / (float)(N + 1)));
	//std::cerr << Fset.n<<" "<<s->which <<" "<< (rand() % (N + 1) / (float)(N + 1)) << std::endl;
}
// Choose a random terminal or function
static void random_terminal_or_function(S_Expression *s)
{
	int i = Tset.n + Fset.n + (ephemeral_constant ? 1 : 0);
	s->which = (int)floor(i * (rand() % (N + 1) / (float)(N + 1)));
	if (s->which < Fset.n)
		s->type = STfunction;
	else {
		s->which -= Fset.n;
		if (s->which >= Tset.n) {
			s->type = STconstant;
			s->val = ephemeral_constant();
		}
		else s->type = STterminal;
	}
}
// Create a random S-Expression
S_Expression *random_sexpression(GenerativeMethod strategy, int maxdepth, int depth)
{
	S_Expression *s = new S_Expression;
	if (!depth) {
		maxdepth = 2 + (int)floor((maxdepth - 1) * (rand() % (N + 1) / (float)(N + 1)));
		if (strategy == RAMPED_HALF_AND_HALF) {
			if ((rand() % (N + 1) / (float)(N + 1)) < 0.5) 
				strategy = GROW;
			else 
				strategy = FULL;
		}
	}
	++depth;
	switch (strategy) {
	case GROW:
		//std::cerr << "GROW\n";
		if (depth == maxdepth)
			random_terminal(s);
		else 
			random_terminal_or_function(s);
		break;
	case FULL:
		//std::cerr << "FULL\n";
		if (depth == maxdepth)
			random_terminal(s);
		else 
			random_function(s);
		break;
	case RAMPED_HALF_AND_HALF:
		std::cerr << "random_sexpression: bad case\n";
		break;
	}
	if (s->type == STfunction)
		for (int i = 0; i < Fset.nargs(s->which); ++i)
			s->args[i] = random_sexpression(strategy, maxdepth, depth);
	return s;
}

void S_Expression::restrict_depth(int maxdepth, int depth)
{
	++depth;
	if (type == STfunction) {
		if (depth == maxdepth - 1) {
			for (int i = 0; i < Fset.nargs(which); ++i)
				if (args[i]->type == STfunction) {
					delete args[i];
					args[i] = random_sexpression(GROW, maxdepth - depth);
				}
		}
		else
			for (int i = 0; i < Fset.nargs(which); ++i)
				args[i]->restrict_depth(maxdepth, depth);
	}
}

void S_Expression::write(char *s, int level)
{
	char buffer[64];
	if (!level)
		s[0] = 0;
	switch (type) {
	case STnone: std::cerr << "Error: bad case in S_Expression::write\n";
		break;
	case STconstant: sprintf(buffer, "%g", val);
		strcat(s, buffer);
		break;
	case STterminal: strcat(s, Tset.getname(which));
		break;
	case STfunction:
		strcat(s, "(");
		strcat(s, Fset.getname(which));
		for (int i = 0; i < Fset.nargs(which); ++i) {
			strcat(s, " ");
			args[i]->write(s, level + 1);
		}
		strcat(s, ")");
		break;
	}
}

std::ostream& operator<< (std::ostream& out, S_Expression *s)
{
	char buffer[10000];
	s->write(buffer);
	return (out << buffer);
}

	static int is_digit(char c)
{
	return ((c >= '0') && (c <= '9'));
}
static int is_alphanum(char c)
{
	return ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') || is_digit(c));
}
static int is_white(char c)
{
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}
static int is_delimiter(char c)
{
	return (c == '(' || c == ')');
}

static char *get_token(char *token, char *s, char& type)
{
	char *tokenbegin = token;
	/* Skip whitespace */
	while (*s && is_white(*s))
		s++;
	if (!*s) // No more tokens
		type = 0;
	else if (is_digit(*s) || *s == '.' ||
		(*s == '-' && (is_digit(s[1]) || s[1] == '.')))
	{
		// It's a number (form: [-]d[d...][.[d...]] )
		type = '0';
		if (*s == '-') *token++ = *s++;
		while (is_digit(*s))
			*token++ = *s++;
		if (*s == '.') *token++ = *s++;
		while (is_digit(*s))
			*token++ = *s++;
	}
	else if (is_delimiter(*s)) { // it's a delimeter
		type = *s;
		*token++ = *s++;
	}
	else { // It's an identifier or a function
		type = 'a';
		while (*s && !is_delimiter(*s) && !is_white(*s))
			*token++ = *s++;
	}
	*token = '\0';
	return (s);
}

S_Expression *sexify(char **text)
{
	char token[64];
	char type;
	char *s = get_token(token, *text, type);
	if (type == 0) {
		// No more tokens
		std::cerr << "sexify Error: Unexpected end of LISP expression\n";
			return NULL;
	}
	S_Expression *se = new S_Expression;
	if (type == '0') {
		// It's a number
		se->type = STconstant;
		se->val = atof(token);
	}
	else if (type == 'a') {
		// It's a terminal
		se->type = STterminal;
		se->which = Tset.index(token);
		if (se->which < 0) {
			std::cerr << "sexify Error: Unrecognized terminal " <<
				token << '\n';
			delete se;
			return NULL;
		}
	}
	else if (*token == '(') {
		// It's a function
		s = get_token(token, s, type);
		se->type = STfunction;
		se->which = Fset.index(token);
		if (se->which < 0) {
			std::cerr << "sexify Error: Unrecognized terminal " << token << '\n';
			delete se;
			return NULL;
		}
		int nargs = Fset.nargs(se->which);
		for (int i = 0; i < nargs; ++i) {
			se->args[i] = sexify(&s);
			if (!se->args[i]) {
				std::cerr << "sexify Error: Bad function argument for "
					<< token << '\n';
				delete se;
				return NULL;
			}
		}
		s = get_token(token, s, type);
		if (type != ')') {
			std::cerr << "sexify Error: Expected ')' not found\n";
			delete se;
			return NULL;
		}
	}
	else std::cout << "Huh?\n";
	*text = s;
	return (se);
}

S_Expression *sexify(char *text)
{
	char *s = text;
	return sexify(&s);
}
EPHEMERAL ephemeral_constant = NULL;