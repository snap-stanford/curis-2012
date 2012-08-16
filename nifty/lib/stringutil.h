#ifndef stringutil_h
#define stringutil_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TStringUtil {
private:
  static TInt LevenshteinDistance(TStr& Content1, TStr& Content2);
  static void LoadCommonEnglishWords();

public:
  static PSwSet StopWordSet;
  static THashSet<TStr> CommonEnglishWordsList;
  static const double MinCommonEnglishRatio;

  static void ParseStringIntoWords(const TStr& OriginalString, TStrV& ParsedString);
  static void FilterSpacesAndSetLowercase(TStr& QtStr);
  static void RemoveStemAndStopWords(const TStrV &ContentV, TStrV& NewContent);
  static void RemovePunctuation(const TStr& OrigString, TStr& NewString);

  static TInt LongestSubsequenceOfWords(const TStrV& Content1, const TStrV& Content2);
  static TInt SubWordListEditDistance(const TStrV& Content1, const TStrV& Content2);
  static TInt WordLevenshteinDistance(TStrV& Content1, TStrV& Content2);

  static bool IsEnglish(const TChA& Quote);
  static bool IsRobustlyEnglish(TStr& Quote);
  static void RemoveEndPunctuations(TChA& Quote);
};

#endif
