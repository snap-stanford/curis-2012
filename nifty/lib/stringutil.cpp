#include "stdafx.h"
#include "stringutil.h"

const double TStringUtil::MinCommonEnglishRatio = 0.25;
PSwSet TStringUtil::StopWordSet = new TSwSet(swstEnMsdn);
TStrSet TStringUtil::CommonEnglishWordsList = THashSet<TStr>();
TStrSet TStringUtil::PublicSuffixSet = THashSet<TStr>();

// TODO: Rename to ParseStringIntoWords
void TStringUtil::ParseStringIntoWords(const TStr& OriginalString, TStrV& ParsedString) {
  OriginalString.SplitOnAllAnyCh(" ", ParsedString);
}

void TStringUtil::FilterSpacesAndSetLowercase(TStr& QtStr) {
 TChA QtChA = TChA(QtStr);
  for (int i = 0; i < QtChA.Len(); i++) {
    if (!(isalpha(QtChA[i]) || QtChA[i] == '\'')) {
      QtChA[i] = ' ';
    } else {
      QtChA[i] = tolower(QtChA[i]);
    }
  }
  QtStr = TStr(QtChA);
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}

void TStringUtil::RemoveStemAndStopWords(const TStrV &ContentV, TStrV& NewContent) {
  for (int i = 0; i < ContentV.Len(); ++i) {
    if (!StopWordSet->IsIn(ContentV[i], false)) {
      NewContent.Add(TPorterStemmer::StemX(ContentV[i]).GetLc());
    }
  }
}

/// Removes all punctuation (i.e. non-alphanumeric/whitespace characters) from the string
void TStringUtil::RemovePunctuation(const TStr& OrigString, TStr& NewString) {
  TChA OrigChA(OrigString);
  TChA NewChA;
  for (int i = 0; i < OrigChA.Len(); i++) {
    if (TCh::IsAlNum(OrigChA[i]) || TCh::IsWs(OrigChA[i])) {
      NewChA.AddCh(OrigChA[i]);
    }
  }
  NewString = TStr(NewChA);
}

/// Finds the length of the longest common subsequence of words
//  Based off the method TQuoteBs::LongestCmnSubSq(const TIntV& WIdV1, const TIntV& WIdV2,
//  int& WIdV1Start, int& WIdV2Start, int& SkipId) in memes.cpp of memecluster
TInt TStringUtil::LongestSubsequenceOfWords(const TStrV& Content1, const TStrV& Content2) {
  const TStrV& V1 = Content1.Len() > Content2.Len() ? Content1:Content2; // longer quote
  const TStrV& V2 = Content1.Len() > Content2.Len() ? Content2:Content1; // shorter quote

  THash<TStr, TInt> WordToId;
  TIntV IdV1, IdV2;
  for (int i = 0; i < V1.Len(); i++) {
    TInt Id;
    if(!WordToId.IsKeyGetDat(V1[i], Id)) {
      Id = WordToId.Len();
      WordToId.AddDat(V1[i], WordToId.Len());
    }
    IdV1.Add(Id);
  }
  for (int i = 0; i < V2.Len(); i++) {
    TInt Id;
    if(!WordToId.IsKeyGetDat(V2[i], Id)) {
      Id = WordToId.Len();
      WordToId.AddDat(V2[i], WordToId.Len());
    }
    IdV2.Add(Id);
  }

  // We assume the quotes are less than 50 words long
  IAssert(IdV1.Len() < 50 && IdV2.Len() < 50);
  int lcs[50][50];
  for (int i = 0; i < IdV1.Len(); i++) {
    for (int j = 0; j < IdV2.Len(); j++) {
      lcs[i][j] = 0;
      if (i > 0) lcs[i][j] = max(lcs[i][j], lcs[i - 1][j]);
      if (j > 0) lcs[i][j] = max(lcs[i][j], lcs[i][j - 1]);
      if (IdV1[i] == IdV2[j]) {
        if (i == 0 || j == 0) lcs[i][j] = max(lcs[i][j], 1);
        else lcs[i][j] = max(lcs[i][j], lcs[i-1][j-1] + 1);
      }
    }
  }
  return lcs[IdV1.Len() - 1][IdV2.Len() - 1];
}

