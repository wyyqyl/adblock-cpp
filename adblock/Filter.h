/*!
 * \file Filter.h
 *
 * \author yorath
 * \date October 11, 2013
 *
 * \details 
 */

#pragma once


#include <stdint.h>
#include <string>
#include <boost/regex.hpp>


namespace NS_ADBLOCK {

  /**
   * Base Filter class
   */
  class Filter {
  public:
    Filter(const std::string &text) { text_ = text; }
    virtual ~Filter() { }

    /**
     * Removes unnecessary whitespace from filter text, will only return
     * null if the input parameter is null.
     */
    static std::string normalize(std::string text);

  protected:
    std::string text_; ///< string representation of the Filter
    static const boost::regex elem_hide_;
    static const boost::regex regexp_;
    static const boost::regex options_;
    friend std::ostream &operator<<(std::ostream &, const Filter &);
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

    /**
     * Defines whether the filter is disabled
     */
    bool get_disabled() const;
    void set_disabled(bool disabled);

    /**
     * Number of hits on the filter since the last reset
     */
    uint32_t get_hit_count() const;
    void set_hit_count(uint32_t hit_count);

    /**
     * Last time the filter had a hit (in milliseconds since the
     * beginning of the epoch)
     */
    time_t get_last_hit() const;
    void set_last_hit(time_t last_hit);

    /**
     * Map containing domains that this filter should match on/not match
     * on or null if the filter should match on all domains
     */


  protected:
    /**
     * Separator character used in domainSource property
     */
    std::string domain_separator_;

    /**
     * Determines whether the trailing dot in domain names isn't important
     * and should be ignored
     */
    bool ignore_trailong_dot_;

  private:
    bool disabled_;
    uint32_t hit_count_;
    time_t last_hit_;
    std::string domain_source_;
  };
}


