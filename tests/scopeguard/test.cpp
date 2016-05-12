#include <bblib/ScopeGuard.h>

//#define DO_BOOST_BIND_TESTS
#if defined DO_BOOST_BIND_TESTS
#	include <boost/bind.hpp>
#	include <boost/lambda/lambda.hpp>
#	include <boost/lambda/bind.hpp>
#endif

#include <functional>
#include <iostream>
#include <cassert>
#define BB_ASSERT assert

int g_val = 0;

void bar_fn ()
{
	++g_val;
	std::cout << " + foo::bar, val = " << g_val << "\n";
}


struct foo
{
	void bar () const { bar_fn(); }
	void bar_1 (int) const { bar_fn(); }
	void bar_2 (int, int) const { bar_fn(); }
	void bar_3 (int, int, int) const { bar_fn(); }
};



int main ()
{
	// std::functional tests
	{
		std::cout << "ptr_fun test\n";
		//BB_SCOPE_GUARD(tmp, std::mem_fun(&bar));
		//std::ptr_fun(&bar_fn)();
		//bb::scope_guard_t tmp = bb::mkScopeGuard(std::ptr_fun(&bar));
		bb::scope_guard_t tmp = bb::mkScopeGuard(&bar_fn);
	}
	BB_ASSERT( g_val == 1 );

	{
		std::cout << "mem_fun test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(std::mem_fun(&foo::bar), &f);
	}
	BB_ASSERT( g_val == 2 );
	{
		std::cout << "mem_fun 1 Dismiss test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(std::mem_fun(&foo::bar_1), &f, 1);
		tmp.Dismiss();
	}
	BB_ASSERT( g_val == 2 );


	// boost::bind tests
#if defined DO_BOOST_BIND_TESTS
	{
		std::cout << "boost::bind test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar, f));
	}
	BB_ASSERT( g_val == 3 );
	{
		std::cout << "boost::bind Dismiss test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar, f));
		tmp.Dismiss();
	}
	BB_ASSERT( g_val == 3 );


	{
		std::cout << "boost::bind arg 1 test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar_1, f, 1));
	}
	BB_ASSERT( g_val == 4 );
	{
		std::cout << "boost::bind arg 1 Dismiss test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar_1, f, 1));
		tmp.Dismiss();
	}
	BB_ASSERT( g_val == 4 );


	{
		std::cout << "boost::bind arg 2 test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar_2, f, 1, 2));
	}
	BB_ASSERT( g_val == 5 );
	{
		std::cout << "boost::bind arg 2 Dismiss test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar_2, f, 1, 2));
		tmp.Dismiss();
	}
	BB_ASSERT( g_val == 5 );
	

	{
		std::cout << "boost::bind arg 3 test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar_3, f, 1, 2, 3));
	}
	BB_ASSERT( g_val == 6 );
	{
		std::cout << "boost::bind arg 3 Dismiss test\n";
		foo f;
		bb::scope_guard_t tmp = bb::mkScopeGuard(boost::bind(&foo::bar_3, f, 1, 2, 3));
		tmp.Dismiss();
	}
	BB_ASSERT( g_val == 6 );

	/////////// boost::lambda test
	{
		std::cout << "boost::lambda arg 1 test\n";
		foo f;
		BB_SCOPE_GUARD(tmp2, boost::lambda::bind(&foo::bar_1, f, 1));
	}
	BB_ASSERT( g_val == 7 );
#endif
}