TInt TStringUtil::SubWordListEditDistance(const TStrV& Content1, const TStrV& Content2) {
  const TStrV& V1 = Content1.Len() > Content2.Len() ? Content1:Content2; // longer quote
  const TStrV& V2 = Content1.Len() > Content2.Len() ? Content2:Content1; // shorter quote

  // We assumet the quotes are less than 50 words long
  IAssert(V1.Len() < 50 && V2.Len() < 50);
  int lcs[50][50];

  THash<TStr, TInt> WordToId;
  TIntV IdV1, IdV2;
  lcs[V1.Len()][0] = 0;
  lcs[0][V2.Len()] = V2.Len();
  for (int i = 0; i < V1.Len(); i++) {
    TInt Id;
    if(!WordToId.IsKeyGetDat(V1[i], Id)) {
      Id = WordToId.Len();
      WordToId.AddDat(V1[i], WordToId.Len());
    }
    IdV1.Add(Id);
    lcs[i][0] = 0;
  }
  for (int i = 0; i < V2.Len(); i++) {
    TInt Id;
    if(!WordToId.IsKeyGetDat(V2[i], Id)) {
      Id = WordToId.Len();
      WordToId.AddDat(V2[i], WordToId.Len());
    }
    IdV2.Add(Id);
    lcs[0][i] = i;
  }

  for (int i = 1; i <= IdV1.Len(); i++) {
    for (int j = 1; j <= IdV2.Len(); j++) {
      if (IdV1[i - 1] == IdV2[j - 1]) {
        lcs[i][j] = lcs[i - 1][j - 1];
      } else {
        lcs[i][j] = min(lcs[i - 1][j] + 1, lcs[i][j - 1] + 1);
        lcs[i][j] = min(lcs[i][j], lcs[i - 1][j - 1] + 1);
      }
    }
  }
  int MinDist = -1;
  for (int i = 0; i <= IdV1.Len(); i++) {
    if (MinDist == -1 || MinDist > lcs[i][IdV2.Len()]) MinDist = lcs[i][IdV2.Len()];
  }
  return MinDist;
}

// TODO: Merge this and below into a thing that takes something with a .Len() and an == operator
// because duplicate code is bad.
TInt TStringUtil::WordLevenshteinDistance(TStrV& Content1, TStrV& Content2) {
  TInt C1Len = Content1.Len() + 1, C2Len = Content2.Len() + 1;
  TInt d[C1Len.Val][C2Len.Val];

  for (int i = 0; i < C1Len ; i++) {
    for (int j = 0; j < C2Len; j++) {
      d[i][j] = 0;
    }
  }

  for (int i = 0; i < C1Len; i++) {
    d[i][0] = i;
  }

  for (int j = 0; j < C2Len; j++) {
    d[0][j] = j;
  }

  for (int j = 1; j < C2Len; j++) {
    for (int i = 1; i < C1Len; i++)
    {
      if (Content1[i-1] == Content2[j-1]) {
        d[i][j] = d[i-1][j-1];
      } else {
        d[i][j] = d[i-1][j] + 1; // a deletion
        if (d[i][j-1] + 1 < d[i][j]) d[i][j] = d[i][j-1] + 1; // an insertion
        if (d[i-1][j-1] + 1 < d[i][j]) d[i][j] = d[i-1][j-1] + 1; // a substitution
      }
    }
  }
  return d[C1Len-1][C2Len-1];
}

TInt TStringUtil::LevenshteinDistance(TStr& Content1, TStr& Content2) {
  TInt C1Len = Content1.Len() + 1, C2Len = Content2.Len() + 1;
  TInt d[C1Len.Val][C2Len.Val];

  for (int i = 0; i < C1Len ; i++) {
    for (int j = 0; j < C2Len; j++) {
      d[i][j] = 0;
    }
  }

  for (int i = 0; i < C1Len; i++) {
    d[i][0] = i;
  }

  for (int j = 0; j < C2Len; j++) {
    d[0][j] = j;
  }

  for (int j = 1; j < C2Len; j++) {
    for (int i = 1; i < C1Len; i++)
    {
      if (Content1[i-1] == Content2[j-1]) {
        d[i][j] = d[i-1][j-1];
      } else {
        d[i][j] = d[i-1][j] + 1; // a deletion
        if (d[i][j-1] + 1 < d[i][j]) d[i][j] = d[i][j-1] + 1; // an insertion
        if (d[i-1][j-1] + 1 < d[i][j]) d[i][j] = d[i-1][j-1] + 1; // a substitution
      }
    }
  }
  return d[C1Len-1][C2Len-1];
}

