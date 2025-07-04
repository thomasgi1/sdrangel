///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2012 maintech GmbH, Otto-Hahn-Str. 15, 97204 Hoechberg, Germany //
// written by Christian Daniel                                                   //
// Copyright (C) 2015-2020 Edouard Griffiths, F4EXB <f4exb06@gmail.com>          //
// Copyright (C) 2023 Jon Beniston, M7RCE <jon@beniston.com>                     //
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

#ifndef REMOTETCPSINKSTARTER_H
#define REMOTETCPSINKSTARTER_H

#include "mainparser.h"
#include "export.h"

#include <QObject>

class DeviceSet;
class DeviceAPI;
class ChannelAPI;

class SDRBASE_API RemoteTCPSinkStarter : public QObject {

    Q_OBJECT

    QString m_dataAddress;
    int m_dataPort;

    DeviceSet *m_deviceSet;

public:
    static void listAvailableDevices();
    static void start(const MainParser& parser);


private:
    RemoteTCPSinkStarter(const QString& address, int port, const QString& hwType, const QString& serial);
    void startDevice();

private slots:
    void deviceOpened(int deviceSetIndex);
    void channelAdded(int deviceSetIndex, ChannelAPI *channel);

};

#endif /* REMOTETCPSINKSTARTER_H */
