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
  printf("URL: %s\n", t.GetUrl().CStr());
  
  printf("Link1: %s\n", t.Links[0].CStr());
  printf("Link2: %s\n", t.Links[1].CStr());

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
  printf("URL: %s\n", t2.GetUrl().CStr());

  printf("Link1: %s\n", t2.Links[0].CStr());
  printf("Link2: %s\n", t2.Links[1].CStr());

  delete DocBase2;
  return 0;
}
