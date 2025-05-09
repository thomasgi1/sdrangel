///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2015-2019, 2021 Edouard Griffiths, F4EXB <f4exb06@gmail.com>    //
// Copyright (C) 2015 John Greb <hexameron@spam.no>                              //

// Copyright (C) 2018 F4HKW                                                      //
// for F4EXB / SDRAngel                                                          //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#ifndef DATVVIDEOPLAYER_H
#define DATVVIDEOPLAYER_H

#include "leansdr/framework.h"
#include "datvideostream.h"
#include "datvudpstream.h"

namespace leansdr
{

template<typename T> struct datvvideoplayer: runnable
{
    datvvideoplayer(
        scheduler *sch,
        pipebuf<T> &_in,
        DATVideostream *videoStream,
        DATVUDPStream *udpStream) :
        runnable(sch, _in.name),
        in(_in),
        m_videoStream(videoStream),
        m_udpStream(udpStream),
        m_atomicUDPRunning(0),
        m_count(0),
        m_sr(1500000)
    {
    }

    void run()
    {
        int size = in.readable() * sizeof(T);

        if (!size)
        {
            if (m_count == 0)
            {
    #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                m_atomicUDPRunning.storeRelaxed(0);
    #else
                m_atomicUDPRunning.store(0);
                #endif
            } else {
                m_count--;
            }
            return;
        }

        int nw;

        m_udpStream->pushData((const char *) in.rd(), in.readable());
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_atomicUDPRunning.storeRelaxed(m_udpStream->isActive() && (size > 0) ? 1 : 0);
#else
        m_atomicUDPRunning.store(m_udpStream->isActive() && (size > 0) ? 1 : 0);
#endif
        m_count = m_sr / 10000;

        if (m_videoStream)
        {
            nw = m_videoStream->pushData((const char *) in.rd(), size);

            if (!nw)
            {
                fatal("leansdr::datvvideoplayer::run: pipe");
                return;
            }

            if (nw < 0)
            {
                fatal("leansdr::datvvideoplayer::run: write");
                return;
            }

            if (nw % sizeof(T))
            {
                fatal("leansdr::datvvideoplayer::run: partial write");
                return;
            }

            if (nw != size) {
                fprintf(stderr, "leansdr::datvvideoplayer::run: nw: %d size: %d\n", nw, size);
            }
        }
        else
        {
            nw = size;
        }

        in.read(nw / sizeof(T));
    }

    bool isUDPRunning() const
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        return m_atomicUDPRunning.loadRelaxed() == 1;
#else
        return m_atomicUDPRunning.load() == 1;
#endif
    }

    void resetUDPRunning()
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_atomicUDPRunning.storeRelaxed(0);
#else
        m_atomicUDPRunning.store(0);
#endif
    }

    void setSymbolRate(unsigned int sr)
    {
        m_sr = sr;
    }

private:
    pipereader<T> in;
    DATVideostream *m_videoStream;
    DATVUDPStream *m_udpStream;
    QAtomicInt m_atomicUDPRunning;
    unsigned int m_count;
    unsigned int m_sr; // Symbol rate in S/s used for UDP running detection
};

}

#endif // DATVVIDEOPLAYER_H
