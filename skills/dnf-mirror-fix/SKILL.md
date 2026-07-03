# DNF Mirror Fix — Fedora Update Failures

Fixes Fedora `dnf update` failures caused by a stale/dead mirror returning 404 on a package download.

## Symptom

```
Cannot download Packages/v/vim-data-9.2.500-1.fc43.noarch.rpm: All mirrors were tried;
Last error: Status code: 404 for http://<some-mirror>/fedora/linux/updates/...
```

The specific package/mirror in the error will vary — the root cause is the same: DNF cached a mirror list that includes a mirror with a missing or stale file.

## Fix

Refresh the repo metadata/mirror cache and retry the update:

```bash
sudo dnf update -y --refresh
```

`--refresh` forces DNF to re-fetch repo metadata and re-resolve mirrors instead of reusing the cached (bad) mirror list. This resolved the issue on the first retry without any other changes.

## Notes

- No need to manually exclude the bad mirror or edit repo configs — a refresh alone was sufficient.
- If `--refresh` doesn't fix it, next steps would be `dnf clean all` followed by the update, or temporarily excluding the specific mirror.
