#include "stdafx.h"
#include "doc.h"

TDoc::TDoc() {
}

TDoc::TDoc(TInt Id, const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links) {
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

TInt TDoc::GetId() const {
  return Id;
}

void TDoc::GetUrl(TStr &Ref) {
  Ref = Url;
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

void TDoc::SetId(TInt Id) {
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
  NextId = 0;
  NumDocs = 0;
}

void TDocBase::Save(TSOut& SOut) const {
  IdToDoc.Save(SOut);
  DocUrlToId.Save(SOut);
  NumDocs.Save(SOut);
  NextId.Save(SOut);
}

void TDocBase::Load(TSIn& SIn) {
  IdToDoc.Load(SIn);
  DocUrlToId.Load(SIn);
  NumDocs.Load(SIn);
  NextId.Load(SIn);
}

int TDocBase::Len() const {
  return NumDocs;
}

TInt TDocBase::GetDocId(const TStr &Url) const {
  if (DocUrlToId.IsKey(Url)) {
    return DocUrlToId.GetDat(Url);
  } else {
    return -1;
  }
}

bool TDocBase::GetDoc(TInt Id, TDoc &RetDoc) const {
  if (IdToDoc.IsKey(Id)) {
    RetDoc = IdToDoc.GetDat(Id);
    return true;
  } else {
    return false;
  }
}

/// Forms a new TDoc from the document information and adds it to the doc base.
TInt TDocBase::AddDoc(const TChA &Url, TSecTm Date, const TChA &Content, const TVec<TChA> &Links) {
  if (!DocUrlToId.IsKey(TStr(Url))) {
    TInt DocId = NextId;
    NextId += 1;
    NumDocs += 1;
    TDoc NewDoc = TDoc(DocId, Url, Date, Content, Links);
    IdToDoc.AddDat(DocId, NewDoc);
    DocUrlToId.AddDat(TStr(Url), DocId);
    return DocId;
  } else {
    return GetDocId(TStr(Url));
  }
}

TInt TDocBase::AddDoc(TDoc &Doc) {
  TStr DocUrl;
  Doc.GetUrl(DocUrl);
  if (!DocUrlToId.IsKey(DocUrl)) {
    TInt DocId = NextId;
    NextId += 1;
    NumDocs += 1;
    Doc.SetId(NextId);
    IdToDoc.AddDat(DocId, Doc);
    DocUrlToId.AddDat(DocUrl, DocId);
    return DocId;
  } else {
    return GetDocId(DocUrl);
  }
}

void TDocBase::RemoveDoc(TInt DocId) {
  if (IdToDoc.IsKey(DocId)) {
    TDoc Doc;
    GetDoc(DocId, Doc);
    TStr DocUrl;
    Doc.GetUrl(DocUrl);

    IdToDoc.DelKey(Doc.GetId());
    DocUrlToId.DelKey(DocUrl);
    NumDocs -= 1;
  }
}

void TDocBase::GetAllDocIds(TVec<TInt> &DocIds) const {
  IdToDoc.GetKeyV(DocIds);
}

