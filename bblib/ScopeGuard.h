#pragma once
/**
 * This is adoption of nice technique for automatic treatment of resources at
 * the end of scope. Original article comes from:
 * [1] http://www.cuj.com/documents/s=8000/cujcexp1812alexandr/
 *
 * ScopeGuard is a generalization of a typical detailementation of the
 * "initialization is resource acquisition" C++ idiom.  The difference is that
 * ScopeGuard focuses only on the cleanup part - you do the resource
 * acquisition, and ScopeGuard takes care of relinquishing the resource.
 */

/**
 * How to use it:
 *      {
 *          bb::scope_guard_t guard = bb::mkScopeGuard(bb::mk_functor(&log_leave));
 *          if (something goes wrong)
 *              throw std::exception; // the guard will call log_leave
 *          else
 *              guard.Dismiss();
 *      }
 *
 *      or simply
 *      {
 *          SCOPE_GUARD(guard, boost::bind(&foo::bar, f, true));
 *          ...
 *          guard.Dismiss(); // optional
 *      }
 *
 *      This code will construct a temporary reference guard which will be destroyed
 *      at the exit of the scope and call log_leave function.
 *
 *      The guard can be revoked by calling its member method Dismiss. 
 */

#define SCOPE_GUARD(guard_name, operation)	\
		bb::scope_guard_t guard_name = bb::mkScopeGuard(operation);

//bb::fakeScopeGuardUsage(&guard_name)


namespace bb { namespace detail {

	struct ScopeGuardBase
	{
		void Dismiss () const { m_Dismissed = true; }
	protected:
		ScopeGuardBase () : m_Dismissed(false) {}
		ScopeGuardBase (ScopeGuardBase const & rhs) : m_Dismissed(rhs.m_Dismissed) { rhs.Dismiss(); }
		~ScopeGuardBase () { } // nonvirtual! see [1] why
		mutable bool m_Dismissed;
	private:
		ScopeGuardBase & operator= (ScopeGuardBase const &); // assignment denied
	};


	template <class _FunctorT>
	struct ScopeGuard_0 : ScopeGuardBase
	{
		ScopeGuard_0 (_FunctorT const & fctor) : m_fctor(fctor) { }

		~ScopeGuard_0 ()
		{
			if (!m_Dismissed)
				m_fctor();
		}

	private:
		_FunctorT m_fctor;
	};


	template <class _FunctorT>
	struct ScopeGuard_1 : ScopeGuardBase
	{
		ScopeGuard_1 (_FunctorT const & fctor, typename _FunctorT::argument_type arg) : m_fctor(fctor), m_arg(arg) { }

		~ScopeGuard_1 ()
		{
			if (!m_Dismissed)
				m_fctor(m_arg);
		}

	private:
		_FunctorT m_fctor;
		typename _FunctorT::argument_type m_arg;
	};


	template <class _FunctorT>
	struct ScopeGuard_2 : ScopeGuardBase
	{
		ScopeGuard_2 (_FunctorT const & fctor, typename _FunctorT::first_argument_type arg1, typename _FunctorT::second_argument_type arg2)
			: m_fctor(fctor), m_arg1(arg1), m_arg2(arg2)
		{ }

		~ScopeGuard_2 ()
		{
			if (!m_Dismissed)
				m_fctor(m_arg1, m_arg2);
		}

	private:
		_FunctorT m_fctor;
		typename _FunctorT::first_argument_type m_arg1;
		typename _FunctorT::second_argument_type m_arg2;
	};

}} // namespace bb::detail


namespace bb {

	typedef detail::ScopeGuardBase const & scope_guard_t;

	template <class _FunctorT>
	detail::ScopeGuard_0<_FunctorT> mkScopeGuard (_FunctorT const & f)
	{
		return detail::ScopeGuard_0<_FunctorT>(f);
	}

	/// creating ScopeGuard from derivate of std::unary_function
	template <class _FunctorT>
	detail::ScopeGuard_1<_FunctorT> mkScopeGuard (_FunctorT const & f, typename _FunctorT::argument_type arg)
	{
		return detail::ScopeGuard_1<_FunctorT>(f, arg);
	}

	/// creating ScopeGuard from derivate of std::binary_function
	template <class _FunctorT>
	detail::ScopeGuard_2<_FunctorT> mkScopeGuard (_FunctorT const & f, typename _FunctorT::first_argument_type arg1, typename _FunctorT::second_argument_type arg2)
	{
		return detail::ScopeGuard_2<_FunctorT>(f, arg1, arg2);
	}

	// dummy call to shut the hell up the compiler's bloody mouth
	inline void fakeScopeGuardUsage (...) { }

} // namespace bb

