# Claude Instructions for Eric

## Skills Management

**GitHub Repository:** https://github.com/ericmilton711/claude-skills

**Local Setup:**
- Git repo: `C:\Users\ericm\claude-skills\`
- Junctions link `~/.claude/skills/` and `~/.claude/commands/` to the git repo

**When user says "save to skills" or "save and document this to skills":**
1. Save the content to `~/.claude/skills/<skill-name>/SKILL.md`
2. **Immediately** commit and push to GitHub:
   ```bash
   cd ~/claude-skills && git add -A && git commit -m "Add <skill-name>" && git push
   ```
3. Do NOT wait for a separate "push" command - auto-push is always on

## GitHub Account

- Username: ericmilton711
- Email: ericmilton711@gmail.com

## Git Config

```bash
git config --global user.email "ericmilton711@gmail.com"
git config --global user.name "ericm"
```
