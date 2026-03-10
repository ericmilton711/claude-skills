# Skill: Mind Muscle Performance Website

## Overview
Built a complete business website for **Mind Muscle Performance** (formerly Joe's Fitness LLC) — a physical fitness business for kids and young adults.

## Live Site
- **GitHub Pages URL:** https://ericmilton711.github.io/mind-muscle-performance/
- **GitHub Repo:** https://github.com/ericmilton711/mind-muscle-performance
- **Local file:** `C:\Users\ericm\joes-fitness\index.html`

## To Update the Site
```bash
cd ~/joes-fitness
# edit index.html
git add index.html
git commit -m "Update site"
git push
```
Changes go live in ~30 seconds.

## Design
- **Colors:** White/khaki background (`#f5f0e8`), purple (`#6a0dad`), yellow (`#f5c518`)
- **Font:** Segoe UI
- **Style:** Modern, responsive, mobile-friendly

## Sections
1. Sticky nav (About, Training, Nutrition, Pricing, Contact)
2. Hero with CTA button
3. About with stat cards (500+ members, 12 trainers, 8+ years, 20+ programs)
4. Fitness Programs (6 cards: Strength, Speed & Agility, Flexibility, Cardio/HIIT, Boxing, Personal Training)
5. Nutrition & Wellness (6 cards + Teen Wellness Challenge banner)
6. Tabbed Pricing Table (3 tabs: Fitness / Nutrition / Total Wellness)
7. Contact form with grouped dropdowns
8. Footer

## Pricing Plans

### Fitness Tab
| Plan | Price |
|---|---|
| Starter | $39/mo |
| Champion | $69/mo (featured) |
| Elite Athlete | $109/mo |
| Family Plan | $119/mo (up to 3 kids) |

### Nutrition Tab
| Plan | Price |
|---|---|
| Nutrition Basics | $29/mo |
| Nutrition Coach | $59/mo (featured) |
| Sports Nutrition | $79/mo |
| Teen Wellness Challenge | $149 one-time (8-week program) |

### Total Wellness Tab
| Plan | Price |
|---|---|
| Wellness Starter | $59/mo |
| Total Wellness | $99/mo (featured) |
| Elite Wellness | $149/mo |
| Family Wellness | $179/mo (up to 3 kids) |

## Custom Domain (Pending)
- User wants to buy a custom domain (e.g. `mindmuscleperformance.com`)
- Recommended registrar: **Namecheap** (~$10-15/yr)
- Once purchased, add a `CNAME` file to the repo and configure DNS on Namecheap
- GitHub handles SSL automatically

## To Connect a Custom Domain (when ready)
```bash
cd ~/joes-fitness
echo "www.mindmuscleperformance.com" > CNAME
git add CNAME
git commit -m "Add custom domain"
git push
```
Then in Namecheap DNS, add:
- Type: `CNAME` | Host: `www` | Value: `ericmilton711.github.io`
- Type: `A` | Host: `@` | Value: `185.199.108.153`
- Type: `A` | Host: `@` | Value: `185.199.109.153`
- Type: `A` | Host: `@` | Value: `185.199.110.153`
- Type: `A` | Host: `@` | Value: `185.199.111.153`

## Notes
- Background was changed from white to warm light khaki (`#f5f0e8`) — user preferred this
- Business was originally named "Joe's Fitness LLC" — renamed to "Mind Muscle Performance"
- Hard refresh (`Ctrl+Shift+R`) needed to clear browser cache after updates
- Namecheap blocks automated domain availability checks — user must check manually
