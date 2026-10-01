// =============================================================================
//  SPACE INVADERS EXTENDED — C++ / Win32 (GDI)
//  Rozbudowana wersja klasycznej gry Space Invaders
//  Extended version of the classic Space Invaders game
// =============================================================================
//
//  Wersja / Version: 1.1.0
//  Autor / Author:   Maciej Sikorski
//  Data / Date:      01.10.2026
//
//  Licencja / License: Apache License, Version 2.0
//                      http://www.apache.org/licenses/LICENSE-2.0
//  SPDX-License-Identifier: Apache-2.0
//
//  Platforma / Platform:   Windows 10/11 x64 (Win32 API + GDI), bez zależności
//                          zewnętrznych / no external dependencies
//
//  Kompilatory / Compilers (wymagany C++17 / C++17 required):
//    - MSVC — Visual Studio 2022 / 2026 (toolset v143+)  [docelowy; zbudowano / target;
//                                                         built: VS 18, cl 19.51]
//    - MinGW-w64 GCC 13+                                 [kompilacja sprawdzona /
//                                                         compile-verified]
//
//  Kompilacja / Compilation (zalecane / recommended: build.bat):
//    MSVC:   cl /nologo /EHsc /O2 /MT /W3 /std:c++17 /utf-8 space.cpp ^
//              /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib msimg32.lib winmm.lib ^
//              /OUT:Space.exe
//    MinGW:  g++ -std=c++17 -O2 -static -mwindows space.cpp -o Space.exe ^
//              -lgdi32 -luser32 -lmsimg32 -lwinmm
//
//  Kodowanie / Encoding:
//    Plik źródłowy w UTF-8. MSVC wymaga flagi /utf-8 (patrz static_assert niżej).
//    Source file is UTF-8. MSVC requires the /utf-8 flag (see static_assert below).
//
//  Sterowanie / Controls:
//    A/D, ←/→       ruch / move
//    W/S, ↑/↓       nawigacja po menu / menu navigation
//    SPACE, ENTER   strzał, w menu zatwierdź / fire, in menus confirm
//    P, ESC         pauza / pause
//
//  Funkcje / Features:
//    - Klasyczna rozgrywka Space Invaders / Classic Space Invaders gameplay
//    - 5 rodzajów wrogów z animacją 2-klatkową / 5 enemy types with 2-frame animation
//    - Ulepszenia (Triple Shot, +1 Life) / Power-ups (Triple Shot, +1 Life)
//    - Bunkry z niszczalnymi blokami / Bunkers with destructible blocks
//    - Latający spodek UFO z bonusowymi punktami / UFO with bonus points
//    - Efekt screen shake i cząsteczki / Screen shake and particles
//    - Proceduralnie generowane dźwięki / Procedurally generated sounds
//    - Zapis wyniku i stanu gry w %LOCALAPPDATA% / High score and save game in %LOCALAPPDATA%
//    - Menu główne i pauza / Main menu and pause
//    - Interfejs PL / EN (automatyczny wybór + przełącznik w menu)
//      PL / EN interface (auto-detected + switch in the menu)
//
//  Zmiany v1.1.0 / Changes v1.1.0:
//    - Stały krok czasowy 60 Hz (QueryPerformanceCounter) zamiast WM_TIMER,
//      który dawał 32 lub 64 FPS.
//    - Pliki zapisu w %LOCALAPPDATA%; nagłówek (magic + wersja) i walidacja
//      zapisu gry — uszkodzony plik nie powoduje już błędu indeksu.
//    - Naprawiono: gwiazdy rysowane na HUD, auto-repeat klawisza pauzy, klawisze
//      "wciśnięte" po utracie fokusu (gra się teraz pauzuje), rekord bez premii
//      +500 i niezapisywany przy wyjściu, "duchy" przy screen shake, błędny opis
//      ESC, brak polskich znaków, niepełny reset nowej gry, kursor po "Usuń zapis".
//    - Zatrzaskiwanie naciśnięć klawiszy — szybkie tapnięcie nie ginie między klatkami.
//    - Inwazjerzy niszczą bunkry przy zetknięciu; ogień wrogów rośnie od fali 2.
//    - Interfejs dwujęzyczny PL/EN, wersja i kompilator widoczne w menu głównym.
//
//    - Fixed 60 Hz time step (QueryPerformanceCounter) instead of WM_TIMER,
//      which gave 32 or 64 FPS.
//    - Save files in %LOCALAPPDATA%; header (magic + version) and validation of
//      the save game — a corrupted file no longer causes an out-of-range index.
//    - Fixed: stars drawn over the HUD, pause key auto-repeat, keys stuck after
//      focus loss (the game now pauses), high score missing the +500 bonus and
//      not saved on exit, screen-shake "ghosts", wrong ESC help text, missing
//      Polish diacritics, incomplete new-game reset, cursor after "Delete save".
//    - Latched key presses — a quick tap is never lost between frames.
//    - Invaders crush bunkers on contact; enemy fire scales from wave 2.
//    - Bilingual PL/EN interface, version and compiler shown in the main menu.
//
//  Zmiany v1.0.0 / Changes v1.0.0:
//    - Pierwsza wersja / Initial version.
//
// =============================================================================
//  LICENCJA / LICENSE
// =============================================================================
//
//  Copyright 2026 Maciej Sikorski
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// =============================================================================

// Makro może być już zdefiniowane flagą /D w build.bat (unika ostrzeżenia C4005)
// The macro may already be defined by /D in build.bat (avoids warning C4005)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Kontrola kodowania: napisy muszą być w UTF-8 (MSVC: /utf-8)
// Encoding guard: string literals must be UTF-8 (MSVC: /utf-8)
static_assert(sizeof("ł") == 3, "Compile as UTF-8 (MSVC: /utf-8)");

// Biblioteki linkowane automatycznie przez MSVC (MinGW: patrz flagi -l w nagłówku)
// Libraries auto-linked by MSVC (MinGW: see the -l flags in the header)
#ifdef _MSC_VER
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"winmm.lib")
#endif

// =============================================================================
//  Stałe globalne — wymiary, kolory, parametry gry
//  Global constants — dimensions, colors, game parameters
// =============================================================================
static const int W      = 800;                  // szerokość obszaru gry / game area width
static const int H      = 520;                  // wysokość obszaru gry / game area height
static const int TOP_H  = 74;                   // górny panel HUD / top HUD panel
static const int BOT_H  = 22;                   // dolny pasek pomocy / bottom help bar
static const int WIN_W  = W;
static const int WIN_H  = TOP_H + H + BOT_H;    // 616

static const int PW = 40;                       // szerokość gracza / player width
static const int PH = 22;                       // wysokość gracza / player height
static const int INVADER_ROWS = 5;
static const int INVADER_COLS = 11;
static const int SCORE_ROW[5] = {30,20,20,10,10}; // punkty za wiersz / points per row
static const int MAX_LIVES    = 5;                // maks. liczba żyć / max lives

static const double PI = 3.14159265358979323846;

// -----------------------------------------------------------------------------
// Informacje o aplikacji / Application info
// -----------------------------------------------------------------------------
static const char APP_NAME[]     = "SPACE INVADERS EXTENDED";
static const char APP_VERSION[]  = "1.1.0";
static const char APP_DATA_DIR[] = "SpaceInvadersExtended";   // %LOCALAPPDATA%\<dir>

// Wersja gry + kompilator użyty do zbudowania (wyświetlane w menu głównym)
// Game version + compiler used to build it (shown in the main menu)
static std::string buildInfo(){
    char b[96];
#if defined(__clang__)
    snprintf(b,sizeof(b),"v%s | Clang %d.%d.%d",APP_VERSION,__clang_major__,__clang_minor__,__clang_patchlevel__);
#elif defined(_MSC_VER)
    snprintf(b,sizeof(b),"v%s | MSVC %d.%02d.%05d",APP_VERSION,
             _MSC_FULL_VER/10000000,(_MSC_FULL_VER/100000)%100,_MSC_FULL_VER%100000);
#elif defined(__GNUC__)
    snprintf(b,sizeof(b),"v%s | GCC %d.%d.%d",APP_VERSION,__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__);
#else
    snprintf(b,sizeof(b),"v%s",APP_VERSION);
#endif
    return b;
}

// =============================================================================
//  Lokalizacja — język interfejsu (EN / PL)
//  Localization — UI language (EN / PL)
//
//  tr(en, pl) zwraca napis w aktualnym języku. Domyślny język pochodzi z
//  ustawień systemu Windows, można go zmienić w menu głównym.
//  tr(en, pl) returns the string in the current language. The default comes
//  from the Windows UI language and can be switched in the main menu.
// =============================================================================
enum { UILANG_EN = 0, UILANG_PL = 1 };
static int g_lang = UILANG_EN;
static const char* tr(const char* en, const char* pl){ return g_lang == UILANG_PL ? pl : en; }

// -----------------------------------------------------------------------------
// Paleta kolorów gry / Game color palette
// -----------------------------------------------------------------------------
static const COLORREF
    C_bg        = RGB(0x02,0x08,0x06),          // tło kosmosu / space background
    C_white     = RGB(0xff,0xff,0xff),
    C_green     = RGB(0x33,0xff,0x57),          // główny zielony / primary green
    C_green2    = RGB(0x88,0xff,0xaa),
    C_green3    = RGB(0x11,0x66,0x22),
    C_amber     = RGB(0xff,0xb3,0x00),          // bursztynowy / amber
    C_red       = RGB(0xff,0x55,0x55),
    C_red2      = RGB(0xcc,0x33,0x33),
    C_grey      = RGB(0xcc,0xcc,0xdd),
    C_grey2     = RGB(0x77,0x77,0x99),
    C_panel     = RGB(0x04,0x14,0x08),          // tło paneli / panel background
    C_border    = RGB(0x1b,0x7a,0x2b),
    C_border2   = RGB(0x0d,0x3a,0x15),
    C_gold      = RGB(0xff,0xee,0x55),
    C_cyan      = RGB(0x33,0xdd,0xff);

// =============================================================================
//  Zasoby GDI — podwójne buforowanie
//  GDI resources — double buffering
// =============================================================================
static HWND    g_hwnd   = 0;
static HDC     g_memDC  = 0;                    // kontekst pamięci / memory DC
static HBITMAP g_memBmp = 0;                    // bitmapa bufora / backbuffer bitmap
static HBITMAP g_memOld = 0;

static HFONT fTitle=0, fBig=0, fMed=0, fSm=0, fXs=0, fXx=0;

// -----------------------------------------------------------------------------
// Przyciemnienie koloru o współczynnik f (0.0–1.0)
// Dim a color by factor f (0.0–1.0)
// -----------------------------------------------------------------------------
static COLORREF dim(COLORREF c, double f){
    if(f<0) f=0;
    if(f>1) f=1;
    return RGB((int)(GetRValue(c)*f),(int)(GetGValue(c)*f),(int)(GetBValue(c)*f));
}

