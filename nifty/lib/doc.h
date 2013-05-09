#ifndef doc_h
#define doc_h

#include "quote.h"

class TQuoteBase;

class TDoc {
private:
  TUInt Id;
  TStr Url;
  TSecTm Date;
  TStr Content;
  TStrV Links;

public:
  TDoc();
  TDoc(TUInt Id, const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links);
  TDoc(TSIn& SIn) : Id(SIn), Url(SIn), Date(SIn), Content(SIn), Links(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TUInt GetId() const;
  void GetUrl(TStr &Ref);
  void GetDomainName(TStr &DomainName);
  TSecTm GetDate() const;
  void GetContent(TStr &Ref);
  void GetLinks(TStrV &RefL);

  void SetId(TUInt Id);
  void SetUrl(const TStr &Url);
  void SetDate(TSecTm Date);
  void SetContent(const TStr &Content);
  void AddLink(const TStr &Link);
};

class TDocBase {
private:
  THash<TUInt, TDoc> IdToDoc;
  THash<TStr, TUInt> DocUrlToId;
  TUInt NextId;

public:
  TDocBase();
  TDocBase& operator=(const TDocBase& QB) {
    NextId = QB.NextId;
    IdToDoc = QB.IdToDoc;
    DocUrlToId = QB.DocUrlToId;
    return *this;
  };
  TDocBase(TUInt OldCounter);
  TUInt GetCounter();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  int Len() const;
  TUInt GetDocId(const TStr &Url) const;
  bool GetDoc(TUInt Id, TDoc &RetDoc) const;
  TUInt AddDoc(const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links);
  TUInt AddDoc(TDoc &Doc);
  void RemoveDoc(TUInt DocId);
  void RemoveNullDocs(TQuoteBase *QB);
  void GetAllDocIds(TVec<TUInt> &DocIds) const;
};

// Compares two docs by their date
class TCmpDocByDate {
private:
  bool IsAsc;
  TDocBase *DocBase;
public:
  TCmpDocByDate(const bool& AscSort=true, TDocBase *DB=NULL) : IsAsc(AscSort) {
    DocBase = DB;
  }
  bool operator () (const TUInt& P1, const TUInt& P2) const {
    TDoc Doc1;
    DocBase->GetDoc(P1, Doc1);
    TDoc Doc2;
    DocBase->GetDoc(P2, Doc2);
    if (IsAsc) {
      return Doc1.GetDate().GetAbsSecs() < Doc2.GetDate().GetAbsSecs();
    } else {
      return Doc2.GetDate().GetAbsSecs() < Doc1.GetDate().GetAbsSecs();
    }
  }
};


#endif
