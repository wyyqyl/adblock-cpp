#include "Matcher.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <sstream>


namespace NS_ADBLOCK {

  void Matcher::clear() {
    filter_by_keyword_.clear();
    keyword_by_filter_.clear();
  }

  void Matcher::add(const RegExpFilterPtr &filter) {
    if (keyword_by_filter_.find(filter->get_text()) != keyword_by_filter_.end()) {
      return;
    }
    
    // Look for a suitable keyword
    std::string keyword = find_keyword(filter);
    filter_by_keyword_[keyword].push_back(filter);
    keyword_by_filter_[filter->get_text()] = keyword;
  }

  void Matcher::remove(const RegExpFilterPtr &filter) {
    if (keyword_by_filter_.find(filter->get_text()) == keyword_by_filter_.end()) {
      return;
    }

    std::string keyword = keyword_by_filter_[filter->get_text()];
    filter_by_keyword_.erase(keyword);
    keyword_by_filter_.erase(filter->get_text());
  }

  std::string Matcher::find_keyword(const RegExpFilterPtr &filter) {
    std::string result;
    std::string text = filter->get_text();

    if (boost::regex_search(text, Filter::RegexRegex)) {
      return result;
    }

    // Remove options
    boost::smatch match;
    if (boost::regex_search(text, match, Filter::OptionsRegex)) {
      text = std::string(text.begin(), match[0].first);
    }

    // Remove whitelist marker
    if (text.substr(0, 2) == "@@") {
      text = text.substr(2);
    }

    boost::to_lower(text);
    auto token_iter = make_regex_token_iterator(text,
      boost::regex("[^a-z0-9%*][a-z0-9%]{3,}(?=[^a-z0-9%*])"), 0);
    decltype(token_iter) token_end;

    if (token_iter == token_end) {
      return result;
    }

    uint32_t result_count = 0xFFFFFF;
    uint32_t result_len = 0;
    while (token_iter != token_end) {
      std::string candidate = token_iter++->str().substr(1);
      uint32_t count = 0;
      auto iter = filter_by_keyword_.find(candidate);
      if (iter != filter_by_keyword_.end()) {
        count = iter->second.size();
      }
      if (count < result_count || (count == result_count && candidate.length() > result_len)) {
        result = candidate;
        result_count = count;
        result_len = candidate.length();
      }
    }
    return result;
  }

  bool Matcher::has_filter(const RegExpFilterPtr &filter) {
    return (keyword_by_filter_.find(filter->get_text()) != keyword_by_filter_.end());
  }

  std::string Matcher::get_keyword(
    const RegExpFilterPtr &filter
    )
  {
    std::string result;
    auto iter = keyword_by_filter_.find(filter->get_text());
    if (iter != keyword_by_filter_.end()) {
      result = iter->second;
    }
    return result;
  }

  RegExpFilterPtr Matcher::matches_any(
    const std::string &location,
    const std::string &content_type,
    const std::string &doc_domain,
    bool third_party
    )
  {
    auto token_iter = make_regex_token_iterator(
      boost::to_lower_copy(location), boost::regex("[a-z0-9%]{3,}"), 0);
    decltype(token_iter) token_end;

    std::vector<std::string> candidates(token_iter, token_end);
    candidates.push_back("");
    for (auto iter = candidates.begin(); iter != candidates.end(); ++iter) {
      std::string substr = *iter;
      if (filter_by_keyword_.find(substr) != filter_by_keyword_.end()) {
        RegExpFilterPtr result = check_entry_match(substr, location,
          content_type, doc_domain, third_party);
        if (result != nullptr) {
          return result;
        }
      }
    }
    return nullptr;
  }

  RegExpFilterPtr Matcher::check_entry_match(
    const std::string &keyword,
    const std::string &location,
    const std::string &content_type,
    const std::string &doc_domain,
    bool third_party
    )
  {
    auto iter = filter_by_keyword_.find(keyword);
    if (iter == filter_by_keyword_.end()) {
      return nullptr;
    }

    for (auto filter = iter->second.begin(); filter != iter->second.end(); ++filter) {
      if ((*filter)->matches(location, content_type, doc_domain, third_party)) {
        return *filter;
      }
    }
    return nullptr;
  }


  const uint32_t CombindMatcher::MaxCacheEntries = 1000;

  void CombindMatcher::clear() {
    blacklist_.clear();
    whitelist_.clear();
    keys_.clear();
    result_cache_.clear();
  }

