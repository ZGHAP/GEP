#include "gp.h"

// Macro to return a floating point value and kill the rest
// of the subtree
inline S_Expression *retnum(double f, S_Expression *s)
{
	S_Expression *s2 = new S_Expression;
	s2->type = STconstant;
	s2->val = f;
	delete s;
	return s2;
}
// Macro to return one arg and kill the rest of the subtree
inline S_Expression *retarg(int a, S_Expression *s)
{
	S_Expression *s2 = s->args[(a)];
	s->args[(a)] = NULL;
	delete s;
	return s2;
}

static double plus(S_Expression **args) {
	return (args[0]->eval() + args[1]->eval());
}
static S_Expression *edit_plus(S_Expression *s)
{
	double f, f2;
	// Rule 1: if either arg is 0, result is other arg
	if (s->args[0]->is_numerical(f) && f == 0.0)
		return retarg(1, s);
	if (s->args[1]->is_numerical(f) && f == 0.0)
		return retarg(0, s);
	// Rule 2: if both args are numbers, result is their sum
	if (s->args[0]->is_numerical(f) &&
		s->args[1]->is_numerical(f2))
		return retnum(f + f2, s);
	return s;
}
void use_addition(void) {
	Fset.add("+", 2, plus, edit_plus);
}
////////////////////////////////////////////////////////////
// Subtraction function
////////////////////////////////////////////////////////////
static double minus(S_Expression **args) {
	return (args[0]->eval() - args[1]->eval());
}
static S_Expression *edit_minus(S_Expression *s)
{
	double f, f2;
	// Rule 1: if second arg is 0, result is first argument
	if (s->args[1]->is_numerical(f) && f == 0.0)
		return retarg(0, s);
	// Rule 2: if both args are numbers, result is a number
	if (s->args[0]->is_numerical(f) &&
		s->args[1]->is_numerical(f2))
		return retnum(f - f2, s);
	// Rule 3: if both args are the same identifier result=0
	if (s->args[0]->type == s->args[1]->type &&
		s->args[0]->type == STterminal &&
		s->args[0]->which == s->args[1]->which)
		return retnum(0, s);
	return s;
}
void use_subtraction(void) {
	Fset.add("-", 2, minus, edit_minus);
}
////////////////////////////////////////////////////////////
// Multiplication function
////////////////////////////////////////////////////////////
static double mult(S_Expression **args) {
	return (args[0]->eval() * args[1]->eval());
}
static S_Expression *edit_mult(S_Expression *s)
{
	double f, f2;
	/* Rule 1: if either argument is 0, result is 0 */
	if ((s->args[0]->is_numerical(f) && f == 0.0) ||
		(s->args[1]->is_numerical(f2) && f2 == 0.0))
		return retnum(0, s);
	// Rule 2: if both args are numbers, result is a number
	if (s->args[0]->is_numerical(f) &&
		s->args[1]->is_numerical(f2))
		return retnum(f*f2, s);
	// Rule 3: if either arg is 1, result is other argument
	if (s->args[0]->is_numerical(f) && f == 1.0)
		return retarg(1, s);
	if (s->args[1]->is_numerical(f) && f == 1.0)
		return retarg(0, s);
	return s;
}
void use_multiplication(void) {
	Fset.add("*", 2, mult, edit_mult);
}
////////////////////////////////////////////////////////////
// Protected division function
////////////////////////////////////////////////////////////
static double protected_div(S_Expression **args)
{
	double denom = args[1]->eval();
	if (denom == 0.0)
		return 1;
	else return (args[0]->eval() / denom);
}
S_Expression *edit_protected_division(S_Expression *s)
{
	double f, f2;
	// Rule 1: if second argument is 0, result is 1
	if (s->args[1]->is_numerical(f) && f == 0.0)
		return retnum(1, s);
	// Rule 2: if first argument is 0, result is 0
	if (s->args[0]->is_numerical(f) && f == 0.0 &&
		!s->args[1]->side_effects())
		return retnum(0, s);
	// Rule 3: if both args are non-zero numbers, result is a number
	if (s->args[0]->is_numerical(f) &&
		s->args[1]->is_numerical(f2))
		return retnum(f / f2, s);
	// Rule 4: if second arg is 1, result is first argument
	if (s->args[1]->is_numerical(f) && f == 1.0)
		return retarg(0, s);
	// Rule 5: if both args are the same terminal, result is 1
	if (s->args[0]->type == s->args[1]->type &&
		s->args[0]->type == STterminal &&
		s->args[0]->which == s->args[1]->which)
		return retarg(1, s);
	return s;
}
void use_protected_division(void) {
	Fset.add("%", 2, protected_div, edit_protected_division);
}
// Sine function
static double sine(S_Expression **args) {
	return sin(args[0]->eval());
}
void use_sin(void) { Fset.add("sin", 1, sine); }
// Cosine function
static double cosine(S_Expression **args) {
	return cos(args[0]->eval());
}
void use_cos(void) { Fset.add("cos", 1, cosine); }
// ATG -- protected arctangent with 2 arguments
static double atg(S_Expression **args) {
	return atan2(args[0]->eval(), args[1]->eval());
}
void use_atg(void) { Fset.add("atg", 2, atg); }
// Exponential function
static double expfun(S_Expression **args) {
	double f = args[0]->eval();
	return exp(f);
}
void use_exp(void) { Fset.add("exp", 1, expfun); }
// Protected logarithm function
static double rlog(S_Expression **args) {
	double x = args[0]->eval();
	return (x > 0.0 ? log(x) : 0);
}
void use_rlog(void) { Fset.add("rlog", 1, rlog); }
// absolute value function
static double abs_imp(S_Expression **args) {
	return fabs(args[0]->eval());
}
void use_abs(void) { Fset.add("abs", 1, abs_imp); }
// greater-than -- return 1 if first arg is > second arg,
// else return -1
static double gt(S_Expression **args) {
	return (args[0]->eval() > args[1]->eval()) ? 1 : -1;
}
void use_gt(void) { Fset.add("gt", 2, gt); }
// Logical AND function
static double logical_and(S_Expression **args)
{
	if (!args[0]->eval()) return 0;
	else return (args[1]->eval());
}
void use_and(void) { Fset.add("and", 2, logical_and); }
// Logical OR function
static double logical_or(S_Expression **args)
{
	if (args[0]->eval()) return 1;
	else return (args[1]->eval());
}
void use_or(void) { Fset.add("or", 2, logical_or); }
// Logical NOT function
static double logical_not(S_Expression **args) {
	return (!args[0]->eval());
}
void use_not(void) { Fset.add("not", 1, logical_not); }
// Logical IF function
static double logical_if(S_Expression **args)
{
	if (args[0]->eval() != 0)
		return (args[1]->eval());
	else return (args[2]->eval());
}
void use_if(void) { Fset.add("if", 3, logical_if); }
// Logical EQ -- equality test
static double logical_eq(S_Expression **args)
{
	if (args[0]->eval() == args[1]->eval())
		return 1;
	else return 0;
}
void use_eq(void) { Fset.add("eq", 2, logical_eq); }
// PROGN2 -- sequence of 2 moves
// PROGN3 -- sequence of 3 moves
static double progn2(S_Expression **args)
{
	args[0]->eval();
	return args[1]->eval();
}
void use_progn2(void)
{
	Fset.add("progn2", 2, progn2);
}
static double progn3(S_Expression **args)
{
	args[0]->eval();
	args[1]->eval();
	return args[2]->eval();
}
void use_progn3(void)
{
	Fset.add("progn3", 3, progn3);
}
// SRT protected square root
static double srt(S_Expression **args) {
	return sqrt(fabs(args[0]->eval()));
}
void use_srt(void) { Fset.add("srt", 1, srt); }
// SQ square operator
static double sq(S_Expression **args)
{
	double f = args[0]->eval();
	return f*f;
}
void use_sq(void) { Fset.add("sq", 1, sq); }
// CUB cube operator
static double cub(S_Expression **args)
{
	double f = args[0]->eval();
	return f*f*f;
}
void use_cub(void) { Fset.add("cub", 1, cub); }
// SIG sign operator
static double sig(S_Expression **args) {
	return ((args[0]->eval() > 0.0) ? 1.0 : -1.0);
}
void use_sig(void) { Fset.add("sig", 1, sig); }
// IFLTZ -- if less than zero
static double ifltz(S_Expression **args)
{
	if (args[0]->eval() < 0)
		return (args[1]->eval());
	else return (args[2]->eval());
}
void use_ifltz(void) { Fset.add("ifltz", 3, ifltz); }
// IFLTE -- if less than or equal
static double iflte(S_Expression **args)
{
	if (args[0]->eval() <= args[1]->eval())
		return (args[2]->eval());
	else return (args[3]->eval());
}
void use_iflte(void) { Fset.add("iflte", 4, iflte); }
// SREXPT -- raise to powers
static double srexpt(S_Expression **args)
{
	double f = args[0]->eval();
	if (fabs(f) > 1.0e-9)
		return pow(f, args[1]->eval());
	else return 0.0;
}
void use_srexpt(void) { Fset.add("srexpt", 2, srexpt); }
// DU -- looping operator
int max_du_iterations = 25;
static double du(S_Expression **args)
{
	for (int iters = 0; args[1]->eval(); ++iters) {
		args[0]->eval();
		if (iters >= max_du_iterations)
			return 0;
	}
	return 1;
}
void use_du(void) { Fset.add("du", 2, du); }