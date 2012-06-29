#ifndef doc_h
#define doc_h

#include "stdafx.h"

class TDoc {
private:
  TInt Id;
  TStr Url;
  TSecTm Date;
  TStr Content;
  TStrV Links;

public:
  TDoc(TInt Id, TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links);
  TInt GetId() const;
  TStr GetUrl() const;
  TSecTm GetDate() const;
  TStr GetContent() const;
  TStrVP GetLinks() const;

  void SetId(TInt Id);
  void SetUrl(TStr Url);
  void SetDate(TSecTm Date);
  void SetContent(TStr Content);
  void AddLink(TStr Link);
};

class TDocBase {
private:
  PHash<TInt, TDoc *> IdToDoc;
  PHash<TStr, TInt> DocUrlToId;
  TInt NumDocs;
  TInt NextId;

public:
  TDocBase();
  TInt Len() const;
  TInt GetDocId(TStr Url) const;
  TDoc *GetDoc(TInt Id) const;
  TInt AddDoc(TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links);
  void RemoveDoc(TDoc *Doc);
  void RemoveDoc(TInt DocId);
};

#endif
