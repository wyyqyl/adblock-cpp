#include "Filter.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/assign/list_of.hpp>


namespace NS_ADBLOCK {

  const boost::regex Filter::elem_hide_ =
    boost::regex("^([^\\/\\*\\|\\@\"!]*?)#(\\@)?(?:([\\w\\-]+|\\*)((?:\\([\\w\\-]+(?:[$^*]?=[^\\(\\)\"]*)?\\))*)|#([^{}]+))$");

  const boost::regex Filter::regexp_ =
    boost::regex("^(@@)?\\/.*\\/(?:\\$~?[\\w\\-]+(?:=[^,\\s]+)?(?:,~?[\\w\\-]+(?:=[^,\\s]+)?)*)?$");

  const boost::regex Filter::options_ =
    boost::regex("\\$(~?[\\w\\-]+(?:=[^,\\s]+)?(?:,~?[\\w\\-]+(?:=[^,\\s]+)?)*)$");

  std::ostream &operator<<(std::ostream &stream, const Filter &filter) {
    return stream << filter.text_;
  }


  std::string Filter::normalize(std::string text) {
    if (text.length() == 0) {
      return text;
    }

    // Remove line breaks and such
    text = boost::regex_replace(text, boost::regex("[^\\S ]"), "");

    if (boost::regex_search(text, boost::regex("^\\s*!"))) {
      // Don't remove spaces inside comments
      return boost::regex_replace(
        boost::regex_replace(text, boost::regex("^\\s+"), ""),
        boost::regex("\\s+$"),
        "");
    } else if (boost::regex_search(text, Filter::elem_hide_)) {
      // Special treatment for element hiding filters, right side is allowed to contain spaces
      boost::smatch match;
      if (boost::regex_match(text, match, boost::regex("^(.*?)(#\\@?#?)(.*)$"))) {
        //std::cout << match[1].str() << " " << match[2].str() << " " << match[3].str() << std::endl;
        return boost::regex_replace(match[1].str(), boost::regex("\\s"), "")
          + match[2].str() + boost::regex_replace(
          boost::regex_replace(match[3].str(), boost::regex("^\\s+"), ""),
          boost::regex("\\s+$"),
          "");
      }
    }
    return boost::regex_replace(text, boost::regex("\\s"), "");
  }


