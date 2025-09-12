
#ifndef CBUFSEG_H
#define CBUFSEG_H 
#include "../debug.h"
#include "../config.h"
#include "types.h"

#include <unistd.h>

#ifdef CONFIG_STYLUS_WITH_LOCKING
#include <QMutex>
#endif


class CBufSeg
{
    public:
        struct tAudio
        {
                char *  Data;
                        tAudio(): Data(0) {}
            private: 
                        tAudio    (tAudio const & rhs);
                tAudio &operator= (tAudio const & rhs);
        };

    private:
        size_t Capacity;
#ifdef CONFIG_STYLUS_WITH_LOCKING
        QMutex   Mutex;
#endif

    public:
        tAudio   Audio;
                 CBufSeg        ();
                ~CBufSeg	();
#ifdef CONFIG_STYLUS_WITH_LOCKING
        void     Lock           () { Mutex.lock(); }
        void     Unlock         () { Mutex.unlock(); }
#else
        void     Lock           () {}
        void     Unlock         () {}

#endif
        CBufSeg& operator=      (CBufSeg const & rhs);
        void     Init           (size_t capacity_in_bytes);

};

#endif
