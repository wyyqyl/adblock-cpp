/*!
 * \file Matcher.h
 *
 * \author yorath
 * \date October 15, 2013
 *
 * \details Matcher class implementing matching addresses against
 * a list of filters.
 */

#pragma once


#include "Filter.h"


namespace NS_ADBLOCK {

  /**
   * Blacklist/whitelist filter matching
   */
  class Matcher {
  public:

    /**
     * Removes all known filters
     */
    void clear();

    /**
     * Adds a filter{RegExpFilter} to the matcher
     */
    void add(const RegExpFilterPtr &filter);

    /**
     * Removes a filter from the matcher
     */
    void remove(const RegExpFilterPtr &filter);

    /**
     * Chooses a keyword to be associated with the filter
     */
    std::string find_keyword(const RegExpFilterPtr &filter);

    /**
     * Checks whether a particular filter is being matched against.
     */
    bool has_filter(const RegExpFilterPtr &filter);

    /**
     * Returns the keyword used for a filter, null for unknown filters.
     */
    std::string get_keyword(const RegExpFilterPtr &filter);

    /**
     * Tests whether the URL matches any of the known filters
     *
     * \param location URL to be tested
     * \param content_type content type identifier of the URL
     * \param doc_domain domain name of the document that loads the URL
     * \param third_party should be true if the URL is a third-party request
     *
     * \return {RegExpFilter} matching filter or null
     */
    RegExpFilterPtr matches_any(const std::string &location,
      const std::string &content_type, const std::string &doc_domain,
      bool third_party);
    
    /**
     * Checks whether the entries for a particular keyword match a URL
     */
    RegExpFilterPtr check_entry_match(const std::string &keyword,
      const std::string &location, const std::string &content_type,
      const std::string &doc_domain, bool third_party);

  private:
    typedef boost::unordered_map<std::string, std::vector<RegExpFilterPtr>> FilterByKeyword;
    /**
     * Lookup table for filters by their associated keyword
     */
    FilterByKeyword filter_by_keyword_;

    typedef boost::unordered_map<std::string, std::string> KeywordByFilter;
    /**
     * Lookup table for keywords by the filter text
     */
    KeywordByFilter keyword_by_filter_;

  };

  typedef boost::shared_ptr<Matcher> MatcherPtr;


  /**
   * Combines a matcher for blocking and exception rules, automatically
   * sorts rules into two Matcher instances.
   */
  class CombindMatcher {
  public:

    /**
     * @see Matcher#clear
     */
    void clear();

    /**
     * @see Matcher#add
     */
    void add(const RegExpFilterPtr &filter);

    /**
     * @see Matcher#remove
     */
    void remove(const RegExpFilterPtr &filter);

    /**
     * @see Matcher#find_keyword
     */
    std::string find_keyword(const RegExpFilterPtr &filter);

    /**
     * @see Matcher#has_filter
     */
    bool has_filter(const RegExpFilterPtr &filter);

    /**
     * @see Matcher#get_keyword
     */
    std::string get_keyword(const RegExpFilterPtr &filter);

    /**
     * Checks whether a particular filter is slow
     */
    bool is_slow_filter(const RegExpFilterPtr &filter);

    /**
     * @see Matcher#matches_any
     */
    RegExpFilterPtr matches_any(const std::string &location,
      const std::string &content_type, const std::string &doc_domain,
      bool third_party);

    /**
     * Looks up whether any filters match the given website key.
     */
    RegExpFilterPtr matches_by_key(const std::string &location, std::string key,
      const std::string &doc_domain);

  private:

    /**
     * Matcher for blocking rules.
     */
    Matcher blacklist_;

    /**
     * Matcher for exception rules.
     */
    Matcher whitelist_;

    typedef boost::unordered_map<std::string, std::string> Keys;
    /**
     * Exception rules that are limited by public keys, mapped by
     * the corresponding keys.
     */
    Keys keys_;

    typedef boost::unordered_map<std::string, RegExpFilterPtr> ResultCache;
    /**
     * Lookup table of previous matchesAny results
     */
    ResultCache result_cache_;

    static const uint32_t MaxCacheEntries;

    /**
     * Optimized filter matching testing both whitelist and blacklist
     * matchers simultaneously. For parameters see Matcher.matches_any().
     * @see Matcher#matches_any
     */
    RegExpFilterPtr matches_any_internal(const std::string &location,
      const std::string &content_type, const std::string &doc_domain,
      bool third_party);

  };

}
