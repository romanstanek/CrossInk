# Personal fork workflow

This fork keeps a small set of personal reader improvements on top of stable
CrossInk releases. Keep changes on feature branches and integrate them through
pull requests after validation.

## Remotes and branches

- `origin`: `https://github.com/romanstanek/CrossInk.git`
- `upstream`: `https://github.com/uxjulia/CrossInk.git`
- Local `main` tracks `origin/main` and contains the tested custom firmware source.
- New changes use `feat/`, `fix/`, or `chore/` branches and target this fork's `main`.

Local pulls use fast-forward only. If branches diverge, inspect the commits and
resolve the difference explicitly. Do not reset the custom `main` to upstream.

## Validation

The `CI (build)` workflow runs the native hyphenation, Unicode, and page-cache
regressions, formatting, and static analysis. Pull requests also build X3/X4 and
X4 Pro firmware. Manual runs include the firmware builds too. The aggregate
`Test Status` check must pass before merging; no separate reviewer is required
for this single-maintainer fork.

Inherited website deployment, issue triage, font publishing, and upstream release
workflows are disabled in this fork. They depend on upstream services or secrets.
Firmware releases are published explicitly after testing.

For changes affecting reading, also verify the result on the X3: check boot and
version, open a Czech book, inspect line breaks and accents, then reopen an English
book. Record any hardware checks still pending rather than treating a build as a
visual check.

## Taking an upstream release

```sh
git switch main
git pull --ff-only origin main
git fetch upstream --tags
git switch -c update/crossink-VERSION
git merge UPSTREAM_RELEASE_TAG
```

Replace `VERSION` and `UPSTREAM_RELEASE_TAG` with the chosen stable release.
Resolve conflicts while retaining the Czech patterns, language aliases, Unicode
normalization, tests, and license. Check page-cache versions against both sides
when resolving layout changes. Push the update branch to `origin` and open a PR
against this fork's `main`. Run CI, build the firmware, and verify it on the reader
before merging the update.

## Installed firmware releases

Use annotated tags such as `v1.6.1-czech.1` for installed custom builds. Publish the
application binary, SHA-256 checksum, license, and build metadata as release
assets. Keep device flash/settings backups and book files local.

The first release archives the existing installed development image. Its embedded
version remains `1.6.1-dev+feat-czech-hyphenation`; the release notes record the
original base commit and dirty build state. It is not relabeled or silently rebuilt.
For subsequent builds, set `CROSSINK_RELEASE_VERSION` to the chosen custom version
so the device version matches its tag, and record the source commit and toolchain.

Always inspect the device's actual partition table and OTA selector before raw
USB flashing. The default build's app0 offset is not a universal install address.
