/*
 * Copyright (c) 2013 Björn Aili
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */
#ifndef FTL_FORWARDS_LIST_H
#define FTL_FORWARDS_LIST_H

#include <forward_list>
#include "concepts/foldable.h"
#include "concepts/monad.h"

namespace ftl {

	/**
	 * \defgroup fwdlist Forward List
	 *
	 * Singly linked list, its concept implementations, etc.
	 *
	 * \code
	 *   #include <ftl/forward_list.h>
	 * \endcode
	 *
	 * This module adds the following concept instances to std::forward_list:
	 * - \ref monoid
	 * - \ref foldable
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \par Dependencies
	 * - <forward_list>
	 * - \ref foldable
	 * - \ref monad
	 */

	/**
	 * Specialisation of re_parametrise for forward_lists.
	 *
	 * This makes sure the allocator is also properly parametrised on the
	 * new element type.
	 *
	 * \ingroup list
	 */
	template<typename T, typename U, typename A>
	struct re_parametrise<std::forward_list<T,A>,U> {
	private:
		using Au = typename re_parametrise<A,U>::type;

	public:
		using type = std::forward_list<U,Au>;
	};

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref container`<B>(A)>`
	 *
	 * \ingroup fwdlist
	 */
	template<
			typename F,
			typename T,
			typename A,
			typename U = typename result_of<F(T)>::value_type,
			typename Au = typename re_parametrise<A,U>::type
	>
	std::forward_list<U,Au> concatMap(
			F&& f,
			const std::forward_list<T,A>& l) {

		std::forward_list<U,Au> result;
		auto nested = std::forward<F>(f) % l;

		auto it = result.before_begin();
		for(auto& el : nested) {
			it = result.insert_after(
					it,
					std::make_move_iterator(el.begin()),
					std::make_move_iterator(el.end())
			);
		}

		return result;
	}

	/**
	 * \overload
	 *
	 * \ingroup fwdlist
	 */
	template<
			typename F,
			typename T,
			typename A,
			typename U = typename result_of<F(T)>::value_type,
			typename Au = typename re_parametrise<A,U>::type
	>
	std::forward_list<U,Au> concatMap(
			F&& f,
			std::forward_list<T,A>&& l) {

		auto nested = std::forward<F>(f) % std::move(l);

		std::forward_list<U,Au> result;

		auto it = result.before_begin();
		for(auto& el : nested) {
			it = result.insert_after(
					it,
					std::make_move_iterator(el.begin()),
					std::make_move_iterator(el.end())
			);
		}

		return result;
	}

	/**
	 * Monoid implementation for std::forward_list
	 *
	 * Identity element is the empty list, monoid operation is list
	 * concatenation.
	 * 
	 * \ingroup fwdlist
	 */
	template<typename...Ts>
	struct monoid<std::forward_list<Ts...>> {
		static std::forward_list<Ts...> id() {
			return std::forward_list<Ts...>();
		}

		static std::forward_list<Ts...> append(
				const std::forward_list<Ts...>& l1,
				const std::forward_list<Ts...>& l2) {

			std::forward_list<Ts...> rl(l2);

			rl.insert_after(rl.before_begin(), l1.begin(), l1.end());
			return rl;
		}

		// Optimised cases for when one list can be spliced into the other
		static std::forward_list<Ts...> append(
				std::forward_list<Ts...>&& l1,
				const std::forward_list<Ts...>& l2) {

			std::forward_list<Ts...> rl(l2);

			rl.splice_after(rl.before_begin(), std::move(l1));

			return rl;
		}

		static std::forward_list<Ts...> append(
				const std::forward_list<Ts...>& l1,
				std::forward_list<Ts...>&& l2) {

			std::forward_list<Ts...> l(l1);

			l2.splice_after(l2.before_begin(), std::move(l));

			return l2;
		}

