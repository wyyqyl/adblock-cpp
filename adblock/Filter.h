/*!
 * \file Filter.h
 *
 * \author yorath
 * \date October 11, 2013
 *
 * \details Different kinds of filters
 */

#pragma once


#include <cstdint>
#include <string>
#include <boost/container/map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/logic/tribool.hpp>


namespace NS_ADBLOCK {

  class Filter;

  /**
   * shared_ptr of Filter to help manage the memory
   */
  typedef boost::shared_ptr<Filter> FilterPtr;

  /**
   * Base Filter class
   */
  class Filter {
  public:
    Filter(const std::string &text) { text_ = text; }
    virtual ~Filter() { }

    /**
     * text -> filter mapping
     */
    typedef boost::unordered_map<std::string, FilterPtr> KnownFilters;

    /**
     * parsed filters is stored in it
     */
    static KnownFilters known_filters_;

    /**
     * Creates a filter of correct type from its text representation
     * - does the basic parsing and calls the right constructor then.
     */
    static FilterPtr from_text(std::string text);

    friend std::ostream &operator<<(std::ostream &, const Filter &);

  protected:
    /**
     * string representation of the Filter
     */
    std::string text_;

    /**
     * Regular expression that element hiding filters should match
     */
    static const boost::regex elem_hide_;

    /**
     * Regular expression that RegExp filters specified as RegExps should match
     */
    static const boost::regex regexp_;

    /**
     * Regular expression that options on a RegExp filter should match
     */
    static const boost::regex options_;

  private:
    /**
     * Removes unnecessary whitespace from filter text, will only return
     * null if the input parameter is null.
     */
    static std::string normalize(std::string text);

  };


  /**
   * Class for invalid filters
   */
  class InvalidFilter: public Filter {
  public:
    InvalidFilter(const std::string &text, const std::string &reason):
        Filter(text) { reason_ = reason; }

  private:
    std::string reason_;
  };


  /**
   * Class for comment filters
   */
  class CommentFilter: public Filter {
  public:
    CommentFilter(const std::string &text): Filter(text) { }
  };


  /**
   * Base class for filters that can get hit
   */
  class ActiveFilter: public Filter {
  public:
    ActiveFilter(const std::string &text, const std::string &domains);

    bool get_disabled() const;
    void set_disabled(bool disabled);

    uint32_t get_hit_count() const;
    void set_hit_count(uint32_t hit_count);

    time_t get_last_hit() const;
    void set_last_hit(time_t last_hit);

    /** \brief Used for element hiding
     *
     * std::string: domain name
     * 
     * bool: true if the domain is active
     */
    typedef boost::container::map<std::string, bool> DomainMap;

    /**
     * Parse domains from class member domain_source_
     */
    const DomainMap &get_domains();

    /**
     * Test if this doc_domain is active according to
     * class member domains_
     */
    bool is_active_on_domain(std::string doc_domain);

  protected:
    /**
     * Defines whether the filter is disabled
     */
    bool disabled_;

    /**
     * Number of hits on the filter since the last reset
     */
    uint32_t hit_count_;

    /**
     * Last time the filter had a hit (in milliseconds since the
     * beginning of the epoch)
     */
    time_t last_hit_;

    /**
     * Separator character used in domainSource property
     */
    std::string domain_separator_;

    /**
     * Determines whether the trailing dot in domain names isn't important
     * and should be ignored
     */
    bool ignore_trailong_dot_;

    /**
     * Map containing domains that this filter should match on/not match
     * on or null if the filter should match on all domains
     */
    DomainMap domains_;

  private:
    std::string domain_source_;
  };


  /**
   * Abstract base class for RegExp-based filters
   */
  class RegExpFilter: public ActiveFilter {
  public:
    RegExpFilter(const std::string &text, const std::string &regex_source,
      uint32_t content_type, bool match_case, const std::string &domains,
      const boost::tribool &third_party);

    const boost::regex &get_regex();

    /**
     * Creates a RegExp filter from its text representation
     */
    static FilterPtr from_text(const std::string &text);

    /*!
     * Tests whether the URL matches this filter
     *
     * \param location URL to be tested
     * \param content_type content type identifier of the URL
     * \param doc_domain domain name of the document that loads this URL
     * \param third_party should be true if the URL is a third-party request
     *
     * \return true if match
     */
    bool matches(const std::string &location, const std::string &content_type,
      const std::string &doc_domain, bool third_party);

    typedef boost::unordered_map<std::string, uint32_t> TypeMap;

    /**
     * Maps type strings like "SCRIPT" or "OBJECT" to bit masks
     */
    const static TypeMap type_map_;

  protected:
    /**
     * Content types the filter applies to, combination of values from
     * RegExpFilter.typeMap
     */
    uint32_t content_type_;

    /**
     * Defines whether the filter should distinguish between lower and
     * upper case letters
     */
    bool match_case_;

    /**
     * Defines whether the filter should apply to third-party or
     * first-party content only
     */
    boost::tribool third_party_;

    /**
     * filter part that the regular expression should be build from
     */
    std::string regex_source_;

    /**
     * Regular expression to be used when testing against this filter
     */
    boost::regex regex_;
  };


  /**
   * Class for blocking filters
   */
  class BlockingFilter: public RegExpFilter {
  public:
    BlockingFilter(const std::string &text, const std::string &regex_source,
      uint32_t content_type, bool match_case, const std::string &domains,
      const boost::tribool &third_party, bool collapse);

  private:
    bool collapse_;
  };


  /**
   * Class for whitelist filters
   */
  class WhitelistFilter: public RegExpFilter {
  public:
    typedef std::vector<std::string> SiteKeys;
    WhitelistFilter(const std::string &text, const std::string &regex_source,
      uint32_t content_type, bool match_case, const std::string &domains,
      const boost::tribool &third_party, const SiteKeys &site_keys);

  private:
    SiteKeys site_keys_;
  };


  /**
   * Base class for element hiding filters
   */
  class ElemHideBase: public ActiveFilter {
  public:
    ElemHideBase(const std::string &text, const std::string &domains,
      const std::string &selector);

    /*
     * Creates an element hiding filter from a pre-parsed text representation
     *
     * \param text same as in Filter()
     * \param domain domain part of the text representation (can be empty)
     * \param tag_name tag name part (can be empty)
     * \param attr_rules attribute matching rules (can be empty)
     * \param selector raw CSS selector (can be empty)
     *
     * \return {ElemHideFilter|ElemHideException|InvalidFilter}
     */
    static FilterPtr from_text(const std::string &text, 
      const std::string &domain, bool is_exception, std::string &tag_name,
      const std::string &attr_rules, std::string &selector);

  protected:
    /**
     * Host name or domain the filter should be restricted to
     * (can be null for no restriction)
     */
    std::string selector_domain_;

    /**
     * CSS selector for the HTML elements that should be hidden
     */
    std::string selector_;
  };


  /**
   * Class for element hiding filters
   */
  class ElemHideFilter: public ElemHideBase {
  public:
    ElemHideFilter(const std::string &text, const std::string &domains,
      const std::string &selector): ElemHideBase(text, domains, selector)
    { }
  };


  /**
   * Class for element hiding exceptions
   */
  class ElemHideException: public ElemHideBase {
  public:
    ElemHideException(const std::string &text, const std::string &domains,
      const std::string &selector): ElemHideBase(text, domains, selector)
    { }
  };

}


