
# Use official VS Code Dev Container base
ARG VARIANT=1
FROM mcr.microsoft.com/vscode/devcontainers/universal:${VARIANT}

# Install Nix (no-daemon)
RUN sh <(curl -L https://nixos.org/nix/install) --no-daemon

# Make Nix available to shell
RUN echo ". /home/codespace/.nix-profile/etc/profile.d/nix.sh" >> /home/codespace/.bashrc \
 && mkdir -p /home/codespace/.config/nixpkgs \
 && echo '{ allowUnfree = true; }' > /home/codespace/.config/nixpkgs/config.nix

# Optional: Install common tools via Nix
RUN . /home/codespace/.nix-profile/etc/profile.d/nix.sh \
 && nix-env -i git direnv

# Clean up permissions
USER vscode
