import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Layouts

Window {
    id: window
    width: 600
    height: 300
    visible: true
    visibility: Window.FullScreen
    property var videos: [
        "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/ForBiggerJoyrides.mp4",
        "https://commondatastorage.googleapis.com/gtv-videos-bucket/sample/ForBiggerFun.mp4"
    ]
    property int currentVideoIndex: 0

    function playNext() {
        console.log("playing next video");
        currentVideoIndex = (currentVideoIndex + 1) % videos.length
        player.source = videos[currentVideoIndex]
    }

    Rectangle {
        anchors.fill: parent
        color: "#686868"

        Video {
            id: player
            source: videos[currentVideoIndex]
            volume: 0
            loops: MediaPlayer.Infinite
            fillMode: VideoOutput.PreserveAspectCrop
            anchors.fill: parent
            onPositionChanged: {
                if (position == duration){
                    console.log("video ended");
                    playNext()
                }
            }
            onSourceChanged: {
                play()
            }
        }
    }

    RowLayout {
        Button {
            text: checked ? "pause" : "play"
            checkable: true
            checked: player.playing
            onClicked: {
                player.playbackState === MediaPlayer.PlayingState ? player.pause() : player.play()
            }
        }

        Button {
            text: checked ? "exit fullscreen" : "enter fullscreen"
            checkable: true
            checked: window.visibility === Window.FullScreen
            onClicked: {
                window.visibility === Window.FullScreen ?
                    window.visibility = Window.Windowed : window.visibility = Window.FullScreen
            }
        }

        Button {
            text: "next"
            onClicked: {
                playNext()
            }
        }
    }

    Connections {
        target: Qt.application
        function onAboutToQuit() {
            console.error("PAUSING ABOUT TO QUIT");
            player.pause()
            player.stop()
        }
    }

    Component.onDestruction: {
        console.error("PAUSING ON DESTRUCTION");
        player.pause()
        player.stop()
    }
}
