/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.1
import com.github.mmdai.VPVM 1.0 as VPVM

Tab {
    id: lightTab
    title: qsTr("Light")
    anchors.margins: propertyPanel.anchors.margins
    RowLayout {
        GroupBox {
            title: qsTr("Light")
            Layout.fillHeight: true
            RowLayout {
                GroupBox {
                    title: qsTr("Color")
                    Layout.fillHeight: true
                    ColorDialog {
                        id: lightColorDialog
                        property color previous
                        onColorChanged: scene.light.color = color
                        onAccepted: scene.light.color = currentColor
                        onRejected: scene.light.color = previous
                    }
                    ColumnLayout {
                        Rectangle {
                            id: lightColorPreview
                            color: scene.light.color
                            Layout.alignment: Qt.AlignCenter
                            width: 50
                            height: 50
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    lightColorDialog.previous = scene.light.color
                                    lightColorDialog.color = scene.light.color
                                    lightColorDialog.open()
                                }
                            }
                        }
                        Binding {
                            target: scene.light
                            property: "color"
                            value: lightColorPreview.color
                        }
                        Button {
                            id: openDialogButton
                            text: qsTr("Open Dialog")
                            onClicked: {
                                lightColorDialog.previous = scene.light.color
                                lightColorDialog.color = scene.light.color
                                lightColorDialog.open()
                            }
                        }
                    }
                }
                AxesSpinBox {
                    id: lightDirectionAxesSpinBox
                    title: qsTr("Direction")
                    Layout.fillHeight: true
                    maximumValue: 1.0
                    minimumValue: -1.0
                    decimals: 3
                    stepSize: 0.01
                    value: scene.light.direction
                }
                Binding {
                    target: scene.light
                    property: "direction"
                    value: lightDirectionAxesSpinBox.value
                    when: lightDirectionAxesSpinBox.hovered
                }
                ColumnLayout {
                    Button {
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Register")
                        onClicked: {
                            var light = scene.light
                            light.motion.addKeyframe(light, timeline.timeIndex)
                        }
                    }
                    Button {
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Reset")
                        onClicked: resetLightAction.trigger()
                    }
                }
            }
        }
        GroupBox {
            title: qsTr("Shadow")
            Layout.fillHeight: true
            ExclusiveGroup {
                id: shadowGroup
                onCurrentChanged: {
                    if (current === selfShadow) {
                        scene.light.shadowType = VPVM.Light.SelfShadow
                    }
                    else if (current === projectiveShadow) {
                        scene.light.shadowType = VPVM.Light.ProjectiveShadow
                    }
                    else {
                        scene.light.shadowType = VPVM.Light.None
                    }
                }
                Component.onCompleted: {
                    switch (scene.light.shadowType) {
                    case VPVM.Light.SelfShadow:
                        current = selfShadow
                        break;
                    case VPVM.Light.ProjectiveShadow:
                        current = projectiveShadow
                        break;
                    default:
                        current = noneShadow
                        break;
                    }
                }
            }
            ColumnLayout {
                RowLayout {
                    RadioButton {
                        id: noneShadow
                        exclusiveGroup: shadowGroup
                        text: qsTr("None")
                    }
                    RadioButton {
                        id: projectiveShadow
                        exclusiveGroup: shadowGroup
                        text: qsTr("Projective")
                    }
                    RadioButton {
                        id: selfShadow
                        exclusiveGroup: shadowGroup
                        text: qsTr("Self Shadow")
                    }
                }
                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    Label { text: qsTr("Distance") }
                    SpinBox {
                        id: shadowDistanceSpinBox
                        value: scene.light.shadowDistance
                        maximumValue: propertyPanel.maximumPositionValue
                        minimumValue: propertyPanel.minimumPositionValue
                    }
                    Binding {
                        target: scene.light
                        property: "distance"
                        value: shadowDistanceSpinBox.value
                        when: shadowDistanceSpinBox.hovered
                    }
                }
            }
        }
        Item { Layout.fillWidth: true }
    }
}
