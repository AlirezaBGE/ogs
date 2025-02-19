/**
 * \author Norihiro Watanabe
 * \date   2013-04-16
 *
 * \file
 * \copyright
 * Copyright (c) 2012-2023, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <utility>
#include <vector>

namespace NumLib
{
struct SerialExecutor
{
    /// Executes \c f for each element from the input container.
    ///
    /// The elements of \c c are dereferenced before being passed to \c f.
    /// Return values of the function call are ignored.
    ///
    /// \note
    /// This function is intended to be used if \c f is a plain function,
    /// a static class method, a lambda or an object having a call operator
    /// defined. For non-static member functions you are encouraged to use
    /// executeMemberDereferenced(), if applicable.
    ///
    /// \tparam F    type of the callback function \c f.
    /// \tparam C    input container type.
    /// \tparam Args types additional arguments passed to \c f.
    ///
    /// \param f    a function that accepts a reference to container's
    ///             elements,
    ///             an index as arguments (and possibly further arguments).
    /// \param c    a container supporting access over operator[].
    ///             The elements of \c c must have pointer semantics, i.e.,
    ///             support dereferencing via unary operator*().
    /// \param args additional arguments passed to \c f.
    ///
    template <typename F, typename C, typename... Args_>
    static void executeDereferenced(F const& f, C const& c, Args_&&... args)
    {
        for (std::size_t i = 0; i < c.size(); i++)
        {
            f(i, *c[i], std::forward<Args_>(args)...);
        }
    }

    /// Executes the given \c method of the given \c object for each element
    /// from the input \c container.
    ///
    /// This method is very similar to executeDereferenced().
    ///
    /// \param container collection of objects having pointer semantics.
    /// \param object    the object whose method will be called.
    /// \param method    the method being called, i.e., a member function
    ///                  pointer
    ///                  to a member function of the class \c Object.
    /// \param args      further arguments passed on to the method
    ///
    /// \see executeDereferenced()
    template <typename Container, typename Object, typename Method,
              typename... Args>
    static void executeMemberDereferenced(Object& object, Method method,
                                          Container const& container,
                                          Args&&... args)
    {
        for (std::size_t i = 0; i < container.size(); i++)
        {
            (object.*method)(i, *container[i], std::forward<Args>(args)...);
        }
    }

    /// Executes the given \c method of the given \c object for the selected
    /// elements from the input \c container.
    ///
    /// This method is very similar to executeDereferenced().
    ///
    /// \param object               the object whose method will be called.
    /// \param method               the method being called, i.e., a member
    ///                             function pointer to a member function of the
    ///                             class \c Object.
    /// \param container            collection of objects having pointer
    ///                             semantics.
    /// \param active_container_ids The IDs of active elements of \c container.
    /// \param args                 further arguments passed on to the method.
    ///
    /// \see executeDereferenced()
    template <typename Container, typename Object, typename Method,
              typename... Args>
    static void executeSelectedMemberDereferenced(
        Object& object, Method method, Container const& container,
        std::vector<std::size_t> const& active_container_ids, Args&&... args)
    {
        if (active_container_ids.empty())
        {
            executeMemberDereferenced(object, method, container,
                                      std::forward<Args>(args)...);
            return;
        }

        for (auto const id : active_container_ids)
        {
            (object.*method)(id, *container[id], std::forward<Args>(args)...);
        }
    }

    /// Executes the given \c method on each element of the input \c container.
    ///
    /// This method is very similar to executeMemberDereferenced().
    ///
    /// \param container collection of objects having pointer semantics.
    /// \param method    the method being called, i.e., a member function
    ///                  pointer.
    ///                  to a member function of the \c container's elements.
    /// \param args      further arguments passed on to the method
    ///
    /// \see executeDereferenced()
    template <typename Container, typename Method, typename... Args>
    static void executeMemberOnDereferenced(Method method,
                                            Container const& container,
                                            Args&&... args)
    {
        for (std::size_t i = 0; i < container.size(); i++)
        {
            ((*container[i]).*method)(i, std::forward<Args>(args)...);
        }
    }

    /// Executes the given \c method on the selected elements of the input \c
    /// container.
    ///
    /// This method is very similar to executeSelectedMemberDereferenced().
    ///
    /// \param method               the method being called, i.e., a member
    ///                             function pointer to a member function of the
    ///                             \c container's elements.
    /// \param container            collection of objects having pointer
    ///                             semantics.
    /// \param active_container_ids The IDs of active elements of \c container.
    /// \param args                 further arguments passed on to the method.
    ///
    /// \see executeDereferenced()
    template <typename Container, typename Method, typename... Args>
    static void executeSelectedMemberOnDereferenced(
        Method method, Container const& container,
        std::vector<std::size_t> const& active_container_ids, Args&&... args)
    {
        if (active_container_ids.empty())
        {
            executeMemberOnDereferenced(method, container,
                                        std::forward<Args>(args)...);
            return;
        }

        for (auto const id : active_container_ids)
        {
            ((*container[id]).*method)(id, std::forward<Args>(args)...);
        }
    }

    /// Same as execute(f, c), but with two containers, where the second one is
    /// modified.
    ///
    /// \tparam F    \c f type.
    /// \tparam C    input container type.
    /// \tparam Data input/output container type.
    ///
    /// \param f    a function that accepts a pointer to container's elements,
    ///             an index, and a second container element as arguments, which
    ///             is modified.
    /// \param c    a container supporting const access over operator[] and
    ///             size().
    ///             The elements of \c c must have pointer semantics, i.e.,
    ///             support dereferencing via unary operator*().
    /// \param data a container supporting non-const access over operator[] and
    ///             size().
    /// \param args additional arguments passed to \c f
    template <typename F, typename C, typename Data, typename... Args_>
    static void transformDereferenced(F const& f, C const& c, Data& data,
                                      Args_&&... args)
    {
        assert(c.size() == data.size());

        for (std::size_t i = 0; i < c.size(); i++)
        {
            data[i] = f(i, *c[i], std::forward<Args_>(args)...);
        }
    }
};

}  // namespace NumLib
