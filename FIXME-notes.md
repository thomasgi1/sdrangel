## meshtasticdemodwebapiadapter.cpp:35
Initializes SWGChirpChatDemodSettings instead of SWGMeshtasticDemodSettings.
Copy-paste artifact from ChirpChat plugin. Fix separately in its own commit.
Branch: feature-meshtastic-more-udp or open a dedicated bug-fix branch.

## meshtasticdemodgui.cpp - checkbox signal pattern
All checkboxes currently use stateChanged(int) pattern.
Consider refactoring all to clicked(bool) for simplicity in a future cleanup commit.
Affects: sendViaUDP, sendJsonViaUDP, and likely other checkboxes in the same file.

## meshtasticdemod.cpp:796 - APRS guard
m_lastMsgBytes[0] is read without an explicit non-empty size check before the APRS detection block.
Pre-existing issue, not introduced by JSON UDP feature. Fix separately in its own commit.
Safe fix: add `&& !m_lastMsgBytes.isEmpty()` to the APRS condition.
