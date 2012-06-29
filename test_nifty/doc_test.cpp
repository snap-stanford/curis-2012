#include "stdafx.h"

int main(int argc, char *argv[]) {
  TDocBase DocBase;
  TChA Url = TChA("http://www.newyorktimes.com/news_story");
  TSecTm Date = TSecTm::GetCurTm();
  TChA Content = TChA("foo bar foo foo");
  TVec<TChA> Links;
  Links.Add(TChA("http://www.google.com"));
  Links.Add(TChA("http://www.yahoo.com"));
  printf("Number of documents: %d\n", DocBase.Len().Val);
  
  printf("%d\n", DocBase.AddDoc(Url, Date, Content, Links));
  
  TDoc t;
  DocBase.GetDoc(0, t);
  printf("%s\n", t.GetUrl().CStr());
  
  printf("%s\n", t.Links[0].CStr());
  printf("%s\n", t.Links[1].CStr());

  return 0;
}
