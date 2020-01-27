#include "number.h"

int main()
{
	jrmwng::number::Cinteger<sizeof(size_t) * 8> u1(1);
	jrmwng::number::Cinteger<sizeof(size_t) * 8> u2(2);
	jrmwng::number::Cinteger<sizeof(size_t) * 8> uNot0(~0);

	auto uTest0 = u1 + uNot0;
	auto uTest1 = uTest0 - u1;
	auto uTest2 = uTest0 - uNot0;
	auto uTest3 = u1 * uNot0;
	auto uTest4 = u2 * uNot0;

	jrmwng::number::Cinteger<1>::accumulator_type<1> accumulator;

	accumulator += jrmwng::number::Cinteger<1>(1);
	accumulator *= jrmwng::number::Cinteger<1>(1);
	accumulator -= jrmwng::number::Cinteger<1>(1);
	return 0;
}