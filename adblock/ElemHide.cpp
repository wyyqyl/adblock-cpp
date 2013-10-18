#include "ElemHide.h"


namespace NS_ADBLOCK {


  void ElemHide::clear() {
    elem_filters_.clear();
    known_exceptions_.clear();
    exceptions_.clear();
  }

  void ElemHide::add(const ElemHideBasePtr &filter) {
    if (filter->get_type() == ELEM_HIDE_EXCEPTION) {
      if (known_exceptions_.insert(filter->get_text()).second == true) {
        exceptions_[filter->get_selector()].push_back(
          boost::dynamic_pointer_cast<ElemHideException>(filter));
      }
    } else {
      elem_filters_.insert(boost::dynamic_pointer_cast<ElemHideFilter>(filter));
    }
  }

  void ElemHide::remove(const ElemHideBasePtr &filter) {
    if (filter->get_type() == ELEM_HIDE_EXCEPTION) {
      if (known_exceptions_.erase(filter->get_text()) == 1) {
        exceptions_.erase(filter->get_selector());
      }
    } else {
      elem_filters_.erase(boost::dynamic_pointer_cast<ElemHideFilter>(filter));
    }
  }

  NS_ADBLOCK::ElemHideExceptionPtr ElemHide::get_exception(
    const ElemHideBasePtr &filter,
    const std::string &doc_domain
    )
  {
    auto exceptions = exceptions_.find(filter->get_selector());
    if (exceptions == exceptions_.end()) {
      return nullptr;
    }

    for (auto exception = exceptions->second.begin();
      exception != exceptions->second.end(); ++exception)
    {
      if ((*exception)->is_active_on_domain(doc_domain)) {
        return *exception;
      }
    }

    return nullptr;
  }

  std::vector<std::string> ElemHide::get_selectors(
    const std::string &domain,
    bool specific
    )
  {
    std::vector<std::string> result;
    for (auto iter = elem_filters_.begin();
      iter != elem_filters_.end(); ++iter)
    {
      auto filter = *iter;
      if (specific) {
        auto domains = filter->get_domains();
        if (domains.size() == 0 || domains.find("")->second) {
          continue;
        }
      }

      if (filter->is_active_on_domain(domain) &&
        get_exception(filter, domain) != nullptr)
      {
        result.push_back(filter->get_selector());
      }
    }
    return result;
  }

}
