# CSHTunes Player

## Overview
A native mobile music player app for **Catholic Schoolhouse (CSH)** memory work music. CSH is a Catholic homeschool/co-op curriculum. The app plays their memorization coursework music for school-aged kids (6-12). Designed to be simple, clean, and accessible offline — no internet needed for playback.

**The app ships empty.** It does not include or distribute any music. Each family rips their own CSH CDs and imports the files into the app. This keeps it copyright-clean.

## Platform
- **Type:** Native mobile app (iOS + Android)
- **Framework:** React Native with Expo
- **Distribution:** App Store (iOS) + Google Play (Android)
- **Audio Storage:** On-device only (downloaded music lives on the phone)
- **Backend:** None — fully offline, no server required
- **Cost:** $0/month (one-time $25 Google Play fee, $99/year Apple Developer)

## Music Import
Families load their own ripped CD files into the app. Three import methods:

1. **WiFi Transfer (primary)** — App runs a local web server on the phone. User opens a page on their computer's browser (same WiFi network), drags MP3 files in. No cables, no accounts.
2. **File Picker** — Select files from the device's local storage (Files app on iOS, file manager on Android).
3. **iTunes/USB File Sharing** — Standard iOS/Android file sharing for wired transfer.

### Import Organization
- On import, the app reads ID3 tags (artist, album, track number) to auto-organize
- User can manually assign tracks to Tour / Quarter if tags are missing
- Cover art pulled from ID3 embedded art, or user can set manually

## Music Collection Structure
- **Structure:** 3 Tours, each with 4 Quarters (one disc per quarter) = 12 discs total
  - Tour 1: Q1, Q2, Q3, Q4
  - Tour 2: Q1, Q2, Q3, Q4
  - Tour 3: Q1, Q2, Q3, Q4
- **How it works:** All grades listen to the same discs. Tours rotate on a 3-year cycle. Each year the school uses a different Tour's set of 4 discs.
- **Size:** ~3-6 GB total as MP3 (all 12 discs)

## Visual Design

### Style
- Blend of clean/minimal and playful — not stark white, not overly vibrant
- Soft, warm background (light cream or soft gray)
- Rounded cards and edges
- Subtle color accents per subject area
- Clean typography, good spacing
- Big, obvious playback controls (kid-friendly tap targets)

### Player View (Primary Reference: iTunes MiniPlayer)
- **Album art dominates** — takes up ~80% of the player view
- Progress bar with elapsed time, scrubber, remaining time
- Large, touch-friendly playback buttons: Rewind / Play-Pause / Forward
- Song title and album name displayed below the art
- Track indicator ("Track 3 of 12")
- Back/home button to return to album list
- No volume, search, or options controls (keep it simple for kids)

### Navigation Flow
```
Tour Select → Quarter/Disc Select → Track List → Player View
```

## Key Features (v1)
- Browse by Tour → Quarter (Disc)
- Tap to play, simple queue — plays through all tracks on a disc in order
- Now Playing screen with large album art
- Works fully offline after music is imported
- Background audio playback (plays while phone is locked or in other apps)
- Lock screen / notification controls (play/pause/skip)
- No login required, no accounts, no internet dependency

## Build Steps
1. Set up Expo project with React Native
2. Build WiFi transfer server (local HTTP server for drag-and-drop import)
3. Build file picker import
4. Build local music library with SQLite (track metadata, file paths, organization)
5. Build UI: Tour grid → Quarter grid → Track list → Player view
6. Build audio engine (expo-av or react-native-track-player) with background playback + lock screen controls
7. Test offline playback on both platforms
8. Publish to App Store and Google Play

## Domain
- **cshtunes.com** available as of 2026-05-13 (for a simple landing page / marketing site if desired)

## Mockup
- **Working mockup:** `mockup.html` in this skills folder (responsive — works on desktop and mobile)
- Desktop: 4-column grid (one row per Tour), horizontal player (art left, controls right)
- Mobile: 2-column grid, vertical player (art on top, controls below)
- To preview locally: `python3 -m http.server 8080` from the skills folder, open in browser

### Permanent Hosted Mockup
- **URL:** http://192.168.12.136:8080/mockup.html
- **Hosted on:** ThinkCentre (.136) — accessible from any device on the MILTONHAUS network
- **Files:** `/var/www/cshtunes/` (mockup.html + cshtunes-cover-t1q1.png)
- **Service:** `cshtunes-mockup.service` (systemd) — auto-starts on boot, restarts on crash
- **Port:** 8080 (firewall opened)

## Reference Images
- iTunes MiniPlayer layout used as design reference for the Now Playing view
  - Located at: ~/.claude/image-cache/4093ee83-8860-4d89-a134-6f0a4a209d6d/1.png
- Catholic Schoolhouse disc cover art (Tour I, Quarter 1) — green tones, schoolhouse logo, "Memory Work" subtitle
  - Located at: `cshtunes-cover-t1q1.png` in this skills folder
  - Also cached at: ~/.claude/image-cache/4093ee83-8860-4d89-a134-6f0a4a209d6d/2.png
