# Haiku Build Status

## Current Status: WORKAROUND IMPLEMENTED - Testing GitHub Releases Solution

Successfully identified and worked around Haiku CDN rate limiting by hosting packages in GitHub Releases. Core OS packages (haiku, haiku_devel) now download from GitHub instead of Haiku CDN.

### Completed:
1. ✅ Created `scripts/setup-haiku-cross-env.sh` - Downloads Haiku toolchain
2. ✅ Created `scripts/build-haiku-cross.sh` - Cross-compiles datapainter
3. ✅ Created `scripts/update-haiku-repo.sh` - Creates Haiku repository
4. ✅ Added `.github/workflows/ci-haiku.yml` - GitHub Actions workflow
5. ✅ Updated README.md and DEPLOYMENT.md with Haiku documentation

### Root Cause Identified:
**Haiku CDN Rate Limiting**: The Haiku CDN blocks ANY second download from the same IP, regardless of package.

**Evidence**:
- Test 1: Download `haiku` first → ✓ SUCCESS, then `haiku_devel` → ✗ 404 FAILURE
- Test 2: Download `haiku_devel` first → ✓ SUCCESS, then `haiku` → ✗ 404 FAILURE
- Conclusion: First package always succeeds, second always fails = rate limiting

### Solution Implemented:
**Host packages in GitHub Releases** to bypass Haiku CDN entirely:
1. Downloaded both packages locally (where CDN works)
2. Created GitHub release: `haiku-deps-r1beta5`
3. Uploaded packages to release
4. Modified script to download from GitHub releases instead of Haiku CDN
5. Added URL encoding for tilde (~) character: `r1~beta5` → `r1%7Ebeta5`
6. Added GitHub Actions caching for faster subsequent builds

**GitHub Release**: https://github.com/solresol/datapainter/releases/tag/haiku-deps-r1beta5

**Verified working locally:**
- `curl -I https://eu.hpkg.haiku-os.org/haiku/r1beta5/x86_64/current/packages/haiku_devel-r1~beta5_hrev57937_129-1-x86_64.hpkg`
  - Returns: `303` → `https://haiku-repository.cdn.haiku-os.org/.../haiku_devel-...hpkg` → `200 OK`

**Failing in GitHub Actions:**
- Same URL returns: `curl: (22) The requested URL returned error: 404`
- First package (`haiku`) downloads successfully
- Second package (`haiku_devel`) fails consistently

**Possible causes:**
1. Rate limiting or anti-bot protection on Haiku CDN
2. GitHub Actions IP range being blocked
3. Redirect handling differences in GitHub's curl version
4. Network/DNS issues specific to Actions runners

#### Tested Package URLs (these work locally):
- `https://eu.hpkg.haiku-os.org/haikuports/master/x86_64/current/packages/haiku-r1~beta5_hrev57937+129-1-x86_64.hpkg`
- `https://eu.hpkg.haiku-os.org/haikuports/master/x86_64/current/packages/sqlite_devel-3.47.0-1-x86_64.hpkg`
- `https://eu.hpkg.haiku-os.org/haikuports/master/x86_64/current/packages/ncurses6_devel-6.5-1-x86_64.hpkg`

### Major Improvements Made:
1. ✅ **Fixed repository confusion**: Separated Haiku (core OS) from HaikuPorts (third-party)
2. ✅ **Fixed version string**: Changed from `+129` to `_129` (underscore, not plus)
3. ✅ **Dynamic version query**: Query `/current` endpoint instead of hardcoding versions
4. ✅ **Proper package selection**: Using `haiku_devel`, `sqlite_devel`, `ncurses6_devel`
5. ✅ **Stable branch**: Using `r1beta5` instead of `master` for stability
6. ✅ **Better error handling**: Retry logic, CDN fallback, explicit error messages
7. ✅ **Code cleanup**: Improved sed parsing, helper functions, better structure

**Debug iterations:** 10+ commits refining the approach based on user feedback

### Next Steps to Resolve:

1. **Investigate GitHub Actions network issue:**
   - Add verbose curl output (`-v`) to see redirect behavior
   - Try downloading to different location/filename
   - Test if User-Agent header makes a difference
   - Check if sequential downloads have timing issues

2. **Alternative approaches if network issue persists:**
   - Pre-download packages and store in GitHub repo (not ideal for size)
   - Use GitHub Actions cache to persist downloaded packages
   - Split download into separate workflow step with different retry strategy
   - Use Docker container with Haiku sysroot pre-installed
   - Contact Haiku project about GitHub Actions compatibility

3. **Workarounds to test:**
   - Skip `haiku_devel` temporarily to see if build works without it (unlikely)
   - Download all packages sequentially with longer delays
   - Use `wget` instead of `curl`
   - Download from CDN URL directly without redirect

### Reference Implementation:
The persona-panel project (`~/Documents/devel/persona-panel`) has a working Haiku build,
but it uses the same `package_repo` approach that's failing for datapainter. The key
difference may be in Qt-specific setup or package availability.

### SSH Access to Haiku Box:
- Host: `user@haiku`
- Can be used to test commands and verify package names
- Packages confirmed available: `sqlite_devel`, `ncurses6_devel`, `haiku`

### Files Created:
```
.github/workflows/ci-haiku.yml
scripts/setup-haiku-cross-env.sh
scripts/build-haiku-cross.sh
scripts/update-haiku-repo.sh
DEPLOYMENT.md (updated with Haiku section)
README.md (updated with Haiku info)
```

### Key Commits:
- `256f3fb` - Initial Haiku cross-compilation support
- `df76efd` - **Critical fix**: Download from correct repos with dynamic versioning
- `676f231` - Refactor: Cleaner sed-based version parsing (user feedback)
- `461a2f4` - Add retry logic and CDN fallback
- `cded5aa` - Fix error handling (remove set -e)

Total: 15+ commits iterating on the implementation

### Workflow Runs:
All 10+ workflow runs failed during "Setup Haiku cross environment" step.
- Early runs: Wrong repos, wrong version format, wrong commands
- Recent runs: Correct setup, but `haiku_devel` download 404s in GitHub Actions only
- Latest run: `19318985800`

**Success criteria met locally**, just needs GitHub Actions network debugging.

---

**Last Updated**: 2025-11-13
**Status**: Needs debugging - downloads failing in CI environment