		static std::forward_list<Ts...> append(
				std::forward_list<Ts...>&& l1,
				std::forward_list<Ts...>&& l2) {

			l2.splice_after(l2.before_begin(), std::move(l1));

			return l2;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad instance of forward_lists
	 *
	 * This instance is equivalent to the other container monads, e.g.
	 * monad<std::list<T>>, and monad<std::vector<T>>.
	 *
	 * \ingroup fwdlist
	 */
	template<typename T, typename A>
	struct monad<std::forward_list<T,A>>
	: deriving_join<std::forward_list<T,A>>
	, deriving_apply<std::forward_list<T,A>> {

		/// Alias to make type signatures more easily read.
		template<typename U>
		using forward_list
			= typename re_parametrise<std::forward_list<T,A>,U>::type;

		/**
		 * Embed a `T` in a forward list.
		 *
		 * Simply creates a singleton list, containing `t`.
		 */
		static forward_list<T> pure(const T& t) {
			forward_list<T> l;
			l.emplace_front(t);
			return l;
		}


		/// \overload
		static forward_list<T> pure(T&& t) {
			forward_list<T> l;
			l.emplace_front(std::move(t));
			return l;
		}

		/**
		 * Maps the given function over all elements in the list.
		 *
		 * Similar to std::transform, except `l` is not mutated and `f` is
		 * allowed to change domain.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static forward_list<U> map(F&& f, const forward_list<T>& l) {

			forward_list<U> rl;
			auto it = rl.before_begin();
			for(const auto& e : l) {
				it = rl.insert_after(it, f(e));
			}

			return rl;
		}

		/// \overload
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename =
					typename std::enable_if<!std::is_same<T,U>::value>::type

		>
		static forward_list<U> map(F&& f, forward_list<T>&& l) {

			forward_list<U> rl;
			auto it = rl.before_begin();
			for(auto& e : l) {
				it = rl.insert_after(it, f(std::move(e)));
			}

			return rl;
		}

		/**
		 * A no-copies map.
		 *
		 * Kicks in if `f` does not change domain and `l` is a temporary.
		 */
		template<
				typename F,
				typename = typename std::enable_if<
					std::is_same<T, result_of<F(T)>>::value
				>::type>
		static forward_list<T> map(F&& f, forward_list<T>&& l) {

			auto rl = std::move(l);
			for(auto& e : rl) {
				e = f(e);
			}

			return rl;
		}

		/**
		 * Monadic bind operation.
		 *
		 * Equivalent of `flip(concatMap)`.
		 */
		template<typename F, typename U = typename result_of<F(T)>::value_type>
		static forward_list<U> bind(const forward_list<T>& l, F&& f) {

			return concatMap(std::forward<F>(f), l);
		}

		/// \overload
		template<typename F, typename U = typename result_of<F(T)>::value_type>
		static forward_list<U> bind(forward_list<T>&& l, F&& f) {

			return concatMap(std::forward<F>(f), std::move(l));
		}

		static constexpr bool instance = true;

	};

	namespace _dtl {
		template<typename F, typename Z, typename It>
		Z fwdfoldr(F&& f, Z&& z, It it, It end) {
			if(it != end) {
				auto& x = *it;
				return f(
					x,
					fwdfoldr(
						std::forward<F>(f),
						std::forward<Z>(z),
						++it,
						end
					)
				);
			}

			return z;
		}
	}

	/**
	 * Instance implementation of Foldable for std::forward_lists.
	 *
	 * \ingroup fwdlist
	 */
	template<typename T, typename A>
	struct foldable<std::forward_list<T,A>> :
			deriving_foldl<std::forward_list<T,A>>,
			deriving_fold<std::forward_list<T,A>>,
			deriving_foldMap<std::forward_list<T,A>> {

		template<
				typename F,
				typename U,
				typename = typename std::enable_if<
					std::is_same<U, result_of<F(T,U)>>::value
				>::type
		>
		static U foldr(F&& f, U&& z, const std::forward_list<T,A>& l) {
			return _dtl::fwdfoldr(
					std::forward<F>(f),
					std::forward<U>(z),
					l.cbegin(), l.cend());
		}

		static constexpr bool instance = true;
	};

}

#endif
