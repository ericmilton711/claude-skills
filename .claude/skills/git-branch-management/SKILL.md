# Git Branch Management

description: Consolidate, merge, and clean up git branches (main/master issues, tracking branches)

## When to Use This Skill

- User has both `main` and `master` branches causing confusion
- Push fails due to branch name mismatch
- Need to consolidate branches
- Need to fix tracking branch configuration

## Common Problem: main vs master Mismatch

When local branch tracks a different remote branch name:

```
fatal: The upstream branch of your current branch does not match
the name of your current branch.
```

## Diagnose Branch State

```bash
# Check current branch and tracking
git branch -vv

# Check remote branches
git branch -r

# Check default branch on GitHub
gh repo view --json defaultBranchRef --jq '.defaultBranchRef.name'

# Compare commits between branches
git log origin/main --oneline -10
git log origin/master --oneline -10
```

## Solution 1: Consolidate to master (Delete main)

```bash
# Fetch all remote branches
git fetch origin

# Switch to master
git checkout master

# Merge main into master
git merge origin/main --no-edit

# Push updated master
git push origin master

# Delete main branch on remote
git push origin --delete main

# Set local tracking to master
git branch -u origin/master master

# Clean up stale remote references
git remote prune origin
```

## Solution 2: Consolidate to main (Delete master)

```bash
# Fetch all remote branches
git fetch origin

# Switch to main (create if needed)
git checkout main || git checkout -b main origin/main

# Merge master into main
git merge origin/master --no-edit

# Push updated main
git push origin main

# Set main as default on GitHub
gh repo edit --default-branch main

# Delete master branch on remote
git push origin --delete master

# Set local tracking to main
git branch -u origin/main main

# Clean up stale remote references
git remote prune origin
```

## Solution 3: Push to Different Branch Name (Quick Fix)

```bash
# Push local master to remote main
git push origin HEAD:main

# Push local main to remote master
git push origin HEAD:master
```

## Fix Tracking Branch

```bash
# Set upstream for current branch
git branch -u origin/master

# Set upstream for specific branch
git branch -u origin/master master

# Check tracking is correct
git branch -vv
```

## Change GitHub Default Branch

```bash
# Via GitHub CLI
gh repo edit --default-branch master

# Verify
gh repo view --json defaultBranchRef
```

## Rename Local Branch

```bash
# Rename current branch
git branch -m new-name

# Rename specific branch
git branch -m old-name new-name

# Push renamed branch and delete old
git push origin -u new-name
git push origin --delete old-name
```

## Clean Up After Consolidation

```bash
# Remove stale remote-tracking branches
git remote prune origin

# Delete local branches that no longer exist on remote
git fetch -p && git branch -vv | grep ': gone]' | awk '{print $1}' | xargs -r git branch -d

# Verify clean state
git branch -a
```

## Troubleshooting

### Push rejected (fetch first)

Remote has commits you don't have locally:

```bash
git fetch origin
git merge origin/master --no-edit
git push origin master
```

### Merge conflicts

```bash
# See conflicted files
git status

# After resolving conflicts
git add .
git commit -m "Merge branch 'main' into master"
git push
```

### Accidentally deleted wrong branch

If you have the commits locally:

```bash
# Recreate branch from local
git checkout -b main
git push origin main
```

If only on remote, check GitHub's branch restore feature (available for ~90 days).
