#pragma once
// Split-flap character index map for generated PNG assets.
// Use this with LittleFS/SPIFFS file names such as /splitflap/A.png or /splitflap/0.png.
// Tile size: 48x72. Half tile size: 48x36.

static const char SPLITFLAP_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789: ";

inline const char* splitflapFileName(char c) {
  switch (c) {
    case 'A': return "/splitflap/A.png";
    case 'B': return "/splitflap/B.png";
    case 'C': return "/splitflap/C.png";
    case 'D': return "/splitflap/D.png";
    case 'E': return "/splitflap/E.png";
    case 'F': return "/splitflap/F.png";
    case 'G': return "/splitflap/G.png";
    case 'H': return "/splitflap/H.png";
    case 'I': return "/splitflap/I.png";
    case 'J': return "/splitflap/J.png";
    case 'K': return "/splitflap/K.png";
    case 'L': return "/splitflap/L.png";
    case 'M': return "/splitflap/M.png";
    case 'N': return "/splitflap/N.png";
    case 'O': return "/splitflap/O.png";
    case 'P': return "/splitflap/P.png";
    case 'Q': return "/splitflap/Q.png";
    case 'R': return "/splitflap/R.png";
    case 'S': return "/splitflap/S.png";
    case 'T': return "/splitflap/T.png";
    case 'U': return "/splitflap/U.png";
    case 'V': return "/splitflap/V.png";
    case 'W': return "/splitflap/W.png";
    case 'X': return "/splitflap/X.png";
    case 'Y': return "/splitflap/Y.png";
    case 'Z': return "/splitflap/Z.png";
    case '0': return "/splitflap/0.png";
    case '1': return "/splitflap/1.png";
    case '2': return "/splitflap/2.png";
    case '3': return "/splitflap/3.png";
    case '4': return "/splitflap/4.png";
    case '5': return "/splitflap/5.png";
    case '6': return "/splitflap/6.png";
    case '7': return "/splitflap/7.png";
    case '8': return "/splitflap/8.png";
    case '9': return "/splitflap/9.png";
    case ':': return "/splitflap/COLON.png";
    case ' ': return "/splitflap/BLANK.png";
    default: return "/splitflap/BLANK.png";
  }
}
