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

The repository is automatically updated by GitHub Actions during releases:

1. GitHub Actions builds the .deb package
2. Generates repository metadata (Packages, Release files)
3. Signs the Release file with GPG
4. Deploys everything to merah

No manual intervention is required - the repository updates automatically on each release.

### Installing from the Repository

Users can install DataPainter with:

```bash
# Add the GPG key
wget -qO- https://packages.industrial-linguistics.com/datapainter/PUBLIC.KEY | \
  sudo gpg --dearmor -o /usr/share/keyrings/datapainter-archive-keyring.gpg

# Add the repository
echo "deb [signed-by=/usr/share/keyrings/datapainter-archive-keyring.gpg] https://packages.industrial-linguistics.com/datapainter stable main" | \
  sudo tee /etc/apt/sources.list.d/datapainter.list

# Update and install
sudo apt-get update
sudo apt-get install datapainter
```

## GPG Package Signing

The APT repository is signed with GPG for package authenticity and security.

### Signing Key Details

- **Key ID**: `FBB431EC`
- **Fingerprint**: `FD98 F565 3C89 DD28 2173  E29D A22C 39A3 FBB4 31EC`
- **Name**: DataPainter Package Signing
- **Email**: packages@industrial-linguistics.com
- **Public Key**: https://packages.industrial-linguistics.com/datapainter/PUBLIC.KEY

### Key Location

- **Server**: datapainter@merah.cassia.ifost.org.au
- **Public key**: `/var/www/vhosts/packages.industrial-linguistics.com/htdocs/datapainter/PUBLIC.KEY`
- **Private key**: Stored in GPG keyring on merah and as GitHub secret `GPG_SIGNING_KEY`

### How Signing Works

1. GitHub Actions builds .deb package
2. Package is deployed to `/var/www/.../datapainter/pool/main/`
3. The `update-apt-repo.sh` script is run on merah, which:
   - Scans packages and generates Packages file
   - Creates Release file with checksums
   - Signs Release file with GPG to create:
     - `Release.gpg` - Detached signature
     - `InRelease` - Clearsigned Release file

### Rotating the GPG Key

If the signing key needs to be rotated:

```bash
# On merah
ssh datapainter@merah.cassia.ifost.org.au

# Generate new key (or import existing)
gpg --batch --gen-key /path/to/key-spec.batch

# Export public key
gpg --armor --export packages@industrial-linguistics.com > \
  /var/www/vhosts/packages.industrial-linguistics.com/htdocs/datapainter/PUBLIC.KEY

# Export private key for GitHub Actions
gpg --armor --export-secret-keys packages@industrial-linguistics.com > /tmp/private.key

# On local machine, update GitHub secret
gh secret set GPG_SIGNING_KEY < /path/to/private.key
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
- [x] Add GPG signing for packages
- [ ] Generate RPM packages for Fedora/CentOS
- [ ] Create install scripts for each platform
- [ ] Support multiple distributions (Ubuntu 20.04, 22.04, Debian 11, 12)
- [ ] Add subkey for signing with different expiration
- [ ] Publish key to public keyservers (keys.openpgp.org)
