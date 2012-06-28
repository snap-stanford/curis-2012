#ifndef doc_h
#define doc_h

#include "stdafx.h"

class TDoc {
private:
  TInt Id;
  TStr Url;
  TStr Title;
  TTm Date;
  TStr Content;
  TStrVP Links;

public:
  TDoc(TInt Id, TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links);
  TInt GetId() const;
  TStr GetUrl() const;
  TStr GetTitle() const;
  TTm GetDate() const;
  TStr GetContent() const;
  TStrVP GetLinks() const;

  void SetId(TInt Id);
  void SetUrl(TStr Url);
  void SetTitle(TStr Title);
  void SetDate(TTm Date);
  void SetContent(TStr Content);
  void AddLink(TStr Link);
};

class TDocBase {
private:
  PHash<TInt, TDoc *> IdToDoc;
  PHash<TStr, TInt> DocUrlToId;
  TInt NumDocs;

public:
  TDocBase();
  TInt GetNumDocs() const;
  TInt GetAndIncNumDocs();
  TInt GetDocId(TStr Url) const;
  TDoc *GetDoc(TInt Id) const;
  void AddDoc(TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links);
  void RemoveDoc(TDoc *Doc);
};

#endif
