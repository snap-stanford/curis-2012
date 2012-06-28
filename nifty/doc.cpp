#include "stdafx.h"
#include "doc.h"

TDoc::TDoc(TChA Url, TSecTm Date, TChA Content, TVec<TChA> Links) {
  // TODO: Check that URLs are not repeated
  static TDocBase DocBase = new TDocBase();
  this->Id = DocBase.GetAndIncNumDocs();
  this->Url = new TStr(Url);
  this->Title = new TStr();
  this->Content = new TStr(Content);
  this->Links = new TStrVP();
  for (int i = 0; i < Links.Len(); i++) {
    TStr ToAdd = new TStr(Links.GetVal(i));
    AddLink(ToAdd);
  }
  DocBase.AddDoc(this);
}

TInt TDoc::GetId() const {
  return Id;
}

TStr TDoc::GetUrl() const {
  return Url;
}

TStr TDoc::GetTitle() const {
  return Title;
}

TTm TDoc::GetDate() const {
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

void TDoc::SetTitle(TStr Title) {
  this->Title = Title;
}

void TDoc::SetDate(TTm Date) {
  this->Date = Date;
}

void TDoc::SetContent(TStr Content) {
  this->Content = Content;
}

void TDoc::AddLink(TStr Link) {
  Links.Add(Link);
}

TDocBase::TDocBase() {
  IdToDoc = new PHash<TInt, TDoc>();
  DocUrlToId = new PHash<TStr, TInt>();
  NumDocs = 0;
}

TInt TDocBase::GetNumDocs() const {
  return NumDocs;
}

TInt TDocBase::GetAndIncNumDocs() {
  NumDocs = NumDocs + 1;
  return NumDocs - 1;
}

TInt TDocBase::GetDocId(TStr Url) const {
  if (DocUrlToId.H.IsKey(Url)) {
    return DocUrlToId.H.GetDat(Url);
  } else {
    return NULL;
  }
}

TDoc TDocBase::GetDoc(TInt Id) const {
  if (IdToDoc.H.IsKey(Id)) {
    return IdToDoc.H.GetDat(Id);
  } else {
    return NULL;
  }
}

void TDocBase::AddDoc(TDoc *Doc) {
  IdToDoc.H.AddDat(Doc->GetId(), Doc);
  DocUrlToId.H.AddDat(Doc->GetUrl(), Doc->GetId());
}

void TDocBase::RemoveDoc(TDoc *Doc) {
  if (IdToDoc.H.IsKey(Doc->GetId())) {
    IdToDoc.H.DelKey(Doc->GetId());
  }
  if (DocUrlToId.H.IsKey(Doc->GetUrl())) {
    DocUrlToId.H.DelKey(Doc->GetUrl());
  }
}
