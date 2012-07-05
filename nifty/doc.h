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
  TDoc();
  TDoc(TInt Id, const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links);
  TDoc(TSIn& SIn) : Id(SIn), Url(SIn), Date(SIn), Content(SIn), Links(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TInt GetId() const;
  void GetUrl(TStr &Ref);
  TSecTm GetDate() const;
  void GetContent(TStr &Ref);
  void GetLinks(TStrV &RefL);

  void SetId(TInt Id);
  void SetUrl(const TStr &Url);
  void SetDate(TSecTm Date);
  void SetContent(const TStr &Content);
  void AddLink(const TStr &Link);
};

class TDocBase {
private:
  THash<TInt, TDoc> IdToDoc;
  THash<TStr, TInt> DocUrlToId;
  TInt NumDocs;
  TInt NextId;

public:
  TDocBase();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  int Len() const;
  TInt GetDocId(const TStr &Url) const;
  bool GetDoc(TInt Id, TDoc &RetDoc) const;
  TInt AddDoc(const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links);
  TInt AddDoc(TDoc &Doc);
  void RemoveDoc(TInt DocId);
  void GetAllDocIds(TVec<TInt> &DocIds) const;
};

#endif
