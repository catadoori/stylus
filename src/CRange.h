#ifndef CRANGE_H
#define CRANGE_H

template <class T>
class CRange
{
    private:
        T       First;
        T       Last;
    public:
                CRange      (): First(0), Last(0) {}
        void    Set         (T first, T last) { First = first; Last = last; }
        void    SetFirst    (T val)           { First = val;                }
        void    SetLast     (T val)           { Last = val;                 }
        T       GetFirst    ()     const      { return First;               }
        T       GetLast     ()     const      { return Last;                }
        T       GetDiff     ()     const      { return Last - First;        }  
};

#endif
