# DataPainter Deployment Documentation

This document describes the automated deployment infrastructure for DataPainter releases.

## Package Distribution Server

Release artifacts are automatically deployed to:
- **Server**: merah.cassia.ifost.org.au
- **User**: datapainter
- **Path**: `/var/www/vhosts/packages.industrial-linguistics.com/htdocs/datapainter/`
- **Public URL**: https://packages.industrial-linguistics.com/datapainter/

Files are served via Cloudflare CDN.

## GitHub Actions Workflows

### CI Workflow (`.github/workflows/ci.yml`)

Runs on every push to `main` and on pull requests:

**Linux Build** (always runs):
1. Install dependencies (cmake, g++, libsqlite3-dev, libncurses-dev)
2. Build with CMake
3. Run C++ unit tests (via ctest)
4. Install Python dependencies (pytest, pyte)
5. Run integration tests
6. Upload build artifacts

**macOS and Windows Builds** (only on releases):
- Conditionally run on release events to save CI time

### Release Workflow (`.github/workflows/release.yml`)

Runs when a GitHub release is published:

1. **Matrix Build**: Builds for Linux, macOS, and Windows
2. **Test**: Runs full test suite on each platform
3. **Package**: Creates platform-specific archives
   - Linux/macOS: `.tar.gz`
   - Windows: `.zip`
4. **Upload to GitHub**: Attaches artifacts to the GitHub release
5. **Deploy to Server** (Linux only): SCPs the Linux tarball to the package server

## SSH Deployment Key

The deployment uses an SSH key stored as a GitHub Actions secret:

- **Secret Name**: `DEPLOYMENT_SSH_KEY`
- **Key Type**: ED25519
- **Key Location** (on merah): `/home/datapainter/.ssh/github_deploy_key`
- **Public Key**: Added to `/home/datapainter/.ssh/authorized_keys`

### Rotating the Deployment Key

If you need to rotate the deployment key:

```bash
# SSH to the deployment server
ssh datapainter@merah.cassia.ifost.org.au

# Generate new key
ssh-keygen -t ed25519 -f ~/.ssh/github_deploy_key -N '' -C 'GitHub Actions deployment key'

# Add to authorized_keys
cat ~/.ssh/github_deploy_key.pub >> ~/.ssh/authorized_keys

# Display the private key (copy this)
cat ~/.ssh/github_deploy_key
```

Then update the GitHub secret:

```bash
# From your local machine
gh secret set DEPLOYMENT_SSH_KEY < /path/to/private_key
```

## Creating a Release

To create a new release and trigger deployment:

```bash
# Tag the release
git tag -a v0.2.0 -m "Release v0.2.0"
git push origin v0.2.0

# Create GitHub release (triggers workflows)
gh release create v0.2.0 \
  --title "DataPainter v0.2.0" \
  --notes "Release notes here"
```

The workflows will:
1. Build for all platforms
2. Run all tests
3. Attach binaries to the GitHub release
4. Deploy the Linux build to packages.industrial-linguistics.com

## Accessing Release Files

After deployment, files are available at:
- Latest release: https://packages.industrial-linguistics.com/datapainter/datapainter-linux-vX.Y.Z.tar.gz
- All files: https://packages.industrial-linguistics.com/datapainter/

## Server Directory Structure

```
/var/www/vhosts/packages.industrial-linguistics.com/
└── htdocs/
    └── datapainter/
        ├── datapainter-linux-v0.1.0.tar.gz
        ├── datapainter-linux-v0.2.0.tar.gz
        └── ... (future releases)
```

## Dependencies for Building

### Ubuntu/Debian
```bash
sudo apt-get install -y cmake g++ libsqlite3-dev libncurses-dev
```

### macOS
```bash
brew install cmake sqlite3
```

### Windows
Uses vcpkg for dependency management (handled by CI).

## APT Repository

An APT repository is available at https://packages.industrial-linguistics.com/datapainter/

### Repository Structure

```
/var/www/vhosts/packages.industrial-linguistics.com/htdocs/datapainter/
├── pool/
│   └── main/                    # .deb packages stored here
│       └── datapainter_*.deb
├── dists/
│   └── stable/
│       ├── Release              # Repository metadata
│       └── main/
│           └── binary-amd64/
│               ├── Packages     # Package index
│               └── Packages.gz
└── *.tar.gz                     # Binary tarballs
```

### Updating the Repository

After uploading new .deb packages, run:

```bash
ssh datapainter@merah.cassia.ifost.org.au
/home/datapainter/update-apt-repo.sh
```

This script:
1. Scans the pool/main/ directory for .deb packages
2. Generates Packages and Packages.gz files
3. Creates/updates the Release file with checksums

The GitHub Actions workflow automatically runs this script after deploying packages.

### Installing from the Repository

Users can install DataPainter with:

```bash
echo "deb [trusted=yes] https://packages.industrial-linguistics.com/datapainter stable main" | \
  sudo tee /etc/apt/sources.list.d/datapainter.list
sudo apt-get update
sudo apt-get install datapainter
```

## Debian Package Building

Debian packages are built automatically by GitHub Actions on release.

The `debian/` directory contains:
- `control` - Package metadata and dependencies
- `rules` - Build instructions
- `changelog` - Version history
- `compat` - Debhelper compatibility level
- `copyright` - License information

To build locally:

```bash
sudo apt-get install debhelper devscripts build-essential
dpkg-buildpackage -us -uc -b
```

## Future Improvements

- [x] Generate Debian/Ubuntu .deb packages
- [x] Create an APT repository at packages.industrial-linguistics.com
- [ ] Add GPG signing for packages
- [ ] Generate RPM packages for Fedora/CentOS
- [ ] Create install scripts for each platform
- [ ] Add checksums/signatures file for verification
- [ ] Support multiple distributions (Ubuntu 20.04, 22.04, Debian 11, 12)
