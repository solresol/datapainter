# Packaging Improvements TODO

## Current State (as of 2025-11-13)

### Linux ✅ COMPLETE
- ✅ Creates `.deb` package
- ✅ Creates APT repository with GPG signing
- ✅ Deploys to merah with timeout protection
- ✅ Properly integrated with system package manager

### macOS ⚠️ BASIC
- ✅ Builds binary successfully
- ✅ Creates `.tar.gz` archive
- ✅ Deploys to merah with timeout protection
- ❌ **TODO**: Create proper `.pkg` installer package
- ❌ **TODO**: Consider `.dmg` with drag-to-Applications

### Windows ⚠️ BASIC
- ✅ Builds binary successfully (after recent vcpkg fix)
- ✅ Creates `.zip` archive
- ✅ Deploys to merah with timeout protection
- ❌ **TODO**: Create proper installer package (MSIX, MSI, or setup.exe)
- ❌ **TODO**: Consider PowerShell install script for PATH integration

### Haiku 🚧 IN PROGRESS
- ✅ Creates `.hpkg` package format
- ✅ ci-haiku.yml workflow configured
- ✅ Deployment to merah configured (in scripts/update-haiku-repo.sh)
- 🚧 **BLOCKED**: Upstream toolchain fetching issue
- ✅ Package CDN workaround complete (all packages in GitHub releases)

## Recommended Next Steps

### Priority 1: macOS Installer
Create a proper `.pkg` installer using `pkgbuild` and `productbuild`:
- Package the binary to `/usr/local/bin/`
- Include man page installation
- Optionally create `.dmg` with drag-to-Applications for user installs

**Implementation approach:**
```bash
# Build component package
pkgbuild --root release/ \
         --identifier com.industrial-linguistics.datapainter \
         --version ${VERSION} \
         --install-location /usr/local/bin \
         datapainter.pkg

# Optional: Create distributable product
productbuild --distribution distribution.xml \
             --package-path . \
             datapainter-installer.pkg
```

### Priority 2: Windows Installer
Choose between MSIX (modern, Store-compatible) or traditional installer:

**Option A: MSIX** (recommended for modern Windows)
- Requires Windows SDK (available in GitHub Actions)
- Can be unsigned for development distribution
- Requires users to enable sideloading or sign for wide distribution
- Best long-term solution for Windows Store

**Option B: MSI via WiX Toolset**
- More traditional, works on older Windows
- Requires WiX Toolset installation
- Can integrate with PATH, create shortcuts, etc.

**Option C: Simple PowerShell Install Script** (easiest short-term)
- Create `install.ps1` that:
  - Copies binary to `C:\Program Files\DataPainter\`
  - Adds to system PATH
  - Creates shortcuts
- No code signing required
- Users run: `powershell -ExecutionPolicy Bypass -File install.ps1`

### Priority 3: Package Manager Integration

**Windows:**
- Submit to Chocolatey: https://community.chocolatey.org/
- Submit to Scoop: https://github.com/ScoopInstaller/Main
- Submit to WinGet: https://github.com/microsoft/winget-pkgs

**macOS:**
- Submit to Homebrew: https://github.com/Homebrew/homebrew-core
- Consider MacPorts

**Linux:**
- ✅ Already have APT repository
- Consider Snap package
- Consider Flatpak
- Consider submission to distribution repos (Debian, Ubuntu, Fedora)

### Priority 4: Code Signing

**Windows:**
- Purchase code signing certificate (e.g., from DigiCert, Sectigo)
- Sign executables with `signtool.exe`
- Required for MSIX distribution outside developer mode

**macOS:**
- Apple Developer Program membership ($99/year)
- Sign with `codesign`
- Notarize with Apple for Gatekeeper compliance

## Resources

### macOS Packaging
- pkgbuild: `man pkgbuild`
- productbuild: `man productbuild`
- Tutorial: https://matthew-brett.github.io/docosx/flat_packages.html

### Windows MSIX
- MSIX Packaging Tool: https://docs.microsoft.com/en-us/windows/msix/
- makeappx command: part of Windows SDK
- Example: https://github.com/microsoft/msix-packaging

### Windows WiX
- WiX Toolset: https://wixtoolset.org/
- GitHub Actions: https://github.com/marketplace/actions/setup-wix

### Code Signing
- Windows: https://docs.microsoft.com/en-us/windows/win32/seccrypto/using-signtool-to-sign-a-file
- macOS: https://developer.apple.com/documentation/xcode/notarizing_macos_software_before_distribution

## Notes

- All deployments now have timeout protection (120s for single files, 180s for APT repo)
- SCP has connection health checks (ServerAliveInterval=10, ServerAliveCountMax=3)
- Deployment failures don't block release (continue-on-error: true)
- Windows and macOS currently deploy simple archives - sufficient for now but not ideal UX
