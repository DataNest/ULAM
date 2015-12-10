/* -*- C++ -*- */

#include "UlamDefs.h"
#include "MDist.h"

namespace MFM {

  template<class EC, u32 POS>
  Ui_Ut_10161u Uq_10105MDist10<EC,POS>::Uf_9212getLastIndex(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10131u Uv_6radius)
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_10161u ret(MDist<R>::get().GetLastIndex(radius));
    return ret;
  }

  template<class EC, u32 POS>
  Ui_Ut_10161u Uq_10105MDist10<EC,POS>::Uf_9213getFirstIndex(const UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10131u Uv_6radius)
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };

    u32 radius = Uv_6radius.read();
    Ui_Ut_10161u ret(MDist<R>::get().GetFirstIndex(radius));
    return ret;
  }
} //MFM
