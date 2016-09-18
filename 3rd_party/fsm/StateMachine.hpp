#pragma once
/*
	Copyright David Abrahams 2003-2004
	Copyright Aleksey Gurtovoy 2003-2004

	Distributed under the Boost Software License, Version 1.0.
	(See accompanying file LICENSE_1_0.txt or copy at
	http://www.boost.org/LICENSE_1_0.txt)
			
	This file was automatically extracted from the source of
	"C++ Template Metaprogramming", by David Abrahams and
	Aleksey Gurtovoy.
*/
#include <type_traits>
#include <cstdio>

//using table = mp_typelist<int, float, int, float, int>;
///////////////////////////////////////////////////////

/**@class mp_typelist
 * @brief	generict type list
 *
 * example:
 *		using flt_types = mp_typelist<float, double>;
 **/
template<class... T> struct mp_typelist { };
template<int... T> struct mp_intlist { };


/**@class int_
 * @brief	shortcut for std::integral_cst
 **/
template<unsigned N>
using int_ = std::integral_constant<unsigned, N>;


template<bool C, class T, class E> struct mp_if_c_impl;
template<class T, class E>
struct mp_if_c_impl<true, T, E>
{
	using type = T;
};
template<class T, class E>
struct mp_if_c_impl<false, T, E>
{
	using type = E;
};
template<bool C, class T, class E>
using mp_if_c = typename mp_if_c_impl<C, T, E>::type;

/**@class mp_if
 * @brief	implements if <C> then T else E;
 *
 *	example:
 *		using tmp = mp_if<std::is_same<A, B>, F, B>;
 **/
template<class C, class T, class E>
using mp_if = typename mp_if_c_impl<C::value != 0, T, E>::type;


template<class L, class V>
struct mp_add_impl;

/**@class mp_push_back
 * @brief	appends element V to end of typelist L
 **/
template<class L, class V>
using mp_add = typename mp_add_impl<L, V>::type;

template<template<class...> class L, class V>
struct mp_add_impl<L<>, V>
{
	using type = L<V>;
};
template<template<class...> class L, class... T, class V>
struct mp_add_impl<L<T...>, V>
{
	using type = L<T...,V>;
};


template<class L, int V>
struct mp_iadd_impl;

/**@class mp_push_back
 * @brief	appends element V to end of typelist L
 **/
template<class L, int V>
using mp_iadd = typename mp_iadd_impl<L, V>::type;

template<template<int...> class L, int V>
struct mp_iadd_impl<L<>, V>
{
	using type = L<V>;
};
template<template<int...> class L, int... T, int V>
struct mp_iadd_impl<L<T...>, V>
{
	using type = L<T...,V>;
};


template<class L>
struct mp_size_impl;

/**@class mp_size
 * @brief	returns size of typelist L in form of int_<N>
 *
 *	example:
 *		using sz = mp_size<L>;
 **/
template<class L>
using mp_size = typename mp_size_impl<L>::type;

template<template<class...> class L, class... T>
struct mp_size_impl<L<T...>>
{
	using type = int_<sizeof...(T)>;
};


template<class L, class V, int N, template<class, class> class Cmp>
struct mp_indicesof_impl;

/**@class mp_indicesof
 * @brief	returns typelist of integral constants containing indices of elements of same type as V
 * @param[in]	L			typelist
 * @param[in]	V			type to search
 * @param[in]	Cmp		comparator to use
 * @returns		typelist of integral constants containing indices of elements of same type as V
 *	example:
 *		using idcs = mp_indicesof<table, float, std::is_same>;
 **/
template<class L, class V, template<class, class> class Cmp>
using mp_indicesof = typename mp_indicesof_impl<L, V, mp_size<L>::value, Cmp>::type;

template<template<class...> class L, class V, int N, template<class, class> class Cmp>
struct mp_indicesof_impl<L<>, V, N, Cmp>
{
	using type = mp_intlist<>;
};

template<template<class...> class L, class T1, class... T, class V, int N, template<class, class> class Cmp>
struct mp_indicesof_impl<L<T1, T...>, V, N, Cmp>
{
	using curr_idx = int_<N - sizeof...(T)>;
	//using curr_idx = int_<N>;
	using rest = typename mp_indicesof_impl<L<T...>, V, N, Cmp>::type;
	using tmp = mp_if<Cmp<T1, V>, mp_iadd<rest, curr_idx::value>, rest>;
	using type = tmp;
};

