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
#ifndef FTL_FOLDABLE_H
#define FTL_FOLDABLE_H

#include "monoid.h"
#include "../prelude.h"

namespace ftl {
	/**
	 * \page foldablepg Foldable
	 *
	 * Abstraction of data structures that can be folded to some accumulated
	 * value.
	 *
	 * Despite what one may be lead to believe, this concept does not quite
	 * correspond to the mathematical notion of a catamorphism. Unlike a _true_
	 * catamorphism, the `Foldable` concept can only traverse structures
	 * "linearly". I.e., it is impossible to use a fold to build an isomorphic
	 * representation of a tree.
	 *
	 * For a concrete definition of what an instance must fulfill, see the
	 * `ftl::foldable` interface.
	 *
	 * \see \ref foldable (module)
	 */

	/**
	 * \defgroup foldable Foldable
	 *
	 * Interface and utilities relating to the \ref foldablepg concept.
	 *
	 * \code
	 *   #include <ftl/concepts/foldable.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref prelude
	 * - \ref monoid
	 */

	/**
	 * \interface foldable
	 *
	 * Concrete definition of the foldable concept.
	 *
	 * The most typical types that are instances of foldable are containers of
	 * different sorts, in which case the fold operation is usually a
	 * generalisation of a "sum" operation on the elements of the container.
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct foldable {
		/// Typedef for easier reference to the type to fold on
		using T = concept_parameter<F>;

// Just as in monad, we don't want the compiler to find these, but the API
// reference generator should.
#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Fold a structure containing a monoidal type.
		 *
		 * Examples:
		 * \code
		 *   std::vector<ftl::sum_monoid<int>> v{sum(2), sum(4), sum(10)};
		 *   ftl::fold(v); // sum(16)
		 * \endcode
		 *
		 * \tparam T must be a \ref monoid
		 */
		template<typename = typename std::enable_if<Monoid<T>()>::type>
		static T fold(const F& f);

		/**
		 * Map each element to a monoid and fold the result.
		 *
		 * Example:
		 * \code
		 *   std::list<int> l{2,2,2};
		 *
		 *   ftl::foldMap(ftl::prod<int>, l); // prod(8)
		 * \endcode
		 *
		 * \tparam Fn must satisfy \ref fn`<`\ref monoidpg`(A)>`
		 */
		template<
				typename Fn,
				typename M = typename std::result_of<Fn(T)>::type,
				typename = typename std::enable_if<Monoid<M>()>::type
		>
		static M foldMap(Fn&& fn, const F& f);

		/**
		 * Right associative fold.
		 *
		 * Example:
		 * \code
		 *   std::forward_list<int> l{4, 8, 5};
		 *
		 *   // (4 - (8 - (5 - 3))) = -2
		 *   ftl::foldr([](int x, int y){ return x-y; }, 3, l);
		 * \endcode
		 *
		 * \param fn Binary folding function
		 * \param z Initial "zero" value to start/end the fold
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<U(T,U)>`
		 */
		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<U, result_of<Fn(T,U)>>::value
				>::type
		>
		static U foldr(Fn&& fn, U&& z, const F& f);

		/**
		 * Left associative fold.
		 *
		 * Example:
		 * \code
		 *   std::forward_list<int> l{4, 8, 5};
		 *
		 *   // (((3 - 4) - 8) - 5) = -14
		 *   ftl::foldl([](int x, int y){ return x-y; }, 3, l);
		 * \endcode
		 *
		 *
		 * \param fn Binary folding function
		 * \param z Zero value to combine first element with
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<U(U,T)>`
		 */
		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<U, result_of<Fn(U,T)>>::value
				>::type
		>
		static U foldl(Fn&& fn, U&& z, const F& f);

#endif

