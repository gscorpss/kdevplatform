/* KDevelop
 *
 * Copyright 2011 Aleix Pol <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.2

GroupBox
{
    id: bg
    property alias tools: toolsLoader.sourceComponent
    property string pageIcon
    readonly property real marginLeft: toolsLoader.x + toolsLoader.width
    property real margins: 5

    Loader {
        id: toolsLoader

        width: bg.width/5

        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }
    }

    Image {
        id: theIcon
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: bg.margins
        }
        source: bg.pageIcon !== "" ? "image://icon/" + bg.pageIcon : ""
        width: 64
        height: width
    }
}
