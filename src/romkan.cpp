#include "romkan_impl.hpp"
#include <iostream>
#include <algorithm>

namespace
{
  RomkanDict d;

  template <typename T, typename Container>
  T join(Container X, T sep)
  {
    auto last = X.back();
    X.pop_back();
    T ret;
    for(auto i : X) { ret.append(i).append(sep); }
    ret.append(last);
    return ret;
  }

  struct takeFirst
  {
    template <typename T>
    auto operator()(T p) -> decltype(p.first)
    { return p.first; }
  };

  struct takeSecond
  {
    template <typename T>
    auto operator()(T p) -> decltype(p.second)
    { return p.second; }
  };

  std::string normalize_double_n(std::string x)
  {
    try
    {
      std::regex_replace(x, std::regex("nn"), std::string("n'"));
      std::regex_replace(x, std::regex("n'([^aiueoyn])"), std::string("n\\1"));
      std::regex_replace(x, std::regex("n'$"), std::string("n"));
    }
    catch(std::regex_error & e)
    {
      std::cerr << "too bad: implementation fault.\n";
      throw;
    }
    return x;
  }
}

RomkanDict::RomkanDict()
{
  typedef std::vector<std::string> vecstr_t;
  vecstr_t roms, kans;
  std::back_insert_iterator<vecstr_t> ir(roms), ik(kans);
  auto update = [this](const std::pair<const char*, const char*> & x)
  {
    std::string kana=x.first, roma=x.second;
    m_kanrom[kana] = roma;
    m_romkan[std::move(roma)] = std::move(kana);
  };
  for(size_t i=0; i < LENGTH(kunrei); ++i) { update(kunrei[i]); }
  for(size_t i=0; i < LENGTH(hepburn); ++i) { update(hepburn[i]); }
  std::transform(m_romkan.begin(), m_romkan.end(), ir, takeFirst());
  std::transform(m_romkan.begin(), m_romkan.end(), ik, takeSecond());
  std::sort(roms.begin(), roms.end(),
            [this](const std::string & x, const std::string & y)
            { return x.size() > y.size(); });
  std::sort(kans.begin(), kans.end(),
            [this](const std::string & x, const std::string & y)
            { return (x.size() != y.size())
                ? x.size() > y.size()
                : m_kanrom[x].size() > m_kanrom[y].size(); });
  try
  {
    m_rompat=std::regex(join(std::move(roms), std::string("|")));
    m_kanpat=std::regex(join(std::move(kans), std::string("|")));
  }
  catch(std::regex_error & e)
  {
    std::cerr << "too bad: implementation fault.\n";
    throw;
  }
}

std::string roma_to_kana(const std::string & roma_string)
{
  std::string tmp=normalize_double_n(roma_string);
  return tmp;
}
