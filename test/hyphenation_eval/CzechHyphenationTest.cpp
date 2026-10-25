#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "lib/Epub/Epub/hyphenation/HyphenationCommon.h"
#include "lib/Epub/Epub/hyphenation/Hyphenator.h"
#include "lib/Epub/Epub/hyphenation/LanguageRegistry.h"

namespace {

// Marked words specify expected UTF-8 byte offsets independently of the engine.
void expectBreaks(const char* marked, const char* language = "cs") {
  std::string word;
  std::vector<size_t> expected;
  for (const char* ptr = marked; *ptr; ++ptr) {
    if (*ptr == '|') {
      expected.push_back(word.size());
    } else {
      word.push_back(*ptr);
    }
  }
  Hyphenator::setPreferredLanguage(language);
  const auto breaks = Hyphenator::breakOffsets(word, false);
  std::vector<size_t> actual;
  for (const auto& point : breaks) {
    EXPECT_TRUE(point.requiresInsertedHyphen);
    actual.push_back(point.byteOffset);
  }
  EXPECT_EQ(actual, expected) << word << " (" << language << ")";
}

class CzechHyphenation : public testing::Test {
  void TearDown() override { Hyphenator::setPreferredLanguage(""); }
};

TEST_F(CzechHyphenation, CommonWordsAndCzechDigraph) {
  ASSERT_NE(getLanguageHyphenatorForPrimaryTag("cs"), nullptr);
  // Both če-sko and čes-ko are allowed: https://prirucka.ujc.cas.cz/?id=135
  for (const char* word : {"pří|liš", "kou|zel|ník", "ma|min|ka", "po|hád|ka", "če|s|ko|slo|ven|ský", "sta|teč|né|ho",
                           "prů|vod|ce", "cha|loup|ka", "PŘÍ|LIŠ", "PRŮ|VOD|CE"}) {
    expectBreaks(word);
  }
}

TEST_F(CzechHyphenation, PublicationLanguageTags) {
  for (const char* tag : {"cs", "CS", "cs-CZ", "cs_CZ", "ces", "CES", "cze", "cz", "CZ-cz"}) {
    expectBreaks("pří|liš", tag);
  }
  // Switching books must not retain the previously selected language.
  expectBreaks("pří|liš", "cs");
  expectBreaks("příliš", "zz");
  expectBreaks("příliš", "");
}

TEST_F(CzechHyphenation, CzechLanguageInstituteExamples) {
  // Two-consonant examples from the Czech Language Institute's reference:
  // https://prirucka.ujc.cas.cz/?id=135 (minimum two letters on each side).
  for (const char* word : {"dok|tor", "lé|čeb|na", "far|mář", "pa|ra|dig|ma", "prac|ka", "děl|nic|ký", "šle|hač|ka",
                           "teč|ka", "ma|nu|fak|tu|ra", "re|dak|ce", "klep|to|man", "fron|ta", "cel|ta", "re|cen|ze",
                           "fi|nan|co|vat", "lep|tat", "čer|pat", "na|jíž|dět"}) {
    expectBreaks(word);
  }
}

TEST_F(CzechHyphenation, UpstreamAdditionalVariants) {
  // The reference above deliberately recommends fewer variants and explicitly
  // allows alternatives. Preserve the upstream patterns' additional choices,
  // including both šach-ta and an-tik-va, without splitting the digraph ch.
  expectBreaks("ša|ch|ta");
  expectBreaks("an|ti|k|va");
}

TEST_F(CzechHyphenation, DecomposedAccentsKeepSourceByteOffsets) {
  expectBreaks("pr\u030ci\u0301|lis\u030c");
  expectBreaks("PR\u030cI\u0301|LIS\u030c");
  expectBreaks("pru\u030a|vod|ce");
  expectBreaks("PRU\u030a|VOD|CE");

  const auto normalized = collectCodepoints(
      "C\u030cc\u030cD\u030cd\u030cE\u030ce\u030cN\u030cn\u030c"
      "R\u030cr\u030cS\u030cs\u030cT\u030ct\u030cZ\u030cz\u030cU\u030au\u030a");
  const auto composed = collectCodepoints("ČčĎďĚěŇňŘřŠšŤťŽžŮů");
  ASSERT_EQ(normalized.size(), composed.size());
  for (size_t i = 0; i < composed.size(); ++i) {
    EXPECT_EQ(normalized[i].value, composed[i].value) << i;
    EXPECT_EQ(normalized[i].byteOffset, i * 3) << i;
  }
}

TEST_F(CzechHyphenation, ShortWordsAndPunctuation) {
  for (const char* word : {"", "a", "se", "kůň", "čaj", "ch"}) expectBreaks(word);
  expectBreaks("„pří|liš,“");
  expectBreaks("pří|liš[12]");
}

TEST_F(CzechHyphenation, ExplicitSoftHyphensRemainUsable) {
  Hyphenator::setPreferredLanguage("cs");
  const std::string word = "pří\u00ADliš";
  const auto breaks = Hyphenator::breakOffsets(word, false);
  ASSERT_EQ(breaks.size(), 1u);
  EXPECT_EQ(breaks[0].byteOffset, std::string("pří\u00AD").size());
  EXPECT_TRUE(breaks[0].requiresInsertedHyphen);
}

}  // namespace
