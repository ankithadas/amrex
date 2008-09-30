#include <winstd.H>

#include <map>

#include <FabArray.H>


FabArrayBase::FabArrayBase ()
{}

FabArrayBase::FabArrayBase (const BoxArray& bxs, int nvar, int ngrow)
    :
    boxarray(bxs),
    distributionMap(boxarray, ParallelDescriptor::NProcs()),
    n_grow(ngrow),
    n_comp(nvar)
{}

FabArrayBase::FabArrayBase (const BoxArray&            bxs,
                            int                        nvar,
                            int                        ngrow,
                            const DistributionMapping& map)
    :
    boxarray(bxs),
    distributionMap(map),
    n_grow(ngrow),
    n_comp(nvar)
{}

FabArrayBase::~FabArrayBase ()
{}

Box
FabArrayBase::fabbox (int K) const
{
    //
    // Do not use fabparray[K] because it may not be valid in parallel.
    //
    return BoxLib::grow(boxarray[K], n_grow);
}

MFIter::MFIter (const FabArrayBase& fabarray)
    :
    fabArray(fabarray),
    currentIndex(0)
{
    //
    // Increment the currentIndex to start at the first valid index
    // for this ParallelDescriptor::MyProc.
    //
    const int MyProc = ParallelDescriptor::MyProc();

    while (fabArray.DistributionMap()[currentIndex] != MyProc)
    {
        ++currentIndex;
    }
}

void
MFIter::operator++ ()
{
    const int MyProc = ParallelDescriptor::MyProc();
    //
    // Go to the next index on this processor.
    //
    do
    {
        ++currentIndex;
    }
    while (fabArray.DistributionMap()[currentIndex] != MyProc);
}

bool
MFIter::isValid ()
{
    BL_ASSERT(currentIndex >= 0);

    bool rc = currentIndex < fabArray.size();

    return rc;
}

const Box&
MFIter::validbox () const
{
    return fabArray.box(currentIndex);
}

Box
MFIter::fabbox () const
{
    return fabArray.fabbox(currentIndex);
}

//
// Used to cache some CommData stuff in CollectData().
//

CommDataCache::CommDataCache ()
    :
    m_valid(false)
{}

void
CommDataCache::operator= (const Array<CommData>& rhs)
{
    m_commdata = rhs;
    m_valid    = true;
}

//
// Stuff used for copy() caching.
//

CopyComTag::CopyComTag () {}

CopyComTag::CopyComTag (const CopyComTag& cct)
    :
    box(cct.box),
    fabIndex(cct.fabIndex),
    srcIndex(cct.srcIndex)
{}

CPC::CPC ()
    :
    m_reused(false)
{}

CPC::CPC (const BoxArray&            dstba,
          const BoxArray&            srcba,
          const DistributionMapping& dstdm,
          const DistributionMapping& srcdm)
    :
    m_dstba(dstba),
    m_srcba(srcba),
    m_dstdm(dstdm),
    m_srcdm(srcdm),
    m_reused(false)
{}

CPC::CPC (const CPC& rhs)
    :
    m_dstba(rhs.m_dstba),
    m_srcba(rhs.m_srcba),
    m_dstdm(rhs.m_dstdm),
    m_srcdm(rhs.m_srcdm),
    m_reused(rhs.m_reused)
{}

CPC::~CPC () {}

bool
CPC::operator== (const CPC& rhs) const
{
    return m_dstba == rhs.m_dstba &&
           m_srcba == rhs.m_srcba &&
           m_dstdm == rhs.m_dstdm &&
           m_srcdm == rhs.m_srcdm;
}

bool
CPC::operator!= (const CPC& rhs) const
{
    return !operator==(rhs);
}

typedef std::multimap<int,CPC> CPCCache;

typedef CPCCache::iterator CPCCacheIter;

static CPCCache TheCopyCache;

CPC&
CPC::TheCPC (const CPC& cpc, bool& got_from_cache)
{
    static bool first               = true;
    static bool use_copy_cache      = true;
    static int  copy_cache_max_size = 25;

    if (first)
    {
        first = false;
        ParmParse pp("fabarray");
        pp.query("use_copy_cache", use_copy_cache);
        pp.query("copy_cache_max_size", copy_cache_max_size);
    }

    got_from_cache = false;

    const int key = cpc.m_dstba.size() + cpc.m_srcba.size();

    if (use_copy_cache)
    {
        std::pair<CPCCacheIter,CPCCacheIter> er_it = TheCopyCache.equal_range(key);

        for (CPCCacheIter it = er_it.first; it != er_it.second; ++it)
        {
            if (it->second == cpc)
            {
                it->second.m_reused = true;
                got_from_cache = true;
                return it->second;
            }
        }

        if (TheCopyCache.size() >= copy_cache_max_size)
        {
            //
            // Don't let the size of the cache get too big.
            //
            for (CPCCacheIter it = TheCopyCache.begin(); it != TheCopyCache.end(); )
            {
                if (!it->second.m_reused)
                {
                    TheCopyCache.erase(it++);
                    //
                    // Only delete enough entries to stay under limit.
                    //
                    if (TheCopyCache.size() < copy_cache_max_size) break;
                }
                else
                {
                    ++it;
                }
            }

            if (TheCopyCache.size() >= copy_cache_max_size)
                //
                // Get rid of entry with the smallest key.
                //
                TheCopyCache.erase(TheCopyCache.begin());
        }
    }
    else
    {
        TheCopyCache.clear();
    }

    CPCCacheIter it = TheCopyCache.insert(std::make_pair(key,cpc));

    return it->second;
}

