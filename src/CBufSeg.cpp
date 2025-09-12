#include "CBufSeg.h"
#include <memory.h>


CBufSeg::CBufSeg():
    Capacity(0)
{
}

void CBufSeg::Init(size_t capacity)
{
    Capacity = capacity;
    delete [] Audio.Data;
    Audio.Data = new char [capacity];
}


CBufSeg::~CBufSeg()
{
	delete [] Audio.Data;
}


CBufSeg & CBufSeg::operator= (CBufSeg const & rhs)
{
#ifdef CONFIG_STYLUS_WITH_LOCKING
    QMutexLocker l (&Mutex);
#endif    
    memcpy((void*)Audio.Data, (void*) rhs.Audio.Data, Capacity);
    return *this;    
}

