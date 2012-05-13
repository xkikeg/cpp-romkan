#include <gtest/gtest.h>

#include <romkan.h>

TEST(roma_to_kana, hepburn)
{
  EXPECT_EQ(u8"あいうえお", roma_to_kana(u8"aiueo"));
}

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

enum VOWEL_TYPE
{
  VOWEL_TYPE_A,
  VOWEL_TYPE_I,
  VOWEL_TYPE_U,
  VOWEL_TYPE_E,
  VOWEL_TYPE_O,
};

enum CONSONANT_TYPE
{
  CONSONANT_TYPE_NONE,
  CONSONANT_TYPE_K,
  CONSONANT_TYPE_S,
  CONSONANT_TYPE_T,
  CONSONANT_TYPE_N,
  CONSONANT_TYPE_H,
  CONSONANT_TYPE_M,
  CONSONANT_TYPE_Y,
  CONSONANT_TYPE_R,
  CONSONANT_TYPE_W,
  CONSONANT_TYPE_G,
  CONSONANT_TYPE_Z,
  CONSONANT_TYPE_D,
  CONSONANT_TYPE_B,
  CONSONANT_TYPE_P,
};

struct RomajiChar
{
  CONSONANT_TYPE siin;
  VOWEL_TYPE boin;
};

BOOST_FUSION_ADAPT_STRUCT(
  RomajiChar,
  (VOWEL_TYPE, boin)
  (CONSONANT_TYPE, siin)
  );

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

struct vowel_ : qi::symbols<char, VOWEL_TYPE>
{
  vowel_()
  {
    add
      ("a", VOWEL_TYPE_A)
      ("i", VOWEL_TYPE_I)
      ("u", VOWEL_TYPE_U)
      ("e", VOWEL_TYPE_E)
      ("o", VOWEL_TYPE_O)
      ;
  }
} vowel;

struct consonant_none_ : qi::symbols<char, CONSONANT_TYPE>
{
  consonant_none_()
  {
    add
      ("", CONSONANT_TYPE_NONE)
      ;
  }
} consonant_none;

struct consonant_one_ : qi::symbols<char, CONSONANT_TYPE>
{
  consonant_one_()
  {
    add
      ("k", CONSONANT_TYPE_K)
      ("s", CONSONANT_TYPE_S)
      ("t", CONSONANT_TYPE_T)
      ("n", CONSONANT_TYPE_N)
      ("h", CONSONANT_TYPE_H)
      ("m", CONSONANT_TYPE_M)
      ("y", CONSONANT_TYPE_Y)
      ("r", CONSONANT_TYPE_R)
      ("w", CONSONANT_TYPE_W)
      ("g", CONSONANT_TYPE_G)
      ("z", CONSONANT_TYPE_Z)
      ("d", CONSONANT_TYPE_D)
      ("b", CONSONANT_TYPE_B)
      ("p", CONSONANT_TYPE_P)
      ;
  }
} consonant_one;

template <typename Iterator>
struct romaji_parser : qi::grammar<Iterator, std::vector<RomajiChar> >
{
  romaji_parser() : romaji_parser::base_type(start)
  {
    using qi::lit;
    using ascii::char_;

    romaji_char %=
      (consonant_one >> vowel)
      | (consonant_none >> vowel);

    start %= *romaji_char;
  }

  qi::rule<Iterator, std::vector<RomajiChar>(), ascii::space_type> start;
  qi::rule<Iterator, RomajiChar()> romaji_char;
};

template <typename Iterator>
bool parse_complex(Iterator first, Iterator last, std::complex<double>& c)
{
  using qi::double_;
  using qi::_1;
  using qi::phrase_parse;
  using ascii::space;
  using phoenix::ref;

  double rN = 0.0;
  double iN = 0.0;
  bool r = phrase_parse(first, last,

                        //  Begin grammar
                        (
                          '(' >> double_[ref(rN) = _1]
                          >> -(',' >> double_[ref(iN) = _1]) >> ')'
                          |   double_[ref(rN) = _1]
                          ),
                        //  End grammar

                        space);

  if (!r || first != last) // fail if we did not get a full match
    return false;
  c = std::complex<double>(rN, iN);
  return r;
}

bool parse_complex(const std::string & str, std::complex<double>& c)
{
  return parse_complex(str.begin(), str.end(), c);
}

template <typename Iterator>
bool parse_romaji_char(Iterator first, Iterator last, RomajiChar & c)
{
  using qi::_1;
  using qi::phrase_parse;
  using ascii::space;
  using phoenix::ref;

  CONSONANT_TYPE & siin=c.siin;
  VOWEL_TYPE & boin=c.boin;
  bool r = phrase_parse(first, last,
                        ( consonant_one[ref(siin) = _1]
                          >> vowel[ref(boin) = _1])
                        |
                        ( consonant_none[ref(siin) = _1]
                          >> vowel[ref(boin) = _1]),
                        space);

  if (!r || first != last) // fail if we did not get a full match
    return false;
  return true;
}

template <typename Iterator>
bool parse_romaji_str(Iterator first, Iterator last, std::vector<RomajiChar> & str)
{
  using qi::_1;
  using qi::phrase_parse;
  using ascii::space;
  using phoenix::push_back;

  bool r = phrase_parse(first, last,
                        *(parse_romaji_char[push_back(phoenix::ref(str), _1)]),
                        space);

  if (!r || first != last) // fail if we did not get a full match
    return false;
  return true;
}

bool parse_romaji(const std::string & str, std::vector<RomajiChar> & c)
{
  typedef std::string::const_iterator iterator_type;
  typedef romaji_parser<iterator_type> romaji_parser;
  using ascii::space;
  romaji_parser g;
  auto itr=str.begin();
  bool r = phrase_parse(itr, str.end(), g, space, c);
  return (r && itr == str.end());
}

TEST(BoostSpirit, example1)
{
  std::complex<double> c;
  EXPECT_TRUE(parse_complex("123", c));
  EXPECT_TRUE(parse_complex("(123, 0.5)", c));
  EXPECT_FALSE(parse_complex("(1232s, 0.5)", c));
}

TEST(BoostSpirit, example2)
{
  std::vector<RomajiChar> c;
  EXPECT_TRUE(parse_romaji_str("aiueo", c));
  EXPECT_TRUE(parse_romaji_str("akasatana", c));
}
