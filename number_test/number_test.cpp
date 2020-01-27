#include "number.h"

int main()
{
	jrmwng::number::Cinteger<1> u0(0U);
	jrmwng::number::Cinteger<1> u1(1U);
	//jrmwng::number::Cinteger<1> u2(2);

	jrmwng::number::Cinteger<1> t0("0");
	jrmwng::number::Cinteger<1> t1("1");
	//jrmwng::number::Cinteger<1> t2("2");
	jrmwng::number::Cinteger<3> t7("7");
	//jrmwng::number::Cinteger<3> t8("8", 8U);
	jrmwng::number::Cinteger<5> t10("10", 16U);

	jrmwng::number::Cinteger<1>::accumulator_type<0> accumulator(0U);
	{
		accumulator += jrmwng::number::Cinteger<1>(1U);
		accumulator *= jrmwng::number::Cinteger<1>(1U);
		accumulator -= jrmwng::number::Cinteger<1>(1U);
	}
	jrmwng::number::Cinteger<32> uNot0(~0U);

	auto uNot0Plus1 = uNot0 + (u1 + u1);
	return 0;
}