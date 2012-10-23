#include "toolkit.h"

namespace ToolKit{

QString fileSize2Str(qint64 size)
{
    const qint64 OneK = 1024;
    const qint64 OneM = 1024 * 1024;
    const qint64 OneG = 1024 * 1024 * 1024;

    if(size <= OneK)
        return QString("%1 B").arg(size);
    else if ( size <= OneM )
        return QString("%1 KB").arg(size / qreal(OneK), 0, 'g', 3);
    else if ( size <= OneG )
        return QString("%1 MB").arg(size / qreal(OneM), 0, 'g', 3);
    else
        return QString("%1 GB").arg(size / qreal(OneG), 0, 'g', 3);
}

}
