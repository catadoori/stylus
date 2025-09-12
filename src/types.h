#ifndef STYLUS_TYPES_H
#define STYLUS_TYPES_H
#include <limits.h>
#include <QtGlobal>

#define SET_LIMITED(lhs, rhs, ll, ul) if ((rhs) < (ll)) (lhs)=(ll); \
                                 else if ((rhs) > (ul)) (lhs)=(ul); else (lhs) = (rhs);
                                 

struct tSample16
{
    short Left;
    short Right;
            tSample16():                    Left(0), Right(0) {}
            tSample16(tSample16 const & r): Left(r.Left), Right(r.Right) {}
    void    Clear    ()                     { Left = Right = 0; }

    void    SetLeft  (int l) { SET_LIMITED(Left, l, SHRT_MIN, SHRT_MAX); }
    void    SetRight (int r) { SET_LIMITED(Right, r, SHRT_MIN, SHRT_MAX); }
    
};

struct tSample32
{
    int Left;
    int Right;
            tSample32(): Left(0), Right(0) {}
    void    Clear    () { Left = Right = 0; }
    void    SetLeft  (qint64 & l) { SET_LIMITED(Left, l, INT_MIN, INT_MAX); }
    void    SetRight (qint64 & r) { SET_LIMITED(Right, r, INT_MIN, INT_MAX); }
};
struct tSample64
{
    long long Left;
    long long Right;
                tSample64(): Left(0), Right(0) {}
    void        Clear    () { Left = Right = 0; }                
};
struct tSampleF
{
    double Left;
    double Right;
            tSampleF():Left(0), Right(0) {}
    void    Clear   ()                   { Left = Right = 0.0; }

};

struct tMeterData
{
    tSampleF RMS;
    tSampleF Peak;
             tMeterData () {}
    void     Clear()       { RMS.Left = RMS.Right = Peak.Left = Peak.Right = -999; }
};


struct tPSample // a profile sample, used mainly in CDetector
{
    unsigned short  AvgRMS;
    unsigned short  Thres;
    unsigned short  AvgLine;

    tSample16       RMS;
    tSample16       Peak;
                    tPSample():  AvgRMS(0), Thres(0), AvgLine(0) {}

                    tPSample(unsigned short avg_rms,  unsigned short thres, unsigned short line,
                             tSample16 const & rms, tSample16 const & peak):
                                AvgRMS(avg_rms), Thres(thres), AvgLine(line),
                                RMS(rms), Peak(peak) {}

    void            Clear   () { AvgRMS = Thres = AvgLine = 0; RMS.Clear(); Peak.Clear(); };
    
};

#endif
