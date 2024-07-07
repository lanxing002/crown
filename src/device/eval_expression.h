#pragma once
namespace crown
{
	struct PyWrapper;
}

class EvalExpr
{
public:
	static void eval(crown::PyWrapper* py_wrapper);

	static bool quit;
};

