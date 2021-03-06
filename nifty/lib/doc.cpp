#include "quote.h"
#include "doc.h"

TDoc::TDoc() {
}

TDoc::TDoc(TUInt Id, const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links) {
  // TODO: Check that URLs are not repeated
  this->Url = TStr(Url);
  this->Date = Date;
  this->Content = TStr(Content);
  this->Links = TStrV();
  for (int i = 0; i < Links.Len(); i++) {
    TStr ToAdd = TStr(Links.GetVal(i));
    AddLink(ToAdd);
  }
}

void TDoc::Save(TSOut& SOut) const {
  Id.Save(SOut);
  Url.Save(SOut);
  Date.Save(SOut);
  Content.Save(SOut);
  Links.Save(SOut);
}

void TDoc::Load(TSIn& SIn) {
  Id.Load(SIn);
  Url.Load(SIn);
  Date.Load(SIn);
  Content.Load(SIn);
  Links.Load(SIn);
}

TUInt TDoc::GetId() const {
  return Id;
}

void TDoc::GetUrl(TStr &Ref) {
  Ref = Url;
}

void TDoc::GetDomainName(TStr &DomainName) {
  DomainName = TStringUtil::GetDomainName(Url);
}

TSecTm TDoc::GetDate() const {
  return Date;
}

void TDoc::GetContent(TStr &Ref) {
  Ref = Content;
}

void TDoc::GetLinks(TStrV &RefL) {
  RefL = Links;
}

void TDoc::SetId(TUInt Id) {
  this->Id = Id;
}

void TDoc::SetUrl(const TStr &Url) {
  this->Url = Url;
}

void TDoc::SetDate(TSecTm Date) {
  this->Date = Date;
}

void TDoc::SetContent(const TStr &Content) {
  this->Content = Content;
}

void TDoc::AddLink(const TStr &Link) {
  Links.Add(Link);
}

TDocBase::TDocBase() {
  NextId = 1;
}

TDocBase::TDocBase(TUInt OldCounter) {
  NextId = OldCounter;
}

TUInt TDocBase::GetCounter() {
  return NextId;
}

void TDocBase::Save(TSOut& SOut) const {
  IdToDoc.Save(SOut);
  DocUrlToId.Save(SOut);
  NextId.Save(SOut);
}

void TDocBase::Load(TSIn& SIn) {
  IdToDoc.Load(SIn);
  DocUrlToId.Load(SIn);
  NextId.Load(SIn);
}

int TDocBase::Len() const {
  return IdToDoc.Len();
}

TUInt TDocBase::GetDocId(const TStr &Url) const {
  if (DocUrlToId.IsKey(Url)) {
    return DocUrlToId.GetDat(Url);
  } else {
    return 0;
  }
}

bool TDocBase::GetDoc(TUInt Id, TDoc &RetDoc) const {
  if (IdToDoc.IsKey(Id)) {
    RetDoc = IdToDoc.GetDat(Id);
    return true;
  } else {
    return false;
  }
}

/// Forms a new TDoc from the document information and adds it to the doc base.
TUInt TDocBase::AddDoc(const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links) {
  if (!DocUrlToId.IsKey(TStr(Url))) {
    TUInt DocId = NextId;
    NextId = NextId + 1;
    TDoc NewDoc = TDoc(DocId, Url, Date, Content, Links);
    IdToDoc.AddDat(DocId, NewDoc);
    DocUrlToId.AddDat(TStr(Url), DocId);
    return DocId;
  } else {
    return GetDocId(TStr(Url));
  }
}

TUInt TDocBase::AddDoc(TDoc &Doc) {
  TStr DocUrl;
  Doc.GetUrl(DocUrl);
  if (!DocUrlToId.IsKey(DocUrl)) {
    TUInt DocId = NextId;
    NextId = NextId + 1;
    Doc.SetId(DocId);
    IdToDoc.AddDat(DocId, Doc);
    DocUrlToId.AddDat(DocUrl, DocId);
    return DocId;
  } else {
    return GetDocId(DocUrl);
  }
}

// Only to be used when storing/loading popular docs
void TDocBase::AddStaticDoc(TUInt DocId, TDoc &Doc) {
  TStr DocUrl;
  Doc.GetUrl(DocUrl);
  if (!DocUrlToId.IsKey(DocUrl)) {
    Doc.SetId(DocId);
    IdToDoc.AddDat(DocId, Doc);
    DocUrlToId.AddDat(DocUrl, DocId);
  }
}

void TDocBase::RemoveDoc(TUInt DocId) {
  IAssert(IdToDoc.IsKey(DocId));
  TDoc Doc;
  GetDoc(DocId, Doc);
  TStr DocUrl;
  Doc.GetUrl(DocUrl);

  IdToDoc.DelKey(DocId);
  DocUrlToId.DelKey(DocUrl);
  IAssert(IdToDoc.Len() == DocUrlToId.Len());
}

void TDocBase::GetAllDocIds(TVec<TUInt> &DocIds) const {
  IdToDoc.GetKeyV(DocIds);
}

void TDocBase::RemoveNullDocs(TQuoteBase *QB) {
  Err("QB Size: %d DB Size: %d\n", QB->Len(), Len());
  THashSet<TUInt> ValidDocs;

  TIntV QuoteIds;
  QB->GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    QB->GetQuote(QuoteIds[i], Q);
    TVec<TUInt> Sources;
    Q.GetSources(Sources);
    for (int j = 0; j < Sources.Len(); j++) {
      ValidDocs.AddKey(Sources[j]);
    }
  }

  int count = 0;
  TVec<TUInt> DocIds;
  GetAllDocIds(DocIds);
  for (int i = 0; i < DocIds.Len(); i++) {
    if(!ValidDocs.IsKey(DocIds[i])) {
      RemoveDoc(DocIds[i]);
      count++;
    }
  }
  Err("Removed %d documents.\n", count);
  Err("DB Size: %d\n", Len());

  IAssert(ValidDocs.Len() == Len());
}