// -----------------------------------------------------------------------------
// Wypełnienie prostokąta kolorem / Fill a rectangle with a color
// -----------------------------------------------------------------------------
static void fillRect(double x,double y,double w,double h,COLORREF c){
    RECT r={(LONG)floor(x),(LONG)floor(y),(LONG)ceil(x+w),(LONG)ceil(y+h)};
    SetDCBrushColor(g_memDC,c);
    FillRect(g_memDC,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}

// -----------------------------------------------------------------------------
// Obrys prostokąta / Rectangle outline
// -----------------------------------------------------------------------------
static void strokeRect(double x,double y,double w,double h,COLORREF c){
    RECT r={(LONG)floor(x),(LONG)floor(y),(LONG)ceil(x+w),(LONG)ceil(y+h)};
    SetDCBrushColor(g_memDC,c);
    FrameRect(g_memDC,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}

// -----------------------------------------------------------------------------
// Linia pozioma / Horizontal line
// -----------------------------------------------------------------------------
static void hline(double y,double x1,double x2,COLORREF c){
    HGDIOBJ op=SelectObject(g_memDC,GetStockObject(DC_PEN));
    SetDCPenColor(g_memDC,c);
    MoveToEx(g_memDC,(int)x1,(int)y,NULL);
    LineTo  (g_memDC,(int)x2,(int)y);
    SelectObject(g_memDC,op);
}

// -----------------------------------------------------------------------------
// Półprzezroczysty prostokąt — efekt nakładki / Semi-transparent rectangle overlay
// Wykorzystuje AlphaBlend na 1x1 bitmapie DIB rozciąganej na obszar
// Uses AlphaBlend on a 1x1 DIB bitmap stretched over the area
// -----------------------------------------------------------------------------
static void blendRect(int x,int y,int w,int h,COLORREF col,int alpha){
    HDC mem=CreateCompatibleDC(g_memDC);
    BITMAPINFO bi={};
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth=1;
    bi.bmiHeader.biHeight=-1;
    bi.bmiHeader.biPlanes=1;
    bi.bmiHeader.biBitCount=32;
    bi.bmiHeader.biCompression=BI_RGB;
    void* bits=0;
    HBITMAP bm=CreateDIBSection(g_memDC,&bi,DIB_RGB_COLORS,&bits,0,0);
    if(!bm){ DeleteDC(mem); return; }
    int r=GetRValue(col)*alpha/255;
    int g=GetGValue(col)*alpha/255;
    int b=GetBValue(col)*alpha/255;
    ((DWORD*)bits)[0]=((DWORD)alpha<<24)|((DWORD)b<<16)|((DWORD)g<<8)|(DWORD)r;
    HGDIOBJ old=SelectObject(mem,bm);
    BLENDFUNCTION bf={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
    AlphaBlend(g_memDC,x,y,w,h,mem,0,0,1,1,bf);
    SelectObject(mem,old);
    DeleteObject(bm);
    DeleteDC(mem);
}

// -----------------------------------------------------------------------------
// Wyrównanie tekstu / Text alignment
// -----------------------------------------------------------------------------
enum { AL_LEFT=0, AL_CENTER=1, AL_RIGHT=2 };

// -----------------------------------------------------------------------------
// Konwersja UTF-8 → UTF-16 (napisy w programie są w UTF-8)
// UTF-8 → UTF-16 conversion (all program strings are UTF-8)
// -----------------------------------------------------------------------------
static std::wstring toWide(const std::string& s){
    if(s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    if(n <= 0) return std::wstring();
    std::wstring w((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

// -----------------------------------------------------------------------------
// Rysowanie tekstu z opcjonalnym efektem poświaty (Unicode / polskie znaki)
// Draw text with optional glow effect (Unicode / Polish diacritics)
// -----------------------------------------------------------------------------
static void txt(const std::string& s,int x,int y,HFONT f,COLORREF col,int align=AL_LEFT,bool glow=false){
    const std::wstring w = toWide(s);
    const int n = (int)w.size();
    SelectObject(g_memDC,f);
    SetBkMode(g_memDC,TRANSPARENT);
    UINT a=TA_TOP;
    if(align==AL_CENTER) a|=TA_CENTER;
    else if(align==AL_RIGHT) a|=TA_RIGHT;
    SetTextAlign(g_memDC,a);
    if(glow){
        COLORREF gg=dim(col,0.40);
        SetTextColor(g_memDC,gg);
        TextOutW(g_memDC,x-1,y,w.c_str(),n);
        TextOutW(g_memDC,x+1,y,w.c_str(),n);
        TextOutW(g_memDC,x,y-1,w.c_str(),n);
        TextOutW(g_memDC,x,y+1,w.c_str(),n);
    }
    SetTextColor(g_memDC,col);
    TextOutW(g_memDC,x,y,w.c_str(),n);
}
static void txt(const char* s,int x,int y,HFONT f,COLORREF col,int align=AL_LEFT,bool glow=false){
    txt(std::string(s),x,y,f,col,align,glow);
}

// -----------------------------------------------------------------------------
// Rysowanie panelu z podwójną ramką / Draw a panel with double border
// -----------------------------------------------------------------------------
static void panel(int bx,int by,int bw,int bh){
    fillRect(bx,by,bw,bh,C_panel);
    strokeRect(bx,by,bw,bh,C_border);
    strokeRect(bx+4,by+4,bw-8,bh-8,C_border2);
}

// -----------------------------------------------------------------------------
// Pomocnicze funkcje czasu i losowości
// Time and randomness helpers
// -----------------------------------------------------------------------------
static double nowMs(){ return (double)GetTickCount64(); }
static double rnd01(){ return (double)rand()/(double)RAND_MAX; }
static int    rndi(int n){ return n>0 ? rand()%n : 0; }

// -----------------------------------------------------------------------------
// Liczba z zerami wiodącymi / Number with leading zeros
// -----------------------------------------------------------------------------
static std::string padNum(long long v,int n){
    char b[32]; snprintf(b,sizeof(b),"%lld",v); std::string s=b;
    while((int)s.size()<n) s="0"+s;
    return s;
}

// =============================================================================
//  Audio — proceduralnie generowane dźwięki WAV
//  Audio — procedurally generated WAV sounds
//
//  Zamiast dołączać pliki .wav, generujemy je w pamięci przy starcie.
//  Instead of embedding .wav files, we generate them in memory at startup.
//  PlaySound z flagą SND_MEMORY odtwarza je bezpośrednio z bufora.
//  PlaySound with SND_MEMORY plays them directly from buffer.
// =============================================================================
static bool g_soundEnabled = true;
static std::vector<BYTE> g_wavShoot, g_wavExplode, g_wavInvader, g_wavUfo, g_wavPowerup;

// -----------------------------------------------------------------------------
// Buduje nagłówek WAV + dane PCM z wektora sampli 16-bit mono
// Builds a WAV header + PCM data from a vector of 16-bit mono samples
// -----------------------------------------------------------------------------
static std::vector<BYTE> generateWav(const std::vector<short>& samples, int sampleRate = 22050) {
    DWORD dataSize = (DWORD)(samples.size() * sizeof(short));
    DWORD chunkSize = 36 + dataSize;
    std::vector<BYTE> wav(44 + dataSize);
    BYTE* p = wav.data();

    memcpy(p, "RIFF", 4); p += 4;
    memcpy(p, &chunkSize, 4); p += 4;
    memcpy(p, "WAVEfmt ", 8); p += 8;
    DWORD subchunk1Size = 16; memcpy(p, &subchunk1Size, 4); p += 4;
    WORD format = 1; memcpy(p, &format, 2); p += 2;
    WORD channels = 1; memcpy(p, &channels, 2); p += 2;
    DWORD sr = sampleRate; memcpy(p, &sr, 4); p += 4;
    DWORD byteRate = sampleRate * 2; memcpy(p, &byteRate, 4); p += 4;
    WORD blockAlign = 2; memcpy(p, &blockAlign, 2); p += 2;
    WORD bitsPerSample = 16; memcpy(p, &bitsPerSample, 2); p += 2;
    memcpy(p, "data", 4); p += 4;
    memcpy(p, &dataSize, 4); p += 4;

    memcpy(p, samples.data(), dataSize);
    return wav;
}

// -----------------------------------------------------------------------------
// Generuje wszystkie efekty dźwiękowe używane w grze
// Generates all sound effects used in the game
// -----------------------------------------------------------------------------
static void initAudio() {
    // --- STRZAŁ: szybki opadający świergot / SHOOT: fast descending chirp ---
    {
        std::vector<short> s((int)(22050 * 0.10));
        for (size_t i = 0; i < s.size(); ++i) {
            double t = (double)i / 22050.0;
            double freq = 900.0 - t * 6000.0; if (freq < 120) freq = 120;
            double env = 1.0 - (t / 0.10);
            s[i] = (short)(sin(2.0 * PI * freq * t) * 12000.0 * env);
        }
        g_wavShoot = generateWav(s);
    }
    // --- EKSPLOZJA: biały szum z kwadratową obwiednią / EXPLOSION: white noise with squared envelope ---
    {
        std::vector<short> s((int)(22050 * 0.20));
        for (size_t i = 0; i < s.size(); ++i) {
            double t = (double)i / 22050.0;
            double env = 1.0 - (t / 0.20);
            double noise = ((rand() % 2000) - 1000) / 1000.0;
            s[i] = (short)(noise * 15000.0 * env * env);
        }
        g_wavExplode = generateWav(s);
    }
    // --- KROK INWAZJERA: fala prostokątna 130 Hz / INVADER STEP: 130 Hz square wave ---
    {
        std::vector<short> s((int)(22050 * 0.05));
        for (size_t i = 0; i < s.size(); ++i) {
            double t = (double)i / 22050.0;
            double env = 1.0 - (t / 0.05);
            double wave = (fmod(t * 130.0, 1.0) > 0.5) ? 1.0 : -1.0;
            s[i] = (short)(wave * 9000.0 * env);
        }
        g_wavInvader = generateWav(s);
    }
    // --- UFO: sinusoidalny świergot z modulacją / UFO: sine chirp with modulation ---
    {
        std::vector<short> s((int)(22050 * 0.15));
        for (size_t i = 0; i < s.size(); ++i) {
            double t = (double)i / 22050.0;
            double freq = 450.0 + sin(2.0 * PI * 18.0 * t) * 180.0;
            s[i] = (short)(sin(2.0 * PI * freq * t) * 10000.0);
        }
        g_wavUfo = generateWav(s);
    }
    // --- POWER-UP: narastający świergot / POWER-UP: rising chirp ---
    {
        std::vector<short> s((int)(22050 * 0.25));
        for (size_t i = 0; i < s.size(); ++i) {
            double t = (double)i / 22050.0;
            double freq = 400.0 + t * 1800.0;
            double env = 1.0 - (t / 0.25);
            s[i] = (short)(sin(2.0 * PI * freq * t) * 12000.0 * env);
        }
        g_wavPowerup = generateWav(s);
    }
}

// -----------------------------------------------------------------------------
// Odtworzenie dźwięku z pamięci (asynchronicznie)
// Play sound from memory (asynchronously)
// -----------------------------------------------------------------------------
static void playSnd(const std::vector<BYTE>& wav) {
    if (!g_soundEnabled || wav.empty()) return;
    PlaySoundA((LPCSTR)wav.data(), NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

// =============================================================================
//  Sprite'y — wzorce bitowe 12x8 dla inwazjerów
//  Sprites — 12x8 bit patterns for invaders
//
//  Każdy sprite to tablica 96 elementów (12 kolumn × 8 wierszy).
//  Wartość 1 = wypełniony piksel, 0 = pusty.
//  Each sprite is a 96-element array (12 cols × 8 rows).
//  1 = filled pixel, 0 = empty.
// =============================================================================
static const int SQUID_A[96]={0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,1,1,0,0,0};
static const int SQUID_B[96]={0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,1,0};
static const int CRAB_A[96] ={0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1,1,0,0,1,1,0,0,0};
static const int CRAB_B[96] ={0,0,1,0,0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,1,1,0,0,1,0,0};
static const int OCTO_A[96] ={0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,1,0,1,0};
static const int OCTO_B[96] ={0,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0};
// Indeks: [0]=squid A, [1]=squid B, [2]=crab A, [3]=crab B, [4]=octo A, [5]=octo B
static const int* SPR_PAT[6]={SQUID_A,SQUID_B,CRAB_A,CRAB_B,OCTO_A,OCTO_B};

// -----------------------------------------------------------------------------
// Rysowanie sprite'a pikselowego / Draw a pixel-art sprite
// -----------------------------------------------------------------------------
static void drawPixelArt(const int* pat,int cols,int rows,double ox,double oy,COLORREF col,int sc){
    for(int r=0;r<rows;r++)for(int c=0;c<cols;c++)
        if(pat[r*cols+c])
            fillRect(ox+c*sc, oy+r*sc, sc, sc, col);
}

// =============================================================================
//  Struktury danych i stan gry
//  Data structures and game state
// =============================================================================
enum GameMode { MODE_MAIN=0, MODE_PLAY, MODE_PAUSE, MODE_OVER };
enum PowerType { PWR_TRIPLE=0, PWR_LIFE };

struct Player  { double x, y; int lives; int cooldown; int flashTimer; };
struct Bullet  { double x, y, vx, vy, w, h; };
struct Invader { double x, y; int row, col; bool alive; int frame; };
struct BunkerBlock { double x, y, w, h; int hp; };
struct UFO     { bool active; double x, y, w, h, speed; int spawnCounter, spawnDelay; };
struct Popup   { double x, y; std::string text; int t; COLORREF color; };
struct Particle{ double x, y, vx, vy; int life, maxLife; COLORREF color; };
struct PowerUp { double x, y, vy; PowerType type; bool active; };

static GameMode g_mode = MODE_MAIN;
static Player g_player = { W/2.0-PW/2.0, (double)(H-50), 3, 0, 0 };
static std::vector<Bullet> g_playerBullets, g_enemyBullets;
static std::vector<Invader> g_invaders;
static std::vector<std::vector<BunkerBlock>> g_bunkers;  // 4 bunkry / 4 bunkers
static UFO g_ufo = { false, 0, 38, 44, 18, 2, 0, 420 };
static std::vector<Popup> g_popups;
static std::vector<Particle> g_particles;
static std::vector<PowerUp> g_powerups;

static int g_shakeTimer = 0;            // licznik drgań ekranu / screen shake counter
static int g_screenFlashTimer = 0;      // czerwony błysk przy trafieniu / red flash on hit
static int g_tripleShotTimer = 0;       // pozostały czas triple shot / remaining triple shot time

static int    g_totalScore = 0;
static int    g_currentWave = 1;
static int    g_invDir = 1;             // kierunek ruchu inwazjerów / invader movement direction
static bool   g_invStepDown = false;    // czy następny krok to zejście / next step is descent?
static int    g_moveTimer = 0;
static int    g_moveDelay = 30;         // opóźnienie między krokami / delay between steps
static int    g_invFrame = 0;           // klatka animacji / animation frame
static int    g_shootTimer = 0;
static double g_enemyBulletSpeed = 3.5;
static int    g_ufoIdx = 0;             // indeks do tablicy punktów UFO / UFO points index

// Konfiguracja trudności / Difficulty configuration
static int g_cfgDiff  = 1;              // 0=EASY 1=NORMAL 2=HARD 3=INSANE
static int g_cfgLives = 2;              // indeks do LIVES_OPTS / index into LIVES_OPTS

struct DiffCfg { double speedMult; double shootProb; double waveStep; int ufoDelay; };
static const DiffCfg DIFF_CFG[4] = {
    {0.6, 0.008, 0.05, 500},
    {1.0, 0.012, 0.07, 420},
    {1.4, 0.018, 0.09, 300},
    {2.0, 0.028, 0.12, 200},
};
static const char* DIFFS_EN[4] = {"EASY","NORMAL","HARD","INSANE"};
static const char* DIFFS_PL[4] = {"ŁATWY","NORMALNY","TRUDNY","SZALONY"};
static const char* diffName(int d){
    if(d < 0 || d > 3) d = 1;
    return tr(DIFFS_EN[d], DIFFS_PL[d]);
}
static const int   LIVES_OPTS[4] = {1,2,3,5};

static int g_hiScore = 0;

// -----------------------------------------------------------------------------
// Struktura zapisu stanu gry / Save game state structure
// -----------------------------------------------------------------------------
struct SaveData { bool has=false; int score=0, wave=1, lives=3, diff=1; };
static SaveData g_save;

// -----------------------------------------------------------------------------
// Krótkotrwały komunikat na dole ekranu / Short-lived on-screen message
// -----------------------------------------------------------------------------
static std::string g_flashMsg;
static int g_flashT = 0;
static void flashMsg(const std::string& m, int tt){ g_flashMsg=m; g_flashT=tt; }

// -----------------------------------------------------------------------------
// Gwiazdy tła / Background stars
// -----------------------------------------------------------------------------
struct Star { double x, y, r, b; };
static Star g_stars[120];

// -----------------------------------------------------------------------------
// Kolor aktywnej fali (zmienia się co falę) / Active wave color (changes per wave)
// -----------------------------------------------------------------------------
static COLORREF waveColor(){
    static const COLORREF cols[5]={
        RGB(0x33,0xff,0x57),RGB(0x44,0xcc,0xff),RGB(0xff,0x55,0x55),
        RGB(0xff,0xdd,0x44),RGB(0xcc,0x77,0xff)};
    return cols[(g_currentWave-1)%5];
}

// =============================================================================
//  Zapis / odczyt danych na dysku
//  Save / load data to disk
// =============================================================================
// -----------------------------------------------------------------------------
// Ścieżka pliku danych: %LOCALAPPDATA%\SpaceInvadersExtended\<name>
// Gdy katalog jest niedostępny — bieżący katalog roboczy.
// Data file path: %LOCALAPPDATA%\SpaceInvadersExtended\<name>
// Falls back to the current working directory if the folder is unavailable.
// -----------------------------------------------------------------------------
static std::string dataPath(const char* name){
    static bool ready = false;
    static std::string dir;
    if(!ready){
        ready = true;
        char base[MAX_PATH];
        DWORD n = GetEnvironmentVariableA("LOCALAPPDATA", base, MAX_PATH);
        if(n > 0 && n < MAX_PATH){
            std::string d = std::string(base) + "\\" + APP_DATA_DIR;
            CreateDirectoryA(d.c_str(), NULL);      // błąd „już istnieje" jest OK / "already exists" is fine
            DWORD attr = GetFileAttributesA(d.c_str());
            if(attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY))
                dir = d + "\\";
        }
    }
    return dir + name;
}

// -----------------------------------------------------------------------------
// Format pliku zapisu gry: nagłówek (magic + wersja) + dane, wszystko walidowane
// Save file format: header (magic + version) + data, everything is validated
// -----------------------------------------------------------------------------
static const char SAVE_MAGIC[4] = {'S','I','E','X'};
static const int  SAVE_VERSION  = 1;
struct SaveFile { char magic[4]; int version; int score, wave, lives, diff; };
static_assert(sizeof(SaveFile) == 24, "SaveFile must have no padding (binary file format)");

// Rekord / High score
static void saveHighScoreToFile() {
    FILE* f = fopen(dataPath("highscore.dat").c_str(), "wb");
    if (!f) return;
    fwrite(&g_hiScore, sizeof(g_hiScore), 1, f);
    fclose(f);
}

static void loadHighScoreFromFile() {
    FILE* f = fopen(dataPath("highscore.dat").c_str(), "rb");
    if (!f) return;
    int v = 0;
    if (fread(&v, sizeof(v), 1, f) == 1 && v >= 0) g_hiScore = v;
    fclose(f);
}

// Zapisz rekord, jeśli bieżący wynik jest wyższy / Save the high score if beaten
static void commitHiScore() {
    if (g_totalScore > g_hiScore) {
        g_hiScore = g_totalScore;
        saveHighScoreToFile();
    }
}

// Zapis gry; zwraca true przy sukcesie / Save game; returns true on success
static bool saveGameToFile() {
    SaveFile s;
    memcpy(s.magic, SAVE_MAGIC, sizeof(s.magic));
    s.version = SAVE_VERSION;
    s.score = g_save.score;
    s.wave  = g_save.wave;
    s.lives = g_save.lives;
    s.diff  = g_save.diff;
    FILE* f = fopen(dataPath("savegame.dat").c_str(), "wb");
    if (!f) return false;
    bool ok = (fwrite(&s, sizeof(s), 1, f) == 1);
    if (fclose(f) != 0) ok = false;
    return ok;
}

// Wczytanie zapisu; uszkodzony / nieznany plik jest ignorowany
// Load a save; a corrupted / unknown file is ignored
static void loadSaveFromFile() {
    g_save = SaveData();
    FILE* f = fopen(dataPath("savegame.dat").c_str(), "rb");
    if (!f) return;
    SaveFile s;
    bool ok = (fread(&s, sizeof(s), 1, f) == 1);
    fclose(f);
    if (!ok || memcmp(s.magic, SAVE_MAGIC, sizeof(s.magic)) != 0 || s.version != SAVE_VERSION) return;
    if (s.diff < 0 || s.diff > 3)          return;     // indeks do DIFF_CFG / index into DIFF_CFG
    if (s.wave < 1 || s.wave > 9999)       return;
    if (s.lives < 1 || s.lives > MAX_LIVES) return;
    if (s.score < 0)                       return;
    g_save.has = true;
    g_save.score = s.score; g_save.wave = s.wave;
    g_save.lives = s.lives; g_save.diff = s.diff;
}

static void deleteSaveFile() {
    g_save.has = false;
    remove(dataPath("savegame.dat").c_str());
}

// =============================================================================
//  Efekty cząsteczkowe / Particle effects
// =============================================================================
static void spawnExplosion(double x, double y, COLORREF color, int count = 16) {
    for(int i=0; i<count; ++i) {
        Particle p;
        p.x = x; p.y = y;
        double angle = rnd01() * 2.0 * PI;
        double speed = rnd01() * 4.0 + 0.8;
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed - 1.2;
        p.life = rndi(25) + 15;
        p.maxLife = p.life;
        p.color = color;
        g_particles.push_back(p);
    }
}

static void updateParticles() {
    for(int i=(int)g_particles.size()-1; i>=0; --i) {
        Particle& p = g_particles[i];
        p.x += p.vx;
        p.y += p.vy;
        p.vy += 0.08;   // grawitacja / gravity
        p.life--;
        if(p.life <= 0) g_particles.erase(g_particles.begin() + i);
    }
}

static void updatePopups() {
    for(int i=(int)g_popups.size()-1; i>=0; --i) {
        g_popups[i].t--;
        if(g_popups[i].t <= 0) {
            g_popups.erase(g_popups.begin() + i);
        }
    }
}

// =============================================================================
//  Logika gry — tworzenie obiektów
//  Game logic — object creation
// =============================================================================

// -----------------------------------------------------------------------------
// Tworzy siatkę inwazjerów 5x11 / Creates a 5x11 invader grid
// -----------------------------------------------------------------------------
static void createInvaders(){
    g_invaders.clear();
    for(int r=0;r<INVADER_ROWS;r++)
        for(int c=0;c<INVADER_COLS;c++){
            Invader inv;
            inv.x = 55 + c*62;
            inv.y = 55 + r*44;
            inv.row = r;
            inv.col = c;
            inv.alive = true;
            inv.frame = 0;
            g_invaders.push_back(inv);
        }
}

// -----------------------------------------------------------------------------
// Tworzy 4 bunkry z bloków 6x6 px / Creates 4 bunkers from 6x6 px blocks
// Kształt bunkru: łuk z wyciętym otworem strzelniczym
// Bunker shape: arch with a cut-out firing slot
// -----------------------------------------------------------------------------
static void createBunkers(){
    g_bunkers.clear();
    const int xp[4] = {100,290,480,670};    // pozycje X bunkrów / bunker X positions
    const int cs=6, bw=16, bh=8;            // rozmiar bloku, szerokość, wysokość / block, width, height
    for(int b=0;b<4;b++){
        std::vector<BunkerBlock> bl;
        double bx = xp[b] - (bw*cs)/2.0;
        double by = H - 110.0;
        for(int r=0;r<bh;r++)for(int c=0;c<bw;c++){
            bool ok=true;
            if(r==0 && (c<3 || c>bw-4)) ok=false;         // górne zaokrąglenie / top rounding
            if(r==1 && (c<2 || c>bw-3)) ok=false;
            if(r==2 && (c<1 || c>bw-2)) ok=false;
            if(r>=bh-2 && (c>=bw/2-3 && c<bw/2+3)) ok=false;  // wycięcie dolne / bottom notch
            if(r<3 && (c>=bw/2-2 && c<bw/2+2)) ok=false;      // otwór strzelniczy / firing slot
            if(ok){
                BunkerBlock bb;
                bb.x=bx + c*cs; bb.y=by + r*cs;
                bb.w=(double)cs; bb.h=(double)cs; bb.hp=3;
                bl.push_back(bb);
            }
        }
        g_bunkers.push_back(bl);
    }
}

// -----------------------------------------------------------------------------
// Inicjalizacja nowej fali — reset wszystkiego oprócz wyniku i żyć
// Initialize a new wave — reset everything except score and lives
// -----------------------------------------------------------------------------
static void initWave(){
    createInvaders();
    createBunkers();
    g_playerBullets.clear();
    g_enemyBullets.clear();
    g_popups.clear();
    g_particles.clear();
    g_powerups.clear();
    g_player.cooldown = 0;
    g_player.flashTimer = 0;
    g_shootTimer = 0;
    g_tripleShotTimer = 0;
    g_invDir = 1;
    g_invStepDown = false;
    g_moveTimer = 0;
    g_ufo.active = false;
    g_ufo.spawnCounter = 0;

    const DiffCfg& cfg = DIFF_CFG[g_cfgDiff];
    g_ufo.spawnDelay = cfg.ufoDelay;
    double md = 30.0 * (1.0 - (g_currentWave-1)*cfg.waveStep);
    if(md < 8) md = 8;
    g_moveDelay = (int)floor(md);
    double ebs = 3.5*cfg.speedMult + (g_currentWave-1)*0.3;
    if(ebs > 10) ebs = 10;
    g_enemyBulletSpeed = ebs;
}

// -----------------------------------------------------------------------------
// Wspólny reset stanu przebiegu (gracz, pociski, efekty, liczniki) + nowa fala
// Common run-state reset (player, bullets, effects, timers) + new wave
// -----------------------------------------------------------------------------
static void resetRun(){
    g_player.x = W/2.0 - PW/2.0;
    g_player.cooldown = 0;
    g_player.flashTimer = 0;
    g_playerBullets.clear();
    g_enemyBullets.clear();
    g_popups.clear();
    g_particles.clear();
    g_powerups.clear();
    g_ufo.active = false;
    g_ufo.spawnCounter = 0;
    g_ufoIdx = 0;
    g_shakeTimer = 0;
    g_screenFlashTimer = 0;
    g_flashT = 0;
    initWave();
}

// -----------------------------------------------------------------------------
// Pełny reset — rozpoczęcie nowej gry / Full reset — start a new game
// -----------------------------------------------------------------------------
static void fullReset(){
    g_totalScore = 0;
    g_currentWave = 1;
    g_player.lives = LIVES_OPTS[g_cfgLives];
    resetRun();
}

// -----------------------------------------------------------------------------
// Wczytanie zapisanej gry z pliku / Load saved game from file
// -----------------------------------------------------------------------------
static void loadSaveData(){
    if(!g_save.has) return;
    g_cfgDiff = g_save.diff;
    g_totalScore = g_save.score;
    g_currentWave = g_save.wave;
    g_player.lives = g_save.lives;
    resetRun();
    g_mode = MODE_PLAY;
}

// =============================================================================
//  Strzał gracza / Player shooting
// =============================================================================
static void playerShoot(){
    if(g_player.cooldown > 0) return;

    if(g_tripleShotTimer > 0) {
        // Potrójny strzał — 3 pociski / Triple shot — 3 bullets
        Bullet b1 = { g_player.x + PW/2.0 - 2, g_player.y - 8,  0.0, -7.0, 4, 14 };
        Bullet b2 = { g_player.x + PW/2.0 - 6, g_player.y - 8, -1.8, -6.5, 4, 14 };
        Bullet b3 = { g_player.x + PW/2.0 + 2, g_player.y - 8,  1.8, -6.5, 4, 14 };
        g_playerBullets.push_back(b1);
        g_playerBullets.push_back(b2);
        g_playerBullets.push_back(b3);
    } else {
        Bullet b = { g_player.x + PW/2.0 - 2, g_player.y - 8, 0.0, -7.0, 4, 14 };
        g_playerBullets.push_back(b);
    }

    g_player.cooldown = 14;
    playSnd(g_wavShoot);
}

// Punkty UFO zależne od indeksu trafienia / UFO points depending on hit index
static const int UFO_PTS[16] = {100,50,50,100,150,100,100,50,300,100,100,100,50,150,100,50};

// =============================================================================
//  Aktualizacja power-upów / Power-up update
// =============================================================================
static void updatePowerups(){
    for(int i=(int)g_powerups.size()-1; i>=0; i--){
        PowerUp& p = g_powerups[i];
        p.y += p.vy;

        // Kolizja z graczem / Collision with player
        if(p.x < g_player.x + PW && p.x + 20 > g_player.x &&
           p.y < g_player.y + PH && p.y + 20 > g_player.y){
            if(p.type == PWR_TRIPLE) {
                g_tripleShotTimer = 360; // ~6 sekund / ~6 seconds
                flashMsg(tr("POWER-UP: TRIPLE SHOTS!", "BONUS: POTRÓJNY STRZAŁ!"), 90);
            } else if(p.type == PWR_LIFE) {
                if(g_player.lives < MAX_LIVES) g_player.lives++;
                flashMsg(tr("POWER-UP: +1 LIFE!", "BONUS: +1 ŻYCIE!"), 90);
            }
            playSnd(g_wavPowerup);
            g_powerups.erase(g_powerups.begin() + i);
            continue;
        }

        if(p.y > H) g_powerups.erase(g_powerups.begin() + i);
    }
}

// =============================================================================
//  Aktualizacja pocisków (gracza i wrogów) z detekcją kolizji
//  Bullets update (player and enemy) with collision detection
// =============================================================================
static void updateBullets(){
    // --- Pociski gracza / Player bullets ---
    for(int i=(int)g_playerBullets.size()-1;i>=0;i--){
        Bullet& b = g_playerBullets[i];
        b.x += b.vx;
        b.y += b.vy;
        bool hit=false;

        // Wyjście za ekran / Off-screen
        if(b.y + b.h < 0 || b.x < 0 || b.x > W){
            g_playerBullets.erase(g_playerBullets.begin()+i); continue;
        }

        // Kolizja z UFO / UFO collision
        if(g_ufo.active && b.x < g_ufo.x+g_ufo.w && b.x+b.w > g_ufo.x &&
           b.y < g_ufo.y+g_ufo.h && b.y+b.h > g_ufo.y){
            int pts = UFO_PTS[g_ufoIdx++ % 16];
            g_totalScore += pts;
            Popup p; p.x=g_ufo.x; p.y=g_ufo.y; p.text="+"+padNum(pts,0);
            p.t=50; p.color=RGB(0xff,0xaa,0x44);
            g_popups.push_back(p);
            spawnExplosion(g_ufo.x + g_ufo.w/2.0, g_ufo.y + g_ufo.h/2.0, C_amber, 24);
            playSnd(g_wavExplode);
            g_shakeTimer = 10;

            // Szansa 50% na drop power-upa / 50% chance for power-up drop
            if(rnd01() < 0.5) {
                PowerUp pw;
                pw.x = g_ufo.x + g_ufo.w/2.0 - 10;
                pw.y = g_ufo.y;
                pw.vy = 2.0;
                pw.type = (rnd01() < 0.7) ? PWR_TRIPLE : PWR_LIFE;
                pw.active = true;
                g_powerups.push_back(pw);
            }

            g_ufo.active = false;
            hit = true;
        }

        // Kolizja z inwazjerem / Invader collision
        if(!hit){
            for(size_t k=0;k<g_invaders.size();k++){
                Invader& inv = g_invaders[k];
                if(inv.alive &&
                   b.x < inv.x+38 && b.x+b.w > inv.x &&
                   b.y < inv.y+24 && b.y+b.h > inv.y){
                    inv.alive = false;
                    int pts = SCORE_ROW[inv.row];
                    g_totalScore += pts;
                    Popup p; p.x=inv.x+19; p.y=inv.y; p.text="+"+padNum(pts,0);
                    p.t=45; p.color=C_green;
                    g_popups.push_back(p);
                    spawnExplosion(inv.x + 19, inv.y + 12, C_green, 16);
                    playSnd(g_wavExplode);
                    hit = true;
                    break;
                }
            }
        }

        // Kolizja z bunkrem / Bunker collision
        if(!hit){
            bool stop=false;
            for(size_t bk=0;bk<g_bunkers.size() && !stop;bk++){
                for(size_t bl=0;bl<g_bunkers[bk].size();bl++){
                    BunkerBlock& bb = g_bunkers[bk][bl];
                    if(bb.hp>0 &&
                       b.x < bb.x+bb.w && b.x+b.w > bb.x &&
                       b.y < bb.y+bb.h && b.y+b.h > bb.y){
                        bb.hp--;
                        spawnExplosion(bb.x, bb.y, RGB(0x33, 0xcc, 0x44), 6);
                        hit = true;
                        stop = true;
                        break;
                    }
                }
            }
        }

        if(hit) g_playerBullets.erase(g_playerBullets.begin()+i);
    }

    // --- Pociski wroga / Enemy bullets ---
    for(int i=(int)g_enemyBullets.size()-1;i>=0;i--){
        Bullet& eb = g_enemyBullets[i];
        eb.y += g_enemyBulletSpeed;
        bool hit=false;

        if(eb.y > H){ g_enemyBullets.erase(g_enemyBullets.begin()+i); continue; }

        // Kolizja z graczem / Collision with player
        if(g_player.flashTimer <= 0) {
            if(eb.x < g_player.x+PW && eb.x+eb.w > g_player.x &&
               eb.y < g_player.y+PH && eb.y+eb.h > g_player.y){
                g_player.lives--;
                g_player.flashTimer = 90;   // nietykalność / invulnerability
                g_shakeTimer = 18;
                g_screenFlashTimer = 8;
                spawnExplosion(g_player.x + PW/2.0, g_player.y + PH/2.0, C_red, 30);
                playSnd(g_wavExplode);
                hit = true;
                if(g_player.lives <= 0){
                    commitHiScore();
                    g_mode = MODE_OVER;
                } else {
                    // Respawn / respawn
                    g_player.x = W/2.0 - PW/2.0;
                    g_playerBullets.clear();
                    g_player.cooldown = 30;
                }
            }
        }

        // Kolizja z bunkrem / Bunker collision
        if(!hit){
            bool stop=false;
            for(size_t bk=0;bk<g_bunkers.size() && !stop;bk++){
                for(size_t bl=0;bl<g_bunkers[bk].size();bl++){
                    BunkerBlock& bb = g_bunkers[bk][bl];
                    if(bb.hp>0 &&
                       eb.x < bb.x+bb.w && eb.x+eb.w > bb.x &&
                       eb.y < bb.y+bb.h && eb.y+eb.h > bb.y){
                        bb.hp--;
                        spawnExplosion(bb.x, bb.y, RGB(0x33, 0xcc, 0x44), 6);
                        hit = true;
                        stop = true;
                        break;
                    }
                }
            }
        }

        if(hit) g_enemyBullets.erase(g_enemyBullets.begin()+i);
    }
}

// =============================================================================
//  Ruch inwazjerów — klasyczny schemat „krok → krawędź → zejście"
//  Invader movement — classic "step → edge → descend" pattern
// =============================================================================
static void moveInvaders(){
    g_moveTimer++;
    if(g_moveTimer < g_moveDelay) return;
    g_moveTimer = 0;
    g_invFrame = 1 - g_invFrame;
    for(size_t i=0;i<g_invaders.size();i++) if(g_invaders[i].alive) g_invaders[i].frame = g_invFrame;

    playSnd(g_wavInvader);

    // Zejście w dół i zmiana kierunku / Descend and reverse direction
    if(g_invStepDown){
        for(size_t i=0;i<g_invaders.size();i++) if(g_invaders[i].alive) g_invaders[i].y += 14;
        g_invDir *= -1;
        g_invStepDown = false;
        return;
    }

    double step = 9.0 * g_invDir;
    bool edge = false;
    for(size_t i=0;i<g_invaders.size();i++){
        Invader& inv = g_invaders[i];
        if(!inv.alive) continue;
        inv.x += step;
        if(inv.x + 38 >= W-10 || inv.x <= 5) edge = true;
    }
    if(edge) g_invStepDown = true;
}

// =============================================================================
//  Inwazjerzy niszczą bunkry, przez które przechodzą
//  Invaders destroy the bunker blocks they overlap
// =============================================================================
static void invadersCrushBunkers(){
    const double bunkerTop = H - 110.0;
    for(size_t i=0;i<g_invaders.size();i++){
        const Invader& inv = g_invaders[i];
        if(!inv.alive || inv.y + 24 < bunkerTop) continue;       // szybkie odrzucenie / early out
        for(size_t bk=0;bk<g_bunkers.size();bk++){
            for(size_t bl=0;bl<g_bunkers[bk].size();bl++){
                BunkerBlock& bb = g_bunkers[bk][bl];
                if(bb.hp>0 &&
                   inv.x < bb.x+bb.w && inv.x+38 > bb.x &&
                   inv.y < bb.y+bb.h && inv.y+24 > bb.y)
                    bb.hp = 0;
            }
        }
    }
}

// =============================================================================
//  Strzał wroga — tylko dolny inwazjer w każdej kolumnie może strzelać
//  Enemy shooting — only the bottom invader in each column can shoot
// =============================================================================
static void enemyShoot(){
    std::vector<int> alive;
    for(size_t i=0;i<g_invaders.size();i++) if(g_invaders[i].alive) alive.push_back((int)i);
    if(alive.empty()) return;

    const DiffCfg& cfg = DIFF_CFG[g_cfgDiff];
    // Prawdopodobieństwo strzału rośnie z falą (wcześniej efekt był widoczny dopiero od fali 9)
    // Shot probability grows with the wave (previously visible only from wave 9)
    double p = cfg.shootProb * 3.0 + (g_currentWave-1)*0.003;
    if(p > 0.20) p = 0.20;
    if(rnd01() > p) return;

    // Znajdź dolnych w każdej kolumnie / Find bottom-most in each column
    std::vector<int> bottom;
    for(size_t a=0;a<alive.size();a++){
        int ia = alive[a];
        bool isBottom = true;
        for(size_t b=0;b<alive.size();b++){
            int ib = alive[b];
            if(g_invaders[ib].col == g_invaders[ia].col &&
               g_invaders[ib].row > g_invaders[ia].row){
                isBottom = false; break;
            }
        }
        if(isBottom) bottom.push_back(ia);
    }
    if(bottom.empty()) bottom = alive;
    int pick = bottom[rndi((int)bottom.size())];
    Invader& s = g_invaders[pick];

    Bullet eb = { s.x + 38/2.0 - 2, s.y + 24, 0.0, 0.0, 4, 10 };
    g_enemyBullets.push_back(eb);
}

// =============================================================================
//  Aktualizacja UFO — pojawia się okresowo i przelatuje przez ekran
//  UFO update — spawns periodically and flies across the screen
// =============================================================================
static void updateUfo(){
    if(!g_ufo.active){
        g_ufo.spawnCounter++;
        if(g_ufo.spawnCounter >= g_ufo.spawnDelay){
            g_ufo.spawnCounter = 0;
            g_ufo.active = true;
            bool fr = rnd01() < 0.5;
            g_ufo.x = fr ? W : -g_ufo.w;    // start z lewej lub prawej / start from left or right
            g_ufo.speed = fr ? -2.2 : 2.2;
            playSnd(g_wavUfo);
        }
    } else {
        g_ufo.x += g_ufo.speed;
        if(g_ufo.x + g_ufo.w < 0 || g_ufo.x > W) g_ufo.active = false;
    }
}

// =============================================================================
//  Sprawdzenie stanu fali — koniec fali, przegrana, przyspieszenie
//  Wave state check — wave clear, loss condition, speed-up
// =============================================================================
static void checkWave(){
    int aliveCount = 0;
    for(size_t i=0;i<g_invaders.size();i++) if(g_invaders[i].alive) aliveCount++;

    // Fala wyczyszczona / Wave cleared
    if(aliveCount == 0){
        g_currentWave++;
        g_totalScore += 500;
        commitHiScore();      // z premią / including the bonus
        flashMsg(tr("WAVE COMPLETED! +500 PTS", "FALA UKOŃCZONA! +500 PKT"), 90);
        initWave();
        return;
    }

    // Dynamiczne przyspieszenie — im mniej wrogów, tym szybciej się ruszają
    // Dynamic speed-up — fewer enemies means faster movement
    const DiffCfg& cfg = DIFF_CFG[g_cfgDiff];
    double baseMd = 30.0 * (1.0 - (g_currentWave - 1) * cfg.waveStep);
    if (baseMd < 8) baseMd = 8;
    double ratio = (double)aliveCount / (double)(INVADER_ROWS * INVADER_COLS);
    int targetDelay = (int)(baseMd * ratio);
    if (targetDelay < 3) targetDelay = 3;
    g_moveDelay = targetDelay;

    // Sprawdź czy wróg dotarł do gracza / Check if invader reached the player
    for(size_t i=0;i<g_invaders.size();i++){
        Invader& inv = g_invaders[i];
        if(inv.alive && inv.y + 24 >= g_player.y + 10){
            commitHiScore();
            g_mode = MODE_OVER;
            return;
        }
    }
}

// =============================================================================
//  Wejście — stan klawiszy i poprzednia klatka dla detekcji zbocza
//  Input — key state and previous frame for edge detection
// =============================================================================
static bool K_LEFT=false, K_RIGHT=false, K_UP=false, K_DOWN=false, K_FIRE=false;
static bool P_LEFT=false, P_RIGHT=false, P_UP=false, P_DOWN=false, P_FIRE=false;
static bool just(bool cur, bool prev){ return cur && !prev; }   // zbocze narastające / rising edge

// Zatrzaśnięte naciśnięcia: krótkie „tapnięcie" (krótsze niż jedna klatka) nie zostanie zgubione.
// Ustawiane w WM_KEYDOWN, czyszczone po obsłużeniu w processInput().
// Latched presses: a quick tap shorter than one frame is never lost.
// Set in WM_KEYDOWN, cleared after being handled in processInput().
static bool Q_LEFT=false, Q_RIGHT=false, Q_UP=false, Q_DOWN=false, Q_FIRE=false;

// =============================================================================
//  Aktualizacja stanu gry w każdej klatce
//  Game state update per frame
// =============================================================================
static void updateGame(){
    if(g_mode != MODE_PLAY) return;
    if(g_player.cooldown > 0) g_player.cooldown--;
    if(g_shootTimer > 0) g_shootTimer--;
    if(g_player.flashTimer > 0) g_player.flashTimer--;
    if(g_tripleShotTimer > 0) g_tripleShotTimer--;
    if(g_shakeTimer > 0) g_shakeTimer--;
    if(g_screenFlashTimer > 0) g_screenFlashTimer--;

    // Ruch gracza / Player movement
    if(K_LEFT  && g_player.x > 5)          g_player.x -= 7;
    if(K_RIGHT && g_player.x + PW < W-5)   g_player.x += 7;

    // Strzał (z ograniczeniem szybkostrzelności) / Shooting (with fire rate limit)
    if(K_FIRE && g_shootTimer == 0){
        playerShoot();
        g_shootTimer = 5;
    }

    moveInvaders();
    invadersCrushBunkers();
    enemyShoot();
    updateUfo();
    updatePowerups();
    updateBullets();
    updateParticles();
    updatePopups();
    checkWave();
}

// =============================================================================
//  Menu główne — wiersze i nawigacja
//  Main menu — rows and navigation
// =============================================================================
struct MenuRow { std::string id; std::string label; bool sel; };
static std::vector<MenuRow> g_mmRows;
static int g_mmCur = 6;      // domyślnie wiersz "start" / default: the "start" row

// -----------------------------------------------------------------------------
// Buduje listę wierszy menu głównego (zależy od obecności zapisu)
// Builds the main menu rows list (depends on save availability)
// -----------------------------------------------------------------------------
static void buildMM(){
    g_mmRows.clear();
    g_mmRows.push_back({"info",  tr("HIGH SCORE",     "REKORD"),           false});
    g_mmRows.push_back({"diff",  tr("DIFFICULTY",     "POZIOM TRUDNOŚCI"), true});
    g_mmRows.push_back({"lives", tr("PLAYER LIVES",   "LICZBA ŻYĆ"),       true});
    g_mmRows.push_back({"sound", tr("SOUND EFFECTS",  "EFEKTY DŹWIĘKOWE"), true});
    g_mmRows.push_back({"lang",  tr("LANGUAGE",       "JĘZYK"),            true});
    g_mmRows.push_back({"sep1",  "",                                       false});
    g_mmRows.push_back({"start", tr("START NEW GAME", "NOWA GRA"),         true});
    if(g_save.has){
        g_mmRows.push_back({"cont", tr("CONTINUE GAME", "KONTYNUUJ GRĘ"), true});
        g_mmRows.push_back({"del",  tr("DELETE SAVE",   "USUŃ ZAPIS"),    true});
    }
    g_mmRows.push_back({"sep2",  "",                                       false});
    g_mmRows.push_back({"exit",  tr("EXIT TO DESKTOP", "WYJDŹ DO PULPITU"), true});
}

// Przejście do następnego/poprzedniego zaznaczalnego wiersza (pomija separatory)
// Move to next/previous selectable row (skips separators)
static void mmMove(int dir) {
    buildMM();
    int n = (int)g_mmRows.size();
    if (n == 0) return;
    int tries = 0;
    do {
        g_mmCur = (g_mmCur + dir + n) % n;
        tries++;
    } while (!g_mmRows[g_mmCur].sel && tries < n);
}

static void mmDown(){ mmMove(1); }
static void mmUp()  { mmMove(-1); }

// Zmiana wartości opcji / Change an option value (dir: -1 = lewo/left, +1 = prawo/right)
static void mmChange(int dir){
    buildMM();
    if (g_mmCur < 0 || g_mmCur >= (int)g_mmRows.size()) return;
    const std::string id = g_mmRows[g_mmCur].id;
    if     (id == "diff")  g_cfgDiff  = (g_cfgDiff  + dir + 4) % 4;
    else if(id == "lives") g_cfgLives = (g_cfgLives + dir + 4) % 4;
    else if(id == "sound") g_soundEnabled = !g_soundEnabled;
    else if(id == "lang")  g_lang = (g_lang == UILANG_PL) ? UILANG_EN : UILANG_PL;
}
static void mmLeft(){  mmChange(-1); }
static void mmRight(){ mmChange(+1); }

// Ustawia kursor na wierszu o danym id / Put the cursor on the row with the given id
static void mmSelect(const char* id){
    buildMM();
    for(int i=0;i<(int)g_mmRows.size();i++)
        if(g_mmRows[i].id == id){ g_mmCur = i; return; }
}

// Zatwierdzenie wybranej opcji / Confirm selected option
static void mmFire(){
    buildMM();
    if (g_mmCur < 0 || g_mmCur >= (int)g_mmRows.size()) return;
    if (!g_mmRows[g_mmCur].sel) return;
    const std::string id = g_mmRows[g_mmCur].id;      // kopia: buildMM() unieważnia referencje / copy: buildMM() invalidates refs

    if(id=="diff" || id=="lives" || id=="sound" || id=="lang"){ mmChange(+1); return; }
    if(id=="start"){ g_mode=MODE_PLAY; fullReset(); return; }
    if(id=="cont") { loadSaveData(); return; }
    if(id=="del")  { deleteSaveFile(); mmSelect("start"); return; }   // kursor wraca na "start" / cursor returns to "start"
    if(id=="exit") { PostMessageA(g_hwnd, WM_CLOSE, 0, 0); return; }
}

// =============================================================================
//  Menu pauzy / Pause menu
// =============================================================================
static const char* pmLabel(int i){
    switch(i){
    case 0:  return tr("RESUME GAME", "WZNÓW GRĘ");
    case 1:  return tr("SAVE GAME",   "ZAPISZ GRĘ");
    case 2:  return tr("MAIN MENU",   "MENU GŁÓWNE");
    default: return tr("EXIT GAME",   "WYJDŹ Z GRY");
    }
}
static const char* PMENU_ACT[4]   = {"resume","save","menu","exit"};
static int g_pmCur = 0;

static void pmDown(){ g_pmCur = (g_pmCur+1) % 4; }
static void pmUp()  { g_pmCur = (g_pmCur-1+4) % 4; }

// Zatwierdzenie pozycji menu pauzy / Confirm pause menu item
static void pmFire(){
    const char* a = PMENU_ACT[g_pmCur];
    if(strcmp(a,"resume")==0) g_mode = MODE_PLAY;
    if(strcmp(a,"save")==0){
        g_save.score = g_totalScore;
        g_save.wave  = g_currentWave;
        g_save.lives = g_player.lives;
        g_save.diff  = g_cfgDiff;
        g_save.has   = saveGameToFile();
        flashMsg(g_save.has ? tr("GAME SAVED TO DISK!", "GRA ZAPISANA NA DYSKU!")
                            : tr("SAVE FAILED!",        "BŁĄD ZAPISU!"), 120);
    }
    if(strcmp(a,"menu")==0){ commitHiScore(); g_mode=MODE_MAIN; buildMM(); mmMove(0); }
    if(strcmp(a,"exit")==0){ PostMessageA(g_hwnd, WM_CLOSE, 0, 0); }
}

// =============================================================================
//  Przetwarzanie wejścia zależnie od trybu gry
//  Input processing depending on game mode
// =============================================================================
static void processInput(){
    if(g_mode == MODE_MAIN){
        if((just(K_UP,P_UP) || Q_UP))       mmUp();
        if((just(K_DOWN,P_DOWN) || Q_DOWN))   mmDown();
        if((just(K_LEFT,P_LEFT) || Q_LEFT))   mmLeft();
        if((just(K_RIGHT,P_RIGHT) || Q_RIGHT)) mmRight();
        if((just(K_FIRE,P_FIRE) || Q_FIRE))   mmFire();
    } else if(g_mode == MODE_PAUSE){
        if((just(K_UP,P_UP) || Q_UP))     pmUp();
        if((just(K_DOWN,P_DOWN) || Q_DOWN)) pmDown();
        if((just(K_FIRE,P_FIRE) || Q_FIRE)) pmFire();
    } else if(g_mode == MODE_OVER){
        if((just(K_FIRE,P_FIRE) || Q_FIRE)){
            g_mode = MODE_MAIN; buildMM(); mmMove(0);
        }
    }
    // Zapamiętaj stan klawiszy do detekcji zbocza / Store key state for edge detection
    P_UP=K_UP; P_DOWN=K_DOWN; P_LEFT=K_LEFT; P_RIGHT=K_RIGHT; P_FIRE=K_FIRE;
    Q_UP=Q_DOWN=Q_LEFT=Q_RIGHT=Q_FIRE=false;
}

// =============================================================================
//  Rysowanie — scena gry, menu, pauza, game over
//  Rendering — game scene, menu, pause, game over
// =============================================================================

// -----------------------------------------------------------------------------
// Rysowanie migoczących gwiazd w tle / Draw twinkling background stars
// Współczynnik a (0–1) pozwala przygasić gwiazdy w menu / Factor a (0–1) dims stars in menu
// -----------------------------------------------------------------------------
static void drawStars(double a){
    double tt = nowMs() * 0.001;
    for(int i=0;i<120;i++){
        Star& s = g_stars[i];
        double tw = 0.4 + 0.6 * fabs(sin(tt + s.x));
        int al = (int)(255 * s.b * tw * a);
        if(al < 0) al = 0;
        if(al > 255) al = 255;
        COLORREF c = RGB(al/3, al, al/3);
        fillRect(s.x, TOP_H + s.y, s.r, s.r, c);   // przesunięcie o HUD / HUD offset
    }
}

// -----------------------------------------------------------------------------
// Rysowanie pełnej sceny gry / Draw the full game scene
// -----------------------------------------------------------------------------
static void drawScene(){
    fillRect(0, TOP_H, W, H, C_bg);
    drawStars(1.0);

    fillRect(0, TOP_H + 30, W, 2, RGB(0x0a,0x1a,0x0a));

    int oy = TOP_H;    // offset pionowy panelu gry / vertical offset of game area

    // --- UFO / UFO sprite ---
    if(g_ufo.active){
        double x=g_ufo.x, y=oy+g_ufo.y;
        double pulse = 0.7 + 0.3*sin(nowMs()*0.01);
        fillRect(x+4, y+6, g_ufo.w-8, 12, RGB(0xcc,0x33,0x33));
        fillRect(x, y+8, g_ufo.w, 8, RGB(0xff,0x55,0x55));
        int al=(int)(255*pulse);
        fillRect(x+10, y, g_ufo.w-20, 8, RGB(0xff,al/3,al/3));
        for(int i=0;i<3;i++) fillRect(x+6+i*12, y+10, 4, 4, RGB(0xff,0xff,0xcc));
        txt("BONUS",(int)x,(int)(y-14),fXx,RGB(0xff,0x77,0x44),AL_LEFT,true);
    }

    // --- Power-upy / Power-ups ---
    for(const auto& p : g_powerups){
        COLORREF c = (p.type == PWR_TRIPLE) ? C_cyan : C_gold;
        fillRect(p.x, oy+p.y, 20, 20, c);
        strokeRect(p.x, oy+p.y, 20, 20, C_white);
        std::string tag = (p.type == PWR_TRIPLE) ? "S" : "L";
        txt(tag, (int)p.x+10, oy+(int)p.y+2, fSm, RGB(0,0,0), AL_CENTER, false);
    }

    // --- Inwazjerzy / Invaders ---
    COLORREF wc = waveColor();
    for(size_t i=0;i<g_invaders.size();i++){
        Invader& inv = g_invaders[i];
        if(!inv.alive) continue;
        const int* pat = 0;
        if(inv.row == 0)      pat = SPR_PAT[inv.frame ? 1 : 0];   // squid
        else if(inv.row <= 2) pat = SPR_PAT[inv.frame ? 3 : 2];   // crab
        else                  pat = SPR_PAT[inv.frame ? 5 : 4];   // octopus
        // Cień (przygaszony kolor) + właściwy sprite / Shadow (dimmed) + main sprite
        drawPixelArt(pat,12,8, inv.x+1, oy+inv.y+1, dim(wc,0.4), 3);
        drawPixelArt(pat,12,8, inv.x,   oy+inv.y,   wc,          3);
    }

    // --- Bunkry / Bunkers ---
    for(size_t bk=0;bk<g_bunkers.size();bk++){
        for(size_t bl=0;bl<g_bunkers[bk].size();bl++){
            BunkerBlock& bb = g_bunkers[bk][bl];
            if(bb.hp <= 0) continue;
            COLORREF c = bb.hp==3 ? RGB(0x33,0xcc,0x44) :
                         bb.hp==2 ? RGB(0x22,0x88,0x33) :
                                    RGB(0x11,0x44,0x22);
            fillRect(bb.x, oy+bb.y, bb.w-1, bb.h-1, c);
            if(bb.hp==3) fillRect(bb.x+1, oy+bb.y+1, 2, 2, RGB(0x55,0xff,0x77));
        }
    }

    // Linia gruntu / Ground line
    fillRect(0, oy+H-22, W, 2, C_green);

    // --- Gracz (miga podczas nietykalności) / Player (blinks when invulnerable) ---
    bool hidePlayer = (g_player.flashTimer > 0) && (((g_player.flashTimer/3)%2)==0);
    if(!hidePlayer){
        double x=g_player.x, y=oy+g_player.y;
        COLORREF pCol = (g_tripleShotTimer > 0) ? C_cyan : C_green;
        fillRect(x+16, y-8,  8,  8, pCol);
        fillRect(x+8,  y,    24, 8, pCol);
        fillRect(x+2,  y+8,  36, 8, pCol);
        fillRect(x,    y+14, 40, 8, pCol);
    }

    // --- Pociski / Bullets ---
    for(size_t i=0;i<g_playerBullets.size();i++){
        Bullet& b = g_playerBullets[i];
        COLORREF bc = (g_tripleShotTimer > 0) ? C_cyan : RGB(0xff,0xff,0xcc);
        fillRect(b.x, oy+b.y, b.w, b.h, bc);
    }
    for(size_t i=0;i<g_enemyBullets.size();i++){
        Bullet& b = g_enemyBullets[i];
        fillRect(b.x, oy+b.y, b.w, b.h, C_red);
    }

    // --- Cząsteczki / Particles ---
    for(const auto& p : g_particles) {
        double factor = (double)p.life / p.maxLife;
        COLORREF c = dim(p.color, factor);
        fillRect(p.x, oy + p.y, 2, 2, c);
    }

    // --- Popupy punktów / Score popups ---
    for(size_t i=0;i<g_popups.size();i++){
        Popup& p = g_popups[i];
        int dy = (int)((50 - p.t) * 0.6);   // unoszenie / rise
        txt(p.text.c_str(), (int)p.x, oy+(int)p.y-dy, fSm, p.color, AL_CENTER, true);
    }

    // --- Krótki komunikat / Flash message ---
    if(g_flashT > 0){
        txt(g_flashMsg.c_str(), W/2, oy+H-40, fSm, C_amber, AL_CENTER, true);
    }

    // --- Czerwony błysk przy trafieniu / Red flash on hit ---
    if(g_screenFlashTimer > 0) {
        blendRect(0, TOP_H, W, H, RGB(255, 0, 0), 90);
    }
}

// -----------------------------------------------------------------------------
// Rysowanie menu głównego / Draw main menu
// -----------------------------------------------------------------------------
static void drawMainMenu(){
    fillRect(0, TOP_H, W, H, C_bg);
    drawStars(0.3);

    int oy = TOP_H;
    int PX=40, PY=oy+10, PW2=W-PX*2, PH2=H-20;
    panel(PX,PY,PW2,PH2);

    // Wersja i kompilator / Version and compiler
    txt(buildInfo(), PX+PW2-14, PY+8, fXx, C_grey2, AL_RIGHT);

    // Pulsujący tytuł / Pulsing title
    double tp = 0.88 + 0.12*sin(nowMs()*0.003);
    COLORREF amberDim = dim(C_amber, tp);
    txt("* SPACE INVADERS *", W/2, PY+18, fTitle, amberDim, AL_CENTER, true);
    txt(tr("MAIN MENU","MENU GŁÓWNE"), W/2, PY+52, fMed, C_green2, AL_CENTER, true);
    hline(PY+76, PX+16, PX+PW2-16, C_border);

    buildMM();

    int LX=PX+50, VX=PX+420, ROW_H=34;
    int ry = PY+90;

    for(int i=0;i<(int)g_mmRows.size();i++){
        const MenuRow& row = g_mmRows[i];
        bool sel = (i == g_mmCur);

        if(row.id=="sep1" || row.id=="sep2"){
            hline(ry-4, PX+16, PX+PW2-16, C_border2);
            ry += 14; continue;
        }

        // Podświetlenie zaznaczonego wiersza / Highlight selected row
        if(sel){
            fillRect(PX+12, ry-4, PW2-24, 30, RGB(0x0a,0x2e,0x0a));
            strokeRect(PX+12, ry-4, PW2-24, 30, C_green);
            if((int)(nowMs()/250)%2 == 0)
                txt(">", PX+24, ry+1, fBig, C_gold, AL_LEFT, true);
        }

        if(row.id=="info"){
            txt(row.label, LX, ry+1, fBig, C_grey);
            txt(padNum(g_hiScore,7), VX, ry+1, fBig, C_gold, AL_LEFT, true);
            ry += ROW_H; continue;
        }

        // Wiersze z opcjami: etykieta + wartość w < > / Option rows: label + value in < >
        if(row.id=="diff" || row.id=="lives" || row.id=="sound" || row.id=="lang"){
            std::string val;
            if     (row.id=="diff")  val = diffName(g_cfgDiff);
            else if(row.id=="lives") val = std::to_string(LIVES_OPTS[g_cfgLives]);
            else if(row.id=="sound") val = g_soundEnabled ? tr("ON","WŁ.") : tr("OFF","WYŁ.");
            else                     val = (g_lang == UILANG_PL) ? "POLSKI" : "ENGLISH";
            txt(row.label, LX, ry+1, fBig, sel ? C_white : C_green2);
            txt("< " + val + " >", VX, ry+1, fBig, sel ? C_gold : C_white, AL_LEFT, sel);
            ry += ROW_H; continue;
        }

        // Wiersze niebezpieczne (exit/delete) na czerwono / Danger rows (exit/delete) in red
        bool danger = (row.id=="exit" || row.id=="del");
        COLORREF col = danger ? (sel ? C_red : C_red2) : (sel ? C_green : C_green2);

        txt(row.label, W/2, ry+1, fBig, col, AL_CENTER, sel);
        ry += ROW_H;
    }

    hline(PY+PH2-46, PX+16, PX+PW2-16, C_border);
    txt(tr("UP / DOWN: Select   |   LEFT / RIGHT: Change option   |   ENTER / SPACE: Confirm",
           "GÓRA / DÓŁ: Wybór   |   LEWO / PRAWO: Zmiana opcji   |   ENTER / SPACE: Zatwierdź"),
        W/2, PY+PH2-34, fSm, C_white, AL_CENTER, true);

    // Informacja o zapisanej grze / Saved game info
    if(g_save.has){
        std::string info = std::string(tr("SAVED GAME: WAVE ", "ZAPISANA GRA: FALA ")) +
                           std::to_string(g_save.wave) + " | " + padNum(g_save.score,7) +
                           tr(" PTS | ", " PKT | ") + diffName(g_save.diff);
        txt(info, W/2, PY+PH2-16, fXs, C_green2, AL_CENTER);
    }
}

// -----------------------------------------------------------------------------
// Rysowanie menu pauzy (scena + przyciemnienie + panel)
// Draw pause menu (scene + dimming + panel)
// -----------------------------------------------------------------------------
static void drawPauseMenu(){
    drawScene();
    blendRect(0, TOP_H, W, H, RGB(0,0,0), 210);
    int oy = TOP_H;
    int bw=360, itemH=42, bh=280;
    int bx=W/2-bw/2, by=oy+H/2-bh/2;
    panel(bx,by,bw,bh);

    txt(tr("- PAUSED -","- PAUZA -"), W/2, by+18, fBig, C_amber, AL_CENTER, true);
    hline(by+48, bx+14, bx+bw-14, C_border);

    for(int i=0;i<4;i++){
        int ry = by+62+i*itemH;
        bool sel = (i == g_pmCur);
        if(sel){
            fillRect(bx+12, ry-3, bw-24, 32, RGB(0x0a,0x2e,0x0a));
            strokeRect(bx+12, ry-3, bw-24, 32, C_green);
            if((int)(nowMs()/250)%2 == 0)
                txt(">", bx+22, ry+4, fMed, C_gold, AL_LEFT, true);
        }
        bool danger = (strcmp(PMENU_ACT[i],"exit")==0);
        COLORREF col = danger ? (sel?C_red:C_red2)
                              : (sel?C_green:C_green2);
        txt(pmLabel(i), W/2, ry+4, fMed, col, AL_CENTER, sel);
    }

    hline(by+bh-35, bx+14, bx+bw-14, C_border);
    txt(tr("UP / DOWN: Select   |   ENTER: Confirm", "GÓRA / DÓŁ: Wybór   |   ENTER: Zatwierdź"),
        W/2, by+bh-22, fXs, C_grey2, AL_CENTER);
}

// -----------------------------------------------------------------------------
// Rysowanie ekranu Game Over / Draw Game Over screen
// -----------------------------------------------------------------------------
static void drawGameOver(){
    drawScene();
    blendRect(0, TOP_H, W, H, RGB(0,0,0), 205);
    int oy=TOP_H;
    int bw=420, bh=220;
    int bx=W/2-bw/2, by=oy+H/2-bh/2;
    panel(bx,by,bw,bh);
    txt("GAME OVER", W/2, by+20, fTitle, C_red, AL_CENTER, true);
    hline(by+60, bx+14, bx+bw-14, C_border);
    txt(std::string(tr("SCORE:    ","WYNIK:    ")) + padNum(g_totalScore,7),
        W/2, by+80, fBig, C_green, AL_CENTER, true);
    txt(std::string(tr("HI-SCORE: ","REKORD:   ")) + padNum(g_hiScore,7),
        W/2, by+120, fMed, C_green2, AL_CENTER, true);
    if(g_totalScore > 0 && g_totalScore >= g_hiScore)
        txt(tr("NEW HIGH SCORE!","NOWY REKORD!"), W/2, by+152, fSm, C_gold, AL_CENTER, true);
    hline(by+bh-40, bx+14, bx+bw-14, C_border);
    if((int)(nowMs()/500)%2 == 0)
        txt(tr("PRESS FIRE / ENTER","NACIŚNIJ STRZAŁ / ENTER"), W/2, by+bh-26, fSm, C_white, AL_CENTER, true);
}

// =============================================================================
//  HUD — górny panel (wynik, fala, życia) i dolny pasek pomocy
//  HUD — top panel (score, wave, lives) and bottom help bar
// =============================================================================
static void drawChrome(){
    // Górny pasek „INSERT COIN" / Top "INSERT COIN" bar
    fillRect(0, 0, W, 22, RGB(0x1a,0x0a,0x00));
    txt("* SPACE INVADERS * INSERT COIN *", W/2, 3, fSm, C_amber, AL_CENTER, true);

    // Panel HUD / HUD panel
    fillRect(0, 22, W, 52, RGB(0,0,0));
    hline(74, 0, W, RGB(0x1a,0x3a,0x1a));

    txt(tr("SCORE","WYNIK"), 12, 28, fXs, C_green, AL_LEFT, true);
    txt(padNum(g_totalScore,7), 12, 44, fMed, C_green, AL_LEFT, true);

    txt(tr("WAVE","FALA"), W/2, 28, fXs, C_green, AL_CENTER, true);
    txt(padNum(g_currentWave,2), W/2, 44, fMed, C_green, AL_CENTER, true);

    // Licznik triple shot / Triple shot counter
    if(g_tripleShotTimer > 0) {
        char buf[32]; snprintf(buf, sizeof(buf), "TRIPLE: %ds", g_tripleShotTimer/60 + 1);
        txt(buf, W/2 - 120, 44, fSm, C_cyan, AL_CENTER, true);
    }

    txt(tr("LIVES","ŻYCIA"), W-12, 28, fXs, C_green, AL_RIGHT, true);
    std::string ls;
    for(int i=0; i<g_player.lives; ++i) ls += "I";  // życia jako pionowe kreski / lives as vertical bars
    txt(ls, W-12, 44, fMed, C_green, AL_RIGHT, true);

    // Dolny pasek pomocy / Bottom help bar
    fillRect(0, TOP_H+H, W, BOT_H, RGB(0x0a,0x0a,0x0a));
    hline(TOP_H+H, 0, W, RGB(0x22,0x22,0x22));
    txt(tr("A/D or ARROWS - MOVE  |  SPACE/ENTER - FIRE  |  P / ESC - PAUSE",
           "A/D lub STRZAŁKI - RUCH  |  SPACE/ENTER - STRZAŁ  |  P / ESC - PAUZA"),
        W/2, TOP_H+H+4, fXs, C_grey, AL_CENTER);
}

// =============================================================================
//  Główna pętla rysowania z efektem screen shake
//  Main render loop with screen shake effect
// =============================================================================
static void render(){
    if(!g_memDC) return;
    // Czyść bufor / Clear backbuffer
    fillRect(0, 0, WIN_W, WIN_H, RGB(0x0a,0x0a,0x0a));

    drawChrome();

    // Rysuj zawartość zależnie od trybu / Draw content depending on mode
    if(g_mode == MODE_MAIN)       drawMainMenu();
    else if(g_mode == MODE_PLAY)  drawScene();
    else if(g_mode == MODE_PAUSE) drawPauseMenu();
    else if(g_mode == MODE_OVER)  drawGameOver();

    // Losowy offset przy screen shake / Random offset for screen shake
    int offsetX = (g_shakeTimer > 0) ? (rndi(9) - 4) : 0;
    int offsetY = (g_shakeTimer > 0) ? (rndi(9) - 4) : 0;

    // Prezentacja bufora na ekranie / Present backbuffer to screen
    HDC hdc = GetDC(g_hwnd);
    BitBlt(hdc, offsetX, offsetY, WIN_W, WIN_H, g_memDC, 0, 0, SRCCOPY);
    // Wyczyść odsłonięte krawędzie (bez tego zostają „duchy" poprzedniej klatki)
    // Clear the exposed edges (otherwise ghosts of the previous frame remain)
    if(offsetX > 0) PatBlt(hdc, 0, 0, offsetX, WIN_H, BLACKNESS);
    if(offsetX < 0) PatBlt(hdc, WIN_W + offsetX, 0, -offsetX, WIN_H, BLACKNESS);
    if(offsetY > 0) PatBlt(hdc, 0, 0, WIN_W, offsetY, BLACKNESS);
    if(offsetY < 0) PatBlt(hdc, 0, WIN_H + offsetY, WIN_W, -offsetY, BLACKNESS);
    ReleaseDC(g_hwnd, hdc);
}

// -----------------------------------------------------------------------------
// Pojedynczy krok logiki gry (stałe 60 Hz) — bez rysowania
// Single game-logic step (fixed 60 Hz) — no drawing
// -----------------------------------------------------------------------------
static void step(){
    processInput();
    updateGame();
    if(g_flashT > 0) g_flashT--;    // również w pauzie, by komunikat zniknął / also while paused so the message expires
}

// =============================================================================
//  Inicjalizacja czcionek i bufora / Font and buffer initialization
// =============================================================================
static HFONT makeFont(int size, bool bold=false){
    return CreateFontA(size, 0, 0, 0, bold?FW_BOLD:FW_NORMAL,
                       FALSE,FALSE,FALSE, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FIXED_PITCH|FF_MODERN,
                       "Consolas");
}

static void initFonts(){
    fTitle = makeFont(32, true);
    fBig   = makeFont(22, true);
    fMed   = makeFont(18, true);
    fSm    = makeFont(14);
    fXs    = makeFont(12);
    fXx    = makeFont(10);
}

// Bufor off-screen do bezmigowego rysowania / Off-screen buffer for flicker-free rendering
static void initBackbuffer(){
    HDC hdc = GetDC(g_hwnd);
    g_memDC = CreateCompatibleDC(hdc);
    g_memBmp = CreateCompatibleBitmap(hdc, WIN_W, WIN_H);
    g_memOld = (HBITMAP)SelectObject(g_memDC, g_memBmp);
    ReleaseDC(g_hwnd, hdc);
}

static void freeBackbuffer(){
    if(g_memDC){
        SelectObject(g_memDC, g_memOld);
        DeleteObject(g_memBmp);
        DeleteDC(g_memDC);
        g_memDC = 0;
    }
}

// Inicjalizacja losowych gwiazd tła / Initialize random background stars
static void initStars(){
    for(int i=0;i<120;i++){
        g_stars[i].x = rnd01() * W;
        g_stars[i].y = rnd01() * H;
        g_stars[i].r = rnd01() * 1.2 + 0.3;
        g_stars[i].b = rnd01() * 0.6 + 0.3;
    }
}

// =============================================================================
//  Procedura okna / Window procedure
// =============================================================================
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
    switch(msg){
    case WM_CREATE: g_hwnd = hwnd; return 0;
    case WM_ERASEBKGND: return 1;    // pomiń domyślne czyszczenie / skip default erase

    case WM_PAINT:{
        // Odrysuj bufor przy żądaniu systemu / Redraw buffer on system request
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(g_memDC) BitBlt(hdc, 0, 0, WIN_W, WIN_H, g_memDC, 0, 0, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:{
        const bool repeat = (lp & (1 << 30)) != 0;    // auto-repeat klawisza / key auto-repeat
        switch(wp){
        case VK_LEFT:  case 'A': K_LEFT = true; if(!repeat) Q_LEFT = true; break;
        case VK_RIGHT: case 'D': K_RIGHT = true; if(!repeat) Q_RIGHT = true; break;
        case VK_UP:    case 'W': K_UP = true; if(!repeat) Q_UP = true; break;
        case VK_DOWN:  case 'S': K_DOWN = true; if(!repeat) Q_DOWN = true; break;
        case VK_SPACE:
        case VK_RETURN: K_FIRE = true; if(!repeat) Q_FIRE = true; break;
        // P i ESC przełączają pauzę (bez auto-repeat) / P and ESC toggle pause (no auto-repeat)
        case 'P':
        case VK_ESCAPE:
            if(repeat) break;
            if(g_mode == MODE_PLAY){ g_mode = MODE_PAUSE; g_pmCur = 0; }
            else if(g_mode == MODE_PAUSE){ g_mode = MODE_PLAY; }
            break;
        }
        return 0;
    }

    case WM_KEYUP:
        switch(wp){
        case VK_LEFT:  case 'A': K_LEFT  = false; break;
        case VK_RIGHT: case 'D': K_RIGHT = false; break;
        case VK_UP:    case 'W': K_UP    = false; break;
        case VK_DOWN:  case 'S': K_DOWN  = false; break;
        case VK_SPACE:
        case VK_RETURN: K_FIRE = false; break;
        }
        return 0;

    case WM_KILLFOCUS:
        // Utrata fokusu: zwolnij klawisze i zapauzuj grę / Focus lost: release keys and pause
        K_LEFT = K_RIGHT = K_UP = K_DOWN = K_FIRE = false;
        if(g_mode == MODE_PLAY){ g_mode = MODE_PAUSE; g_pmCur = 0; }
        return 0;

    case WM_DESTROY:
        commitHiScore();             // nie gub rekordu przy zamknięciu / keep the high score on exit
        freeBackbuffer();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

// =============================================================================
//  Punkt wejścia aplikacji / Application entry point
// =============================================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nShow){
    srand((unsigned)time(0));
    g_lang = (PRIMARYLANGID(GetUserDefaultUILanguage()) == LANG_POLISH) ? UILANG_PL : UILANG_EN;
    initStars();
    initAudio();
    loadHighScoreFromFile();
    loadSaveFromFile();

    // Rejestracja klasy okna / Window class registration
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "SpaceInvadersClass";

    if(!RegisterClassExA(&wc)) return 1;

    // Dopasuj rozmiar okna do obszaru klienta / Fit window size to client area
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT r = {0, 0, WIN_W, WIN_H};
    AdjustWindowRect(&r, style, FALSE);
    int ww = r.right - r.left;
    int wh = r.bottom - r.top;
    int sx = (GetSystemMetrics(SM_CXSCREEN) - ww) / 2;
    int sy = (GetSystemMetrics(SM_CYSCREEN) - wh) / 2;

    const std::string title = std::string(APP_NAME) + " v" + APP_VERSION;
    HWND hwnd = CreateWindowExA(0, "SpaceInvadersClass", title.c_str(),
                                style, sx, sy, ww, wh, 0, 0, hInst, 0);
    if(!hwnd) return 1;

    initFonts();
    initBackbuffer();
    buildMM();
    mmMove(0);

    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    // Pętla główna ze stałym krokiem 60 Hz. WM_TIMER (16 ms) bywa kwantowany do ~15,6 ms,
    // co dawało 32 lub 64 FPS zamiast 60 — tu czas liczy QueryPerformanceCounter.
    // Main loop with a fixed 60 Hz step. WM_TIMER (16 ms) is quantized to ~15.6 ms,
    // which gave 32 or 64 FPS instead of 60 — here QueryPerformanceCounter keeps time.
    timeBeginPeriod(1);                          // dokładny Sleep() / accurate Sleep()
    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);
    const double STEP = 1.0 / 60.0;
    double acc = 0.0;

    MSG msg = {};
    bool running = true;
    while(running){
        while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)){
            if(msg.message == WM_QUIT){ running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if(!running) break;

        LARGE_INTEGER cur;
        QueryPerformanceCounter(&cur);
        acc += (double)(cur.QuadPart - prev.QuadPart) / (double)freq.QuadPart;
        prev = cur;
        if(acc > 0.25) acc = 0.25;               // po długiej przerwie nie „goń" / don't spiral after a long stall

        if(acc >= STEP){
            for(int n = 0; acc >= STEP && n < 5; ++n){ step(); acc -= STEP; }
            render();
        } else {
            Sleep(1);                            // oddaj CPU / yield the CPU
        }
    }
    timeEndPeriod(1);

    // Sprzątanie czcionek / Font cleanup
    if(fTitle) DeleteObject(fTitle);
    if(fBig)   DeleteObject(fBig);
    if(fMed)   DeleteObject(fMed);
    if(fSm)    DeleteObject(fSm);
    if(fXs)    DeleteObject(fXs);
    if(fXx)    DeleteObject(fXx);

    return (int)msg.wParam;
}
