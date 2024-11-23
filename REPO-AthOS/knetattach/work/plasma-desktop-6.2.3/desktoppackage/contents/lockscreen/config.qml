import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcmutils as KCM

Kirigami.FormLayout {
    property alias cfg_alwaysShowClock: alwaysClock.checked
    property alias cfg_showMediaControls: showMediaControls.checked
    property bool cfg_alwaysShowClockDefault: true
    property bool cfg_showMediaControlsDefault: false

    twinFormLayouts: parentLayout

    QQC2.CheckBox {
        id: alwaysClock
        Kirigami.FormData.label: i18ndc("plasma_shell_org.kde.plasma.desktop",
                                        "@title: group",
                                        "Clock:")
        text: i18ndc("plasma_shell_org.kde.plasma.desktop",
                     "@option:check",
                     "Show clock")

        KCM.SettingHighlighter {
            highlight: cfg_alwaysShowClockDefault != cfg_alwaysShowClock
        }
    }

    QQC2.CheckBox {
        id: showMediaControls
        Kirigami.FormData.label: i18ndc("plasma_shell_org.kde.plasma.desktop",
                                        "@title: group",
                                        "Media controls:")
        text: i18ndc("plasma_shell_org.kde.plasma.desktop",
                     "@option:check",
                     "Show under unlocking prompt")

        KCM.SettingHighlighter {
            highlight: cfg_showMediaControlsDefault != cfg_showMediaControls
        }
    }
}