template<class L, std::size_t I>
struct mp_at_c_impl;

template<class L, std::size_t N>
using mp_at_c = typename mp_at_c_impl<L, N>::type;

template<class L, class N>
using mp_at = typename mp_at_c_impl<L, N::value>::type;

template<template<class...> class L, class T1, class... T>
struct mp_at_c_impl<L<T1, T...>, 0>
{
  using type = T1;
};

template<template<class...> class L, class T1, class... T, std::size_t N>
struct mp_at_c_impl<L<T1, T...>, N>
{
  using type = mp_at_c<L<T...>, N - 1>;
};

//template<class V>
//using indices_of = mp_indicesof<table, V>;

//using result = indices_of<float>;

//print<result> p;




namespace fsm {

	template <class Derived> class StateMachine;

	/**@class		StateMachine
	 * @brief		very simple compile-time state machine
	 **/
	template <class Derived>
	class StateMachine
	{
	protected:
		int m_state;

		StateMachine () : m_state(Derived::initial_state) { }
		
		void reset () { m_state = Derived::initial_state; }

		template <int CurrentState, class Event, int NextState, void (Derived::*action)(Event const &) >
		struct row
		{
			static int const current_state = CurrentState; // @TODO: std::integral_constant
			static int const next_state = NextState;
			typedef Event event;
			typedef Derived fsm_t;

			static void execute (Derived & fsm, Event const & e) // do the transition action
			{
				(fsm.*action)(e);
			}
		};
		
		template <class Event>
		int call_no_transition (int state, Event const & e)
		{
			return static_cast<Derived*>(this)->no_transition(state, e);
		}

	
public:
		int State () const { return m_state; }


		template<int TransIndex>
	  bool transition_fn (int currentState, int & newState)
		{
			using Table = typename Derived::transition_table;
      using row_t = mp_at_c<Table, TransIndex>;
			printf("%i: row= %i, ev, %i, op, \n", row_t::current_state, row_t::next_state);
      //print<row_t> p;
			printf("%i: state=%i --> state=%i\n", TransIndex, currentState, newState);
			return false;
		}

		template<class T>
		struct transition_select;

		template<template<int...> class L, int... T>
		struct transition_select<L<T...>>
		{
      StateMachine<Derived> * m_fsm;
      
      transition_select (StateMachine<Derived> * fsm) : m_fsm(fsm) { }

			int operator() (int state)
			{
				using transition_fn_prototype = bool (StateMachine::*)(int, int &);
				constexpr static transition_fn_prototype funcs[] = { &StateMachine::transition_fn<T>... };

				int new_state = -1;
				for (auto & fn : funcs)
          if (const bool accept = (m_fsm->*fn)(state, new_state))
            return new_state;
        //assert(0);
				return new_state;
			}
		};

		template <class Event>
		int dispatch_transition (int state, Event const & e)
		{
			using Table = typename Derived::transition_table;

			//using sz = mp_size<Table>;
			using indices = mp_indicesof<Table, Event, cmp_row>;
			//print<indices> p;

			transition_select<indices> ts(this);
			int const new_state = ts(state);
			printf("new state = %i\n", new_state);
			return new_state;

			//static_assert(false, "ee");
			//cassert(0); // transition is missing in transition_table
		}


		template <class Event>
		int ProcessEvent (Event const & event)
		{
			int const curr_state = this->m_state;
			int const next_state = dispatch_transition(curr_state, event);
			this->m_state = next_state;
		}

		template<class RowT, class V>
		using cmp_row = mp_if<std::is_same<typename RowT::event, V>, std::true_type, std::false_type>;



		template <class Event>
		int no_transition (int state, Event const & e)
		{
			//static_assert(false, "ee");
			//cassert(0); // transition is missing in transition_table
			return state;
		}
	};

/*
	// get the Event associated with a transition.
	template <class Transition>
	struct transition_event
	{
		typedef typename Transition::event type;
	};

	template <class Table, class Event>
	struct generate_dispatcher
		: boost::mpl::fold<
				boost::mpl::filter_view< Table , boost::is_same<Event, transition_event<boost::mpl::_1>> >
				, default_event_dispatcher
				, event_dispatcher<boost::mpl::_2, boost::mpl::_1>
			>
	{ };

*/

} // namespace fsm

