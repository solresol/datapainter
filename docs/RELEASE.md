# Release Process for DataPainter

This document describes the complete release process for DataPainter, including version numbering, pre-release testing, creating releases, and deployment.

## Version Numbering

DataPainter follows [Semantic Versioning 2.0.0](https://semver.org/):

```
MAJOR.MINOR.PATCH[-PRERELEASE][+BUILD]
```

- **MAJOR**: Incremented for incompatible API changes or major architectural changes
- **MINOR**: Incremented for new features in a backward-compatible manner
- **PATCH**: Incremented for backward-compatible bug fixes
- **PRERELEASE**: Optional suffix for pre-release versions (e.g., `-alpha.1`, `-beta.2`, `-rc.1`)
- **BUILD**: Optional build metadata (e.g., `+20241107.abc123`)

### Examples

- `1.0.0` - First stable release
- `1.1.0` - Added new feature (table view mode)
- `1.1.1` - Fixed bug in viewport rendering
- `2.0.0` - Changed database schema (breaking change)
- `1.2.0-rc.1` - Release candidate for 1.2.0
- `1.0.0+20241107.abc123` - Build with metadata

### Current Version

The current version is defined in:
- `CMakeLists.txt` - `project(datapainter VERSION X.Y.Z)`
- `docs/datapainter.1` - `.TH DATAPAINTER 1 "Month Year" "DataPainter X.Y.Z"`

## Pre-Release Checklist

Before creating a release, ensure all items are complete:

### Code Quality

- [ ] All unit tests pass locally: `cd build && ctest --output-on-failure`
- [ ] All integration tests pass locally: `uv run pytest tests/integration/ -v`
- [ ] No compiler warnings in Release build
- [ ] Code formatted with clang-format: `find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i`
- [ ] No outstanding TODO comments for this release
- [ ] No debug print statements left in code

### Documentation

- [ ] README.md is up to date with new features
- [ ] CHANGELOG.md has entry for this release
- [ ] Man page (`docs/datapainter.1`) reflects current CLI options
- [ ] docs/ARCHITECTURE.md updated if architecture changed
- [ ] docs/TESTING.md updated if testing approach changed
- [ ] Version number updated in `CMakeLists.txt`
- [ ] Version number updated in `docs/datapainter.1`

### Testing

- [ ] Tested on Linux (Ubuntu 20.04 or later)
- [ ] Tested on macOS (12.x or later)
- [ ] Tested on Windows (if applicable)
- [ ] Tested with large datasets (1M+ rows)
- [ ] Tested all keyboard shortcuts
- [ ] Tested all CLI commands
- [ ] Tested database migration (if schema changed)

### GitHub

- [ ] All pull requests merged to `main`
- [ ] CI pipeline passing on `main` branch
- [ ] No open critical bugs
- [ ] Release notes drafted

## Creating a Release

### 1. Update Version Numbers

Edit `CMakeLists.txt`:

```cmake
project(datapainter VERSION 1.2.0)
```

Edit `docs/datapainter.1`:

```groff
.TH DATAPAINTER 1 "November 2024" "DataPainter 1.2.0" "User Commands"
```

### 2. Update CHANGELOG.md

Create a new section at the top of CHANGELOG.md:

```markdown
## [1.2.0] - 2024-11-07

### Added
- New table view mode for editing data in tabular format
- Tab navigation through UI fields and buttons
- Random point generation dialog

### Changed
- Improved viewport rendering performance
- Enhanced error messages for invalid input

### Fixed
- Fixed cursor positioning in narrow terminals
- Fixed undo/redo with multiple overlapping points

### Deprecated
- None

### Removed
- None

### Security
- None
```

### 3. Commit Version Changes

```bash
git add CMakeLists.txt docs/datapainter.1 CHANGELOG.md
git commit -m "Bump version to 1.2.0"
git push origin main
```

### 4. Create Git Tag

```bash
git tag -a v1.2.0 -m "Release version 1.2.0"
git push origin v1.2.0
```

### 5. Create GitHub Release

#### Via Web Interface

1. Go to https://github.com/solresol/datapainter/releases/new
2. Choose tag: `v1.2.0`
3. Release title: `DataPainter v1.2.0`
4. Description: Copy from CHANGELOG.md, then add:

```markdown
## Installation

### Linux (Ubuntu/Debian)
```bash
wget https://github.com/solresol/datapainter/releases/download/v1.2.0/datapainter-linux-v1.2.0.tar.gz
tar -xzf datapainter-linux-v1.2.0.tar.gz
sudo cp datapainter /usr/local/bin/
sudo cp datapainter.1 /usr/local/share/man/man1/
```

### macOS
```bash
wget https://github.com/solresol/datapainter/releases/download/v1.2.0/datapainter-macos-v1.2.0.tar.gz
tar -xzf datapainter-macos-v1.2.0.tar.gz
sudo cp datapainter /usr/local/bin/
sudo cp datapainter.1 /usr/local/share/man/man1/
```

### Windows
Download `datapainter-windows-v1.2.0.zip`, extract, and add to PATH.

## Full Changelog

See [CHANGELOG.md](https://github.com/solresol/datapainter/blob/main/CHANGELOG.md)
```

5. Check "Set as the latest release"
6. Click "Publish release"

#### Via GitHub CLI

```bash
gh release create v1.2.0 \
  --title "DataPainter v1.2.0" \
  --notes-file docs/release-notes-1.2.0.md \
  --latest
```

### 6. Verify Release Artifacts

The GitHub Actions `release.yml` workflow will automatically:

1. Build for Linux, macOS, and Windows
2. Run all tests on each platform
3. Create platform-specific archives
4. Upload artifacts to the GitHub release
5. Deploy to packages.industrial-linguistics.com (if configured)

Wait for the workflow to complete (~10 minutes) and verify:

- [ ] `datapainter-linux-v1.2.0.tar.gz` uploaded
- [ ] `datapainter-macos-v1.2.0.tar.gz` uploaded
- [ ] `datapainter-windows-v1.2.0.zip` uploaded
- [ ] All artifacts download correctly
- [ ] Binaries run and show correct version

### 7. Deploy to Package Server

If `DEPLOYMENT_SSH_KEY` secret is configured, the release workflow will automatically deploy the Linux artifact to `packages.industrial-linguistics.com`. Verify:

```bash
curl -I https://packages.industrial-linguistics.com/datapainter/datapainter-linux-v1.2.0.tar.gz
# Should return 200 OK
```

If manual deployment is needed:

```bash
scp datapainter-linux-v1.2.0.tar.gz deploy@packages.industrial-linguistics.com:/var/www/packages/datapainter/
```

### 8. Announce Release

Post announcements to:

- [ ] GitHub Discussions
- [ ] Project mailing list
- [ ] Social media (if applicable)
- [ ] Internal company channels

## Post-Release

### Verify Release

After 24 hours, verify:

- [ ] GitHub release downloads are working
- [ ] Package server downloads are working
- [ ] No critical bug reports filed
- [ ] CI/CD pipeline still green

### Monitor Issues

Watch for:

- Installation issues on different platforms
- Regression bugs
- Performance problems
- User feedback

### Prepare Next Release

- [ ] Create milestone for next version
- [ ] Update TODO.md with planned features
- [ ] Triage open issues into milestones

## Hotfix Releases

For critical bugs in production, follow this accelerated process:

### 1. Create Hotfix Branch

```bash
git checkout -b hotfix/1.2.1 v1.2.0
```

### 2. Fix Bug and Test

```bash
# Make fixes
git add .
git commit -m "Fix critical bug in viewport rendering"

# Run tests
cd build && ctest --output-on-failure
cd .. && uv run pytest tests/integration/ -v
```

### 3. Update Version to Patch Release

```bash
# Edit CMakeLists.txt: 1.2.0 -> 1.2.1
# Edit docs/datapainter.1: 1.2.0 -> 1.2.1
# Update CHANGELOG.md with hotfix notes

git add CMakeLists.txt docs/datapainter.1 CHANGELOG.md
git commit -m "Bump version to 1.2.1"
```

### 4. Merge and Release

```bash
git checkout main
git merge hotfix/1.2.1
git push origin main

git tag -a v1.2.1 -m "Hotfix release 1.2.1"
git push origin v1.2.1

# Create GitHub release as described above
```

## Release Cadence

### Planned Schedule

- **Major releases**: Annually or when breaking changes accumulate
- **Minor releases**: Quarterly (January, April, July, October)
- **Patch releases**: As needed for bug fixes

### Exceptions

- Critical security vulnerabilities: Release immediately
- Critical data corruption bugs: Release within 24 hours
- Minor bugs: Wait for next scheduled release

## Rollback Procedure

If a release has critical issues:

### 1. Mark Release as Pre-release

1. Go to GitHub release page
2. Edit release
3. Check "Set as a pre-release"
4. Save changes

### 2. Announce Issue

Post to:
- GitHub Discussions
- Release page
- Mailing list

### 3. Create Rollback Tag (if needed)

```bash
git tag -a v1.2.1 v1.2.0 -m "Rollback to 1.2.0 due to critical bug"
git push origin v1.2.1
```

### 4. Fix and Re-release

Follow hotfix release process above.

## Release Artifacts

Each release should include:

### Source Code

- Automatically created by GitHub from git tag

### Binary Packages

- `datapainter-linux-v{VERSION}.tar.gz` - Linux x86_64 binary
- `datapainter-macos-v{VERSION}.tar.gz` - macOS ARM64/x86_64 universal binary
- `datapainter-windows-v{VERSION}.zip` - Windows x86_64 binary

### Package Contents

Each binary package contains:
- `datapainter` (or `datapainter.exe`) - Executable
- `README.md` - User documentation
- `LICENSE` - License file
- `datapainter.1` - Man page

### Checksums

Generate SHA256 checksums for all artifacts:

```bash
sha256sum datapainter-linux-v1.2.0.tar.gz > datapainter-linux-v1.2.0.tar.gz.sha256
sha256sum datapainter-macos-v1.2.0.tar.gz > datapainter-macos-v1.2.0.tar.gz.sha256
sha256sum datapainter-windows-v1.2.0.zip > datapainter-windows-v1.2.0.zip.sha256
```

Upload checksum files to GitHub release.

## GitHub Secrets Configuration

For automated deployment, configure these secrets in GitHub repository settings:

### DEPLOYMENT_SSH_KEY

SSH private key for deploying to `packages.industrial-linguistics.com`.

To generate:

```bash
ssh-keygen -t ed25519 -C "datapainter-deploy" -f datapainter-deploy-key
# Add public key to authorized_keys on server
# Add private key to GitHub Secrets as DEPLOYMENT_SSH_KEY
```

## Troubleshooting

### Release Workflow Fails

**Problem**: GitHub Actions release workflow fails

**Solutions**:
1. Check workflow logs for specific error
2. Re-run failed jobs
3. If dependency installation fails, update workflow to use newer package versions
4. If tests fail, fix tests and create new tag

### Artifact Upload Fails

**Problem**: Binary artifacts not uploading to release

**Solutions**:
1. Check that workflow completed successfully
2. Verify binary exists in workflow artifacts
3. Check GitHub Actions permissions (Settings > Actions > General > Workflow permissions)
4. Manually upload artifacts if needed

### Package Server Deployment Fails

**Problem**: Deployment to packages.industrial-linguistics.com fails

**Solutions**:
1. Check `DEPLOYMENT_SSH_KEY` secret is configured
2. Verify SSH key has correct permissions on server
3. Check server disk space: `ssh deploy@packages.industrial-linguistics.com df -h`
4. Manually deploy: `scp datapainter-linux-v1.2.0.tar.gz deploy@packages.industrial-linguistics.com:/var/www/packages/datapainter/`

### Version Mismatch

**Problem**: Binary reports different version than release tag

**Solutions**:
1. Ensure `CMakeLists.txt` version was updated before creating tag
2. Delete tag: `git tag -d v1.2.0 && git push origin :refs/tags/v1.2.0`
3. Fix version, commit, and recreate tag
4. Delete and recreate GitHub release

## Contact

For questions about the release process:
- Open an issue: https://github.com/solresol/datapainter/issues
- Email maintainer: greg@industrial-linguistics.com
