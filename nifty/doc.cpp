#include "stdafx.h"
#include "doc.h"

TDoc::TDoc() {
}

TDoc::TDoc(TInt Id, TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links) {
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

TInt TDoc::GetId() const {
  return Id;
}

TStr TDoc::GetUrl() const {
  return Url;
}

TSecTm TDoc::GetDate() const {
  return Date;
}

TStr TDoc::GetContent() const {
  return Content;
}

TStrVP TDoc::GetLinks() const {
  return Links;
}

void TDoc::SetId(TInt Id) {
  this->Id = Id;
}

void TDoc::SetUrl(TStr Url) {
  this->Url = Url;
}

void TDoc::SetDate(TSecTm Date) {
  this->Date = Date;
}

void TDoc::SetContent(TStr Content) {
  this->Content = Content;
}

void TDoc::AddLink(TStr Link) {
  Links.Add(Link);
}

TDocBase::TDocBase() {
  NextId = 0;
  NumDocs = 0;
}

TInt TDocBase::Len() const {
  return NumDocs;
}

TInt TDocBase::GetDocId(TStr Url) const {
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
TInt TDocBase::AddDoc(TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links) {
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

void TDocBase::RemoveDoc(TInt DocId) {
  if (IdToDoc.IsKey(DocId)) {
    TDoc Doc;
    GetDoc(DocId, Doc);
    IdToDoc.DelKey(Doc.GetId());
    DocUrlToId.DelKey(Doc.GetUrl());
    NumDocs -= 1;
  }
}

void TDocBase::GetAllDocIds(TVec<TInt> &DocIds) const {
  IdToDoc.GetKeyV(DocIds);
}