void
CPC::FlushCache ()
{
    if (ParallelDescriptor::IOProcessor() && TheCopyCache.size())
        std::cout << "CPC::TheCopyCache.size() = " << TheCopyCache.size() << std::endl;
    TheCopyCache.clear();
}

FabArrayBase::SI::SI ()
    :
    m_ngrow(-1),
    m_reused(false)
{}

FabArrayBase::SI::SI (const BoxArray&            ba,
                      const DistributionMapping& dm,
                      int                        ngrow)
    :
    m_ba(ba),
    m_dm(dm),
    m_ngrow(ngrow),
    m_reused(false)
{
    BL_ASSERT(ngrow >= 0);
}

FabArrayBase::SI::SI (const FabArrayBase::SI& rhs)
    :
    m_cache(rhs.m_cache),
    m_commdata(rhs.m_commdata),
    m_sirec(rhs.m_sirec),
    m_ba(rhs.m_ba),
    m_dm(rhs.m_dm),
    m_ngrow(rhs.m_ngrow),
    m_reused(rhs.m_reused)
{}

FabArrayBase::SI::~SI () {}

bool
FabArrayBase::SI::operator== (const FabArrayBase::SI& rhs) const
{
    return m_ngrow == rhs.m_ngrow && m_ba == rhs.m_ba && m_dm == rhs.m_dm;
}

bool
FabArrayBase::SI::operator!= (const FabArrayBase::SI& rhs) const
{
    return !operator==(rhs);
}

typedef std::multimap<int,FabArrayBase::SI> SIMMap;

typedef SIMMap::iterator SIMMapIter;

static SIMMap SICache;

void
FabArrayBase::FlushSICache ()
{
    if (ParallelDescriptor::IOProcessor() && SICache.size())
        std::cout << "FabArrayBase::SICacheSize() = " << SICache.size() << std::endl;
    SICache.clear();
}

int
FabArrayBase::SICacheSize ()
{
    return SICache.size();
}

FabArrayBase::SI&
FabArrayBase::BuildFBsirec (const FabArrayBase::SI& si,
                            const FabArrayBase&     mf)
{
    BL_ASSERT(si.m_ngrow >= 0);
    BL_ASSERT(mf.nGrow() == si.m_ngrow);
    BL_ASSERT(mf.boxArray() == si.m_ba);

    const int key = mf.nGrow() + mf.size();

    SIMMapIter it = SICache.insert(std::make_pair(key,si));

    const BoxArray&            ba     = mf.boxArray();
    const DistributionMapping& DMap   = mf.DistributionMap();
    const int                  MyProc = ParallelDescriptor::MyProc();
    std::vector<SIRec>&        sirec  = it->second.m_sirec;
    Array<int>&                cache  = it->second.m_cache;

    cache.resize(ParallelDescriptor::NProcs(),0);

    for (MFIter mfi(mf); mfi.isValid(); ++mfi)
    {
        const int i = mfi.index();

        std::vector< std::pair<int,Box> > isects = ba.intersections(mfi.fabbox());

        for (int ii = 0; ii < isects.size(); ii++)
        {
            const Box& bx  = isects[ii].second;
            const int  iii = isects[ii].first;

            if (i != iii)
            {
                sirec.push_back(SIRec(i,iii,bx));

                if (DMap[iii] != MyProc)
                    //
                    // If we intersect them then they'll intersect us.
                    //
                    cache[DMap[iii]] += 1;
            }
        }

        BL_ASSERT(cache[DMap[i]] == 0);
    }

    return it->second;
}

FabArrayBase::SI&
FabArrayBase::TheFBsirec (int                 scomp,
                          int                 ncomp,
                          const FabArrayBase& mf)
{
    BL_ASSERT(ncomp >  0);
    BL_ASSERT(scomp >= 0);

    static bool first             = true;
    static bool use_fb_cache      = true;
    static int  fb_cache_max_size = 25;

    if (first)
    {
        first = false;
        ParmParse pp("fabarray");
        pp.query("use_fb_cache", use_fb_cache);
        pp.query("fb_cache_max_size", fb_cache_max_size);
    }

    const FabArrayBase::SI si(mf.boxArray(), mf.DistributionMap(), mf.nGrow());

    const int key = mf.nGrow() + mf.size();

    if (use_fb_cache)
    {
        std::pair<SIMMapIter,SIMMapIter> er_it = SICache.equal_range(key);
    
        for (SIMMapIter it = er_it.first; it != er_it.second; ++it)
        {
            if (it->second == si)
            {
                it->second.m_reused = true;
                //
                // Adjust the ncomp & scomp in CommData.
                //
                Array<CommData>& cd = it->second.m_commdata.theCommData();

                for (int i = 0; i < cd.size(); i++)
                {
                    cd[i].nComp(ncomp);
                    cd[i].srcComp(scomp);
                }

                return it->second;
            }
        }

        if (SICache.size() >= fb_cache_max_size)
        {
            //
            // Don't let the size of the cache get too big.
            //
            for (SIMMapIter it = SICache.begin(); it != SICache.end(); )
            {
                if (!it->second.m_reused)
                {
                    SICache.erase(it++);
                    //
                    // Only delete enough entries to stay under limit.
                    //
                    if (SICache.size() < fb_cache_max_size) break;
                }
                else
                {
                    ++it;
                }
            }

            if (SICache.size() >= fb_cache_max_size)
                //
                // Get rid of entry with the smallest key.
                //
                SICache.erase(SICache.begin());
        }
    }
    else
    {
        SICache.clear();
    }

    return BuildFBsirec(si,mf);
}