void TStringUtil::LoadCommonEnglishWords() {
  PSIn EnglishWords = TFIn::New("resources/common_english_words.txt");
  TStr Word;
  while (!EnglishWords->Eof() && EnglishWords->GetNextLn(Word)) {
    TStringUtil::CommonEnglishWordsList.AddKey(Word);
  }
}

bool TStringUtil::IsEnglish(const TChA& Quote) {
  return Quote.CountCh('?') <= Quote.Len()/2;
}

/// Assumes lower case characters only format
bool TStringUtil::IsRobustlyEnglish(TStr& Quote) {
  if (TStringUtil::CommonEnglishWordsList.Len() == 0 ) { LoadCommonEnglishWords(); }
  TStr CleanQuote;
  RemovePunctuation(Quote, CleanQuote);
  TStrV Parsed;
  TStringUtil::ParseStringIntoWords(CleanQuote, Parsed);
  TInt EnglishCount = 0;
  for (int i = 0; i < Parsed.Len(); ++i) {
    if (Parsed[i].Len() > 30) { return false; }
    if (TStringUtil::CommonEnglishWordsList.IsKey(Parsed[i])) {
      EnglishCount++;
    }
  }
  //printf("%f: %s\n", EnglishCount * 1.0 / Parsed.Len(), Quote.CStr());
  return EnglishCount * 1.0 / Parsed.Len() >= TStringUtil::MinCommonEnglishRatio;
}

void TStringUtil::RemoveEndPunctuations(TChA& Quote) {
  for (int i = Quote.Len() - 1; i >= 0; i--) {
    if (isalpha(Quote[i]) || Quote[i] == '\'' || Quote[i] == ')') {
      break;
    } else {
      Quote[i] = ' ';
    }
  }
}

void TStringUtil::LoadPublicSuffixList() {
  PSIn EnglishWords = TFIn::New("resources/public_suffix.txt");
  TStr Word;
  while (!EnglishWords->Eof() && EnglishWords->GetNextLn(Word)) {
    TStringUtil::PublicSuffixSet.AddKey(Word);
  }
}

TStr TStringUtil::GetDomainName(const TStr &Url) {
  if (TStringUtil::PublicSuffixSet.Len() == 0) { LoadPublicSuffixList(); }
  TStr DomainName;
  TChA UrlChA = TChA(Url);

  int EndSlash = UrlChA.SearchCh('/', 7)-1; // skip starting http://
  if (EndSlash > 0) {
    const int BegSlash = UrlChA.SearchChBack('/', EndSlash);
    if (BegSlash > 0) { DomainName = UrlChA.GetSubStr(BegSlash+1, EndSlash).ToLc(); }
    else { DomainName = UrlChA.GetSubStr(0, UrlChA.SearchCh('/', 0)-1).ToLc(); }
  } else {
    if (UrlChA.IsPrefix("http://")) { DomainName = UrlChA.GetSubStr(7, UrlChA.Len()-1).ToLc(); }
    EndSlash = UrlChA.SearchCh('/', 0);
    if (EndSlash > 0) { DomainName = UrlChA.GetSubStr(0, EndSlash-1).ToLc(); }
    else { DomainName = TChA(UrlChA).ToLc(); }
  }

  // Get rid of www
  if (DomainName.IsPrefix("www.")) { DomainName = DomainName.GetSubStr(4, TInt::Mx); }

  // Handle Special Domains
  TStrV ParsedDomainName;
  DomainName.SplitOnAllAnyCh(".", ParsedDomainName);
  int StartIndex = 0;
  for (int i = ParsedDomainName.Len() - 1; i >= 0; i--) {
    if (!PublicSuffixSet.IsKey(ParsedDomainName[i])) {
      StartIndex = i;
      break;
    }
  }

  DomainName = "";
  for (int i = StartIndex; i < ParsedDomainName.Len(); i++) {
    if (DomainName.Len() > 0)  DomainName.InsStr(DomainName.Len(),".");
    DomainName.InsStr(DomainName.Len(), ParsedDomainName[i]);
  }

  //printf("%s -> %s\n", Url.CStr(), DomainName.CStr());
  return DomainName;
}