  ActiveFilter::ActiveFilter(
    const std::string &text,
    const std::string &domains
    ): Filter(text)
  {
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

  const ActiveFilter::DomainMap & ActiveFilter::get_domains() {
    if (domain_source_.length() > 0) {
      std::vector<std::string> list;
      boost::split(list, domain_source_, boost::is_any_of(domain_separator_), boost::token_compress_on);
      if (list.size() == 1 && list.front().front() != '~') {
        // Fast track for the common one-domain scenario
        domains_[""] = false;
        if (ignore_trailong_dot_) {
          list.front() = boost::regex_replace(list.front(), boost::regex("\\.+$"), "");
        }
        domains_[list.front()] = true;
      } else {
        bool hasIncludes = false;
        for (auto iter = list.begin(); iter != list.end(); ++iter) {
          std::string domain = *iter;
          if (ignore_trailong_dot_) {
            domain = boost::regex_replace(domain, boost::regex("\\.+$"), "");
          }
          if (domain.length() == 0) {
            continue;
          }

          bool include = false;
          if (domain.front() == '~') {
            include = false;
            domain = domain.substr(1);
          } else {
            include = true;
            hasIncludes = true;
          }

          domains_[domain] = include;
        }
        domains_[""] = !hasIncludes;
      }
      domain_source_.clear();
    }
    return domains_;
  }

  bool ActiveFilter::is_active_on_domain(std::string doc_domain) {
    const DomainMap &domains = get_domains();
    if (domains.size() == 0) {
      return true;
    }

    if (doc_domain.length() == 0) {
      return domains.find("")->second;
    }

    if (ignore_trailong_dot_) {
      doc_domain = boost::regex_replace(doc_domain, boost::regex("\\.+$"), "");
    }
    boost::to_upper(doc_domain);

    while (true) {
      auto iter = domains.find(doc_domain);
      if (iter != domains.end()) {
        return iter->second;
      }

      size_t next_dot = doc_domain.find('.');
      if (next_dot == std::string::npos) {
        break;
      }
      doc_domain = doc_domain.substr(next_dot + 1);
    }
    return domains.find("")->second;
  }


#define TYPE_OTHER              1 << 0
#define TYPE_SCRIPT             1 << 1
#define TYPE_IMAGE              1 << 2
#define TYPE_STYLESHEET         1 << 3
#define TYPE_OBJECT             1 << 4
#define TYPE_SUBDOCUMENT        1 << 5
#define TYPE_DOCUMENT           1 << 6
#define TYPE_XBL                1 << 0
#define TYPE_PING               1 << 0
#define TYPE_XMLHTTPREQUEST     1 << 11
#define TYPE_OBJECT_SUBREQUEST  1 << 12
#define TYPE_DTD                1 << 0
#define TYPE_MEDIA              1 << 14
#define TYPE_FONT               1 << 15
#define TYPE_BACKGROUND         1 << 2
#define TYPE_POPUP              1 << 29
#define TYPE_ELEMHIDE           1 << 30

  const RegExpFilter::TypeMap RegExpFilter::type_map_ = boost::assign::map_list_of
    ("OTHER", TYPE_OTHER)
    ("SCRIPT", TYPE_SCRIPT)
    ("IMAGE", TYPE_IMAGE)
    ("STYLESHEET", TYPE_STYLESHEET)
    ("OBJECT", TYPE_OBJECT)
    ("SUBDOCUMENT", TYPE_SUBDOCUMENT)
    ("DOCUMENT", TYPE_DOCUMENT)
    ("XBL", TYPE_XBL)
    ("PING", TYPE_PING)
    ("XMLHTTPREQUEST", TYPE_XMLHTTPREQUEST)
    ("OBJECT_SUBREQUEST", TYPE_OBJECT_SUBREQUEST)
    ("DTD", TYPE_DTD)
    ("MEDIA", TYPE_MEDIA)
    ("FONT", TYPE_FONT)
    ("BACKGROUND", TYPE_BACKGROUND)
    ("POPUP", TYPE_POPUP)
    ("ELEMHIDE", TYPE_ELEMHIDE);

  RegExpFilter::RegExpFilter(
    const std::string &text,
    const std::string &regex_source,
    uint32_t content_type,
    bool match_case,
    const std::string &domains,
    boost::tribool third_party
    ): ActiveFilter(text, domains)
  {
    domain_separator_ = "|";

    content_type_ = content_type;
    match_case_ = match_case;
    third_party_ = third_party;

    if (regex_source.length() >= 2 && regex_source.front() == '/'
      && regex_source.back() == '/')
    {
      // The filter is a regular expression - convert it immediately to
      // catch syntax errors
      regex_ = boost::regex(regex_source.substr(1, regex_source.length() - 2),
        match_case_ ? 0 : boost::regex::icase);
    } else {
      // No need to convert this filter to regular expression yet, do it on demand
      regex_source_ = regex_source;
    }
  }

  const boost::regex &RegExpFilter::get_regex() {
    if (regex_source_.length() > 0) {
      // Remove multiple wildcards
      std::string source = boost::regex_replace(regex_source_, boost::regex("\\*+"), "*");

      // Remove leading wildcards
      if (source.front() == '*') {
        source = source.substr(1);
      }

      // Remove trailing wildcards
      int32_t pos = source.length() - 1;
      if (pos >= 0 && source[pos] == '*') {
        source = source.substr(0, pos);
      }

      // remove anchors following separator placeholder
      source = boost::regex_replace(source, boost::regex("\\^\\|$"), "^");
      // escape special symbols
      source = boost::regex_replace(source, boost::regex("\\W"), "\\$&");
      // replace wildcards by .*
      source = boost::regex_replace(source, boost::regex("\\\\\\*"), ".*");
      // process separator placeholders (all ANSI characters but
      // alphanumeric characters and _%.-)
      source = boost::regex_replace(source, boost::regex("\\\\\\^"), "(?:[\\x00-\\x24\\x26-\\x2C\\x2F\\x3A-\\x40\\x5B-\\x5E\\x60\\x7B-\\x80]|$)");
      // process extended anchor at expression start
      source = boost::regex_replace(source, boost::regex("^\\\\\\|\\\\\\|"), "^[\\w\\-]+:\\/+(?!\\/)(?:[^.\\/]+\\.)*?");
      // process anchor at expression start
      source = boost::regex_replace(source, boost::regex("^\\\\\\|"), "^");
      // process anchor at expression end
      source = boost::regex_replace(source, boost::regex("\\\\\\|$"), "$");

      regex_ = boost::regex(source, match_case_ ? 0 : boost::regex::icase);
      regex_source_.clear();
    }
    return regexp_;
  }

  bool RegExpFilter::matches(
    const std::string &location,
    const std::string &content_type,
    const std::string &doc_domain,
    bool third_party
    )
  {
    if (boost::regex_search(location, get_regex()) &&
      (third_party_ == boost::indeterminate || third_party_ == third_party) &&
      is_active_on_domain(doc_domain))
    {
      auto iter = type_map_.find(content_type);
      if (iter != type_map_.end() && (iter->second & content_type_) != 0) {
        return true;
      }
    }
    return false;
  }


  BlockingFilter::BlockingFilter(
    const std::string &text,
    const std::string &regex_source,
    uint32_t content_type,
    bool match_case,
    const std::string &domains,
    const boost::tribool &third_party,
    bool collapse
    ): RegExpFilter(text, regex_source, content_type, match_case, domains, third_party)
  {
    collapse_ = collapse;
  }


  WhitelistFilter::WhitelistFilter(
    const std::string &text,
    const std::string &regex_source,
    uint32_t content_type,
    bool match_case,
    const std::string &domains,
    const boost::tribool &third_party,
    const SiteKeys &site_keys
    ): RegExpFilter(text, regex_source, content_type, match_case, domains, third_party)
  {
    site_keys_ = site_keys;
  }


  ElemHideBase::ElemHideBase(
    const std::string &text,
    const std::string &domains,
    const std::string &selector
    ): ActiveFilter(text, boost::to_upper_copy(domains))
  {
    selector_domain_ = boost::regex_replace(domains, boost::regex(",~[^,]+"), "");
    selector_domain_ = boost::regex_replace(selector_domain_, boost::regex("^~[^,]+,?"), "");
    boost::to_lower(selector_domain_);

    selector_ = selector;
    domain_separator_ = ",";
    ignore_trailong_dot_ = false;
  }

}
