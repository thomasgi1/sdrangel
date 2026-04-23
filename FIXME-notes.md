## meshtasticdemodwebapiadapter.cpp:35
Initializes SWGChirpChatDemodSettings instead of SWGMeshtasticDemodSettings.
Copy-paste artifact from ChirpChat plugin. Fix separately in its own commit.
Branch: feature-meshtastic-more-udp or open a dedicated bug-fix branch.

## meshtasticdemodgui.cpp - checkbox signal pattern
All checkboxes currently use stateChanged(int) pattern.
Consider refactoring all to clicked(bool) for simplicity in a future cleanup commit.
Affects: sendViaUDP, sendJsonViaUDP, and likely other checkboxes in the same file.
