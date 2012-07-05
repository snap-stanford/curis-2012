#include "stdafx.h"

int main(int argc, char *argv[]) {
  TDocBase *DocBase = new TDocBase;
  TChA Url = TChA("http://www.newyorktimes.com/news_story");
  TSecTm Date = TSecTm::GetCurTm();
  TChA Content = TChA("foo bar foo foo");
  TVec<TChA> Links;
  Links.Add(TChA("http://www.google.com"));
  Links.Add(TChA("http://www.yahoo.com"));
  DocBase->AddDoc(Url, Date, Content, Links);

  printf("Number of documents: %d\n", DocBase->Len());
  
  TDoc t;
  DocBase->GetDoc(0, t);
  TStr tUrl;
  t.GetUrl(tUrl);
  printf("URL: %s\n", tUrl.CStr());
  
  TStrV l;
  t.GetLinks(l);
  printf("Link1: %s\n", l[0].CStr());
  printf("Link2: %s\n", l[1].CStr());

  { TFOut FOut("tmp.bin"); DocBase->Save(FOut); }
  printf("Save data successfully\n");

  delete DocBase;

  TFIn FIn("tmp.bin");
  printf("Load data successfully\n");
  TDocBase *DocBase2 = new TDocBase;
  DocBase2->Load(FIn);

  printf("Number of documents: %d\n", DocBase2->Len());

  TDoc t2;
  DocBase2->GetDoc(0, t2);
  TStr t2Url;
  t2.GetUrl(t2Url);
  printf("URL: %s\n", t2Url.CStr());

  t2.GetLinks(l);
  printf("Link1: %s\n", l[0].CStr());
  printf("Link2: %s\n", l[1].CStr());

  delete DocBase2;
  return 0;
}
