#include "stdafx.h"
#include "stringutil.h"

TStrSet TStringUtil::PublicSuffixSet = TStringUtil::LoadPublicSuffixList();

TStrSet TStringUtil::LoadPublicSuffixList() {
  TStrSet WordSet;
  PSIn EnglishWords = TFIn::New("resources/public_suffix.txt");
  TStr Word;
  while (!EnglishWords->Eof() && EnglishWords->GetNextLn(Word)) {
    WordSet.AddKey(Word);
  }
  return WordSet;
}

TStr TStringUtil::GetDomainName(const TStr &Url) {
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
