#ifndef doc_h
#define doc_h

#include "quote.h"

class TQuoteBase;

class TDoc {
private:
  TInt Id;
  // Number of quotes in the QuoteBase that have this document as a source
  TInt NumQuotes;
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
  TInt GetNumQuotes() const;
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
  TInt NextId;
  void RemoveDoc(TInt DocId);

public:
  TDocBase();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  int Len() const;
  TInt GetDocId(const TStr &Url) const;
  bool GetDoc(TInt Id, TDoc &RetDoc) const;
  TInt AddDoc(const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links);
  TInt AddDoc(TDoc &Doc);
  void RemoveNullDocs(TQuoteBase *QB);
  void GetAllDocIds(TVec<TInt> &DocIds) const;
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
  bool operator () (const TInt& P1, const TInt& P2) const {
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
