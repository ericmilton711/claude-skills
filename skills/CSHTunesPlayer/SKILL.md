# CSHTunes Player

## Overview
A web-based music audio player app for **Catholic Schoolhouse (CSH)** memory work music. CSH is a Catholic homeschool/co-op curriculum. The app plays their memorization coursework music for school-aged kids (6-12). Designed to be simple, clean, and accessible from anywhere via the internet.

## Platform
- **Type:** Web app (PWA — installable on any device)
- **Framework:** Vue 3 + Vite
- **Hosting:** Cloudflare Pages (free)
- **Audio Storage:** Cloudflare R2 (free egress, free up to 10GB)
- **Backend/API:** Cloudflare Workers (free tier)
- **Music Metadata:** JSON files (no database needed)
- **Estimated Cost:** $0-5/month

## Domain
- **Preferred:** cshtunes.com (available as of 2026-05-13)
- **Also available:** cshtunes.org, cshtunes.net
- **Registrar recommendation:** Cloudflare Registrar (~$9-10/year, at-cost pricing)

## Music Collection
- **Content:** Catholic Schoolhouse memory work music for school-aged kids (6-12)
- **Structure:** 3 Tours, each with 4 Discs (one per quarter of the school year) = 12 discs total
  - Tour 1: Q1, Q2, Q3, Q4
  - Tour 2: Q1, Q2, Q3, Q4
  - Tour 3: Q1, Q2, Q3, Q4
- **How it works:** All grades listen to the same discs. Tours rotate on a 3-year cycle. Each year the school uses a different Tour's set of 4 discs. The 4 discs change each year.
- **Source:** Ripped from CDs (some in hand, some still to acquire)
- **Size:** 12 discs total (~3-6 GB as MP3)

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
- Tap to play, simple queue
- Now Playing screen with large album art
- Works on phones, tablets, laptops
- Installable as PWA (home screen icon)
- No login required

## Build Steps
1. Set up project scaffold (Vue 3 + Vite + PWA plugin)
2. Design and build UI components (album grid, track list, player view)
3. Rip CDs → MP3, tag with metadata + cover art
4. Set up Cloudflare R2 bucket + Workers API
5. Build the audio player engine (HTML5 Audio API)
6. Deploy to Cloudflare Pages
7. Test on multiple devices (phone, tablet, laptop)
8. Add music as more CDs are acquired

## Mockup
- **Working mockup:** `mockup.html` in this skills folder (responsive — works on desktop and mobile)
- Desktop: 4-column grid (one row per Tour), horizontal player (art left, controls right)
- Mobile: 2-column grid, vertical player (art on top, controls below)
- To preview: `python3 -m http.server 8080` from the skills folder, open in browser

## Reference Images
- iTunes MiniPlayer layout used as design reference for the Now Playing view
  - Located at: ~/.claude/image-cache/4093ee83-8860-4d89-a134-6f0a4a209d6d/1.png
- Catholic Schoolhouse disc cover art (Tour I, Quarter 1) — green tones, schoolhouse logo, "Memory Work" subtitle
  - Each of the 12 discs has its own cover art in this style, used in both the library grid and the player view
  - Located at: `cshtunes-cover-t1q1.png` in this skills folder
  - Also cached at: ~/.claude/image-cache/4093ee83-8860-4d89-a134-6f0a4a209d6d/2.png
