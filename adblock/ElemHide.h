/*!
 * \file ElemHide.h
 *
 * \author yorath
 * \date October 16, 2013
 *
 * \details Element hiding implementation.
 */

#pragma once


#include "Filter.h"
#include <boost/unordered_set.hpp>


namespace NS_ADBLOCK {

  class ElemHide {
  public:

    /**
     * Removes all known filters
     */
    void clear();

    /**
     * Add a new element hiding filter
     */
    void add(const ElemHideBasePtr &filter);

    /**
     * Removes an element hiding filter
     */
    void remove(const ElemHideBasePtr &filter);

    /**
     * Checks whether an exception rule is registered for a filter
     * on a particular domain
     */
    ElemHideExceptionPtr get_exception(const ElemHideBasePtr &filter,
      const std::string &doc_domain);

    /**
     * Returns a list of all selectors active on a particular domain
     * (currently used only in Chrome).
     */
    std::vector<std::string> get_selectors(const std::string &domain,
      bool specific);

  private:

    typedef boost::unordered_set<ElemHideFilterPtr> ElemFilters;
    /**
     * Element hiding filters
     */
    ElemFilters elem_filters_;

    typedef boost::unordered_set<std::string> KnownExceptions;
    /**
     * Known element hiding exceptions
     */
    KnownExceptions known_exceptions_;

    typedef boost::unordered_map<std::string, std::vector<ElemHideExceptionPtr>> Exceptions;
    /**
     * Lookup table, lists of element hiding exceptions by selector
     */
    Exceptions exceptions_;
  };

}
