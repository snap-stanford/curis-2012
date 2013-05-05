(same as the memetracker.org website)
Data format: Tab separated file with the following nested structure. Each block of the data has the following structure:
    A:  <ClId>  <Start> <End>   <NmVar> <TotFq> <NmPks> <RtLen> <Root>  <RpURL> <First> <Last>  <PkTm>  <Archived>  <DisSt>
    B:          <QtId>  <QtFq>  <NmPks> <Len>   <Quote> <RpURL> <First> <Last>  <PkTm>
    C:                  <DocId> <Tm>   <Url>
    
    
    Id\tSize\tPeaks\tNumWords\tQuote String\tRepresentative URL\tFirst Mention\tLast Mention\tPeak Time";
    
    
  TStr Response = "# Clusters: Id\tStart\tEnd\t";
  Response += "Variants\tSize\tPeaks\tNumWords\tLongest Variant\tRepresentative URL\t";
  Response += "First Mention Time\tLast Mention Time\tPeak Time\tArchived\tDiscard State";
    
<ClId>: cluster id.
<Start>: date the cluster was born
<End>: date the cluster was retired.
<NmVar>: number of unique phrase variants in the cluster (number of B records).
<TotFq>: total frequency (number of mentions) of all the phrases/variants in the cluster.
<NmPks>: number of computed peaks in cluster
<RtLen>: length (in words) of root variant
<Root>: root variant, i.e. the longest phrase in the cluster
<RpURL>: representative url of the cluster
<First>: time of first recorded URL mention of any phrase variant in the cluster
<Last>: time of last recorded URL mention of any phrase variant in the cluster
<PkTm>: time of highest recorded peak in cluster.
<Archived>: whether cluster was in an archived state at time of completion. 0 false, 1 true.
<DisSt>: whether cluster had been filtered out during post-clustering step. 0 no, 1-2 yes.

<QtId>: phrase/variant id.
<QtFq>: total frequency (number of mentions) of the phrase.
<NmPks>: number of peaks of mentions in the phrase
<Len>: length (in words) of phrase
<Quote>: the quote phrase
<RpURL>: representative url of the quote phrase

<DocId>: document/URL id
<Tm>: time when the article/post <Url>: was published.
<Url>: URL of the blog post/news article.