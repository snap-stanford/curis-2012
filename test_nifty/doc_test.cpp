#include "stdafx.h"

int main(int argc, char *argv[]) {
  TDocBase *DocBase = new TDocBase();
  TChA Url = TChA("http://www.newyorktimes.com/news_story");
  TSecTm Date = TSecTm::GetCurTm();
  TChA Content = TChA("foo bar foo foo");
  TVec<TChA> Links = TVec<TChA>();
  Links.Add(TChA("http://www.google.com"));
  Links.Add(TChA("http://www.yahoo.com"));
  
  DocBase->AddDoc(Url, Date, Content, Links);
  printf("Number of documents: %d", DocBase->GetNumDocs().Val);
  return 0;
}
