#ifndef CVORBISENC_H
#define CVORBISENC_H

#include "CBaseEnc.h"
#include<ogg/ogg.h>
#include<vorbis/codec.h>


class CVorbisEnc: public CBaseEnc
{
    public:
        bool Init(tInfo const & info, bool local, bool remote);

        void Process(CBufSeg const * seg);
        bool Flush  () { return true; }

        CVorbisEnc();
        ~CVorbisEnc();

    private:
        vorbis_block     Block;
        vorbis_comment   Comment;
        vorbis_dsp_state Dsp;
        vorbis_info      Info;
        ogg_stream_state Stream;
        ogg_page         Page;
        ogg_packet       Packet;

        float            Scale;

        typedef void (CVorbisEnc::*tProcFunc)(CBufSeg const * seg, float ** buffer);
        tProcFunc ProcFunc;

        void do_vorbis();
        void process_16(CBufSeg const * seg, float ** buffer);
        void process_32(CBufSeg const * seg, float ** buffer);
        void write();

};



#endif // CVORBISENC_H
