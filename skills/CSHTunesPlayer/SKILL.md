# CSHTunes Player

## Overview
A web-based music audio player app for school-aged kids (6-12). Plays a curated selection of educational memorization coursework music. Designed to be simple, clean, and accessible from anywhere via the internet.

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
- **Size:** 10-30 albums (~300-500 tracks, ~3-6 GB as MP3)
- **Content:** Educational memorization coursework for younger kids
- **Source:** Ripped from CDs (some in hand, some still to acquire)
- **Organization:** By album and/or subject area / grade level

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
Album/Subject Grid → Track List → Player View
```

## Key Features (v1)
- Browse by album or subject
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

## Reference Image
- iTunes MiniPlayer layout used as design reference for the Now Playing view
- Located at: ~/.claude/image-cache/4093ee83-8860-4d89-a134-6f0a4a209d6d/1.png
