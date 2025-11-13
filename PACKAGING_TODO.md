# Packaging Improvements TODO

## Current State (as of 2025-11-13)

### Linux ✅ COMPLETE
- ✅ Creates `.deb` package
- ✅ Creates APT repository with GPG signing
- ✅ Deploys to merah with timeout protection
- ✅ Properly integrated with system package manager

### macOS ✅ COMPLETE
- ✅ Builds binary successfully
- ✅ Creates `.pkg` installer package
- ✅ Includes binary in `/usr/local/bin/`
- ✅ Includes man page in `/usr/local/share/man/man1/`
- ✅ Deploys to merah with timeout protection
- ✅ Automated via `scripts/build-macos-pkg.sh`
- ❌ **Future**: Consider `.dmg` with drag-to-Applications for alternative distribution
- ❌ **Future**: Code signing with Apple Developer certificate

### Windows ✅ IMPROVED
- ✅ Builds binary successfully (after recent vcpkg fix)
- ✅ Creates `.zip` archive with installer scripts
- ✅ Includes `install.ps1` - PowerShell installer script
- ✅ Includes `uninstall.ps1` - PowerShell uninstaller script
- ✅ Installer copies to `C:\Program Files\DataPainter\`
- ✅ Installer adds to system PATH automatically
- ✅ Deploys to merah with timeout protection
- ❌ **Future**: Create proper installer package (MSIX or MSI)
- ❌ **Future**: Code signing with Windows certificate

### Haiku 🚧 IN PROGRESS
- ✅ Creates `.hpkg` package format
- ✅ ci-haiku.yml workflow configured
- ✅ Deployment to merah configured (in scripts/update-haiku-repo.sh)
- 🚧 **BLOCKED**: Upstream toolchain fetching issue
- ✅ Package CDN workaround complete (all packages in GitHub releases)

## Recommended Next Steps

### Priority 1: macOS Installer ✅ COMPLETE
~~Create a proper `.pkg` installer using `pkgbuild` and `productbuild`~~

**IMPLEMENTED:**
- ✅ Created `scripts/build-macos-pkg.sh` - Automated .pkg builder
- ✅ Packages binary to `/usr/local/bin/`
- ✅ Includes man page in `/usr/local/share/man/man1/`
- ✅ Integrated into release workflow
- ✅ Deploys to merah automatically

**Future enhancements:**
- Consider `.dmg` with drag-to-Applications for alternative distribution
- Add code signing with Apple Developer certificate

### Priority 2: Windows Installer ✅ IMPROVED
~~Choose between MSIX (modern, Store-compatible) or traditional installer~~

**IMPLEMENTED: PowerShell Install Script (Option C)**
- ✅ Created `scripts/install.ps1` - Simple PowerShell installer
- ✅ Created `scripts/uninstall.ps1` - Uninstaller script
- ✅ Copies binary to `C:\Program Files\DataPainter\`
- ✅ Adds to system PATH automatically
- ✅ Includes admin privilege checks
- ✅ Integrated into release workflow
- ✅ Deploys to merah automatically

**Future enhancements:**
- Create proper MSIX or MSI installer package
- Add code signing with Windows certificate

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
- macOS now deploys proper .pkg installer with automated installation to `/usr/local/bin/`
- Windows now includes PowerShell installer scripts for easy PATH integration
- Installation is significantly improved on all platforms (as of 2025-11-13)