  void CombindMatcher::add(const RegExpFilterPtr &filter) {
    if (filter->get_type() == WHITELIST_FILTER) {
      auto wfilter = boost::dynamic_pointer_cast<WhitelistFilter>(filter);
      if (wfilter->get_key_num() > 0) {
        for (uint32_t idx = 0; idx < wfilter->get_key_num(); ++idx) {
          keys_[wfilter->get_key(idx)] = wfilter->get_text();
        }
      } else {
        whitelist_.add(filter);
      }
    } else {
      blacklist_.add(filter);
    }

    if (result_cache_.size() > 0) {
      result_cache_.clear();
    }
  }

  void CombindMatcher::remove(const RegExpFilterPtr &filter) {
    if (filter->get_type() == WHITELIST_FILTER) {
      auto wfilter = boost::dynamic_pointer_cast<WhitelistFilter>(filter);
      if (wfilter->get_key_num() > 0) {
        for (uint32_t idx = 0; idx < wfilter->get_key_num(); ++idx) {
          keys_.erase(wfilter->get_key(idx));
        }
      } else {
        whitelist_.remove(filter);
      }
    } else {
      blacklist_.remove(filter);
    }

    if (result_cache_.size() > 0) {
      result_cache_.clear();
    }
  }

  std::string CombindMatcher::find_keyword(const RegExpFilterPtr &filter) {
    Matcher matcher = filter->get_type() == WHITELIST_FILTER ? whitelist_ : blacklist_;
    return matcher.find_keyword(filter);
  }

  bool CombindMatcher::has_filter(const RegExpFilterPtr &filter) {
    Matcher matcher = filter->get_type() == WHITELIST_FILTER ? whitelist_ : blacklist_;
    return matcher.has_filter(filter);
  }

  std::string CombindMatcher::get_keyword(const RegExpFilterPtr &filter) {
    Matcher matcher = filter->get_type() == WHITELIST_FILTER ? whitelist_ : blacklist_;
    return matcher.get_keyword(filter);
  }

  bool CombindMatcher::is_slow_filter(const RegExpFilterPtr &filter) {
    Matcher matcher = filter->get_type() == WHITELIST_FILTER ? whitelist_ : blacklist_;
    if (matcher.has_filter(filter)) {
      return matcher.get_keyword(filter).length() == 0;
    } else {
      return matcher.find_keyword(filter).length() == 0;
    }
  }

  RegExpFilterPtr CombindMatcher::matches_any_internal(
    const std::string &location,
    const std::string &content_type,
    const std::string &doc_domain,
    bool third_party
    )
  {
    auto token_iter = make_regex_token_iterator(
      boost::to_lower_copy(location), boost::regex("[a-z0-9%]{3,}"), 0);
    decltype(token_iter) token_end;

    std::vector<std::string> candidates(token_iter, token_end);
    candidates.push_back("");
    RegExpFilterPtr blacklisthit = nullptr;
    for (auto iter = candidates.begin(); iter != candidates.end(); ++iter) {
      std::string substr = *iter;
      RegExpFilterPtr result = whitelist_.check_entry_match(substr,
        location, content_type, doc_domain, third_party);
      if (result != nullptr) {
        return result;
      }
      if (blacklisthit == nullptr) {
        result = blacklist_.check_entry_match(substr, location,
          content_type, doc_domain, third_party);
        if (result != nullptr) {
          blacklisthit = result;
        }
      }
    }
    return blacklisthit;
  }

  RegExpFilterPtr CombindMatcher::matches_any(
    const std::string &location,
    const std::string &content_type,
    const std::string &doc_domain,
    bool third_party
    )
  {
    std::stringstream key;
    key << std::boolalpha << location << " " << content_type << " " << doc_domain << " " << third_party;

    auto iter = result_cache_.find(key.str());
    if (iter != result_cache_.end()) {
      return iter->second;
    }

    RegExpFilterPtr result = matches_any_internal(location, content_type, doc_domain, third_party);
    if (result_cache_.size() >= MaxCacheEntries) {
      result_cache_.clear();
    }

    result_cache_[key.str()] = result;

    return result;
  }

  RegExpFilterPtr CombindMatcher::matches_by_key(
    const std::string &location,
    std::string key,
    const std::string &doc_domain
    )
  {
    boost::to_upper(key);
    auto key_iter = keys_.find(key);
    if (key_iter != keys_.end()) {
      auto filter_iter = Filter::known_filters_.find(key_iter->second);
      if (filter_iter != Filter::known_filters_.end()) {
        auto filter = boost::dynamic_pointer_cast<RegExpFilter>(filter_iter->second);
        if (filter->matches(location, "DOCUMENT", doc_domain, false)) {
          return filter;
        }
      }
    }
    return nullptr;
  }

}
