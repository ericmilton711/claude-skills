# Skill: Business Webpage Builder

Build a complete, modern single-page business website with HTML/CSS/JS and deploy it to GitHub Pages.

## What This Produces

A single `index.html` file with:
- Sticky nav bar with smooth scroll
- Hero section with CTA
- About section with stat cards
- Services/Programs grid
- Optional: Nutrition & Wellness section
- Tabbed pricing table
- Contact form with success animation
- Footer
- Fully responsive (mobile-friendly)

## Color Scheme

Ask the user for colors. Store as CSS variables in `:root`:
```css
:root {
  --purple: #6a0dad;
  --purple-light: #9b30d9;
  --purple-dark: #4a007a;
  --yellow: #f5c518;
  --yellow-light: #ffe066;
  --white: #f5f0e8;  /* warm light khaki background */
  --gray: #ede8dc;
  --text-dark: #1a1a2e;
  --text-mid: #444;
}
```

## Pricing Table Pattern

Use **tabbed pricing panels** for multiple categories:
```html
<div class="pricing-tabs">
  <button class="tab-btn active" onclick="showTab('fitness')">Fitness</button>
  <button class="tab-btn" onclick="showTab('nutrition')">Nutrition</button>
  <button class="tab-btn" onclick="showTab('wellness')">Total Wellness</button>
</div>
<div class="pricing-panel active" id="tab-fitness"> ... </div>
<div class="pricing-panel" id="tab-nutrition"> ... </div>
```

JS to switch tabs:
```js
function showTab(tab) {
  document.querySelectorAll('.pricing-panel').forEach(p => p.classList.remove('active'));
  document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
  document.getElementById('tab-' + tab).classList.add('active');
  event.target.classList.add('active');
}
```

Mark featured/popular card with class `featured` and add a `.badge` div inside it.

## Deploying to GitHub Pages

```bash
# 1. Init repo and commit
cd ~/project-folder
git init
git add index.html
git commit -m "Initial commit"

# 2. Create GitHub repo and push
gh repo create <repo-name> --public --source=. --remote=origin --push

# 3. Enable GitHub Pages
gh api repos/<username>/<repo-name>/pages -X POST \
  --field 'source[branch]=master' \
  --field 'source[path]=/'
```

Live URL will be: `https://<username>.github.io/<repo-name>/`
(Takes 1-2 minutes to go live after enabling Pages.)

## To Update the Live Site

```bash
cd ~/project-folder
git add index.html
git commit -m "Update site"
git push
```

GitHub Pages auto-rebuilds on every push. Changes go live in ~30 seconds.

## Notes

- Background: use a warm light khaki (`#f5f0e8`) instead of pure white — looks much better
- Contact form dropdown: use `<optgroup>` to group options by category
- Teen Wellness Challenge: highlight as a special 8-week program with a colored banner card
- Smooth scroll: `scrollIntoView({ behavior: 'smooth' })` on all `a[href^="#"]` links