		/// Compile time constant to check if a type is an instance.
		static constexpr bool instance = false;
	};

	/**
	 * Concepts lite-compatible check for foldable instances.
	 *
	 * Can also be used with SFINAE, for much the same purposes.
	 *
	 * Example:
	 * \code
	 *   template<
	 *       typename F,
	 *       typename = typename std::enable_if<Foldable<F>()>::type
	 *   >
	 *   void foo(const F& f) {
	 *       // Perform folds on f
	 *   }
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	constexpr bool Foldable() noexcept {
		return foldable<F>::instance;
	}

	/**
	 * An inheritable implementation of `foldable::foldl`.
	 *
	 * Any type that satisfies \ref fwditerable may have their \ref foldablepg
	 * instance simply inherit this implementation of `foldl` instead of
	 * implementing it manually.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct foldable<MyContainer<T>> : deriving_foldl<MyContainer<T>> {
	 *       // Rest of implementation
	 *   };
	 * \endcode
	 */
	template<
			typename F,
			typename = typename std::enable_if<ForwardIterable<F>()>::type
	>
	struct deriving_foldl {
		using T = concept_parameter<F>;

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_convertible<
						typename std::result_of<Fn(U,T)>::type,
						U
					>::value
				>::type
		>
		static U foldl(Fn&& fn, U z, const F& f) {
			for(auto& e : f) {
				z = fn(z, e);
			}

			return z;
		}

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_convertible<
						std::result_of<Fn(U,T)>,
						plain_type<U>
					>::value
				>::type
		>
		static U foldl(Fn&& fn, U z, F&& f) {
			for(auto& e : f) {
				z = fn(z, std::move(e));
			}

			return z;
		}
	};

	/**
	 * Inheritable implementation of `foldable::foldMap`.
	 *
	 * Foldable instances implementing `foldable::foldl` can simply inherit from
	 * this struct to get `foldable::foldMap` for "free". Naturally, it works
	 * together with `ftl::deriving_foldl`.
	 *
	 * Example:
	 * \code
	 *     template<>
	 *     struct foldable<MyFoldable> : deriving_foldMap<MyFoldable> {
	 *         // Implementation of foldl and foldr
	 *     };
	 * \endcode
	 *
	 * \tparam F the foldable for which to implement `foldMap`.
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_foldMap {
		template<
				typename Fn,
				typename T = concept_parameter<F>,
				typename M = result_of<Fn(T)>,
				typename = typename std::enable_if<Monoid<M>()>::type
		>
		static M foldMap(Fn fn, const F& f) {
			return foldable<F>::foldl(
					[fn](const T& a, const M& b) {
						return monoid<M>::append(
							fn(a),
							b);
					},
					monoid<M>::id(),
					f);
		}
	};

	/**
	 * Inheritable implementation of foldable::fold.
	 *
	 * Foldable specialisations implementing foldable::foldMap can inherit from
	 * this struct to get `foldable::fold` for "free".
	 *
	 * It is entirely possible for a foldable implementation to use both 
	 * `deriving_foldMap<F>` and `deriving_fold<F>`, even in reverse order.
	 *
	 * \tparam F the foldable instance (not implementation) for which `fold`
	 *           should apply.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct foldable<myListType<T>>
	 *   : deriving_foldMap<myListType<T>>, deriving_fold<myListType<T>> {
	 *       
	 *       // Implementations of foldl and foldr
	 *   };
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_fold {
		template<
				typename M = concept_parameter<F>,
				typename = typename std::enable_if<Monoid<M>()>::type
		>
		static M fold(const F& f) {
			return foldable<F>::foldMap(id, f);
		}
	};

	/**
	 * Convenience function alias of foldable<T>::fold.
	 *
	 * \ingroup foldable
	 */
	template<
			typename F,
			typename M = concept_parameter<F>,
			typename = typename std::enable_if<Foldable<F>()>::type,
			typename = typename std::enable_if<Monoid<M>()>::type
	>
	M fold(const F& f) {
		return foldable<F>::fold(f);
	}

	/**
	 * Convenience function object alias of foldable<T>::foldMap.
	 *
	 * Supports curried calling convention, where a user can supply one
	 * parameter at a time.
	 *
	 * \ingroup foldable
	 */
	struct FoldMap
#ifndef DOCUMENTATION_GENERATOR
	: _dtl::curried_binf<FoldMap>
#endif
	{
		template<
				typename F,
				typename T = concept_parameter<F>,
				typename Fn,
				typename = typename std::enable_if<Foldable<F>()>::type
		>
		auto operator() (Fn&& fn, const F& f) const
		-> decltype(foldable<F>::foldMap(std::forward<Fn>(fn), f)) {
			return foldable<F>::foldMap(std::forward<Fn>(fn), f);
		}

		using curried_binf<FoldMap>::operator();
	};

	/**
	 * Compile time convenience instance of FoldMap.
	 *
	 * Makes `foldable::foldMap` generally a lot cleaner to use, as well as
	 * easier to pass to higher order functions.
	 *
	 * \ingroup foldable
	 */
	constexpr FoldMap foldMap{};

	/**
	 * Convenience function object alias of `foldable::foldr`.
	 *
	 * Allows curried calling semantics, e.g. any of:
	 * \code
	 *   ftl::foldr(fn, z, f);
	 *   ftl::foldr(fn)(z, f);
	 *   ftl::foldr(fn)(z)(f);
	 * \endcode
	 * are valid ways of invoking `foldr`.
	 *
	 * \ingroup foldable
	 */
	struct foldR
#ifndef DOCUMENTATION_GENERATOR
	: _dtl::curried_ternf<foldR>
#endif
	{
		template<
				typename F,
				typename Fn,
				typename U,
				typename T = concept_parameter<F>,
				typename = typename std::enable_if<Foldable<F>()>::type,
				typename = typename std::enable_if<
					std::is_same<plain_type<U>, result_of<Fn(T,U)>>::value
				>::type
		>
		plain_type<U> operator() (Fn&& fn, U&& z, const F& f) const {
			return foldable<F>::foldr(std::forward<Fn>(fn), std::forward<U>(z), f);
		}

		using curried_ternf<foldR>::operator();
	};

	/**
	 * Compile time convenience instance of foldR.
	 *
	 * \ingroup foldable
	 */
	constexpr foldR foldr{};

	/**
	 * Convenience function object alias of foldable<T>::foldl.
	 *
	 * \ingroup foldable
	 */
	struct foldL
#ifndef DOCUMENTATION_GENERATOR
	: _dtl::curried_ternf<foldL>
#endif
	{
		template<
				typename F,
				typename Fn,
				typename U,
				typename T = concept_parameter<F>,
				typename = typename std::enable_if<Foldable<F>()>::type,
				typename = typename std::enable_if<
					std::is_same<plain_type<U>, result_of<Fn(U,T)>>::value
				>::type
		>
		plain_type<U> operator() (Fn&& fn, U&& z, const F& f) const {
			return foldable<F>::foldl(std::forward<Fn>(fn), std::forward<U>(z), f);
		}

		using curried_ternf<foldL>::operator();
	};

	/**
	 * Compile time convenience instance of foldL.
	 *
	 * \ingroup foldable
	 */
	constexpr foldL foldl{};

}

#endif
