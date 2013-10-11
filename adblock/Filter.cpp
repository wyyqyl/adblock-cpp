#include "Filter.h"


namespace NS_ADBLOCK {

  /**
   * Regular expression that element hiding filters should match
   */
  const boost::regex Filter::elem_hide_ =
    boost::regex("^([^\\/\\*\\|\\@\"!]*?)#(\\@)?(?:([\\w\\-]+|\\*)((?:\\([\\w\\-]+(?:[$^*]?=[^\\(\\)\"]*)?\\))*)|#([^{}]+))$");

  /**
   * Regular expression that RegExp filters specified as RegExps should match
   */
  const boost::regex Filter::regexp_ =
    boost::regex("^(@@)?\\/.*\\/(?:\\$~?[\\w\\-]+(?:=[^,\\s]+)?(?:,~?[\\w\\-]+(?:=[^,\\s]+)?)*)?$");

  /**
   * Regular expression that options on a RegExp filter should match
   */
  const boost::regex Filter::options_ =
    boost::regex("\\$(~?[\\w\\-]+(?:=[^,\\s]+)?(?:,~?[\\w\\-]+(?:=[^,\\s]+)?)*)$");


  std::string Filter::normalize(std::string text) {
    if (text.length() == 0) {
      return text;
    }

    // Remove line breaks and such
    text = boost::regex_replace(text, boost::regex("[^\\S ]"), std::string(""));

    if (boost::regex_search(text, boost::regex("^\\s*!"))) {
      // Don't remove spaces inside comments
      return boost::regex_replace(
        boost::regex_replace(text, boost::regex("^\\s+"), std::string("")),
        boost::regex("\\s+$"),
        std::string(""));
    } else if (boost::regex_match(text, Filter::elem_hide_)) {
      // Special treatment for element hiding filters, right side is allowed to contain spaces
      boost::smatch what;
      if (boost::regex_match(text, what, boost::regex("^(.*?)(#\\@?#?)(.*)$"))) {
        //std::cout << what[1].str() << " " << what[2].str() << " " << what[3].str() << std::endl;
        return boost::regex_replace(what[1].str(), boost::regex("\\s"), std::string(""))
          + what[2].str() + boost::regex_replace(
          boost::regex_replace(what[3].str(), boost::regex("^\\s+"), std::string("")),
          boost::regex("\\s+$"),
          std::string(""));
      }
    }
    return boost::regex_replace(text, boost::regex("\\s"), std::string(""));
  }


  std::ostream &operator<<(std::ostream &stream, const Filter &filter) {
    return stream << filter.text_;
  }


  ActiveFilter::ActiveFilter(const std::string &text, const std::string &domains): Filter(text) {
    domain_source_ = domains;
  }

  bool ActiveFilter::get_disabled() const {
    return disabled_;
  }

  void ActiveFilter::set_disabled(bool disabled) {
    if (disabled_ != disabled) {
      bool old = disabled_;
      disabled_ = disabled;
      // TODO
    }
  }

  uint32_t ActiveFilter::get_hit_count() const {
    return hit_count_;
  }

  void ActiveFilter::set_hit_count(uint32_t hit_count) {
    if (hit_count_ != hit_count) {
      uint32_t old = hit_count_;
      hit_count_ = hit_count;
      // TODO
    }
  }

  time_t ActiveFilter::get_last_hit() const {
    return last_hit_;
  }

  void ActiveFilter::set_last_hit(time_t last_hit) {
    if (last_hit_ != last_hit) {
      time_t old = last_hit_;
      last_hit_ = last_hit;
      // TODO
    }
  }

}
